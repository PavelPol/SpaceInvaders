#ifndef GAME_H
#define GAME_H

#include <string>
#include "graphic.h"

class Game
{
    static const std::string LevelFileName;
    static const int FieldWidth;
    static const int FieldHeight;
    static const int PlayerStartX;
    static const int PlayerStartY;

    
    static const int PlayerShipSpeed;
    static const int EnemyShipSpeed;
    static const int PlayerRocketSpeed;
    static const int EnemyRocketSpeed;
    static const int PlayerFireSpeed;
    static const int EnemyFireSpeed;
    static const int EnemyFireChance;
    static const int EnemyCorrectFireChance;
    static const int BangTime;
    static const int TimerInterval;

    static const int StartLifes;
    static const int StartHomingRockets;
    static const int HomingRocketsLevelBonus;

    static const int MaxFieldItem;

    struct FieldItem {
        FieldItem(int _type, int _x, int _y, int _xSpeed = 0, int _ySpeed = 0) 
            : type(_type), x(_x), y(_y), dead(false),
              xSpeed(_xSpeed), ySpeed(_ySpeed),
              lastMoveX(0), lastMoveY(0), lastFire(0) {
            ZeroMemory(&vertex, sizeof(vertex));
        }
        FieldItem() 
            : type(0), x(0), y(0), dead(true),
              xSpeed(0), ySpeed(0),
              lastMoveX(0), lastMoveY(0), lastFire(0) {
            ZeroMemory(&vertex, sizeof(vertex));
        }
        int type;
        int x;
        int y;
        int xSpeed;
        int ySpeed;
        Graphic::CUSTOMVERTEX vertex[24];
        bool dead;
        time_t lastMoveX;
        time_t lastMoveY;
        time_t lastFire;
    } * field;
    enum ItemTypes {
        PLAYER_SHIP,
        ENEMY_SHIP_1,
        ENEMY_SHIP_2,
        ENEMY_SHIP_3,
        WALL,
        PLAYER_ROCKET,
        PLAYER_HOMING_ROCKET,
        ENEMY_ROCKET,
        BANG
    };
    Graphic graph;

    int level;
    int lifes;
    int homingRockets;
    int fieldItemCount;
    int enemyCount;
    bool stop;
    void loadLevel();
    FieldItem* player;

    static inline time_t Speed2Time(int speed);

    // синглтон
    Game();
    static Game *instance;

    void rocketAction(FieldItem *rocket);
    void timerAction();
    void updateStatusText();
    FieldItem* getHomingTarget(FieldItem *rocket);
public:
    ~Game();
    static Game* getInstance();
   
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
    bool run();
    void updateField();
    void exit();
};

#endif