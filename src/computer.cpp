#include "computer.h"
#include "game.h"

#include <cstdlib>

Move getRandomMove(char board[3][3]) {
    Move freeCells[9];
    int count = 0;
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            if (board[row][column] == ' ') {
                freeCells[count++] = {row, column};
            }
        }
    }
    return count > 0 ? freeCells[std::rand() % count] : Move{};
}

static Move findWinningMove(char board[3][3], char symbol) {
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            if (board[row][column] != ' ') continue;
            board[row][column] = symbol;
            const bool wins = evaluate(board) == symbol;
            board[row][column] = ' ';
            if (wins) return {row, column};
        }
    }
    return {};
}

Move getMediumMove(char board[3][3], char computer, char human) {
    if (evaluate(board) != ' ') return {};

    Move move = findWinningMove(board, computer);
    if (move.row != -1) return move;

    move = findWinningMove(board, human);
    if (move.row != -1) return move;

    if (board[1][1] == ' ') return {1, 1};
    return getRandomMove(board);
}

int minimax(char board[3][3], int depth, bool maximizing, char computer, char human) {
    const char result = evaluate(board);
    if (result == computer) return 10 - depth;
    if (result == human) return depth - 10;
    if (result == 'D') return 0;

    int best = maximizing ? -100 : 100;
    const char symbol = maximizing ? computer : human;
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            if (board[row][column] != ' ') continue;
            board[row][column] = symbol;
            const int score = minimax(board, depth + 1, !maximizing, computer, human);
            board[row][column] = ' ';
            if (maximizing && score > best) best = score;
            if (!maximizing && score < best) best = score;
        }
    }
    return best;
}

Move getBestMove(char board[3][3], char computer, char human) {
    if (evaluate(board) != ' ') return {};

    int bestScore = -100;
    Move bestMove;
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            if (board[row][column] != ' ') continue;
            board[row][column] = computer;
            const int score = minimax(board, 1, false, computer, human);
            board[row][column] = ' ';
            if (score > bestScore) {
                bestScore = score;
                bestMove = {row, column};
            }
        }
    }
    return bestMove;
}

Move getComputerMove(char board[3][3], int difficulty, char computer, char human) {
    if (evaluate(board) != ' ') return {};
    if (difficulty == static_cast<int>(Difficulty::Hard)) {
        return getBestMove(board, computer, human);
    }
    if (difficulty == static_cast<int>(Difficulty::Medium)) {
        return getMediumMove(board, computer, human);
    }
    return getRandomMove(board);
}

