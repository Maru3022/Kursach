#pragma once
#include <array>
#include <string>

enum class PlayerType { Human=0, Easy=1, Medium=2, Hard=3 };
struct Participant { std::string name; PlayerType type=PlayerType::Human; };
struct MatchSeries {
    int target=1, wins[2]={0,0}, draws=0, games=0, xPlayer=0;
    bool roundRecorded=false, finished=false;
};
void resetSeries(MatchSeries&,int target);
bool recordGame(MatchSeries&,char result);
bool prepareNextGame(MatchSeries&);
int seriesWinner(const MatchSeries&);

struct BracketMatch {
    int player[2]={-1,-1};
    MatchSeries series;
    bool available=false, completed=false;
    int winner=-1;
};
struct Championship {
    std::array<Participant,8> players{};
    std::array<BracketMatch,7> matches{};
    int target=1, currentMatch=-1, champion=-1;
    bool started=false;
};
void createChampionship(Championship&,const std::array<Participant,8>&,int target);
bool startBracketMatch(Championship&,int matchIndex);
bool recordBracketGame(Championship&,int matchIndex,char result);
bool nextBracketGame(Championship&,int matchIndex);
int nextAvailableMatch(const Championship&);
bool saveChampionship(const std::string& path,const Championship&);
bool loadChampionship(const std::string& path,Championship&);
