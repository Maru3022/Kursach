#include "game.h"
#include "ai.h"
#include "tournament.h"
#include "statistics.h"
#include <iostream>
#include <cstdlib>
int checks=0;
void check(bool ok){checks++;if(!ok){std::cerr<<"FAIL "<<checks<<'\n';std::exit(1);}}
void explore(Game g,char computer){check(g.result!=other(computer));if(g.result!=' ')return;if(g.turn==computer){Move m=bestMove(g.board,computer);check(play(g,m.row,m.column));explore(g,computer);}else for(int r=0;r<3;r++)for(int c=0;c<3;c++)if(g.board[r][c]==' '){Game next=g;check(play(next,r,c));explore(next,computer);}}
int main(){Game g;resetGame(g);check(g.turn=='X');check(play(g,0,0));check(!play(g,0,0));int lines[8][3]={{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};for(auto& line:lines){resetGame(g);for(int cell:line)g.board[cell/3][cell%3]='X';check(evaluate(g.board)=='X');}resetGame(g);const char* draw[]={"XOX","XOO","OXX"};for(int r=0;r<3;r++)for(int c=0;c<3;c++)g.board[r][c]=draw[r][c];check(evaluate(g.board)=='D');resetGame(g);g.board[0][0]='X';g.board[0][1]='X';check(chooseMove(g.board,'O',1).column==2);for(int i=0;i<100;i++){Move m=randomMove(g.board);check(g.board[m.row][m.column]==' ');}resetGame(g);explore(g,'X');for(int r=0;r<3;r++)for(int c=0;c<3;c++){Game next=g;check(play(next,r,c));explore(next,'O');}Tournament t;resetTournament(t,3);check(recordRound(t,'D')&&t.draws==1);check(!recordRound(t,'D'));check(nextRound(t));for(int i=0;i<3;i++){check(recordRound(t,t.xPlayer?'O':'X'));if(i<2)check(nextRound(t));}check(t.finished&&t.wins[0]==3);Statistics s;s.games=1;s.wins=1;check(saveStatistics("/tmp/ttt-test-stats.txt",s));Statistics loaded;check(loadStatistics("/tmp/ttt-test-stats.txt",loaded)&&loaded.wins==1);std::cout<<"Passed "<<checks<<" checks\n";}
