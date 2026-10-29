#pragma once
struct Move { int row=-1; int column=-1; };
Move randomMove(char board[3][3]);
Move winningMove(char board[3][3], char symbol);
Move bestMove(char board[3][3], char computer);
Move chooseMove(char board[3][3], char computer, int difficulty);
