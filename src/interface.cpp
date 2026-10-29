#include "interface.h"
#include "game.h"
#include "ai.h"
#include "tournament.h"
#include "statistics.h"
#include "raylib.h"
#include <string>
#include <filesystem>
#include <cstdlib>

enum Screen { MENU, SETUP, GAME, STATS, RULES };
const Color background={16,23,39,255}, panel={27,39,60,255}, accent={57,116,235,255};
const Color ink={231,238,250,255}, muted={155,174,202,255}, cross={41,211,190,255}, circle={255,151,97,255};
Font font;
void text(const std::string& s,float x,float y,float size=24,Color color=ink) {
    DrawTextEx(font,s.c_str(),{x,y},size,1,color);
}
void fitted(const std::string& s,Rectangle r,float size,Color color=ink) {
    while(size>10 && MeasureTextEx(font,s.c_str(),size,1).x>r.width-20) size--;
    Vector2 m=MeasureTextEx(font,s.c_str(),size,1);
    text(s,r.x+(r.width-m.x)/2,r.y+(r.height-size)/2,size,color);
}
bool button(const std::string& s,Rectangle r,bool selected=false) {
    bool hover=CheckCollisionPointRec(GetMousePosition(),r);
    DrawRectangleRounded(r,0.18f,8,hover?accent:(selected?Color{46,80,139,255}:panel));
    fitted(s,r,23);
    return hover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}
