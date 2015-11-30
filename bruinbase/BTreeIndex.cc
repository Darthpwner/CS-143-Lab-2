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
#include <cstdlib>
#include <iostream>
#include "string.h"

using namespace std;

/*
 * BTreeIndex constructor
 */
BTreeIndex::BTreeIndex()
{
    rootPid = -1;
    treeHeight = 0;	//Root of the tree is defined as level 0

    /* Buffer: needed to communicate with rootPid and treeHeight when we 
       use open and close functions */
    std::fill(buffer, buffer + PageFile::PAGE_SIZE, 0); /* clear the buffer if necessary */
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
    /* Use PageFile's open method with same parameters */
    RC error = pf.open(indexname, mode);
    if (error != 0)
    	return error;
    /* if endPid() is 0, we want to initialize everything again */
    if (pf.endPid() == 0){
    	rootPid = -1;
    	treeHeight = 0;
    	RC error = pf.write(0, buffer);
    	if (error != 0)
    		return error;
    	return 0;
    }
    /* Now, we can read buffer's data from the disk */
    error = pf.read(0, buffer);
    if (error != 0)
    	return error;

    /* create variables to hold pid and height */
    int tmpPid;
    int tmpHeight;
    memcpy(&tmpPid, buffer, sizeof(int));
    memcpy(&tmpHeight, buffer + 4, sizeof(int));

    /* set rootPid to tmpPid and treeHeight to tmpHeight */
    if (tmpPid > 0 && tmpHeight > 0){
    	rootPid = tmpPid;
    	treeHeight = tmpHeight;
    }
    return 0;
}

/*
 * Close the index file.
 * @return error code. 0 if no error
 */
RC BTreeIndex::close()
{
    /* we want to close the PageFile */
    memcpy(buffer, &rootPid, sizeof(int));
    memcpy(buffer + 4, &treeHeight, sizeof(int));

    /* write buffer into PageFile */ 
    RC error = pf.write(0, buffer);
    if (error != 0)
    	return error;
    return pf.close();
}

/*
 * Insert (key, RecordId) pair to the index.
 * @param key[IN] the key for the value inserted into the index
 * @param rid[IN] the RecordId for the record being inserted into the index
 * @return error code. 0 if no error
 */
RC BTreeIndex::insert(int key, const RecordId& rid)
{
    if (key < 0)
    	return RC_INVALID_ATTRIBUTE;
    /* if tree doesn't exist, we start from root */
    if (treeHeight == 0) {
    	BTLeafNode newTree;
    	newTree.insert(key, rid);
    	/* update the root pid
    	   if endPid is 0, set it to 1
    	   set rootPid should start from endPid()*/
    	if (pf.endPid() == 0)
    		rootPid = 1;
    	else
    		rootPid = pf.endPid();
    	/* increment tree height by 1 after insertion */
    	treeHeight++;
    	return newTree.write(rootPid, pf);
    }

    /* if tree does exist, use our helper function to recursively find where
       to insert. Start with pid = 1, and start with current height = 1 */
    int insertKey = -1;
    PageId insertPid = -1;
    RC error;
    error = insert_recur(key, rid, 1, rootPid, insertKey, insertPid);
    if (error != 0)
    	return error;
    /* successfully inserted */
    return 0;
}

