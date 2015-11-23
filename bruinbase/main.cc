/**
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "Bruinbase.h"
#include "SqlEngine.h"
#include <cstdio>

//TESTING PURPOSES
 #include "BTreeIndex.h"
 #include "BTreeNode.h"
//END TESTING PURPOSES

int main()
{
	//TESTING PURPOSES
  //printf("HELLO WORLD\n");
  BTreeIndex b;
  BTLeafNode leaf;
  BTNonLeafNode nonLeaf;

  PageFile pf;	
  RecordFile rf;

 //How do we initialize a recordId?
  PageId pid;	//integer
 	RecordId rid;	//has a PageId pid and int sid
	rid.sid = 1;
  	rid.pid = 1;

  	RecordId rid1;
  	rid1.sid = 2;
  	rid1.sid = 2;

  	if(rid > rid1) {
  		printf("rid is bigger");
  	} else {
  		printf("rid1 is bigger");
  	}

//  leaf.read();

  // nonLeaf.insert(15, rid);
  // nonLeaf.insert(1, rid);
  // nonLeaf.insert(312, rid);

  leaf.insert(10, rid);
  leaf.insert(50, rid1);
  //leaf.insert(20, rid);
  //leaf.insert(32, rid);

  leaf.print();

  nonLeaf.print();

  //b.insert(10, 'good');
 // RecordFile::append(10, 'good');
  // b.print();
	//END TESTING PURPOSES

  // run the SQL engine taking user commands from standard input (console).
  SqlEngine::run(stdin);


  return 0;
}
