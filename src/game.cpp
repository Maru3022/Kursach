#include "game.h"
char other(char symbol) { return symbol == 'X' ? 'O' : 'X'; }
void resetGame(Game& g) {
    for (int r=0;r<3;r++) for(int c=0;c<3;c++) g.board[r][c]=' ';
    g.turn='X'; g.result=' '; for(int i=0;i<3;i++) g.line[i]=-1;
}
char evaluate(const char b[3][3], int* line) {
    const int lines[8][3]={{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};
    for(int i=0;i<8;i++) {
        int a=lines[i][0], d=lines[i][1], e=lines[i][2];
        char s=b[a/3][a%3];
        if(s!=' ' && s==b[d/3][d%3] && s==b[e/3][e%3]) {
            if(line) for(int j=0;j<3;j++) line[j]=lines[i][j];
            return s;
        }
    }
    for(int r=0;r<3;r++) for(int c=0;c<3;c++) if(b[r][c]==' ') return ' ';
    return 'D';
}
bool play(Game& g,int r,int c) {
    if(g.result!=' ' || r<0 || r>2 || c<0 || c>2 || g.board[r][c]!=' ') return false;
    g.board[r][c]=g.turn; g.result=evaluate(g.board,g.line);
    if(g.result==' ') g.turn=other(g.turn);
    return true;
}
