# Bulls-Cows-Game

  Bulls and Cows is a code-breaking mind or paper and pencil game for two or more players. 
The game is played in turns by two opponents who aim to decipher the other's secret code by trial and error.
The numerical version of the game is usually played with 4 digits, and is how this implementation works.

On a sheet of paper, the players each write a 4-digit secret number. The digits must be all different. 
Then, in turn, the players try to guess their opponent's number who gives the number of matches. The 
digits of the number guessed also must all be different. If the matching digits are in their right 
positions, they are "bulls", if in different positions, they are "cows". Example:

Secret number: 4271
Opponent's try: 1234
Answer: 1 bull and 2 cows. (The bull is "2", the cows are "4" and "1".)


In this implementation, there will only be one player that is trying to guess a 4-digit code, while the server 
determines the code the client (player) is trying to predict, and providing hints in the same manner that another
player would.

