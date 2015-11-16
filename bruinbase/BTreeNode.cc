#include "BTreeNode.h"
#include <cstdlib>
#include <iostream>
#include "string.h"
#include <iostream>
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
	int keyCount = 0;
	char* tempBuffer = buffer;

	//Loop through all indices in the tempBuffer, increment by 12 bytes to go to the next key
	//Need 1024 bytes of main memory to "load" the content of the node from the disk
	//In a leaf node, we read at most 1020 bytes with 4 left over (1024/12 == 85 R 4)
	for(int i = 0; i < PageFile::PAGE_SIZE; i += PAIR_SIZE) {
		int key;
		memcpy(&key, tempBuffer, sizeof(int));	//Save the key inside the tempBuffer
		if(key == 0) {	//Key of value 0 indicates we do not have a key here
			break;
		}
		keyCount++;	//Increment count whenever we can move down the buffer

		tempBuffer += PAIR_SIZE;	//Jump to the next key (8 bytes away) in the temporary buffer 
	}

	return keyCount;
}

/*
 * Insert a (key, rid) pair to the node.
 * @param key[IN] the key to insert
 * @param rid[IN] the RecordId to insert
 * @return 0 if successful. Return an error code if the node is full.
 */
RC BTLeafNode::insert(int key, const RecordId& rid) {
	//Store the last 4 bytes (the pid) to rebuild the inserted leaf node later
	PageId nextNodePtr = getNextNodePtr();

	//Page has 1024 bytes if we need to store 12 bytes (key, rid)
	//We can fit 1024/12 == 85 with 4 bytes left over
	if(getKeyCount() + 1 > NUM_OF_TOTAL_PAIRS) {	//Return an error code if the null is full.
		return RC_NODE_FULL;
	}

	//Temporary buffer holds the buffer's original stored keys and manipulates how to add a new key.
	char* tempBuffer = buffer;

	//Otherwise, go through the buffer's keys to see where to store the new node
	//1008 is the largest index at which we can store a new node since we do not have enough memory at 1020
	int i = 0;
	for(; i < LARGEST_INDEX; i += PAIR_SIZE) {
		int insertKey;
		memcpy(&insertKey, tempBuffer, sizeof(int));	//Save the insertKey inside tempBuffer

		//If the key at index i for the buffer is NULL or the key is smaller than an inside key, stop execution
		if(insertKey == 0 || key < insertKey) {
			break;
		}

		tempBuffer += PAIR_SIZE;	//Jump to the next key in the temporary buffer
	}

	//After we perform our check to see for free space, index i has the appropriate index of where to insert the pair
	char* newBuffer = (char*) malloc (PageFile::PAGE_SIZE);
	fill(newBuffer, newBuffer + PageFile::PAGE_SIZE, 0);	//Clear the buffer as appropriate

	//Copy all values from buffer into new Buffer up until i
	memcpy(newBuffer, buffer, i);

	//Values to insert as new (key, rid) pair
	PageId pid = rid.sid;
	int sid = rid.sid;

	memcpy(newBuffer + i, &key, sizeof(int));
	memcpy(newBuffer + i + sizeof(int), &rid, sizeof(RecordId));

	//The remaining memcpy operations will copy the remaining values into newBuffer
	//Neglecting the results of nextNodePtr. This needs to be manually copied in!
	memcpy(newBuffer + i + PAIR_SIZE, buffer + i, getKeyCount() * PAIR_SIZE - i);
	memcpy(newBuffer + PageFile::PAGE_SIZE - sizeof(PageId), &nextNodePtr, sizeof(PageId));

	//Copy the newBuffer into buffer, then delete temporary newBuffer to prevent any memory leaks
	memcpy(buffer, newBuffer, PageFile::PAGE_SIZE);
	free(newBuffer);

	//Successively inserted leaf node, so we increment number of keys
	numKeys++;

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
	//Store the last 4 bytes (the pid) to rebuild the inserted leaf node later
	PageId nextNodePtr = getNextNodePtr();

	int numberOfTotalPairs = (PageFile::PAGE_SIZE - sizeof(PageId)) / PAIR_SIZE;

	//Only split the node if inserting causes an overflow. Return an error otherwise
	if(getKeyCount() < numberOfTotalPairs) {	//If getKeyCount() < numberOfTotalPairs, you still have space so you don't need to split
		return RC_INVALID_FILE_FORMAT;
	}

	//If sibling node is not empty, return error
	if(sibling.getKeyCount() != 0) {
		return RC_INVALID_ATTRIBUTE;
	}

	//Clear sibling buffer to be safe
	fill(sibling.buffer, sibling.buffer + PageFile::PAGE_SIZE, 0);	//Clear the buffer if necessary

	//Calculate the keys still in the first half
	int numberOfHalfKeys = ((int) (getKeyCount() + 1) / 2);

	//Get the halfway index where we split the node block
	int indexAtHalf = numberOfHalfKeys * PAIR_SIZE;

	//Copy everything from the right side of the halfIndex into our sibling's buffer except the pid
	memcpy(sibling.buffer, buffer + indexAtHalf, PageFile::PAGE_SIZE - sizeof(PageId) - indexAtHalf);

	//Update sibling's number of keys and set sibling's pid to current node's pid ptr
	sibling.numKeys = getKeyCount() - numberOfHalfKeys;
	sibling.setNextNodePtr(getNextNodePtr());

	//Clear the 2nd half of the current buffer except for the pid and then update the keys
	fill(buffer + indexAtHalf, buffer + PageFile::PAGE_SIZE - sizeof(PageId), 0);
	numKeys = numberOfHalfKeys;

	//Check which buffer to put the new (key, rid) pair in
	int firstHalfKeyVal;
	memcpy(&firstHalfKeyVal, sibling.buffer, sizeof(int));

	//Insert the pair and increment the number of keys
	if(key >= firstHalfKeyVal) {	//In this case, the key belongs in the 2nd buffer of our sorted B+ tree
		sibling.insert(key, rid);
	} else {	//Otherwise, place it in the 1st buffer
		insert(key, rid);
	}

	//Copy over sibling's 1st key and rid
	memcpy(&siblingKey, sibling.buffer, sizeof(int));

	RecordId siblingRid;
	siblingRid.pid = -1;
	siblingRid.sid = -1;
	memcpy(&siblingRid, sibling.buffer + sizeof(int), sizeof(RecordId));

	//Leave next node pointer alone because changing it will destroy the index tree's leaf node mapping
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
	char* tempBuffer = buffer;

	//Loop through all the indices in the tempBuffer and incremeny by 12 bytes to jump to the next key
	int i = 0;
	for(; i < getKeyCount() * PAIR_SIZE; i += PAIR_SIZE) {
		int key;
		memcpy(&key, tempBuffer, sizeof(int));	//Save the current key inside buffer

		//If the key is larger than or equal to the searchKey, set eid
		if(key >= searchKey) {
			//eid = current byte index divided by size of a pair entry
			eid = i/PAIR_SIZE;
			return 0;
		}

		tempBuffer += PAIR_SIZE;
	}

	//If we reach this point, every key inside the buffer was less than the searchKey parameter
	eid = getKeyCount();
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
	//If eid is out of bounds, return an error code
	if(eid >= getKeyCount() || eid < 0) {
		return RC_NO_SUCH_RECORD;
	}

	//Position in bytes of the entry
	int byteIndex = eid * PAIR_SIZE;

	char* tempBuffer = buffer;

	//Copy the data into parameters
	memcpy(&key, tempBuffer + byteIndex, sizeof(int));
	memcpy(&rid, tempBuffer + byteIndex + sizeof(int), sizeof(RecordId));

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
	memcpy(&nextPid, tempBuffer + PageFile::PAGE_SIZE-sizeof(PageId), sizeof(PageId));	/*Go to the last PageId section of the buffer 
																						and copy the information to the nextPid variable*/

	return nextPid;
}

