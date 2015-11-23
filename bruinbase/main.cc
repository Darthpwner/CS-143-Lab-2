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
  printf("%d\n", leaf.getKeyCount()); //Appears there are originally 4 leaves?
  BTNonLeafNode nonLeaf;
  BTNonLeafNode nonLeaf1;

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
  		printf("rid is bigger\n");
  	} else {
  		printf("rid1 is bigger\n");
  	}

//  leaf.read();


int x = 0;
PageId pidPtr = 8;
//Prints out pointers :)

printf("%d\n", leaf.getKeyCount()); //Prints 4

  nonLeaf.insertAndSplit(99, 5, nonLeaf1, x);

  nonLeaf.insert(15, 5);
  nonLeaf.insert(1, 6);
  nonLeaf.insert(312, 8);
  nonLeaf.insert(-5, 8);
  nonLeaf.insert(312, 8);
  nonLeaf.insert(98, 8);
  nonLeaf.insert(312, 8);
  nonLeaf.insert(312, 8);
  nonLeaf.locateChildPtr(312, pidPtr);

  leaf.insert(10, rid);
  leaf.insert(50, rid1);
  printf("%d\n", leaf.getKeyCount()); //Prints 6
  //leaf.insert(20, rid);
  //leaf.insert(32, rid);

  leaf.print();

  nonLeaf.print();
  printf("\n");
  int result =   nonLeaf.getKeyCount();
  printf("%d\n", result); //8

  //b.insert(10, 'good');
 // RecordFile::append(10, 'good');
  // b.print();
	//END TESTING PURPOSES

  // run the SQL engine taking user commands from standard input (console).
  SqlEngine::run(stdin);


  return 0;
}
