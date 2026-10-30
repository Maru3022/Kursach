#include "computer.h"
#include "game.h"
#include "statistics.h"
#include "tournament.h"
#include <array>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>

static int checks=0;
static void check(bool ok,const char* what){++checks;if(!ok){std::cerr<<"FAIL: "<<what<<" (#"<<checks<<")\n";std::exit(1);}}
static void copyBoard(const char a[3][3],char b[3][3]){std::memcpy(b,a,9);}
static void unchanged(const char a[3][3],const char b[3][3],const char* what){check(std::memcmp(a,b,9)==0,what);}

static void explore(Game g,char computer){
    check(g.result!=other(computer),"Hard never loses");
    if(g.result!=' ')return;
    if(g.turn==computer){
        char before[3][3];copyBoard(g.board,before);
        Move m=getBestMove(g.board,computer,other(computer));
        unchanged(before,g.board,"Hard search preserves board");
        check(play(g,m.row,m.column),"Hard legal move");explore(g,computer);
    }else for(int r=0;r<3;r++)for(int c=0;c<3;c++)if(g.board[r][c]==' '){Game n=g;check(play(n,r,c),"Human reply legal");explore(n,computer);}
}
static std::array<Participant,8> people(){std::array<Participant,8> p;for(int i=0;i<8;i++)p[i]={"P"+std::to_string(i),PlayerType::Human};return p;}
static void winSeries(Championship& c,int match,int side){check(startBracketMatch(c,match),"start bracket match");while(!c.matches[match].completed){char r=c.matches[match].series.xPlayer==side?'X':'O';check(recordBracketGame(c,match,r),"record bracket game");check(!recordBracketGame(c,match,r),"no double count");if(!c.matches[match].completed)check(nextBracketGame(c,match),"next bracket game");}}

