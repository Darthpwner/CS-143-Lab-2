/*
 * Copyright (C) 2008 by The Regents of the University of California
 * Redistribution of this file is permitted under the terms of the GNU
 * Public License (GPL).
 *
 * @author Junghoo "John" Cho <cho AT cs.ucla.edu>
 * @date 3/24/2008
 */
 
#include "BTreeIndex.h"
#include "BTreeNode.h"

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
    treeHeight = 0;	//Root of the tree is defined as level 0
}

/*
 * Open the index file in read or write mode.
 * Under 'w' mode, the index file should be created if it does not exist.
 * @param indexname[IN] the name of the index file
 * @param mode[IN] 'r' for read, 'w' for write
 * @return error code. 0 if no error
 */
RC BTreeIndex::open(const string& indexname, char mode)
{
    return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
    return 0;
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
    return 0;
}

/**
 * Run the standard B+Tree key search algorithm and identify the
 * leaf node where searchKey may exist. If an index entry with
 * searchKey exists in the leaf node, set IndexCursor to its location
 * (i.e., IndexCursor.pid = PageId of the leaf node, and
 * IndexCursor.eid = the searchKey index entry number.) and return 0.
 * If not, set IndexCursor.pid = PageId of the leaf node and
 * IndexCursor.eid = the index entry immediately after the largest
 * index key that is smaller than searchKey, and return the error
 * code RC_NO_SUCH_RECORD.
 * Using the returned "IndexCursor", you will have to call readForward()
 * to retrieve the actual (key, rid) pair from the index.
 * @param key[IN] the key to find
 * @param cursor[OUT] the cursor pointing to the index entry with
 *                    searchKey or immediately behind the largest key
 *                    smaller than searchKey.
 * @return 0 if searchKey is found. Othewise an error code
 */

 //Check this function please
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor)
{
	RC error;
	BTNonLeafNode middleNode;
	BTLeafNode leaf;

	int eid;
	int currentHeight = 1;
	PageId nextPid = rootPid;

	while(currentHeight < treeHeight) {
		error = middleNode.read(nextPid, pf);

		if(error != 0) {	//Read function should return 0 unless there is an error
			return RC_NO_SUCH_RECORD;
		}

		//Locate child node to look at next given the search key; update nextPId
		error = middleNode.locateChildPtr(searchKey, nextPid);

		if(error != 0) {
			return RC_NO_SUCH_RECORD;
		}

		currentHeight++;
	}

	error = leaf.read(nextPid, pf);

	if(error != 0) {
		return RC_NO_SUCH_RECORD;
	}

	//Set up the IndexCursor with the found eid and nextPid (which is now the current pid)
	cursor.eid = eid;
	cursor.pid = nextPid;

	// //Error
	// IndexCursor.pid = PageId;
	// IndexCursor.eid = searchKey;

	//index entry immediately after largest index key that is smaller than searchKey

	//No error

    return 0;
}

/*
 * Read the (key, rid) pair at the location specified by the index cursor,
 * and move foward the cursor to the next entry.
 * @param cursor[IN/OUT] the cursor pointing to an leaf-node index entry in the b+tree
 * @param key[OUT] the key stored at the index cursor location.
 * @param rid[OUT] the RecordId stored at the index cursor location.
 * @return error code. 0 if no error
 */
RC BTreeIndex::readForward(IndexCursor& cursor, int& key, RecordId& rid)
{
	RC error;

	//Gets position in the page from the cursor parameter
	PageId cursorPid = cursor.pid;
	int cursorEid = cursor.eid;

	//Load data for the cursor's leaf
	BTLeafNode leaf;
	error = leaf.read(cursorPid, pf);

	if(error != 0) {
		return error;
	}

	//Based on the cursor's eid, obtain the key and the rid
	error = leaf.readEntry(cursorEid, key, rid);

	if(error != 0) {
		return error;
	}

	//Make sure the cursor's PageId does not go beyond an uninitialized page
	if(cursorPid <= 0) {
		return RC_INVALID_CURSOR;
	}

	//Now we need to increment the cursorEid
	//Be careful that the cursorEid does not exceed the max index of the leaf's buffer

	//Make sure that incrementing cursorEid would not exceed the maximumeid index as determined by key count
	if(cursorEid + 1 >= leaf.getKeyCount()) {
		//If we exceed eid bounds, reset cursor's eid to 0
		//Increment the pid
		cursorEid = 0;
		cursorPid = leaf.getNextNodePtr();
	} else {
		cursorEid++;
	}

	//Write the new position back into the cursor parameter
	cursor.eid = cursorEid;
	cursor.pid = cursorPid;
    return 0;
}

//This function only prints up to two levels of nodes
// void BTreeIndex::print() {
// 	if(treeHeight == 1) {
// 		BTLeafNode root;
// 		root.read(rootPid, pf);
// 		root.print();
// 	} else if(treeHeight > 1) {
// 		BTNonLeafNode root;
// 		root.read(rootPid, pf);
// 		root.print(&first, root.buffer, sizeof(PageId));

// 		BTLeafNode firstLeaf, leaf;
// 		firstLeaf.read(first, pf);
// 		firstLeaf.print();

// 		//Print the rest of the leaf ndoes
// 		for(int i = 0; i < root.getKeyCount(); i++) {
// 			memcpy(&rest, root.buffer + 12 + (8 * i), sizeof(PageId));
// 			leaf.read(rest, pf);
// 			leaf.print();
// 		}

// 		//Print each leaf node's current pid and next pid
// 		cout << "-------" << endl;

// 		for(int i = 0; i < root.getKeyCount(); i++) {
// 			if(i == 0) {
// 				cout << "leaf0 (pid=" << first << ") has next pid: " << firstLeaf.getNextNodePtr() << endl;
// 			}

// 			BTLeafNode tempLeaf;
// 			PageId tempPid;
// 			memcpy(&tempPid, root.buffer + 12 + (8 * i), sizeof(PageId));

// 			tempLeaf.read(tempPid, pf);

// 			cout << "Leaf" << i + 1 << " (pid=" << tempPid << ") has next pid: " << tempLeaf.getNextNodePtr() << endl;
// 		}
// 	}
// }
