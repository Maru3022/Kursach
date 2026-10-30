#include "interface.h"
#include "game.h"
#include "computer.h"
#include "tournament.h"
#include "statistics.h"
#include "raylib.h"
#include <array>
#include <cstdlib>
#include <filesystem>
#include <string>

// raylib and Win32 expose several palette names as macros. The UI uses its
// own course-project palette, so remove those macro aliases locally.
#undef BLUE
#undef GREEN
#undef ORANGE
#undef RED
#undef TEXT

enum class Screen { Menu, QuickSetup, SeriesSetup, TournamentSetup, Game, Bracket, Stats, Rules };
enum class Mode { Quick, Series, Tournament };
static const Color BG={16,24,39,255}, PANEL={27,41,64,255}, PANEL2={35,52,78,255};
static const Color BLUE={79,140,255,255}, TEAL={37,214,197,255}, ORANGE={255,173,102,255};
static const Color GREEN={53,212,154,255}, TEXT={245,247,252,255}, MUTED={170,184,204,255}, RED={242,105,112,255};
static Font uiFont{};

static void label(const std::string& s,float x,float y,float size=22,Color color=TEXT) { DrawTextEx(uiFont,s.c_str(),{x,y},size,1,color); }
static void centered(const std::string& s,Rectangle r,float size=22,Color color=TEXT) {
    while(size>11 && MeasureTextEx(uiFont,s.c_str(),size,1).x>r.width-18) size-=1;
    Vector2 v=MeasureTextEx(uiFont,s.c_str(),size,1); label(s,r.x+(r.width-v.x)/2,r.y+(r.height-v.y)/2,size,color);
}
static bool button(const std::string& s,Rectangle r,bool selected=false,bool enabled=true) {
    bool hover=enabled&&CheckCollisionPointRec(GetMousePosition(),r);
    DrawRectangleRounded(r,.18f,8,!enabled?Color{39,49,65,255}:hover?Color{66,118,215,255}:selected?BLUE:PANEL2);
    centered(s,r,20,enabled?TEXT:Color{105,116,132,255});
    return hover&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
static void editText(std::string& value,Rectangle r,bool active,const std::string& hint) {
    DrawRectangleRounded(r,.13f,8,active?PANEL2:PANEL); label(value.empty()?hint:value,r.x+14,r.y+13,19,value.empty()?MUTED:TEXT);
    if(!active) return;
    for(int code=GetCharPressed();code>0;code=GetCharPressed()) if(code>=32&&code<=0x04ff&&value.size()<40) { int n=0; value.append(CodepointToUTF8(code,&n),n); }
    if(IsKeyPressed(KEY_BACKSPACE)&&!value.empty()) { size_t p=value.size()-1; while(p>0&&(static_cast<unsigned char>(value[p])&0xc0)==0x80)--p; value.erase(p); }
}
static std::filesystem::path dataDir() {
#ifdef _WIN32
    const char* base=std::getenv("LOCALAPPDATA");
#else
    const char* base=std::getenv("XDG_DATA_HOME");
    if(!base) base=std::getenv("HOME");
#endif
    std::filesystem::path p=base?base:GetApplicationDirectory(); p/="TicTacToe"; std::error_code ec; std::filesystem::create_directories(p,ec); return p;
}
static int computerDifficulty(PlayerType type) { return type==PlayerType::Hard?2:type==PlayerType::Medium?1:0; }
static std::string typeName(PlayerType t) { const char* n[]={"Человек","Компьютер: лёгкий","Компьютер: средний","Компьютер: сложный"}; return n[static_cast<int>(t)]; }
static void drawMark(char mark,Rectangle cell) {
    if(mark=='X') { DrawLineEx({cell.x+30,cell.y+30},{cell.x+cell.width-30,cell.y+cell.height-30},8,BLUE); DrawLineEx({cell.x+cell.width-30,cell.y+30},{cell.x+30,cell.y+cell.height-30},8,BLUE); }
    else if(mark=='O') DrawRing({cell.x+cell.width/2,cell.y+cell.height/2},cell.width*.27f,cell.width*.33f,0,360,64,ORANGE);
}
static void drawBoard(Game& g,bool inputAllowed,Rectangle area) {
    float gap=10, side=(area.width-gap*2)/3;
    for(int r=0;r<3;r++) for(int c=0;c<3;c++) {
        Rectangle cell={area.x+c*(side+gap),area.y+r*(side+gap),side,side}; bool win=false;
        for(int k:g.line) if(k==r*3+c) win=true;
        bool hover=inputAllowed&&g.board[r][c]==' '&&CheckCollisionPointRec(GetMousePosition(),cell);
        DrawRectangleRounded(cell,.12f,8,win?Color{35,91,83,255}:hover?Color{42,63,94,255}:PANEL);
        if(hover&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) play(g,r,c);
        drawMark(g.board[r][c],cell);
    }
}
static std::string participantName(const Championship& c,int i) { return i<0?"Ожидание победителя":c.players[i].name; }
static Rectangle cardRect(int i) {
    if(i<4) return {45.0f,145.0f+i*142,300,104};
    if(i<6) return {490.0f,216.0f+(i-4)*284,300,104};
    return {935,358,300,104};
}
static void drawConnections() {
    for(int i=0;i<4;i++) { Rectangle a=cardRect(i),b=cardRect(4+i/2); float y=a.y+a.height/2, by=b.y+b.height/2; DrawLineEx({a.x+a.width,y},{410,y},2,PANEL2); DrawLineEx({410,y},{410,by},2,PANEL2); DrawLineEx({410,by},{b.x,by},2,PANEL2); }
    for(int i=4;i<6;i++) { Rectangle a=cardRect(i),b=cardRect(6); float y=a.y+a.height/2,by=b.y+b.height/2; DrawLineEx({a.x+a.width,y},{860,y},2,PANEL2); DrawLineEx({860,y},{860,by},2,PANEL2); DrawLineEx({860,by},{b.x,by},2,PANEL2); }
}
static bool drawMatchCard(const Championship& c,int i) {
    const BracketMatch& m=c.matches[i]; Rectangle r=cardRect(i); bool current=c.currentMatch==i;
    Color fill=m.completed?Color{27,69,61,255}:current?Color{29,65,119,255}:PANEL;
    bool hover=m.available&&!m.completed&&CheckCollisionPointRec(GetMousePosition(),r); if(hover) fill=PANEL2;
    DrawRectangleRounded(r,.10f,8,fill); if(current) DrawRectangleRoundedLinesEx(r,.10f,8,2,BLUE); else if(m.completed) DrawRectangleRoundedLinesEx(r,.10f,8,2,GREEN);
    Color a=m.completed&&m.winner!=m.player[0]?MUTED:TEXT,b=m.completed&&m.winner!=m.player[1]?MUTED:TEXT;
    label(participantName(c,m.player[0]),r.x+13,r.y+12,17,a); label(std::to_string(m.series.wins[0]),r.x+r.width-30,r.y+12,18,a);
    label(participantName(c,m.player[1]),r.x+13,r.y+43,17,b); label(std::to_string(m.series.wins[1]),r.x+r.width-30,r.y+43,18,b);
    label(m.completed?"Завершён":m.available?(current?"Идёт матч":"Готов к игре"):"Ожидание",r.x+13,r.y+76,13,m.completed?GREEN:current?BLUE:MUTED);
    return hover&&IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

int runInterface() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE); InitWindow(1280,800,"Крестики-нолики"); SetWindowMinSize(1280,800);
    // Fixed inset stays fully visible with Windows display scaling (125/150%).
    SetWindowPosition(30,30);
    SetTargetFPS(60); SetExitKey(KEY_NULL);
    int codes[400]; int count=0; for(int i=32;i<=126;i++) codes[count++]=i; for(int i=0x400;i<=0x4ff;i++) codes[count++]=i; codes[count++]=0x2014; codes[count++]=0x2026;
    std::string fp=std::string(GetApplicationDirectory())+"assets/fonts/DejaVuSans.ttf"; uiFont=LoadFontEx(fp.c_str(),48,codes,count);
    if(uiFont.texture.id==0) { CloseWindow(); return 2; } SetTextureFilter(uiFont.texture,TEXTURE_FILTER_BILINEAR);
    auto dir=dataDir(); Statistics stats; loadStatistics((dir/"statistics.txt").string(),stats);
    Championship champ; bool hasSave=loadChampionship((dir/"tournament.txt").string(),champ);
    std::array<Participant,8> setupPlayers; for(int i=0;i<8;i++) setupPlayers[i]={"Игрок "+std::to_string(i+1),PlayerType::Human};
    Screen screen=Screen::Menu; Mode mode=Mode::Quick; Game game; resetGame(game); MatchSeries series;
    Participant duel[2]={{"Игрок 1",PlayerType::Human},{"Компьютер: сложный",PlayerType::Hard}}; int quickX=0, target=3, activeInput=-1, matchIndex=-1;
    bool recorded=false, confirmExit=false, running=true; double lastMove=GetTime(); std::string toast;
    auto resetRound=[&](){ resetGame(game); recorded=false; lastMove=GetTime(); };
    auto beginMatch=[&](int i){ if(startBracketMatch(champ,i)) { matchIndex=i; mode=Mode::Tournament; resetRound(); screen=Screen::Game; saveChampionship((dir/"tournament.txt").string(),champ); } };
    while(running&&!WindowShouldClose()) {
        BeginDrawing(); ClearBackground(BG);
        label("КРЕСТИКИ-НОЛИКИ",42,24,30); label("C++17 · raylib · локальные алгоритмы",44,61,16,MUTED);
        if(screen==Screen::Menu) {
            label("Три в ряд. Один победитель.",390,135,30); const char* items[]={"Быстрая игра","Серия матчей","Турнирная сетка","Статистика","Правила","Выход"};
            for(int i=0;i<6;i++) if(button(items[i],{420.0f,205.0f+i*78,440,58})) {
                if(i==0) screen=Screen::QuickSetup; if(i==1) screen=Screen::SeriesSetup;
                if(i==2) screen=hasSave?Screen::Bracket:Screen::TournamentSetup; if(i==3) screen=Screen::Stats; if(i==4) screen=Screen::Rules; if(i==5) running=false;
            }
        } else if(screen==Screen::QuickSetup||screen==Screen::SeriesSetup) {
            bool isSeries=screen==Screen::SeriesSetup; label(isSeries?"СЕРИЯ МАТЧЕЙ":"БЫСТРАЯ ИГРА",55,115,28);
            for(int p=0;p<2;p++) { label("Участник "+std::to_string(p+1),80+p*590,175,18,MUTED); Rectangle ir={80.0f+p*590,208,530,50};
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(GetMousePosition(),ir)) activeInput=p;
                editText(duel[p].name,ir,activeInput==p,"Введите имя");
                for(int t=0;t<4;t++) if(button(typeName(static_cast<PlayerType>(t)),{80.0f+p*590,280.0f+t*58,530,45},duel[p].type==static_cast<PlayerType>(t))) { duel[p].type=static_cast<PlayerType>(t); if(t) duel[p].name=typeName(duel[p].type); }
            }
            if(!isSeries) { label("Кто играет за X",80,535,18,MUTED); if(button("Участник 1",{80,568,250,48},quickX==0)) quickX=0; if(button("Участник 2",{350,568,250,48},quickX==1)) quickX=1; }
            else { label("До скольких побед",80,535,18,MUTED); int ts[]={3,5,7}; for(int i=0;i<3;i++) if(button(std::to_string(ts[i]),{80.0f+i*175,568,155,48},target==ts[i])) target=ts[i]; }
            if(button("Назад",{80,680,250,55})) screen=Screen::Menu;
            if(button("Начать",{950,680,250,55})) { if(duel[0].name.empty()) duel[0].name="Игрок 1"; if(duel[1].name.empty()) duel[1].name="Игрок 2"; mode=isSeries?Mode::Series:Mode::Quick; if(isSeries) resetSeries(series,target); resetRound(); screen=Screen::Game; }
        } else if(screen==Screen::TournamentSetup) {
            label("СОЗДАНИЕ ТУРНИРА",42,104,27); label("Нажмите тип, чтобы переключить уровень компьютерного соперника",42,142,17,MUTED);
            for(int i=0;i<8;i++) { float x=i<4?55:660,y=190+(i%4)*100; Rectangle ir={x,y,390,42};
                if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)&&CheckCollisionPointRec(GetMousePosition(),ir)) activeInput=i;
                editText(setupPlayers[i].name,ir,activeInput==i,"Имя участника");
                if(button(typeName(setupPlayers[i].type),{x+410,y,150,42})) { int t=(static_cast<int>(setupPlayers[i].type)+1)%4; setupPlayers[i].type=static_cast<PlayerType>(t); if(t) setupPlayers[i].name=typeName(setupPlayers[i].type)+" "+std::to_string(i+1); }
            }
            label("Побед в противостоянии:",55,615,18,MUTED); int ts[]={1,3,5}; for(int i=0;i<3;i++) if(button(std::to_string(ts[i]),{300.0f+i*90,600,75,45},target==ts[i])) target=ts[i];
            if(button("Главное меню",{55,690,260,52})) screen=Screen::Menu;
            if(button("Создать турнир",{930,690,295,52})) { for(int i=0;i<8;i++) if(setupPlayers[i].name.empty()) setupPlayers[i].name="Игрок "+std::to_string(i+1); createChampionship(champ,setupPlayers,target); hasSave=true; saveChampionship((dir/"tournament.txt").string(),champ); screen=Screen::Bracket; }
        } else if(screen==Screen::Bracket) {
            label("ТУРНИРНАЯ СЕТКА",42,98,27); label("ЧЕТВЕРТЬФИНАЛ",80,122,16,MUTED); label("ПОЛУФИНАЛ",570,122,16,MUTED); label("ФИНАЛ",1050,122,16,MUTED);
            drawConnections(); int clicked=-1; for(int i=0;i<7;i++) if(drawMatchCard(champ,i)) clicked=i;
            if(clicked>=0) beginMatch(clicked); if(champ.champion>=0) { Rectangle cr={850,520,375,78}; DrawRectangleRounded(cr,.12f,8,Color{27,78,66,255}); centered("ЧЕМПИОН: "+champ.players[champ.champion].name,cr,22,GREEN); }
            int next=nextAvailableMatch(champ); if(next>=0&&button("Начать следующий матч",{470,680,340,52})) beginMatch(next);
            if(champ.currentMatch>=0&&button("Вернуться к матчу",{470,680,340,52})) { matchIndex=champ.currentMatch; mode=Mode::Tournament; screen=Screen::Game; }
            if(button("Новый турнир",{42,680,250,52})) { hasSave=false; screen=Screen::TournamentSetup; }
            if(button("Главное меню",{980,680,250,52})) { confirmExit=true; }
        } else if(screen==Screen::Game) {
            Participant* p=duel; MatchSeries* s=mode==Mode::Series?&series:nullptr; int xSide=quickX;
            if(mode==Mode::Series) xSide=series.xPlayer;
            if(mode==Mode::Tournament) { BracketMatch& m=champ.matches[matchIndex]; duel[0]=champ.players[m.player[0]]; duel[1]=champ.players[m.player[1]]; p=duel; s=&m.series; xSide=m.series.xPlayer; }
            int turnSide=game.turn=='X'?xSide:1-xSide; bool computerTurn=p[turnSide].type!=PlayerType::Human&&game.result==' ';
            if(computerTurn&&GetTime()-lastMove>.55) { Move m=getComputerMove(game.board,computerDifficulty(p[turnSide].type),game.turn,other(game.turn)); if(play(game,m.row,m.column)) lastMove=GetTime(); }
            drawBoard(game,!computerTurn&&game.result==' ',{75,170,450,450});
            label(p[xSide].name+"  —  X",610,175,25,BLUE); label(p[1-xSide].name+"  —  O",610,220,25,ORANGE);
            if(s) { label("Счёт серии: "+std::to_string(s->wins[0])+" : "+std::to_string(s->wins[1]),610,292,28); label("Ничьи: "+std::to_string(s->draws)+" · цель: "+std::to_string(s->target),610,332,19,MUTED); }
            std::string status=game.result==' '?"Ход: "+p[turnSide].name:game.result=='D'?"Ничья":"Победил: "+p[game.result=='X'?xSide:1-xSide].name;
            if(s&&s->finished) status="Серия выиграна: "+p[seriesWinner(*s)].name; label(status,610,400,25,game.result==' '?TEXT:GREEN);
            if(game.result!=' '&&!recorded) { recorded=true; if(mode==Mode::Tournament) { recordBracketGame(champ,matchIndex,game.result); if(matchIndex==6&&champ.champion>=0) { ++stats.tournaments; saveStatistics((dir/"statistics.txt").string(),stats); } saveChampionship((dir/"tournament.txt").string(),champ); }
                else if(mode==Mode::Series) recordGame(series,game.result); else { stats.games++; if(game.result=='D') stats.draws++; else if((game.result=='X'?xSide:1-xSide)==0) stats.wins++; else stats.losses++; saveStatistics((dir/"statistics.txt").string(),stats); } }
            if(game.result!=' ') { bool done=s&&s->finished; if(!done&&button(s?"Следующая партия":"Новая партия",{610,480,330,52})) { if(s) { if(mode==Mode::Tournament) nextBracketGame(champ,matchIndex); else prepareNextGame(series); } resetRound(); }
                if(done&&button(mode==Mode::Tournament?"К сетке":"Завершить серию",{610,480,330,52})) screen=mode==Mode::Tournament?Screen::Bracket:Screen::Menu; }
            if(mode==Mode::Tournament&&button("Посмотреть сетку",{610,550,330,52})) screen=Screen::Bracket;
            if(mode!=Mode::Tournament&&button("Главное меню",{610,620,330,52})) screen=Screen::Menu;
        } else if(screen==Screen::Stats) {
            label("СТАТИСТИКА",55,120,28); std::string rows[]={"Всего быстрых партий: "+std::to_string(stats.games),"Побед участника 1: "+std::to_string(stats.wins),"Поражений участника 1: "+std::to_string(stats.losses),"Ничьих: "+std::to_string(stats.draws),"Завершено турниров: "+std::to_string(stats.tournaments)};
            for(int i=0;i<5;i++) { DrawRectangleRounded({55,175.0f+i*72,650,54},.12f,8,PANEL); label(rows[i],80,190.0f+i*72,20); }
            if(button("Сбросить",{55,560,250,52})) { stats=Statistics{}; saveStatistics((dir/"statistics.txt").string(),stats); toast="Статистика сброшена"; }
            if(button("В меню",{55,640,250,52})) screen=Screen::Menu;
        } else if(screen==Screen::Rules) {
            label("ПРАВИЛА И АЛГОРИТМЫ",55,115,28); const char* r[]={"X всегда ходит первым. Побеждают три одинаковых символа в ряд.","Лёгкий компьютер выбирает случайную свободную клетку.","Средний сначала выигрывает, блокирует угрозу, занимает центр или ходит случайно.","Сложный использует оптимальный Minimax и при корректной игре не проигрывает.","В серии ничьи учитываются отдельно, а X меняется после каждой партии.","В сетке победители реально сыгранных серий переходят в следующий раунд."};
            for(int i=0;i<6;i++) { DrawRectangleRounded({55,175.0f+i*70,1165,52},.08f,8,PANEL); label(r[i],75,190.0f+i*70,18); }
            if(button("В меню",{55,650,250,52})) screen=Screen::Menu;
        }
        if(confirmExit) { DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),Color{0,0,0,170}); Rectangle box={390,270,500,230}; DrawRectangleRounded(box,.08f,8,PANEL); centered("Турнир сохранён. Выйти в меню?",{410,300,460,60},22); if(button("Выйти",{430,410,190,50})) { screen=Screen::Menu; confirmExit=false; } if(button("Остаться",{660,410,190,50})) confirmExit=false; }
        if(!toast.empty()) label(toast,900,755,15,MUTED);
        EndDrawing();
    }
    UnloadFont(uiFont); CloseWindow(); return 0;
}
