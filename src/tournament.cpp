#include "tournament.h"
#include <fstream>
#include <iomanip>

static int validTarget(int n) { return n==3 || n==5 || n==7 ? n : 1; }
void resetSeries(MatchSeries& s,int n) { s=MatchSeries{}; s.target=validTarget(n); }
bool recordGame(MatchSeries& s,char result) {
    if(s.roundRecorded || s.finished || (result!='X' && result!='O' && result!='D')) return false;
    s.roundRecorded=true; ++s.games;
    if(result=='D') ++s.draws; else ++s.wins[result=='X' ? s.xPlayer : 1-s.xPlayer];
    s.finished=s.wins[0]>=s.target || s.wins[1]>=s.target; return true;
}
bool prepareNextGame(MatchSeries& s) {
    if(!s.roundRecorded || s.finished) return false;
    s.xPlayer=1-s.xPlayer; s.roundRecorded=false; return true;
}
int seriesWinner(const MatchSeries& s) { return !s.finished ? -1 : (s.wins[0]>=s.target ? 0 : 1); }

void createChampionship(Championship& c,const std::array<Participant,8>& p,int target) {
    c=Championship{}; c.players=p; c.target=validTarget(target); c.started=true;
    for(auto& m:c.matches) resetSeries(m.series,c.target);
    for(int i=0;i<4;i++) { c.matches[i].player[0]=2*i; c.matches[i].player[1]=2*i+1; c.matches[i].available=true; }
}
bool startBracketMatch(Championship& c,int i) {
    if(!c.started || i<0 || i>=7 || !c.matches[i].available || c.matches[i].completed) return false;
    if(c.currentMatch!=-1 && c.currentMatch!=i) return false;
    c.currentMatch=i; return true;
}
static void advance(Championship& c,int i,int winner) {
    if(i<4) { int n=4+i/2, slot=i%2; c.matches[n].player[slot]=winner; c.matches[n].available=c.matches[n].player[0]>=0&&c.matches[n].player[1]>=0; }
    else if(i<6) { int slot=i-4; c.matches[6].player[slot]=winner; c.matches[6].available=c.matches[6].player[0]>=0&&c.matches[6].player[1]>=0; }
    else c.champion=winner;
}
bool recordBracketGame(Championship& c,int i,char result) {
    if(i<0 || i>=7 || c.currentMatch!=i || c.matches[i].completed) return false;
    BracketMatch& m=c.matches[i]; if(!recordGame(m.series,result)) return false;
    if(m.series.finished) { m.winner=m.player[seriesWinner(m.series)]; m.completed=true; advance(c,i,m.winner); c.currentMatch=-1; }
    return true;
}
bool nextBracketGame(Championship& c,int i) { return i>=0&&i<7&&c.currentMatch==i&&!c.matches[i].completed&&prepareNextGame(c.matches[i].series); }
int nextAvailableMatch(const Championship& c) { for(int i=0;i<7;i++) if(c.matches[i].available&&!c.matches[i].completed) return i; return -1; }

bool saveChampionship(const std::string& path,const Championship& c) {
    std::ofstream out(path,std::ios::trunc); if(!out) return false;
    out<<"TTTCH2 "<<c.target<<' '<<c.currentMatch<<' '<<c.champion<<' '<<c.started<<'\n';
    for(const auto& p:c.players) out<<static_cast<int>(p.type)<<' '<<std::quoted(p.name)<<'\n';
    for(const auto& m:c.matches) out<<m.player[0]<<' '<<m.player[1]<<' '<<m.available<<' '<<m.completed<<' '<<m.winner<<' '<<m.series.target<<' '<<m.series.wins[0]<<' '<<m.series.wins[1]<<' '<<m.series.draws<<' '<<m.series.games<<' '<<m.series.xPlayer<<' '<<m.series.roundRecorded<<' '<<m.series.finished<<'\n';
    return bool(out);
}
bool loadChampionship(const std::string& path,Championship& c) {
    std::ifstream in(path); std::string sig; Championship v;
    if(!(in>>sig>>v.target>>v.currentMatch>>v.champion>>v.started)||sig!="TTTCH2") return false;
    for(auto& p:v.players) { int t; if(!(in>>t>>std::quoted(p.name))||t<0||t>3) return false; p.type=static_cast<PlayerType>(t); }
    for(auto& m:v.matches) if(!(in>>m.player[0]>>m.player[1]>>m.available>>m.completed>>m.winner>>m.series.target>>m.series.wins[0]>>m.series.wins[1]>>m.series.draws>>m.series.games>>m.series.xPlayer>>m.series.roundRecorded>>m.series.finished)) return false;
    if((v.target!=1&&v.target!=3&&v.target!=5)||v.currentMatch<-1||v.currentMatch>6||v.champion<-1||v.champion>7) return false;
    for(const auto& m:v.matches) if(m.player[0]<-1||m.player[0]>7||m.player[1]<-1||m.player[1]>7||m.winner<-1||m.winner>7||m.series.target!=v.target||m.series.wins[0]<0||m.series.wins[1]<0||m.series.draws<0) return false;
    c=v; return true;
}
