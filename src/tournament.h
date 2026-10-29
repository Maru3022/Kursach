#pragma once
struct Tournament { int target=3; int wins[2]={0,0}; int draws=0; int games=0; int xPlayer=0; bool recorded=false; bool finished=false; };
void resetTournament(Tournament& t,int target);
bool recordRound(Tournament& t,char result);
bool nextRound(Tournament& t);
