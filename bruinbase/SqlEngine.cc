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
  IndexCursor c;   // cursor for navigating through B+ TREE
  BTreeIndex tree; // BTree for indexing

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

  SelCond tmp; /* temp var for storing the select condition */
  bool cond_exist = false;    /* check if any valid select conditions exist */
  bool valcond_exist = false; /* chek if any value select conditions exist */
  bool index_exist = false;   /* we assume B+ tree is not used unless specified */

  bool cond_GE = false; /* if true, key >= min */
  bool cond_LE = false; /* if true, key <= max */
  int max = -1;
  int min = -1;
  int equalVal = -1; 

  int cond_index = -1;

  /* checker for conflicting conditions */
  bool valueConflict = false;
  std::string valueEqual = "";

  /* loop through SelConds to find condition */
  for (int i = 0; i < cond.size(); i++) {
    tmp = cond[i];
    int tmpVal = atoi(tmp.value);

    if (tmp.attr == 1 && tmp.comp != SelCond::NE){
      cond_exist = true; /* set this to true when reaching a valid condition, like now */

      if (tmp.comp == SelCond::EQ){ /* if keys are equal, we can break, since there's only one select condition */
        equalVal = tmpVal;
        cond_index = i;
        break;
      }
      else if (tmp.comp == SelCond::GE){ /* key >= min */
        if (tmpVal > min || min == -1){
          cond_GE = true;
          min = tmpVal;
        }
      }
      else if (tmp.comp == SelCond::GT){ /* key > min */
        if (tmpVal >= min || min == -1){
          cond_GE = false;
          min = tmpVal;
        }
      }
      else if (tmp.comp == SelCond::LE){
        if (tmpVal < max || max == -1){
          cond_LE = true;
          max = tmpVal;
        }
      }
      else if (tmp.comp == SelCond::LT){
        if (tmpVal <= max || max == -1){
          cond_LE = false;
          max = tmpVal;
        }
      }
    } /* if (tmp.attr == 1 && tmp.comp != SelCond::NE) */
    else if (tmp.attr == 2){ /* if we hit a value condition, we want to update hasValueCond */
      valcond_exist = true;
      if (tmp.comp == SelCond::EQ){
        if (strcmp(value.c_str(), cond[i].value)== 0 || valueEqual == "")
          valueEqual = tmpVal;
        else
          valueConflict = true;
      }
    }
  }

  /* we can end the quary early by checking if the conditions are valid */
  if (valueConflict || (max != -1 && min != -1 && max < min))
    goto end_select_early;
  if (max != -1 && min != -1 && !cond_GE && !cond_LE && max == min)
    goto end_select_early;


  /* if the index file DNE, we can use regular select */
  if(tree.open(table + ".idx", 'r')!=0 || (!cond_exist && attr!=4))
  {
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
  } /* for normal select */
  else {  /* else, we know that the table's index file exists, and we need B+Tree */
    count = 0;
    rid.pid = 0;
    rid.sid = 0;
    index_exist = true;

    /* set starting position for IdnexCursor c */
    if (equalVal != -1)                /* this means the keys are equal */
      tree.locate(equalVal, c);
    else if (min != -1 && !cond_GE) /* this means key is greater than min */
      tree.locate(min+1, c);
    else if (min != -1 && cond_GE)  /* this means the key is greater than or equal to min */
      tree.locate(min, c);
    else 
      tree.locate(0, c);

    while(tree.readForward(c, key, rid) == 0){
      if (!valcond_exist && attr == 4){
        if (equalVal != -1 && key != equalVal)
          goto end_select_early;

        if (max != -1){
          if (cond_LE && key > max)
            goto end_select_early;
          else if (!cond_LE && key >= max)
            goto end_select_early;
        }

        if (min != -1){
          if (cond_GE && key < min)
            goto end_select_early;
          else if (!cond_GE && key <= min)
            goto end_select_early;
        }

        /* at this point, key passes all conditions, increment count and continue */
        count++;
        continue;
      }

      /* read tuple (from skeleton code) */
      if ((rc = rf.read(rid, key, value)) < 0) {
        fprintf(stderr, "Error: while reading a tuple from table %s\n", table.c_str());
        goto exit_select;
      }

      /* check conditions from the tuple */
      for (unsigned i = 0; i < cond.size(); i++){
        /* compute difference between tuple value and condition value */
        if (cond[i].attr == 1)
          diff = key - atoi(cond[i].value);
        if (cond[i].attr == 2)
          diff = strcmp(value.c_str(), cond[i].value);

        /* skip tuple if any condition is not met */
        switch(cond[i].comp){
          case SelCond::NE:
            if (diff == 0) 
              goto continue_while;
            break; /* break when keys invalidly match */
          case SelCond::EQ:
            if (diff != 0){
              if (cond[i].attr == 1)
                goto end_select_early;
              goto continue_while;
            }
          case SelCond::GT:
            if (diff <= 0)
              goto continue_while; 
            break; /* if !(key > cond value), break */
          case SelCond::GE:
            if (diff < 0)
              goto continue_while; /* if !(key >= cond value), break */
            break;
          case SelCond::LT:
            if (diff >= 0){
              if (cond[i].attr == 1)  
                goto end_select_early;
              goto continue_while;
            }
            break;
          case SelCond::LE:
            if (diff > 0){
              if (cond[i].attr == 1)
                goto end_select_early;
              goto continue_while;
            }
            break;
        }
      }

      /* increment count because we matched conditions */
      count++;

      /* print the tuple */
      switch(attr){
        case 1: /* 1 --> SELECT key */
          fprintf(stdout, "%d\n", key);
          break;
        case 2: /* 2 --> SELECT value */
          fprintf(stdout, "%s\n", value.c_str());
          break;
        case 3: /* 3 --> SELECT * */
          fprintf(stdout, "%d '%s'\n", key, value.c_str());
          break;
      }
      continue_while:
      cout << "";
    }
  }    

  end_select_early:

  // print matching tuple count if "select count(*)"
  if (attr == 4) {
    fprintf(stdout, "%d\n", count);
  }
  rc = 0;

  // close the table file and return
  exit_select:
  if (index_exist)
    tree.close(); /* close the tree, since we used it */
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

  //Key value pair stored at each record
  string line;  //A line from a load file (string)
  int    key; //The key field of the tuple in the line (integer)
  string value; //The value field of the tuple in the line (string)

  //For Part A, assume index is always FALSE

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