void input(std::string& value,Rectangle r,bool active) {
    DrawRectangleRounded(r,0.15f,8,active?Color{44,64,95,255}:panel);
    fitted(value.empty()?"Введите имя":value,r,23,value.empty()?muted:ink);
    if(!active) return;
    int code=GetCharPressed();
    while(code>0) {
        if(code>=32 && code<=0x04ff && value.size()<48) { int size=0; const char* utf=CodepointToUTF8(code,&size); value.append(utf,size); }
        code=GetCharPressed();
    }
    if(IsKeyPressed(KEY_BACKSPACE) && !value.empty()) {
        int i=static_cast<int>(value.size())-1;
        while(i>0 && (static_cast<unsigned char>(value[i])&0xc0)==0x80) i--;
        value.erase(i);
    }
}
std::string statisticsPath() {
#ifdef _WIN32
    const char* base=std::getenv("LOCALAPPDATA");
#else
    const char* base=std::getenv("XDG_DATA_HOME");
    if(!base) { const char* home=std::getenv("HOME"); if(home) { std::filesystem::path dir=std::filesystem::path(home)/".local/share/TicTacToe"; std::error_code ec; std::filesystem::create_directories(dir,ec); return (dir/"statistics.txt").string(); } }
#endif
    std::filesystem::path dir=base?std::filesystem::path(base)/"TicTacToe":std::filesystem::path(GetApplicationDirectory())/"data";
    std::error_code ec; std::filesystem::create_directories(dir,ec); return (dir/"statistics.txt").string();
}
int runInterface() {
    InitWindow(1000,700,"TicTacToe"); SetTargetFPS(60); SetExitKey(KEY_NULL);
    int codes[1200]; int count=0;
    for(int c=32;c<=126;c++) codes[count++]=c;
    for(int c=0x400;c<=0x4ff;c++) codes[count++]=c;
    codes[count++]=0x2014; codes[count++]=0x2026;
    std::string fontPath=std::string(GetApplicationDirectory())+"assets/fonts/DejaVuSans.ttf";
    font=LoadFontEx(fontPath.c_str(),48,codes,count);
    if(font.texture.id==0) { CloseWindow(); return 1; }
    SetTextureFilter(font.texture,TEXTURE_FILTER_BILINEAR);
    Screen screen=MENU; bool quit=false, computer=true, tournament=false, confirm=false;
    int difficulty=2,target=3,active=0; char human='X';
    std::string names[2]={"",""}, players[2];
    Game game; resetGame(game); Tournament tour; Statistics stats;
    std::string path=statisticsPath(); bool loaded=loadStatistics(path,stats);
    std::string notice=loaded?"":"Статистика отсутствует или повреждена: начато с нуля.";
    bool recorded=false; double turnStart=0;
    while(!WindowShouldClose() && !quit) {
        BeginDrawing(); ClearBackground(background);
        text("КРЕСТИКИ-НОЛИКИ",50,30,34); text("C++17 / raylib / Minimax",52,76,18,muted);
        if(screen==MENU) {
            text("Три в ряд. Один победитель.",255,140,28);
            const char* labels[]={"Играть","Турнир","Статистика","Правила","Выход"};
            for(int i=0;i<5;i++) if(button(labels[i],{310.0f,210.0f+i*76,380,58})) {
                if(i<2) { tournament=i==1; screen=SETUP; }
                if(i==2) {screen=STATS;confirm=false;}
                if(i==3) screen=RULES;
                if(i==4) quit=true;
            }
        } else if(screen==SETUP) {
            text(tournament?"Настройка турнира":"Новая партия",60,125,30);
            if(button("Против компьютера",{60,185,420,52},computer)) computer=true;
            if(button("Два игрока",{510,185,420,52},!computer)) computer=false;
            Rectangle first={60,285,420,54},second={510,285,420,54};
            text("Первый участник",60,252,20,muted); text(computer?"Соперник: Компьютер":"Второй участник",510,252,20,muted);
            if(IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) { if(CheckCollisionPointRec(GetMousePosition(),first)) active=0; if(CheckCollisionPointRec(GetMousePosition(),second)) active=1; }
            input(names[0],first,active==0); if(!computer) input(names[1],second,active==1);
            if(computer) {
                const char* levels[]={"Лёгкий","Средний","Сложный"};
                for(int i=0;i<3;i++) if(button(levels[i],{60.0f+i*295,370,275,52},difficulty==i)) difficulty=i;
            }
            if(tournament) {
                text("Серия до побед (участники меняют X/O)",60,450,22,muted);
                for(int i=0;i<3;i++) if(button(std::to_string(3+2*i),{60.0f+i*295,490,275,52},target==3+2*i)) target=3+2*i;
            } else if(computer) {
                if(button("Играть за X",{60,475,420,52},human=='X')) human='X';
                if(button("Играть за O",{510,475,420,52},human=='O')) human='O';
            }
            if(button("Начать",{510,590,420,58})) {
                players[0]=names[0].empty()?"Игрок 1":names[0]; players[1]=computer?"Компьютер":(names[1].empty()?"Игрок 2":names[1]);
                resetTournament(tour,target); resetGame(game); recorded=false; turnStart=GetTime(); screen=GAME;
            }
            if(button("В меню",{60,590,420,58})) screen=MENU;
        } else if(screen==GAME) {
            int xPlayer=tournament?tour.xPlayer:(computer && human=='O'?1:0);
            char computerSymbol=xPlayer==1?'X':'O';
            bool aiTurn=computer && game.turn==computerSymbol && game.result==' ';
            if(aiTurn && GetTime()-turnStart>=0.4) {
                Move m=chooseMove(game.board,computerSymbol,difficulty); play(game,m.row,m.column); turnStart=GetTime();
            }
            for(int r=0;r<3;r++) for(int c=0;c<3;c++) {
                Rectangle cell={60.0f+c*146,160.0f+r*146,136,136}; bool win=false;
                for(int k=0;k<3;k++) if(game.line[k]==r*3+c) win=true;
                DrawRectangleRounded(cell,0.12f,6,win?Color{42,88,93,255}:panel);
                if(!aiTurn && CheckCollisionPointRec(GetMousePosition(),cell) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    if(play(game,r,c)) turnStart=GetTime();
                }
                float x=cell.x,y=cell.y;
                if(game.board[r][c]=='X') {DrawLineEx({x+35,y+35},{x+101,y+101},7,cross); DrawLineEx({x+101,y+35},{x+35,y+101},7,cross);}
                if(game.board[r][c]=='O') DrawRing({x+68,y+68},40,47,0,360,60,circle);
            }
            if(game.result!=' ' && !recorded) {
                recorded=true;
                int winner=game.result=='D'?-1:(game.result=='X'?xPlayer:1-xPlayer);
                if(tournament) {
                    recordRound(tour,game.result);
                    if(tour.finished) {stats.tournaments++; if(computer) {if(tour.wins[0]>=target) stats.tournamentWins++; else stats.tournamentLosses++;}}
                } else { stats.games++; if(computer) {if(winner<0) stats.draws++; else if(winner==0) stats.wins++; else stats.losses++;} }
                if(!saveStatistics(path,stats)) notice="Не удалось сохранить статистику.";
            }
            fitted(players[xPlayer]+" — X",{530,155,420,50},26,cross);
            fitted(players[1-xPlayer]+" — O",{530,210,420,50},26,circle);
            if(tournament) {
                text("До "+std::to_string(target)+" побед",550,280,24);
                text("Счёт: "+std::to_string(tour.wins[0])+" : "+std::to_string(tour.wins[1]),550,320,28);
                text("Ничьих: "+std::to_string(tour.draws),550,362,22,muted);
                text("Партия № "+std::to_string(tour.games+(recorded?0:1)),550,397,22,muted);
            }
            std::string status;
            if(game.result==' ') status=aiTurn?"Компьютер думает…":"Ход: "+players[game.turn=='X'?xPlayer:1-xPlayer];
            else if(game.result=='D') status="Ничья!";
            else status="Победил: "+players[game.result=='X'?xPlayer:1-xPlayer];
            if(tournament && tour.finished) status="Победитель турнира: "+players[tour.wins[0]>=target?0:1];
            fitted(status,{525,450,430,65},26);
            if(!tournament || game.result!=' ') {
                std::string label=tournament?(tour.finished?"Новый турнир":"Следующая партия"):"Новая игра";
                if(button(label,{540,530,400,54})) {
                    if(tournament && tour.finished) {screen=SETUP;}
                    else if(!tournament || nextRound(tour)) { resetGame(game);recorded=false;turnStart=GetTime(); }
                }
            }
            if(button("Главное меню",{540,605,400,54})) screen=MENU;
        } else if(screen==STATS) {
            text("Статистика",60,130,30);
            std::string rows[]={"Обычных партий: "+std::to_string(stats.games),"Побед над компьютером: "+std::to_string(stats.wins),"Поражений от компьютера: "+std::to_string(stats.losses),"Ничьих с компьютером: "+std::to_string(stats.draws),"Завершено турниров: "+std::to_string(stats.tournaments),"Турниров выиграно у компьютера: "+std::to_string(stats.tournamentWins),"Турниров проиграно компьютеру: "+std::to_string(stats.tournamentLosses)};
            for(int i=0;i<7;i++) text(rows[i],65,190.0f+i*43,24);
            if(confirm) {
                text("Удалить все результаты?",65,510,24,circle);
                if(button("Да, сбросить",{60,555,420,52})) {stats=Statistics{}; notice=saveStatistics(path,stats)?"Статистика сброшена.":"Ошибка записи статистики.";confirm=false;}
                if(button("Отмена",{510,555,420,52})) confirm=false;
            } else if(button("Сбросить статистику",{60,555,420,52})) confirm=true;
            if(button("В меню",{510,620,420,48})) {screen=MENU;confirm=false;}
        } else if(screen==RULES) {
            text("Правила игры",60,130,30);
            const char* rules[]={"X ходит первым. Затем участники ходят по очереди.","Нажмите свободную клетку, чтобы поставить символ.","Три символа в строке, столбце или по диагонали — победа.","Если поле заполнено без победителя — ничья.","Лёгкий: случайный ход. Средний: победа, защита, центр.","Сложный: Minimax перебирает продолжения и не проигрывает.","Турнир идёт до 3, 5 или 7 побед. Ничьи не дают побед.","После каждой партии турнира участники меняются X и O.","Статистика партий и завершённых турниров хранится отдельно."};
            for(int i=0;i<9;i++) fitted(rules[i],{50,190.0f+i*40,900,36},22);
            if(button("В меню",{310,600,380,58})) screen=MENU;
        }
        if(!notice.empty() && screen!=GAME) fitted(notice,{40,675,920,20},14,muted);
        EndDrawing();
    }
    UnloadFont(font); CloseWindow(); return 0;
}
