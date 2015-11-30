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
PageId pidPtr1 = 29;
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
  int error = nonLeaf.locateChildPtr(313, pidPtr1);
  printf("locateChildPtr: %d\n", error);  //Always gives 0 for some reason?

  int a = leaf.insert(10, rid);
  int c = leaf.insert(50, rid1);
  printf("%d\n", a); //Prints 0 Success!
  printf("%d\n", c); //Prints 0 Success!
  printf("%d\n", leaf.getKeyCount()); //Prints 6
  //leaf.insert(20, rid);
  //leaf.insert(32, rid);

  leaf.print(); //WHY DOES THIS GIVE SOME WEIRD ASCII?

  nonLeaf.print();
  printf("\n");
  int result =   nonLeaf.getKeyCount();
  printf("%d\n", result); //8

  //b.insert(10, 'good');
 // RecordFile::append(10, 'good');
  // b.print();

  int eid = -69;
  int myKey = -100;
  printf("TEST PART B MOFO\n\n");
  printf("BTLeafNode:\n");
  printf("read: %d\n", leaf.read(pidPtr, pf));  //Why does this give an error
  printf("write: %d\n", leaf.read(pidPtr, pf));  //Why does this give an error
  printf("getKey: %d\n", leaf.getKeyCount());  //Why does this return 6?
  printf("insert: %d\n", leaf.insert(5, rid));  //Why does this give an error
  printf("insertAndSplit: %d\n", leaf.insertAndSplit(7, rid, leaf, myKey));  //Why does this give an error
  printf("locate: %d\n", leaf.locate(5, eid));  //Should return 0 :)
  printf("eid has value: %d\n", eid);
  printf("locate: %d\n", leaf.locate(-100, eid));  //Why does this give an error
  printf("eid has value: %d\n", eid);
  printf("locate: %d\n", leaf.locate(7, eid));  //Why does this give an error
  printf("eid has value: %d\n", eid);
  

//  printf("readEntry: %d\n", leaf.readEntry(eid));

  //read(PageId)
	//END TESTING PURPOSES

  // run the SQL engine taking user commands from standard input (console).
  SqlEngine::run(stdin);


  return 0;
}
