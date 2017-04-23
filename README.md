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

BCE has played games on freechess.org under two accounts:
* BACE - for the engine with learning
   * Blitz - 1908 (best: 1995)
   * Lightning - 2028 (best: 2078)
   * Wild - 1760
* ddlchess - for the engine without learning
   * Blitz - 1947 (best: 2054)
   * Lightning - 2057 (best: 2179)
   * Wild - 1742
   
## Acknowledgements

Thanks to Dann Corbit for his many helpful hints, suggestions, and
code. 
