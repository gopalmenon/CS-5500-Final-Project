# CS-5500-Final-Project
Connect Four Game using Intel TBB for parallel evaluation of move in response to a move by user

I am writing a Connect Four game (see http://en.wikipedia.org/wiki/Connect_Four) that the user can play against the computer. The game consists of 7 columns and 6 rows. The user can drop coins into any of the 7 columns and the computer will respond with its own play. The aim of the game is to win by having 4 coins in a horizontal, vertical or diagonal (sloping down or up) row.

https://en.wikipedia.org/wiki/File:Connect_Four.gif

Every time the user makes a move, the computer will respond by choosing one of a maximum of seven moves so as to maximize its chances of winning. The user can select the level of difficulty of the game. Based on the level of difficulty, the computer will compute a heuristic score for each of 7 possible moves and then subtract the heuristic score for each potential user response to its move. This will continue recursively to a level decided by the difficulty level.

The computation of heuristic scores and evaluation of the best move in response to a user move will be done in parallel.
