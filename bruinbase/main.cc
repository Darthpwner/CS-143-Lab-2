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
  printf("Keys for leaves: %d\n", leaf.getKeyCount()); //Appears there are originally 4 leaves?
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
  leaf.insert(20, rid);
  leaf.insert(32, rid);

  printf("HERE ARE THE LEAVES: ");
  leaf.print(); //NOT PRINTING ANYTHING?
  printf("Key count: %d\n", leaf.getKeyCount());

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
  printf("getKey: %d\n", leaf.getKeyCount());  //4
  printf("insert: %d\n", leaf.insert(5, rid));  //0
  printf("getKey: %d\n", leaf.getKeyCount());  //5
  printf("insert: %d\n", leaf.insert(0, rid));  //0
  printf("getKey: %d\n", leaf.getKeyCount());  //6, why does this give 0
  printf("insert: %d\n", leaf.insert(5, rid));  //0
  printf("getKey: %d\n", leaf.getKeyCount());  //7, why does this give 1

  printf("insertAndSplit: %d\n", leaf.insertAndSplit(7, rid, leaf, myKey));  //Why does this give an error
  printf("locate: %d\n", leaf.locate(5, eid));  //Should return 0 :)
  printf("eid has value: %d\n", eid);
  printf("locate: %d\n", leaf.locate(5, eid));  //Should return 0 :)
  printf("eid has value: %d\n", eid);

  printf("locate: %d\n", leaf.locate(-100, eid));  //Why does this give an error
  printf("eid has value: %d\n", eid);
  printf("locate: %d\n", leaf.locate(7, eid));  //Why does this give an error
  printf("eid has value: %d\n", eid);
 
  printf("\n"); 
  printf("BTNonLeafNode:\n");
  BTNonLeafNode nonLeaf2;
  int midKey;
  printf("read: %d\n", nonLeaf2.read(pidPtr, pf));  //Why does this give an error
  printf("write: %d\n", nonLeaf2.write(pidPtr, pf));  //Why does this give an error
  printf("getKey: %d\n", nonLeaf2.getKeyCount());  //0
  printf("insert: %d\n", nonLeaf2.insert(3, pid));  //3
  printf("getKey: %d\n", nonLeaf2.getKeyCount());  //1
  printf("insert: %d\n", nonLeaf2.insert(39, pid));  //0
  printf("insert: %d\n", nonLeaf2.insert(326, pid));  //0
  printf("insert: %d\n", nonLeaf2.insert(101, pid));  //0
  printf("getKey: %d\n", nonLeaf2.getKeyCount());  //4
  nonLeaf2.print();
  
  printf("insertAndSplit: %d\n", nonLeaf2.insertAndSplit(4, pid, nonLeaf, midKey));
  printf("locateChildPtr: %d\n", nonLeaf2.locateChildPtr(3, pid));  //3
  printf("initializeRoot: %d\n", nonLeaf2.initializeRoot(pidPtr, 20, pidPtr1));  


//  printf("readEntry: %d\n", leaf.readEntry(eid));

  //read(PageId)
	//END TESTING PURPOSES

  // run the SQL engine taking user commands from standard input (console).
  SqlEngine::run(stdin);


  return 0;
}