static void testGame(){
    Game g;resetGame(g);check(g.turn=='X',"X starts");check(play(g,0,0),"legal move");check(g.turn=='O',"alternate");check(!play(g,0,0),"occupied rejected");check(!play(g,-1,0),"range rejected");
    int lines[8][3]={{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};
    for(auto& line:lines){resetGame(g);for(int cell:line)g.board[cell/3][cell%3]='X';check(evaluate(g.board)=='X',"eight wins");}
}
static void testEasy(){
    Game g;resetGame(g);g.board[0][0]='X';char before[3][3];copyBoard(g.board,before);
    for(int i=0;i<100;i++){Move m=getRandomMove(g.board);check(m.row>=0&&g.board[m.row][m.column]==' ',"Easy free cell");}unchanged(before,g.board,"Easy preserves board");
    for(auto& row:g.board)for(char& cell:row)cell='X';g.board[2][1]=' ';Move m=getRandomMove(g.board);check(m.row==2&&m.column==1,"Easy one free cell");g.board[2][1]='O';m=getRandomMove(g.board);check(m.row==-1&&m.column==-1,"Easy full board");
}
static void testMedium(){
    Game g;char before[3][3];resetGame(g);g.board[1][0]=g.board[1][1]='O';Move m=getMediumMove(g.board,'O','X');check(m.row==1&&m.column==2,"Medium wins");
    resetGame(g);g.board[0][0]=g.board[0][1]='X';m=getMediumMove(g.board,'O','X');check(m.row==0&&m.column==2,"Medium blocks");
    resetGame(g);g.board[0][0]=g.board[0][1]='O';g.board[1][0]=g.board[1][1]='X';m=getMediumMove(g.board,'O','X');check(m.row==0&&m.column==2,"Medium win before block");
    resetGame(g);g.board[0][0]='X';m=getMediumMove(g.board,'O','X');check(m.row==1&&m.column==1,"Medium center");
    resetGame(g);g.board[0][0]=g.board[2][2]='O';g.board[1][1]='X';m=getMediumMove(g.board,'O','X');check((m.row==0&&m.column==2)||(m.row==2&&m.column==0),"Medium creates fork");
    resetGame(g);g.board[0][0]=g.board[2][2]='X';g.board[1][1]='O';m=getMediumMove(g.board,'O','X');check((m.row+m.column)%2==1,"Medium blocks opposite-corner forks with side");
    resetGame(g);g.board[1][1]='X';g.board[0][0]='O';copyBoard(g.board,before);m=getMediumMove(g.board,'O','X');check(m.row>=0&&g.board[m.row][m.column]==' ',"Medium positional fallback legal");unchanged(before,g.board,"Medium preserves board");
}
static void testHard(){
    Game g;char before[3][3];resetGame(g);g.board[0][0]=g.board[0][1]='X';copyBoard(g.board,before);Move m=getBestMove(g.board,'X','O');check(m.row==0&&m.column==2,"Hard wins as X");unchanged(before,g.board,"Hard preserves board");
    resetGame(g);g.board[0][0]=g.board[0][1]='X';g.board[1][1]='O';m=getBestMove(g.board,'O','X');check(m.row==0&&m.column==2,"Hard blocks as O");
    const char* draw[]={"XOX","XOO","OXX"};resetGame(g);for(int r=0;r<3;r++)for(int c=0;c<3;c++)g.board[r][c]=draw[r][c];m=getBestMove(g.board,'X','O');check(m.row==-1,"Hard handles draw");
    resetGame(g);g.board[0][0]=g.board[0][1]=g.board[0][2]='X';m=getBestMove(g.board,'O','X');check(m.row==-1,"Hard handles finished game");
    resetGame(g);explore(g,'X');resetGame(g);for(int r=0;r<3;r++)for(int c=0;c<3;c++){Game n=g;check(play(n,r,c),"human opening");explore(n,'O');}
}
static void testComputerMatches(){
    for(int x=0;x<3;x++)for(int o=0;o<3;o++){Game g;resetGame(g);int moves=0;while(g.result==' '){int d=g.turn=='X'?x:o;Move m=getComputerMove(g.board,d,g.turn,other(g.turn));check(play(g,m.row,m.column),"computer match legal");check(++moves<=9,"computer match terminates");}}
    for(char hardSymbol:{'X','O'}){Game g;resetGame(g);while(g.result==' '){int d=g.turn==hardSymbol?static_cast<int>(Difficulty::Hard):static_cast<int>(Difficulty::Medium);Move m=getComputerMove(g.board,d,g.turn,other(g.turn));check(play(g,m.row,m.column),"Hard versus Medium legal");}check(g.result!=other(hardSymbol),"Medium never outperforms Hard");}
}
static void testTournamentAndFiles(){
    for(int target:{1,3,5}){MatchSeries s;resetSeries(s,target);for(int i=0;i<target;i++){check(recordGame(s,s.xPlayer==0?'X':'O'),"series result");check(!recordGame(s,'D'),"series no double count");if(i+1<target)check(prepareNextGame(s),"X alternates");}check(s.finished&&seriesWinner(s)==0&&s.wins[0]==target,"target reached");}
    MatchSeries ds;resetSeries(ds,3);check(recordGame(ds,'D')&&ds.draws==1&&ds.wins[0]==0,"draw handling");
    Championship c;createChampionship(c,people(),1);for(int i=0;i<4;i++)winSeries(c,i,0);check(c.matches[4].available&&c.matches[5].available,"QF advancement");winSeries(c,4,0);winSeries(c,5,1);check(c.matches[6].available,"SF advancement");winSeries(c,6,1);check(c.champion==c.matches[6].player[1],"champion");
    const char* tf="tournament-test.txt";check(saveChampionship(tf,c),"save tournament");Championship loaded;check(loadChampionship(tf,loaded)&&loaded.champion==c.champion&&loaded.players[0].name=="P0","load tournament");std::remove(tf);
    Statistics st;st.games=3;st.wins=1;st.losses=1;st.draws=1;const char* sf="statistics-test.txt";check(saveStatistics(sf,st),"save stats");Statistics sl;check(loadStatistics(sf,sl)&&sl.games==3&&sl.draws==1,"load stats");std::remove(sf);
}
int main(){std::srand(7);testGame();testEasy();testMedium();testHard();testComputerMatches();testTournamentAndFiles();std::cout<<"Passed "<<checks<<" checks\n";}
