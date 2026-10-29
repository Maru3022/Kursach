#include "ai.h"
#include "game.h"
#include <cstdlib>
Move randomMove(char b[3][3]) {
    Move free[9]; int count=0;
    for(int r=0;r<3;r++) for(int c=0;c<3;c++) if(b[r][c]==' ') free[count++]={r,c};
    return count ? free[std::rand()%count] : Move{};
}
Move winningMove(char b[3][3],char s) {
    for(int r=0;r<3;r++) for(int c=0;c<3;c++) if(b[r][c]==' ') {
        b[r][c]=s; bool win=evaluate(b)==s; b[r][c]=' ';
        if(win) return {r,c};
    }
    return {};
}
int minimax(char b[3][3],char turn,char computer,int depth) {
    char result=evaluate(b);
    if(result==computer) return 10-depth;
    if(result==other(computer)) return depth-10;
    if(result=='D') return 0;
    bool maximize=turn==computer; int best=maximize ? -100 : 100;
    for(int r=0;r<3;r++) for(int c=0;c<3;c++) if(b[r][c]==' ') {
        b[r][c]=turn;
        int score=minimax(b,other(turn),computer,depth+1);
        b[r][c]=' '; // Отменяем пробный ход перед проверкой следующей ветви.
        if((maximize && score>best) || (!maximize && score<best)) best=score;
    }
    return best;
}
Move bestMove(char b[3][3],char computer) {
    if(evaluate(b)!=' ') return {};
    int best=-100; Move move;
    for(int r=0;r<3;r++) for(int c=0;c<3;c++) if(b[r][c]==' ') {
        b[r][c]=computer; int score=minimax(b,other(computer),computer,1); b[r][c]=' ';
        if(score>best) { best=score; move={r,c}; }
    }
    return move;
}
Move chooseMove(char b[3][3],char computer,int difficulty) {
    if(evaluate(b)!=' ') return {};
    if(difficulty==2) return bestMove(b,computer);
    if(difficulty==1) {
        Move m=winningMove(b,computer); if(m.row!=-1) return m;
        m=winningMove(b,other(computer)); if(m.row!=-1) return m;
        if(b[1][1]==' ') return {1,1};
    }
    return randomMove(b);
}
