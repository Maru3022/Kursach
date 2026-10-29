#include "statistics.h"
#include <fstream>
#include <cstdio>
bool loadStatistics(const std::string& path,Statistics& s) {
    s=Statistics{}; Statistics value; std::ifstream in(path); std::string signature, extra;
    if(!(in>>signature>>value.games>>value.wins>>value.losses>>value.draws>>value.tournaments>>value.tournamentWins>>value.tournamentLosses) || signature!="TTT1" || (in>>extra)) return false;
    if(value.games<0 || value.wins<0 || value.losses<0 || value.draws<0 || value.tournaments<0 || value.tournamentWins<0 || value.tournamentLosses<0) return false;
    if((long long)value.wins+value.losses+value.draws>value.games || (long long)value.tournamentWins+value.tournamentLosses>value.tournaments) return false;
    s=value; return true;
}
bool saveStatistics(const std::string& path,const Statistics& s) {
    std::ofstream out(path,std::ios::trunc);
    out<<"TTT1\n"<<s.games<<' '<<s.wins<<' '<<s.losses<<' '<<s.draws<<' '<<s.tournaments<<' '<<s.tournamentWins<<' '<<s.tournamentLosses<<'\n';
    out.close(); return bool(out);
}
