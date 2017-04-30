## BCE - Boo's Chess Engine (aka Bowron - Abernethy Chess Engine)

Once upon a time I wrote a small chess engine in Lisp.  Then I wanted
that engine to go faster, so I rewrote it in C.  Then a while later I
wanted to do a better chess engine, with more features.  I am a big
fan of Lisp so I wrote it in that.  Then, again I wanted it to be
faster so I rewrote it in C.  Then for an AI course project, I added
learning to the C version.  The result has evolved into what you will
find in this archive. It is alternatively known as BACE (Bowron-Abernethy Chess Engine) or BCE (Boo's Chess Engine).  Rob Abernethy and I worked together to add the learning algorithm.

BCE was the basis for [pspChess](https://github.com/cwbowron/pspchess), an open-source chess game for the PlayStation Portable.

* [Project Proposal Presentation](doc/BACE_proposal.ppt)
* [Final Presentation](doc/BACE_final_presentation.ppt)
* [Project Paper](doc/BACE.pdf)

### Features

* Search
  * NegaScout search
  * Quiescence search
  * Iterative deepening
  * Null move search
  * Transposition tables
  * Aspiration Windows
  * Move ordering
* Pondering
* Opening book
* icsdrone / WinBoard interface
* Adjustable search depth
* FEN parsing
* Temporal difference learning

### Rating 

BCE has played games on freechess.org under two accounts:

#### BACE (learning enabled)

```
Last disconnected: Tue Mar  4, 14:37 EST 2003

          rating     RD      win    loss    draw   total   best
Blitz      1908    350.0    6740    3215     503   10458   1995 (25-Dec-2000)
Standard   1529    350.0       1       5       0       6
Lightning  2028    350.0     423     202      15     640   2078 (19-Dec-2000)
Wild       1760    350.0       9       9       0      18
```

#### ddlchess (learning disabled)

```
Last disconnected: Sat Sep 22, 10:04 EST 2001

          rating     RD      win    loss    draw   total   best
Blitz      1947    350.0    6151    4913     813   11877   2054 (05-Jan-2001)
Standard   1677    350.0    1180     868      81    2129   1975 (12-Aug-2000)
Lightning  2057    350.0    2652    1648     170    4470   2179 (30-Aug-2000)
Wild       1742    350.0       6      32       0      38
```
   
### Acknowledgements

Thanks to Dann Corbit for his many helpful hints, suggestions, and
code. 
