#include "BTreeNode.h"
#include <cstdlib>
#include <iostream>
#include "string.h"
using namespace std;

BTLeafNode::BTLeafNode() {
	numKeys = 0;
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::read(PageId pid, const PageFile& pf) {
	return pf.read(pid, buffer);	//Reads the PageFile using its implementation, returns 0 on success or error code on an error.
}

/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::write(PageId pid, PageFile& pf) {
	return pf.write(pid, buffer);	//Writes the PageFile using its implementation, returns 0 on success or error code on an error.
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTLeafNode::getKeyCount() {
	//
	int pairSize = sizeof(RecordId) + sizeof(int);
	int keyCount = 0;
	char* tempBuffer = buffer;

	//Loop through all indices in the tempBuffer, increment by 12 bytes to go to the next key
	//Need 1024 bytes of main memory to "load" the content of the node from the disk
	for(int i = 0; i < BUFFER_SIZE; i += pairSize) {
		if(*tempBuffer == 0) {	//Element of tempBuffer's index has a 0 value meaning we do not have a key here
			break;
		}
		keyCount++;	//Increment count whenever we can move down the bugger

		tempBuffer += pairSize;	//Jump to the next key in the temporary buffer
	}

	return 0;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid) {
	return 0;
}

/*
 * Insert the (key, rid) pair to the node
 * and split the node half and half with sibling.
 * The first key of the sibling node is returned in siblingKey.
 * @param key[IN] the key to insert.
 * @param rid[IN] the RecordId to insert.
 * @param sibling[IN] the sibling node to split with. This node MUST be EMPTY when this function is called.
 * @param siblingKey[OUT] the first key in the sibling node after split.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::insertAndSplit(int key, const RecordId& rid,
                              BTLeafNode& sibling, int& siblingKey) {
	return 0;
}

/**
 * If searchKey exists in the node, set eid to the index entry
 * with searchKey and return 0. If not, set eid to the index entry
 * immediately after the largest index key that is smaller than searchKey,
 * and return the error code RC_NO_SUCH_RECORD.
 * Remember that keys inside a B+tree node are always kept sorted.
 * @param searchKey[IN] the key to search for.
 * @param eid[OUT] the index entry number with searchKey or immediately
                   behind the largest key smaller than searchKey.
 * @return 0 if searchKey is found. Otherwise return an error code.
 */
RC BTLeafNode::locate(int searchKey, int& eid) {
	return 0;
}

/*
 * Read the (key, rid) pair from the eid entry.
 * @param eid[IN] the entry number to read the (key, rid) pair from
 * @param key[OUT] the key from the entry
 * @param rid[OUT] the RecordId from the entry
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::readEntry(int eid, int& key, RecordId& rid) {
	return 0;
}

/*
 * Return the pid of the next slibling node.
 * @return the PageId of the next sibling node
 */
PageId BTLeafNode::getNextNodePtr() {
	PageId nextPid = 0;	//Default pid is 0 if we do not find the next sibling node
	
	char* tempBuffer = buffer;

	/*memcpy
	* void* destination
	* const void* source
	* size_t num
	*/
	memcpy(&nextPid, tempBuffer + PageFile::PAGE_SIZE-sizeof(PageId), sizeof(PageId));	//Go to the last PageId section of the buffer and copy the information to the nextPid variable

	return nextPid;
}

/*
 * Set the pid of the next slibling node.
 * @param pid[IN] the PageId of the next sibling node
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTLeafNode::setNextNodePtr(PageId pid) {
	//Return an error code if an error.
	if(pid < 0) {
		return RC_INVALID_PID;
	}

	char* tempBuffer = buffer;

	//Copy the pid parameter into our tempBuffer
	memcpy(tempBuffer + PageFile::PAGE_SIZE-sizeof(PageId), &pid, sizeof(PageId));

	//Return 0 if successful.
	return 0;
}

/*
 *   Print the keys of the node
*/
void BTLeafNode::print() {
	//This is the size in bytes of an empty pair
	int pairSize = sizeof(RecordId) + sizeof(int);

	char* tempBuffer = buffer;

	for(int i = 0; i < getKeyCount() * pairSize; i += pairSize) {
		cout << tempBuffer[i] << " ";	//Print out each possible emptyPair
	} 

	cout << endl;
}
//--------------------------------------------------------------------------------------------------------------------------------------------------


/*******************************************************************************
NONLEAF NODE MEMBER FUNCTIONS **************************************************
*******************************************************************************/
/* Nonleaf Node Constructor */
BTNonLeafNode::BTNonLeafNode() {
  numKeys = 0;
  std::fill(buffer, buffer + PageFile::PAGE_SIZE, 0); /* clear the buffer */
}

/*
 * Read the content of the node from the page pid in the PageFile pf.
 * @param pid[IN] the PageId to read
 * @param pf[IN] PageFile to read from
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::read(PageId pid, const PageFile& pf) {
  return pf.read(pid, buffer); /*Use PageFile to read from selected page into buffer*/
}

/*
 * Write the content of the node to the page pid in the PageFile pf.
 * @param pid[IN] the PageId to write to
 * @param pf[IN] PageFile to write to
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::write(PageId pid, PageFile& pf) {
  return pf.write(pid, buffer); /*Use PageFile to write from buffer into selected page*/
}