/*
 * Set the pid of the next sibling node.
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
	char* tempBuffer = buffer;

	for(int i = 0; i < getKeyCount() * PAIR_SIZE; i += PAIR_SIZE) {
		cout << tempBuffer[i] << " ";	//Print out each possible emptyPair

		tempBuffer += PAIR_SIZE;	//tempBuffer jumps to the next key
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
  /* 1: find size (bytes) of an entry pair (done in a const in header) */
  int count = 0; // counter to return at the end

  /* 2.  traverse through buffer's sorted key to see where new keys are*/
  /* NONLEAF ONLY: skip the first 8 bytes (4 bytes pid, 4 bytes empty)*/
  char* tmp_bufIndex = buffer+8;
  int i;
  /* 4: Loop through all indexes in temp buffer; increment by 8 bytes to jump to next key*/
  /* 1016 is the largest possible index of the next inserted pair b/c 1024 - 8 = 1016*/
  /* NONLEAF ONLY: start the (key, pid) entry at index 8*/
  /* return the count after the loop*/
  for (i = 8; i < BUFFER_SIZE; i+= PAIR_SIZE) {
    int tmp_Key;
    memcpy(&tmp_Key, tmp_bufIndex, sizeof(int)); /* save the current key in insideKey */
    if (tmp_Key == 0) /* break out once hitting a key of 0 */
      break;
    tmp_bufIndex += PAIR_SIZE;
    count++;
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
       Nonleaf nodes have pairs of integer keys and PageIds, with another PageId at the front
       NUM_TOTAL_PAIRS already calculated */

    /* Page as 1024 bytes; we are trying to store 12 bytes (key, rid)
       This means we can fit 85 with 4 bytes left over for pid pointer to next leaf */
    /* Check if adding one more (key, rid) pair will exceed our size limit */
    if (getKeyCount()+1 > NUM_TOTAL_PAIRS) {
      return RC_NODE_FULL; // node is full, so return
    }

    /* Go through buffer's sorted keys to see where the new key goes
       Nonleaf Node: skip first 8 bytes (4 byte pid, 4 byte empty)*/
    char* tmp_bufIndex = buffer+8;
    int i;
    /* loop through all indexes in the buffer and increment by pairSize
      1016 is the largest possible index for the next inserted pair
      Nonleaf node: start the entry at 8 */
    for (i = 8; i < BUFFER_SIZE; i += PAIR_SIZE) {
      int tmp_Key;
      memcpy(&tmp_Key, tmp_bufIndex, sizeof(int)); // save the current key inside buffer
      // stop when insideKey is NULL OR insideKey is greater than or equal to key, stop
      if (tmp_Key == 0 || key <= tmp_Key)
        break;
      tmp_bufIndex += PAIR_SIZE; // increment tmp to next key
    } /* i now holds the index to insert the pair
        tmp is now the buffer at that index*/
    char* tmp_buffer = (char*)malloc(PageFile::PAGE_SIZE);
    std::fill(tmp_buffer, tmp_buffer + PageFile::PAGE_SIZE, 0); // clears the buffer if necessary

    /* copy values from buffer to tmp_buffer until i */
    memcpy(tmp_buffer, buffer, i);

    /* copy key and pid into tmp_buffer */
    memcpy(tmp_buffer + i, &key, sizeof(int));
    memcpy(tmp_buffer + i + sizeof(int), &pid, sizeof(PageId));

    /* copy the rest of the values back
       Nonleaf Node: add in 8 bytes in the beginning
       because we can't count the first as a key */
    memcpy(tmp_buffer + i + PAIR_SIZE, buffer + i, getKeyCount()*PAIR_SIZE - i + 8);
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
RC BTNonLeafNode::insertAndSplit(int key, PageId pid, BTNonLeafNode& sibling, int& midKey) {
  /*Nonleaf node: pairs of integer keys and PageIds, with anotehr PageId at the front
    NUM_TOTAL_PAIRS already defined in header */

  /* We only want to split if insertion causes overflow, so return error if it does not */
  if (!(getKeyCount() >= NUM_TOTAL_PAIRS))
    return RC_INVALID_FILE_FORMAT;
  /* if sibling node is not empty, we want to return error */
  if (sibling.getKeyCount() != 0)
    return RC_INVALID_ATTRIBUTE;
  /* clear the sibling buffer (sanity check)*/
  std::fill(sibling.buffer, sibling.buffer + PageFile::PAGE_SIZE, 0);
  /* calculate number of keys that we have left in first half*/
  int numHalfKeys = ((int)((getKeyCount() + 1) / 2));
  /* Find index to split the node's buffer
     Nonleaf Node: add offset of 8 bytes */
  int splitIndex = numHalfKeys*PAIR_SIZE + 8;

  /* Now, we want to remove the Median Key, because we don't need it */
  /* find the last key of the first half and the first key of the second half
     key1 - last key of the first half
     key2 - first key of the second half */
  int key1 = -1;
  int key2 = -1;
  memcpy(&key1, buffer + splitIndex - 8, sizeof(int));
  memcpy(&key2, buffer + splitIndex, sizeof(int));

  if (key < key1) { /* key1 is the median key to be removed */
    /* copy everything on right side of halfIndex to sibling's buffer
       For Nonleaf Node: remember to add an offset of 8*/
    memcpy(sibling.buffer + 8, buffer + splitIndex, PageFile::PAGE_SIZE - splitIndex);
    /* update sibling's number of keys */
    sibling.numKeys = getKeyCount() - numHalfKeys;

    /* Copy median key into midKey before freeing it in the buffer */
    memcpy(&midKey, buffer + splitIndex - 8, sizeof(int));

    /* Copy sibling.buffer before freeing it in the buffer */
    memcpy(sibling.buffer, buffer+splitIndex - 4, sizeof(PageId));

    /* Clear current buffer's second half, and update number of keys*/
    std::fill(buffer + splitIndex - 8, buffer + PageFile::PAGE_SIZE, 0);
    numKeys = numHalfKeys - 1;

    /* call insert to insert the key and pid pair into the buffer
       because key is smaller than key1 (median key)*/
    insert(key, pid);
  }
  else if (key > key2){ /* key2 is the median key to be removed */
    /* copy everything on the right side of the halfIndex to sibling's buffer
       but do not add the first key, since key2 is the median key to be removed
      Nonleaf node: add an offset of 8*/
      memcpy(sibling.buffer + 8, buffer + splitIndex + 8, PageFile::PAGE_SIZE - splitIndex - 8);
      /* update sibling's number of keys. */
      sibling.numKeys = getKeyCount() - numHalfKeys - 1;

      /* Copy median key into midKey before freeing it in the buffer */
      memcpy(&midKey, buffer + splitIndex, sizeof(int));

      /* Clear current buffer's second half, and update number of keys */
      memcpy(sibling.buffer, buffer + splitIndex + 4, sizeof(PageId));

      /* Clear current buffer's second half, update number of keys */
      std::fill(buffer + splitIndex, buffer + PageFile::PAGE_SIZE, 0);
      numKeys = numHalfKeys - 1;

      /* insert the (key, pid) pair into sibling
         because key is larger than median key */
      sibling.insert(key, pid);
  }
  else { /* key is the median key to be removed */
    /* copy everything on the right side of the halfIndex to sibling's buffer
      Nonleaf node: add an offset of 8*/
      memcpy(sibling.buffer + 8, buffer + splitIndex, PageFile::PAGE_SIZE - splitIndex);
      /* update sibling's number of keys */
      sibling.numKeys = getKeyCount() - numHalfKeys;

      /* We insert key into midKey, so no need continue insertion process */
      midKey = key;

      /* Clear the second half of current buffer & update number of keys */
      std::fill(buffer + splitIndex, buffer + PageFile::PAGE_SIZE, 0);
      numKeys = numHalfKeys;

      /* Set sibling pid from median key parameter */
      memcpy(sibling.buffer, &pid, sizeof(PageId));
  }
  /* we have split the node and inserted successfully */
  return 0;
}

/*
 * Given the searchKey, find the child-node pointer to follow and
 * output it in pid.
 * @param searchKey[IN] the searchKey that is being looked up.
 * @param pid[OUT] the pointer to the child node to follow.
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::locateChildPtr(int searchKey, PageId& pid) {
  /* Nonleaf Node: skip the first 8 offsets */
  char* tmp_bufIndex = buffer + 8;

  /* Loop through all indexes in tmp buffer;
     Each key will be 8 bytes */
  int i;
  for (i = 8; i < getKeyCount()*PAIR_SIZE + 8; i += PAIR_SIZE) {
    int tmp_Key;
    /* save current key into insideKey */
    memcpy(&tmp_Key, tmp_bufIndex, sizeof(int));
    if (i == 8 && tmp_Key > searchKey) {
      /* If searchKey is less than first key, return initial pid
         We need this because first pid is at different buffer position*/
      memcpy(&pid, buffer, sizeof(PageId));
      return 0;
    }
    else if (tmp_Key > searchKey) {
      memcpy(&pid, tmp_bufIndex - 4, sizeof(PageId));
      return 0;
    }
    /* If we reach here, we know that searchKey is >= insideKey, continue loop */
    tmp_bufIndex += PAIR_SIZE;
  }

  /* If we get here, searchKey is >= insideKey
     Copy over last right-most pid before returning function
     Decrement tmp by 4 bytes since when coming out of for loop,
     it is on next non-existent node's position */
  memcpy(&pid, tmp_bufIndex - 4, sizeof(PageId));
  return 0;
}

