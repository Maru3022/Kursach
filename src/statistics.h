#pragma once
#include <string>
struct Statistics { int games=0, wins=0, losses=0, draws=0, tournaments=0, tournamentWins=0, tournamentLosses=0; };
bool loadStatistics(const std::string& path,Statistics& s);
bool saveStatistics(const std::string& path,const Statistics& s);
