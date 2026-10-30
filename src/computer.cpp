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

static int countWinningMoves(char board[3][3], char symbol) {
    int count = 0;
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            if (board[row][column] != ' ') continue;
            board[row][column] = symbol;
            if (evaluate(board) == symbol) ++count;
            board[row][column] = ' ';
        }
    }
    return count;
}

static int countForkMoves(char board[3][3], char symbol, Move* firstFork = nullptr) {
    int count = 0;
    for (int row = 0; row < 3; ++row) {
        for (int column = 0; column < 3; ++column) {
            if (board[row][column] != ' ') continue;
            board[row][column] = symbol;
            const bool fork = countWinningMoves(board, symbol) >= 2;
            board[row][column] = ' ';
            if (fork) {
                if (count == 0 && firstFork) *firstFork = {row, column};
                ++count;
            }
        }
    }
    return count;
}

static Move randomFrom(char board[3][3], const int cells[][2], int size) {
    Move available[4];
    int count = 0;
    for (int i = 0; i < size; ++i) {
        const int row = cells[i][0], column = cells[i][1];
        if (board[row][column] == ' ') available[count++] = {row, column};
    }
    return count ? available[std::rand() % count] : Move{};
}

Move getMediumMove(char board[3][3], char computer, char human) {
    if (evaluate(board) != ' ') return {};

    Move move = findWinningMove(board, computer);
    if (move.row != -1) return move;

    move = findWinningMove(board, human);
    if (move.row != -1) return move;

    // Create two simultaneous winning threats when possible.
    if (countForkMoves(board, computer, &move) > 0) return move;

    // Against a single fork, occupy its cell. Against several possible forks,
    // choose the move that removes the most of them, preferring a forcing threat.
    const int humanForks = countForkMoves(board, human, &move);
    if (humanForks == 1) return move;
    if (humanForks > 1) {
        Move best;
        int fewestForks = 10;
        bool bestCreatesThreat = false;
        for (int row = 0; row < 3; ++row) {
            for (int column = 0; column < 3; ++column) {
                if (board[row][column] != ' ') continue;
                board[row][column] = computer;
                const int remainingForks = countForkMoves(board, human);
                const bool createsThreat = countWinningMoves(board, computer) > 0;
                board[row][column] = ' ';
                if (remainingForks < fewestForks ||
                    (remainingForks == fewestForks && createsThreat && !bestCreatesThreat)) {
                    fewestForks = remainingForks;
                    bestCreatesThreat = createsThreat;
                    best = {row, column};
                }
            }
        }
        if (best.row != -1) return best;
    }

    if (board[1][1] == ' ') return {1, 1};

    // If the opponent owns a corner, the opposite corner is strategically useful.
    const int oppositeCorners[4][4]={{0,0,2,2},{0,2,2,0},{2,0,0,2},{2,2,0,0}};
    for (const auto& pair : oppositeCorners) {
        if (board[pair[0]][pair[1]] == human && board[pair[2]][pair[3]] == ' ')
            return {pair[2], pair[3]};
    }

    const int corners[4][2]={{0,0},{0,2},{2,0},{2,2}};
    move = randomFrom(board, corners, 4);
    if (move.row != -1) return move;

    const int sides[4][2]={{0,1},{1,0},{1,2},{2,1}};
    return randomFrom(board, sides, 4);
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

