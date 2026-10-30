#pragma once

struct Move {
    int row = -1;
    int column = -1;
};

enum class Difficulty {
    Easy = 0,
    Medium = 1,
    Hard = 2
};

Move getRandomMove(char board[3][3]);
Move getMediumMove(char board[3][3], char computer, char human);
int minimax(char board[3][3], int depth, bool maximizing, char computer, char human);
Move getBestMove(char board[3][3], char computer, char human);
Move getComputerMove(char board[3][3], int difficulty, char computer, char human);