/*
 * Return the number of keys stored in the node.
 * @return the number of keys in the node
 */
int BTNonLeafNode::getKeyCount() {
  /* 1: find size (bytes) of an entry pair*/
  int pairSize = sizeof(PageId) + sizeof(int);
  int numTotalPairs = (PageFile::PAGE_SIZE - sizeof(PageId)) / pairSize;
  int count = 0; // counter to return at the end

  /* 2.  traverse through buffer's sorted key to see where new keys are*/
  /* NONLEAF ONLY: skip the first 8 bytes (4 bytes pid, 4 bytes empty)*/
  char* tmp = buffer+8;
  int i;
  /* 4: Loop through all indexes in temp buffer; increment by 8 bytes to jump to next key*/

  /* 1016 is the largest possible index of the next inserted pair*/
  /* NONLEAF ONLY: start the (key, pid) entry at index 8*/
  /* return the count after the loop*/
  for (i = 8; i < 1016; i+= pairSize) {
    int insideKey;
    memcpy(&insideKey, tmp, sizeof(int)); // save the current key inside buffer
    if (insideKey == 0) // break out once hitting a key of 0
      break;
    count++;
    tmp += pairSize;
  }
  return count;
}


/*
 * Insert a (key, pid) pair to the node.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTNonLeafNode::insert(int key, PageId pid) {
    /* find size of an entry pair in bytes
       Nonleaf nodes have pairs of integer keys and PageIds, with another PageId at the front*/
    int pairSize = sizeof(PageId) + sizeof(int);
    int numTotalPairs = (PageFile::PAGE_SIZE - sizeof(PageId)) / pairSize;
    /* Page as 1024 bytes; we are trying to store 12 bytes (key, rid)
       This means we can fit 85 with 4 bytes left over for pid pointer to next leaf */
    /* Check if adding one more (key, rid) pair will exceed our size limit */
    if (getKeyCount()+1 > numTotalPairs) {
      return RC_NODE_FULL; // node is full, so return
    }

    /* Go through buffer's sorted keys to see where the new key goes
       Nonleaf Node: skip first 8 bytes (4 byte pid, 4 byte empty)*/
    char* tmp = buffer+8;
    int i;
    /* loop through all indexes in the buffer and increment by pairSize
      1016 is the largest possible index for the next inserted pair
      Nonleaf node: start the entry at 8 */
    for (i = 8; i < 1016; i += pairSize) {
      int insideKey;
      memcpy(&insideKey, tmp, sizeof(int)); // save the current key inside buffer
      // stop when insideKey is NULL OR insideKey is greater than or equal to key, stop
      if (insideKey == 0 || key <= insideKey)
        break;
      tmp += pairSize; // increment tmp to next key
    } /* i now holds the index to insert the pair
        tmp is now the buffer at that index*/
    char* tmp_buffer = (char*)malloc(PageFile::PAGE_SIZE);
    std::fill(tmp_buffer, tmp_buffer + PageFile::PAGE_SIZE, 0); // clears the buffer if necessary

    // copy values from buffer to tmp_buffer until i
    memcpy(tmp_buffer, buffer, i);

    // copy key and pid into tmp_buffer
    memcpy(tmp_buffer + i, &key, sizeof(int));
    memcpy(tmp_buffer + i + sizeof(int), &pid, sizeof(PageId));

    /* copy the rest of the values back
       Nonleaf Node: add in 8 bytes in the beginning
       because we can't count the first as a key */
    memcpy(tmp_buffer + i + pairSize, buffer + i, getKeyCount()*pairSize - i + 8);
    /* Copy tmp_buffer into buffer, delete tmp_buffer */
    memcpy(buffer, tmp_buffer, PageFile::PAGE_SIZE);
    free(tmp_buffer);

    /* increment count of keys, and return, since now insertion is complete */
    numKeys++;
    return 0;
}

/*
 * Insert the (key, pid) pair to the node
 * and split the node half and half with sibling.
 * The middle key after the split is returned in midKey.
 * @param key[IN] the key to insert
 * @param pid[IN] the PageId to insert
 * @param sibling[IN] the sibling node to split with. This node MUST be empty when this function is called.
 * @param midKey[OUT] the key in the middle after the split. This key should be inserted to the parent node.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey)
{ return 0; }

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid)
{ return 0; }

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2)
{ return 0; }
