## BCE - Bowron - Abernethy Chess Engine

Once upon a time I wrote a small chess engine in Lisp.  Then I wanted
that engine to go faster, so I rewrote it in C.  Then a while later I
wanted to do a better chess engine, with more features.  I am a big
fan of Lisp so I wrote it in that.  Then, again I wanted it to be
faster so I rewrote it in C.  Then for an AI course project, I added
learning to the C version.  The result has evolved into what you will
find in this archive.

This program is alternatively known as BACE for Bowron - Abernethy
Chess Engine (for Rob Abernethy who was my partner is my AI project)
or BCE for Boo's Chess Engine.  Either one works for me.   

I have two accounts at freechess.org that I sometimes run my engines
under:
	BACE     - for the engine with learning
	ddlchess - for the engine without learning

If you make any modifications to BACE that you would like to
contribute to me, please send me email at cwbowron@gmail.com

If you are running BACE on a chess server please let me know the
account name and server, I would be interested to see how it is doing.

## Known Errors / To Do

1. sometimes fails to find a valid move under robofics/xboard (this
   could have been related to #2)

2. Null move search is not quite right... I believe that when doing a
   search and getting interrupted causes board conflict problems, ie
   pieces not getting replaced... should probably turn off polling
   during a null search or something... or keep track of
   the number of fake_domoves... 

   most likely the undomove function should check to see if the move
   it is undoing is an actual move, or a fake move... test to see if
   move==dummymove or some such test... 
   
3. learning seems to be FUBAR... 
   
## Acknowledgements

Thanks to Dann Corbit for his many helpful hints, suggestions and
code. 

