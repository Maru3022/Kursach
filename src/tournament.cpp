#include "tournament.h"
void resetTournament(Tournament& t,int target) { t=Tournament{}; t.target=(target==5 || target==7)?target:3; }
bool recordRound(Tournament& t,char result) {
    if(t.recorded || t.finished || (result!='X' && result!='O' && result!='D')) return false;
    t.recorded=true; t.games++;
    if(result=='D') t.draws++;
    else t.wins[result=='X'?t.xPlayer:1-t.xPlayer]++;
    t.finished=t.wins[0]>=t.target || t.wins[1]>=t.target;
    return true;
}
bool nextRound(Tournament& t) {
    if(!t.recorded || t.finished) return false;
    t.xPlayer=1-t.xPlayer; t.recorded=false; return true;
}
