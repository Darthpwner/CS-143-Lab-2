/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include "Bruinbase.h"
#include "SqlEngine.h"
#include "BTreeIndex.h"

using namespace std;

// external functions and variables for load file and sql command parsing 
extern FILE* sqlin;
int sqlparse(void);


RC SqlEngine::run(FILE* commandline)
{
  fprintf(stdout, "Bruinbase> ");

  // set the command line input and start parsing user input
  sqlin = commandline;
  sqlparse();  // sqlparse() is defined in SqlParser.tab.c generated from
               // SqlParser.y by bison (bison is GNU equivalent of yacc)

  return 0;
}

RC SqlEngine::select(int attr, const string& table, const vector<SelCond>& cond)
{
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning

  RC     rc;
  int    key;     
  string value;
  int    count;
  int    diff;

  // open the table file
  if ((rc = rf.open(table + ".tbl", 'r')) < 0) {
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;
  }

  // scan the table file from the beginning
  rid.pid = rid.sid = 0;
  count = 0;
  while (rid < rf.endRid()) {
    // read the tuple
    if ((rc = rf.read(rid, key, value)) < 0) {
      fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
      goto exit_select;
    }

    // check the conditions on the tuple
    for (unsigned i = 0; i < cond.size(); i++) {
      // compute the difference between the tuple value and the condition value
      switch (cond[i].attr) {
      case 1:
	diff = key - atoi(cond[i].value);
	break;
      case 2:
	diff = strcmp(value.c_str(), cond[i].value);
	break;
      }

      // skip the tuple if any condition is not met
      switch (cond[i].comp) {
      case SelCond::EQ:
	if (diff != 0) goto next_tuple;
	break;
      case SelCond::NE:
	if (diff == 0) goto next_tuple;
	break;
      case SelCond::GT:
	if (diff <= 0) goto next_tuple;
	break;
      case SelCond::LT:
	if (diff >= 0) goto next_tuple;
	break;
      case SelCond::GE:
	if (diff < 0) goto next_tuple;
	break;
      case SelCond::LE:
	if (diff > 0) goto next_tuple;
	break;
      }
    }

    // the condition is met for the tuple. 
    // increase matching tuple counter
    count++;

    // print the tuple 
    switch (attr) {
    case 1:  // SELECT key
      fprintf(stdout, "%d\n", key);
      break;
    case 2:  // SELECT value
      fprintf(stdout, "%s\n", value.c_str());
      break;
    case 3:  // SELECT *
      fprintf(stdout, "%d '%s'\n", key, value.c_str());
      break;
    }

    // move to the next tuple
    next_tuple:
    ++rid;
  }

  // print matching tuple count if "select count(*)"
  if (attr == 4) {
    fprintf(stdout, "%d\n", count);
  }
  rc = 0;

  // close the table file and return
  exit_select:
  rf.close();
  return rc;
}

//How to verify we are right, make sure that "select * from movie" in Bruinbase produces the same result as "load movie from 'movie.del'"
//We could also run a diff command between the two files

//Project 2A
/*Opens the input loadfile and the RecordFile
  Parses each line of the loadfile to read a tuple (use SqlEngine::parseLoadLine())
  Insert the tuple to the RecordFile
*/
//table[IN] - the table name in the LOAD command
//loadfile[IN] - the file name of the load file
//index[IN] - true if "WITH INDEX" option was specified
RC SqlEngine::load(const string& table, const string& loadfile, bool index)
{
  //Local variable declarations (look at select function as an example)
  RC rc;  //RC means error code
  RecordFile rf;   // RecordFile containing the table
  RecordId   rid;  // record cursor for table scanning
  BTreeIndex treeIndex; //Used to implement indexing to speed up queries

  //Key value pair stored at each record
  string line;  //A line from a load file (string)
  int    key; //The key field of the tuple in the line (integer)
  string value; //The value field of the tuple in the line (string)

  //Use any standard C++ file I/O functions to read the loadfile i.e. fstream
  //ifstream is limits the I/O to input only
  ifstream tableData(loadfile.c_str()); //c_str() converts the string into a C-string that is has a zero byte at the end

  if(!tableData.is_open()) {  //If tableData cannot be opened, return an error message
    fprintf(stderr, "Error: table %s does not exist\n", table.c_str());
    return rc;  //Return the default value of rc
  }

  //Create an empty file with the "table" variable as the name with the .tbl suffix  
  rc = rf.open(table + ".tbl", 'w');  //If file does not exist in 'w' mode, it is created and stored into rc

  //Loop while we still have lines in tableData (indicated by \n) and stop when we reach the zero byte (\0)
  while(getline(tableData, line)) {
    parseLoadLine(line, key, value);  //Parses each line of the loadfile to read a tuple
    rc = rf.append(key, value, rid);  //Append the records to the load file to the end of the table
  }
  
  //Close the Record File and the table to prevent unexpected bugs after using it.
  rf.close();
  tableData.close();

  return rc;
}

//Parse a line from the load file into the (key, value) pair
//line[IN] - a line from a load file
//key[OUT] - the key field fo the tuple in the line
//value[OUT] - the value file do fthe tuple in the line
RC SqlEngine::parseLoadLine(const string& line, int& key, string& value)
{
    const char *s;
    char        c;
    string::size_type loc;
    
    // ignore beginning white spaces
    c = *(s = line.c_str());
    while (c == ' ' || c == '\t') { c = *++s; }

    // get the integer key value
    key = atoi(s);

    // look for comma
    s = strchr(s, ',');
    if (s == NULL) { return RC_INVALID_FILE_FORMAT; }

    // ignore white spaces
    do { c = *++s; } while (c == ' ' || c == '\t');
    
    // if there is nothing left, set the value to empty string
    if (c == 0) { 
        value.erase();
        return 0;
    }

    // is the value field delimited by ' or "?
    if (c == '\'' || c == '"') {
        s++;
    } else {
        c = '\n';
    }

    // get the value string
    value.assign(s);
    loc = value.find(c, 0);
    if (loc != string::npos) { value.erase(loc); }

    return 0;
}
