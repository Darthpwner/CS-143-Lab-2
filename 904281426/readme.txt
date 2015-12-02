For Project 2A, Frank and Matthew noticed that the page reads were not identical for the original movie.tbl file and the test version that we created. In some cases, we noticed that the two versions ran at around more or less the same speed with occasional deviation that could be attributed to chance.

For Project 2B, Frank and Matthew attempted to optimize the locate() function by checking up to the condition where key >= searchKey. This is more of a trivial improvement since inevitably we would have to run through the entire 1024 byte block in the worst case, but we could not find any condition in which we could optimize it further as of yet.

For Project 2C, Frank and Matthew tried to minimize the number of page reads by using recursive algorithms for insertion and look-up. Our main premise for both recursive algorithms was to check if our current level was the same as the leaf nodes, and this was our base case. Otherwise, we would recursively call our helper functions until we reached the same height as the leaf nodes. This meant we made “n” recursive calls where n is the height of the tree.

For Project 2D, Frank and Matthew compiled a table of page reads without indexing, with indexing, and the expected results from output.txt.

			---------------------------------------------------------
			| Without Indexing 	| With Indexing | Expected Results 	|
---------------------------------------------------------------------
|	xsmall	|		  1 		|		3		|		2-3			|
|			|		  1			| 		3		| 		2-3			|
|	small	|		  6			|		3		|		2			|
| 	 		| 		  6 		| 		7		| 		6-7			|					  
|	medium	|		  13		|		5		|		4			|
| 			| 		  13 		| 		5		| 		4			|					 
|	large	|		  113		|		20		|		21			|
| 			| 		  113		| 		17		| 		15-16		| 					  
|			| 		  113		|		17		| 		15-16		| 					  
|	xlarge	|		  1366		|		222		|		219			|
| 			|		  1366		| 		6	 	|	 	5			|					  
|			|		  1366		|		71		|		69-73		|					  
---------------------------------------------------------------------

As we can see from the table, indexing leads to a very noticeable increase in performance and reduces the nubeer of page reads significantly! We noticed that without indexing, the results of each SQL query was the same based on the size, but with indexing, the results had deviations within the size tuple based on what the query asked for. The difference was most magnified for the xlarge size because we had such a large B+ tree, giving us a tangible example of the performance improvements that indexing offers. Our results deviated slightly from the expected results in output.txt, probably becase our root node is NOT always 0 when we split. We know this because some of our page views are greater than the expected results.

Emails
Matthew Lin: matthewallenlin@gmail.com
Kang (Frank) Chen: frankchen1698@gmail.com
