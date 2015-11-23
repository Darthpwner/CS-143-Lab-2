For Project 2A, Frank and Matthew noticed that the page reads were not identical for the original movie.tbl file and the test version that we created. In some cases, we noticed that the two versions ran at around more or less the same speed with occasional deviation that could be attributed to chance.

For Project 2B, Frank and Matthew attempted to optimize the locate() function by checking up to the condition where key >= searchKey. This is more of a trivial improvement since inevitably we would have to run through the entire 1024 byte block in the worst case, but we could not find any condition in which we could optimize it further as of yet.

For Project 2C, Frank and Matthew tried to minimize the number of page reads by using recursive algorithms for insertion and look-up. Our main premise for both recursive algorithms was to check if our current level was the same as the leaf nodes, and this was our base case. Otherwise, we would recursively call our helper functions until we reached the same height as the leaf nodes. This meant we made “n” recursive calls where n is the height of the tree.

Emails
Matthew Lin: matthewallenlin@gmail.com
Kang (Frank) Chen: frankchen1698@gmail.com



