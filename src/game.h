#pragma once
struct Game { char board[3][3]; char turn; char result; int line[3]; };
char other(char symbol);
void resetGame(Game& game);
char evaluate(const char board[3][3], int* line = nullptr);
bool play(Game& game, int row, int column);