/**
* Recursive function to insert key into the correct Nonleaf and leaf nodes
*/
RC BTreeIndex::insert_recur(int key, const RecordId& rid, int curHeight, PageId curPid, int& tmpKey, PageId& tmpPid)
{
	RC error; /* for returning error */

	/* tmp variables for splitting and parent inserting */
	tmpKey = -1;
	tmpPid = -1;

	/* if curHeight is equal to max, add a leaf node */
	if (curHeight == treeHeight){
		BTLeafNode newLeaf;
		newLeaf.read(curPid, pf);
		/* insert leaf node here, return if successful */
		if (newLeaf.insert(key, rid) == 0){
			newLeaf.write(curPid, pf);
			return 0;
		}
		/* insert by splitting because can't insert before */
		BTLeafNode Leaf2;
		int key2;
		error = newLeaf.insertAndSplit(key, rid, Leaf2, key2);
		if (error != 0)
			return error;

		/* we know that key2 is the median key, and we need to place that in parent */
		int lastPid = pf.endPid();
		tmpKey = key2;
		tmpPid = lastPid;
		/* write content into our leaf objects */
		Leaf2.setNextNodePtr(newLeaf.getNextNodePtr());
		newLeaf.setNextNodePtr(lastPid);
		/* check for errors */
		error = Leaf2.write(lastPid, pf);
		if (error != 0)
			return error;
		error = newLeaf.write(curPid, pf);
		if (error != 0)
			return error;

		/* Adding the non leaf node */
		if (treeHeight == 1) {
			/* pid pointers to both new children */
			BTNonLeafNode newRoot;
			newRoot.initializeRoot(curPid, key2, lastPid);
			treeHeight++;

			rootPid = pf.endPid();
			newRoot.write(rootPid, pf);
		}
		return 0;
	}
	/* this else indicates that we are in the middle of the tree (we didn't get the max height) */
	else {
		BTNonLeafNode midNode;
		midNode.read(curPid, pf);
		/* find the childPid using locateChildPtr */
		PageId childPid = -1;
		midNode.locateChildPtr(key, childPid);
		int insertKey = -1;
		PageId insertPid = -1;
		/* Recursion: traverse through tree, inserting at the next node until reaching leaf */
		error = insert_recur(key, rid, curHeight+1, childPid, insertKey, insertPid);

		/* if there's an error, it means that we reached max
		   we will need to add a median key to parent node */
		if (!(insertKey == -1 && insertPid == -1)){
			/* try inserting into midNode */
			if (midNode.insert(insertKey, insertPid) == 0){
				midNode.write(curPid, pf);
				return 0;
			}
			/* otherwise, insert and split, and move median key up the level */
			BTNonLeafNode midNode2;
			int key2;

			midNode.insertAndSplit(insertKey, insertPid, midNode2, key2);

			/* save the median key for the parent */
			int lastPid = pf.endPid();
			tmpKey = key2;
			tmpPid = lastPid;

			error = midNode.write(curPid, pf);
			if (error != 0)
				return error;

			error = midNode2.write(lastPid, pf);
			if (error != 0)
				return error;
			/* special case: if root was split, 
			   we need new single non-leaf nodes as root*/
			if (treeHeight == 1){
				BTNonLeafNode newRoot;
				newRoot.initializeRoot(curPid, key2, lastPid);
				treeHeight++;

				rootPid = pf.endPid();
				newRoot.write(rootPid, pf);
			}
		}
		return 0; /* success */
	}
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
RC BTreeIndex::locate(int searchKey, IndexCursor& cursor){
    return locate_recur(searchKey, cursor, 1, rootPid);
}

//Recursive function to locate where the search key belongs
//Runs until we hit the base case of finding the search key's corresponding leaf node
RC BTreeIndex::locate_recur(int searchKey, IndexCursor& cursor, int curHeight, PageId& nextPid) {
	//Safety check to ensure keys are not negative
	if(searchKey < 0) {
		return RC_INVALID_ATTRIBUTE;
	}

	RC error;
	
	//Base case (leaf node)
	if(curHeight == treeHeight) {
		int eid = -1;

		//Load data for leaf node
		BTLeafNode leaf;
		error = leaf.read(nextPid, pf);	//Read the contents of the node

		if(error != 0) {
			return RC_NO_SUCH_RECORD;
		}

		//Locate leaf node corresponding to the search key and update eid
		error = leaf.locate(searchKey, eid);	//Returns either 0 or RC_NO_SUCH_RECORD

		if(error != 0) {
			return error;
		}

		//Assign to the IndexCursor the values of eid and nextPid
		cursor.eid = eid;
		cursor.pid = nextPid;

		return 0;	//This is important as the stopping condition!
	}
	
	//Non-leaf node
	//load data for the non-leaf node
	BTNonLeafNode nonLeaf;
	error = nonLeaf.read(nextPid, pf);

	if(error != 0) {
		return RC_NO_SUCH_RECORD;
	}

	//Locate the child node of the search key and update nextPid
	error = nonLeaf.locateChildPtr(searchKey, nextPid);

	if(error != 0) {
		return RC_NO_SUCH_RECORD;
	}

	//Recursive step
	return locate_recur(searchKey, cursor, curHeight + 1, nextPid);
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
void BTreeIndex::print() {
	if(treeHeight==1)
	{	
		BTLeafNode root;
		root.read(rootPid, pf);
		root.print();
	}
	else if(treeHeight>1)
	{
		BTNonLeafNode root;
		root.read(rootPid, pf);
		root.print();
		
		PageId first, rest;
		memcpy(&first, root.getBuffer(), sizeof(PageId));
		BTLeafNode firstLeaf, leaf;
		firstLeaf.read(first, pf);
		firstLeaf.print();
		
		//print the rest of the leaf nodes
		for(int i=0; i<root.getKeyCount(); i++)
		{
			memcpy(&rest, root.getBuffer()+12+(8*i), sizeof(PageId));
			leaf.read(rest, pf);
			leaf.print();
		}
		
		//print each leaf node's current pid and next pid
		cout << "----------" << endl;
		
		for(int i=0; i<root.getKeyCount(); i++)
		{
			if(i==0)
				cout << "leaf0 (pid=" << first << ") has next pid: " << firstLeaf.getNextNodePtr() << endl;
		
			BTLeafNode tempLeaf;
			PageId tempPid;
			memcpy(&tempPid, root.getBuffer()+12+(8*i), sizeof(PageId));
		
			tempLeaf.read(tempPid, pf);;
			
			cout << "leaf" << i+1 << " (pid=" << tempPid << ") has next pid: " << tempLeaf.getNextNodePtr() << endl;
		}
	}	
}
