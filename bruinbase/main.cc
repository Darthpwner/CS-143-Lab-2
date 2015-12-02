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
#include <iostream>

//TESTING PURPOSES
 #include "BTreeIndex.h"
 #include "BTreeNode.h"
//END TESTING PURPOSES

int main()
{
	//TESTING PURPOSES We ARE ALMOST THERE
  //printf("HELLO WORLD\n");

//   BTreeIndex b;
//   BTLeafNode leaf;
//   printf("Keys for leaves: %d\n", leaf.getKeyCount()); //Appears there are originally 4 leaves?
//   BTNonLeafNode nonLeaf;
//   BTNonLeafNode nonLeaf1;

//   PageFile pf;	
//   RecordFile rf;

//  //How do we initialize a recordId?
//   PageId pid;	//integer
//  	RecordId rid;	//has a PageId pid and int sid
// 	rid.sid = 1;
//   	rid.pid = 1;

//   	RecordId rid1;
//   	rid1.sid = 2;
//   	rid1.sid = 2;

//   	if(rid > rid1) {
//   		printf("rid is bigger\n");
//   	} else {
//   		printf("rid1 is bigger\n");
//   	}

// //  leaf.read();


// int x = 0;
// PageId pidPtr = 8;
// PageId pidPtr1 = 29;
// //Prints out pointers :)

// printf("%d\n", leaf.getKeyCount()); //Prints 4

//   nonLeaf.insertAndSplit(99, 5, nonLeaf1, x);

//   nonLeaf.insert(15, 5);
//   nonLeaf.insert(1, 6);
//   nonLeaf.insert(312, 8);
//   nonLeaf.insert(-5, 8);
//   nonLeaf.insert(312, 8);
//   nonLeaf.insert(98, 8);
//   nonLeaf.insert(312, 8);
//   nonLeaf.insert(312, 8);
//   int error = nonLeaf.locateChildPtr(313, pidPtr1);
//   printf("locateChildPtr: %d\n", error);  //Always gives 0 for some reason?

//   int a = leaf.insert(10, rid);
//   int c = leaf.insert(50, rid1);
//   printf("%d\n", a); //Prints 0 Success!
//   printf("%d\n", c); //Prints 0 Success!
//   printf("%d\n", leaf.getKeyCount()); //Prints 6
//   leaf.insert(20, rid);
//   leaf.insert(32, rid);

//   printf("HERE ARE THE LEAVES: ");
//   leaf.print(); //NOT PRINTING ANYTHING?
//   printf("Key count: %d\n", leaf.getKeyCount());

//   nonLeaf.print();
//   printf("\n");
//   int result =   nonLeaf.getKeyCount();
//   printf("%d\n", result); //8

//   //b.insert(10, 'good');
//  // RecordFile::append(10, 'good');
//   // b.print();

//   int eid = -69;
//   int myKey = -100;
//   printf("TEST PART B MOFO\n\n");
//   printf("BTLeafNode:\n");
//   printf("read: %d\n", leaf.read(pidPtr, pf));  //Why does this give an error
//   printf("write: %d\n", leaf.read(pidPtr, pf));  //Why does this give an error
//   printf("getKey: %d\n", leaf.getKeyCount());  //4
//   printf("insert: %d\n", leaf.insert(5, rid));  //Why does this give an error
//   printf("getKey: %d\n", leaf.getKeyCount());  //5
//   printf("insert: %d\n", leaf.insert(0, rid));  //Why does this give an error
//   printf("getKey: %d\n", leaf.getKeyCount());  //0

//   printf("insertAndSplit: %d\n", leaf.insertAndSplit(7, rid, leaf, myKey));  //Why does this give an error
//   printf("locate: %d\n", leaf.locate(5, eid));  //Should return 0 :)
//   printf("eid has value: %d\n", eid);
//   printf("locate: %d\n", leaf.locate(-100, eid));  //Why does this give an error
//   printf("eid has value: %d\n", eid);
//   printf("locate: %d\n", leaf.locate(7, eid));  //Why does this give an error
//   printf("eid has value: %d\n", eid);
  
  // printf("insertAndSplit: %d\n", nonLeaf2.insertAndSplit(4, pid, nonLeaf, midKey));
  // printf("locateChildPtr: %d\n", nonLeaf2.locateChildPtr(3, pid));  //3
  // printf("initializeRoot: %d\n", nonLeaf2.initializeRoot(pidPtr, 20, pidPtr1));  


//  printf("readEntry: %d\n", leaf.readEntry(eid));

  //read(PageId)
	//END TESTING PURPOSES

  /* Frank's Testing */
  //TESTING BELOW IS FOR BTreeNode
  // std::cout << sizeof(RecordId) + sizeof(int) << std::endl; //size 12
  // std::cout << sizeof(PageId) << std::endl; //size 4
   
  // //Initialize new leaf node
  // BTLeafNode poop;
  // std::cout << "Initial key count: " << poop.getKeyCount() << std::endl;
  
  // int zeroPls;
  
  
  // //Try stuffing our node with (key, rid) pairs until node is full
  // RecordId poopRid = {1,2};
  // for(int i=0; i<86; i++)
  // {
  //   zeroPls = poop.insert(5, poopRid);
  //   std::cout << "Am I zero? " << zeroPls << std::endl;
  //   poop.print();
  // }
  
  // std::cout << "Final key count: " << poop.getKeyCount() << std::endl;

  // //Test location by finding key with 5
  // int eid = -1;
  // poop.locate(5, eid);
  // std::cout << "Found key=5 at eid: " << eid << std::endl;
  
  // //Test location by finding key with 6 (doesn't exist - should be -1)
  // int eid2 = -1;
  // poop.locate(6, eid2);
  // std::cout << "Found key=6 at eid2: " << eid2 << std::endl;
  // run the SQL engine taking user commands from standard input (console).
  SqlEngine::run(stdin);


  return 0;
}
