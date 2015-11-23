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
  printf("HELLO WORLD\n");
  BTreeIndex b;
  BTLeafNode leaf;
  BTNonLeafNode nonLeaf;

  PageFile pf;	//integer
  RecordFile rf;	//has a PageId pid and int sid
 
 //How do we initialize a recordId?
 RecordId rid;

//  leaf.read();

  leaf.insert(10, rid);

  //b.insert(10, 'good');
 // RecordFile::append(10, 'good');
  b.print();
	//END TESTING PURPOSES

  // run the SQL engine taking user commands from standard input (console).
  SqlEngine::run(stdin);


  return 0;
}