/*
 * Initialize the root node with (pid1, key, pid2).
 * @param pid1[IN] the first PageId to insert
 * @param key[IN] the key that should be inserted between the two PageIds
 * @param pid2[IN] the PageId to insert behind the key
 * @return 0 if successful. Return an error code if there is an error.
 */
RC BTNonLeafNode::initializeRoot(PageId pid1, int key, PageId pid2) {
  std::fill(buffer, buffer + PageFile::PAGE_SIZE, 0);

  /* Initializing Root, so no need to skip 8 bytes */
  char *tmp_bufIndex = buffer;

  /* Copy the initial pid into buffer */
  memcpy(tmp_bufIndex, &pid1, sizeof(PageId));
  /* Copy the first pair into buffer using insert function */
  RC result;
  result = insert(key, pid2);
  /* return result if it has significant value */
  if (result != 0)
    return result;
  return 0;
}

/*
 * Print the keys of the node to cout
 */
void BTNonLeafNode::print() {
  /* once again, Nonleaf node: skip first 8 bytes as offset */
  char* tmp_bufIndex = buffer + 8;
  int i;
  /* traverse through and print out insideKey */
  for (i = 8; i < getKeyCount()*PAIR_SIZE + 8; i += PAIR_SIZE) {
    int tmp_Key;
    memcpy(&tmp_Key, tmp_bufIndex, sizeof(int));
    cout << tmp_Key << " ";
    tmp_bufIndex += PAIR_SIZE;
  }
  cout << "" << endl;
}
