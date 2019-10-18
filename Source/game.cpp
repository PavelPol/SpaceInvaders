#include "version.h"
#include <string>
#include <fstream>
#include "game.h"

const std::string Game::LevelFileName = "levels.txt";

const int Game::FieldWidth   = 130;
const int Game::FieldHeight  = 150;
const int Game::MaxFieldItem = 2048;

const int Game::StartLifes              = 10;
const int Game::StartHomingRockets      = 5;
const int Game::HomingRocketsLevelBonus = 3;


const int Game::TimerInterval = 10;

const int Game::PlayerStartX           = FieldWidth / 2 - 1;
const int Game::PlayerStartY           = FieldHeight - 10;
const int Game::PlayerShipSpeed        = 70;
const int Game::EnemyShipSpeed         = 40;
const int Game::PlayerRocketSpeed      = 100;
const int Game::EnemyRocketSpeed       = 40;
const int Game::PlayerFireSpeed        = 3;
const int Game::EnemyFireSpeed         = 1;
const int Game::BangTime               = 400;
const int Game::EnemyFireChance        = 25;
const int Game::EnemyCorrectFireChance = 90;

Game *Game::instance = 0;

Game::Game() : graph(FieldWidth * Graphic::CellSize, FieldHeight * Graphic::CellSize, WndProc), 
               field(0), fieldItemCount(0), lifes(StartLifes), homingRockets(StartHomingRockets),
               enemyCount(0), level(1), stop(false)
{
    try {
        srand(GetTickCount());
        loadLevel();
        updateField();
    } catch(...) {
        throw;
    }
}

Game::~Game()
{
    instance = 0;
    if (field) {
        delete[] field;
    }
}

Game* Game::getInstance()
{
    if (!instance) {
        instance = new Game();
    }
    return instance;
}

void Game::loadLevel()
{
    enum _MODE {
        LEVEL_NUM, ITEM_COUNT, ITEMS,
        E, W, C_X, C_Y, S_X, S_Y
    };

    if (field) {
        delete[] field;
        field = 0;
        enemyCount = 0;
    }
    std::ifstream file(LevelFileName);
    if (file.fail()) {
        throw "Не найден файл уровней";
    }

    std::string in;
    while (!file.eof()) {
        std::getline(file, in);
        std::string val;
        int currentLevelNum = 0, 
            itemCount       = 0, 
            valCount        = 0;
        
        int mode = LEVEL_NUM;
        
        val.resize(5);
        for (unsigned int i = 0; i < in.length(); i++) {
            switch (in[i]) {
                case ':':
                    if (mode == LEVEL_NUM) {
                        currentLevelNum = stoi(val);
                        mode = ITEM_COUNT;
                        valCount = 0;
                        val.clear();
                        val.resize(5);
                    } else if(mode == ITEM_COUNT) {
                        fieldItemCount = stoi(val) + 1;
                        if (fieldItemCount < 0 || fieldItemCount > MaxFieldItem) {
                            throw "Ошибка в файле уровней";
                        }
                        field = new FieldItem[MaxFieldItem];
                        field[0] = FieldItem(PLAYER_SHIP, PlayerStartX, PlayerStartY);
                        player = &field[0];
                        mode = ITEMS;
                        itemCount = 1;
                        valCount = 0;
                        val.clear();
                        val.resize(5);
                    }
                    break;
                case ',':
                    if (mode == E || mode == W) {
                        throw "Ошибка в файле уровней";
                    } else if (mode == ITEMS) {
                        itemCount++;
                        if (fieldItemCount < itemCount + 1) {
                            throw "Ошибка в файле уровней";
                        }
                        continue;
                    } else if (mode == C_X) {
                        field[itemCount].x = stoi(val);
                        mode = C_Y;
                    } else if (mode == C_Y) {
                        field[itemCount].y = stoi(val);
                        if (field[itemCount].type != WALL) {
                            mode = S_X;
                        }
                    } else if (mode == S_X) {
                        int xSpeed = stoi(val);
                        xSpeed = (xSpeed > 0 ? EnemyShipSpeed : (xSpeed < 0 ? -EnemyShipSpeed : 0));
                        field[itemCount].xSpeed = xSpeed;
                        mode = S_Y;
                    } else if (mode == S_Y) {
                        int ySpeed = stoi(val);
                        ySpeed = (ySpeed > 0 ? EnemyShipSpeed : (ySpeed < 0 ? -EnemyShipSpeed : 0));
                        field[itemCount].ySpeed = ySpeed;
                    }
                    valCount = 0;
                    val.clear();
                    val.resize(5);
                    break;
                case '(':
                    if (mode == E || mode == W) {
                        mode = C_X;
                    } else {
                        throw "Ошибка в файле уровней";
                    }
                    break;
                case ')':
                    if (mode == S_Y) {
                        field[itemCount].ySpeed = stoi(val);
                        mode = ITEMS;
                    } else if (mode == C_Y && field[itemCount].type == WALL) {
                        field[itemCount].y = stoi(val);
                        mode = ITEMS;
                    } else {
                        throw "Ошибка в файле уровней";
                    }
                    valCount = 0;
                    val.clear();
                    val.resize(5);
                    break;
                case 'E':
                    if (mode == ITEMS) {
                        mode = E;
                    } else {
                        throw "Ошибка в файле уровней";
                    }
                    break;
                case 'W':
                    if (mode == ITEMS) {
                        mode = W;
                        field[itemCount].type = WALL;
                        field[itemCount].dead = false;
                    } else {
                        throw "Ошибка в файле уровней";
                    }
                    break;
                case '0': case '1': case '2': case '3': case '4':
                case '5': case '6': case '7': case '8': case '9':
                case '-': case '+':
                    if (mode == E) {
                        switch (in[i]) {
                            case '1': 
                                field[itemCount].type = ENEMY_SHIP_1;
                                break;
                            case '2': 
                                field[itemCount].type = ENEMY_SHIP_2;
                                break;
                            case '3': 
                                field[itemCount].type = ENEMY_SHIP_3;
                                break;
                            default:
                                throw "Ошибка в файле уровней";
                        }
                        enemyCount++;
                        field[itemCount].dead = false;
                    } else if (mode == C_X || mode == C_Y || mode == S_X || mode == S_Y || 
                               mode == LEVEL_NUM || mode == ITEM_COUNT) {
                        val[valCount++] = in[i];
                    } else {
                        throw "Ошибка в файле уровней";
                    }
                    break;
                default:
                    throw "Ошибка в файле уровней";
                    
            }

            if (mode == ITEM_COUNT && 
                currentLevelNum != level) {
                break;
            }
        }
    } // while(EOF)
    if (!field) {
        level = 1;
        loadLevel();
    }
}

bool Game::run()
{
    if (stop) {
        delete this;
        return false;
    }

    graph.execMessages();
    return true;
}

void Game::updateField()
{
    int currentVertexCount = 0;
    int sumVertexCount = 0;
    Graphic::CUSTOMVERTEX commonVertex[MaxFieldItem];
    for (int i = 0; i < fieldItemCount; i++) {
        if (!field[i].dead) {
            switch(field[i].type) {
                case PLAYER_SHIP:
                    currentVertexCount = graph.drawPlayerShip(field[i].x, field[i].y,
                                                        Graphic::WHITE, field[i].vertex);
                    break;
                case ENEMY_SHIP_1:
                case ENEMY_SHIP_2:
                case ENEMY_SHIP_3:
                    currentVertexCount = graph.drawEnemyShip(field[i].x, field[i].y, 
                        (field[i].type == ENEMY_SHIP_1 ? Graphic::WHITE : 
                         (field[i].type == ENEMY_SHIP_2 ? Graphic::BLUE : Graphic::RED)), field[i].vertex);
                    break;
                case WALL:
                    currentVertexCount = graph.drawWall(field[i].x, field[i].y, 
                                                        Graphic::WHITE, field[i].vertex);
                    break;
                case PLAYER_ROCKET:
                    currentVertexCount = graph.drawPlayerRocket(field[i].x, field[i].y, 
                                                                Graphic::WHITE, field[i].vertex);
                    break;
                case PLAYER_HOMING_ROCKET:
                    currentVertexCount = graph.drawPlayerHomingRocket(field[i].x, field[i].y, 
                                                                Graphic::RED, field[i].vertex);
                    break;
                case ENEMY_ROCKET:
                    currentVertexCount = graph.drawEnemyRocket(field[i].x, field[i].y, 
                                                                Graphic::WHITE, field[i].vertex);
                    break;
                case BANG:
                    currentVertexCount = graph.drawBang(field[i].x, field[i].y, 
                                                        Graphic::RED, field[i].vertex);
                    break;
                default:
                    field[i].dead = true;
                    continue;
            }
            memcpy(&commonVertex[sumVertexCount],  field[i].vertex,
                currentVertexCount * sizeof(Graphic::CUSTOMVERTEX));
            sumVertexCount += currentVertexCount;
        }
    }
    graph.setVertex(commonVertex, sumVertexCount / 3);
}

LRESULT CALLBACK Game::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (!instance && uMsg != WM_CREATE) {
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    switch(uMsg){
        case WM_CREATE:
            SetTimer(hWnd, 0, TimerInterval, NULL);
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
        case WM_PAINT: {
			PAINTSTRUCT ps;
			BeginPaint(hWnd, &ps);
            Game::getInstance()->graph.render();
			EndPaint(hWnd, &ps);
			return 0;
		}
        case WM_DESTROY:
            KillTimer(hWnd, 0);
            PostQuitMessage(0);
            Game::getInstance()->exit();
            return 0;
        case WM_TIMER:
            Game::getInstance()->timerAction();
            return 0;
        case WM_KEYUP: {
            Game *game = Game::getInstance();
            switch (wParam) {
                case VK_SUBTRACT:
                    if (game->level > 1) {
                        game->level--;
                        game->loadLevel();
                    }
                    break;
                case VK_ADD:
                    game->level++;
                    game->loadLevel();
                    game->updateStatusText();
                    break;
			    case VK_LEFT:
                case VK_RIGHT:
                    game->player->xSpeed = 0;
                    break;
                case VK_SHIFT:
                case VK_CONTROL: {
                    bool isHomingRocket = wParam == VK_SHIFT;
                    if (game->player->type == BANG) {
                        break;
                    }
                    time_t currentTime = GetTickCount();
                    if (currentTime - game->player->lastFire >=
                                    Speed2Time(PlayerFireSpeed)) {
                        if (isHomingRocket) {
                            if (game->homingRockets == 0) {
                                break;
                            } else {
                                game->homingRockets--;
                            }
                        }
                        FieldItem* rocket = 0;
                        for (int i = 0; i < game->fieldItemCount; i++) {
                            if (game->field[i].dead) {
                                rocket = &game->field[i];
                                break;
                            }
                        }
                        if (!rocket) {
                            rocket = &game->field[game->fieldItemCount++];
                        }
                        rocket->type = isHomingRocket ? PLAYER_HOMING_ROCKET : PLAYER_ROCKET;
                        rocket->x = game->player->x + 1;
                        rocket->y = game->player->y - 2;
                        rocket->xSpeed = 0;
                        rocket->ySpeed = -PlayerRocketSpeed;
                        rocket->lastMoveY = currentTime;
                        rocket->dead = false;
                        game->player->lastFire = currentTime;
                        game->updateField();
                    }
                    break;
                }
            }
            return 0;
        }
        case WM_KEYDOWN:
            switch (wParam) {
			    case VK_LEFT:
                case VK_RIGHT:
                    if (Game::getInstance()->player->type != BANG &&
                        !Game::getInstance()->player->dead) {
                        Game::getInstance()->player->xSpeed += 
                            (wParam == VK_LEFT ? (-PlayerShipSpeed) : PlayerShipSpeed);
                    }
            }
            return 0;
        default:
            return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
}

void Game::exit()
{
    stop = true;
}

void Game::timerAction()
{
    bool updateFlag = false;
    time_t currentTime = GetTickCount();

    for (int i = 0; i < fieldItemCount; i++) {
        if (field[i].dead) {
            // пустые лоты
            continue;
        }
        if (field[i].xSpeed != 0) {
            // движение по горизонтали
            if (currentTime - field[i].lastMoveX >= 
                    Speed2Time(abs(field[i].xSpeed))) {
                field[i].x += (field[i].xSpeed < 0 ? (-1) : 1);
                if (field[i].x < 0) {
                    field[i].x = 0;
                    if (field[i].type != PLAYER_SHIP) {
                        field[i].xSpeed *= (-1);
                    }
                }
                if (field[i].x > FieldWidth - 3) {
                    field[i].x = FieldWidth - 3;
                    if (field[i].type != PLAYER_SHIP) {
                        field[i].xSpeed *= (-1);
                    }
                }
                field[i].lastMoveX = currentTime;

                if (field[i].type == PLAYER_HOMING_ROCKET) {
                    rocketAction(&field[i]);
                }

                updateFlag = true;
            }
        }
        if (field[i].ySpeed != 0) {
            // движение по вертикали
            if (currentTime - field[i].lastMoveY >= 
                    Speed2Time(abs(field[i].ySpeed))) {
                field[i].y += (field[i].ySpeed < 0 ? (-1) : 1);
                field[i].lastMoveY = currentTime;
                            
                if (field[i].type == PLAYER_ROCKET ||
                    field[i].type == PLAYER_HOMING_ROCKET ||
                    field[i].type == ENEMY_ROCKET) {
                    rocketAction(&field[i]);
                }

                if (field[i].type == PLAYER_SHIP && 
                    enemyCount == 0 &&
                    field[i].y <= 0) {
                        level++;
                        loadLevel();
                        updateStatusText();
                        return;
                }

                updateFlag = true;
            }
        }
        if (field[i].type == ENEMY_SHIP_1 ||
            field[i].type == ENEMY_SHIP_2 || 
            field[i].type == ENEMY_SHIP_3) {
            // стрельба противников
            if (currentTime - field[i].lastFire >= 
                                Speed2Time(EnemyFireSpeed)) {
                float fireChance = EnemyFireChance / (1000.f / TimerInterval);
                if (field[i].x + 1 >= player->x &&
                    field[i].x + 1 <= player->x + 2) {
                    fireChance = EnemyCorrectFireChance;
                }
                if ((rand() % 10000) <= int(fireChance * 100)) {
                    FieldItem* rocket = 0;
                    for (int j = 0; j < fieldItemCount; j++) {
                        if (field[j].dead) {
                            rocket = &field[j];
                            break;
                        }
                    }
                    if (!rocket) {
                        rocket = &field[fieldItemCount++];
                    }
                    rocket->type = ENEMY_ROCKET;
                    rocket->x = field[i].x + 1;
                    rocket->y = field[i].y + 4;
                    rocket->xSpeed = 0;
                    rocket->ySpeed = EnemyRocketSpeed;
                    rocket->lastMoveY = currentTime;
                    rocket->dead = false;

                    field[i].lastFire = currentTime;
                }
                
                updateFlag = true;
            }
        }
        if (field[i].type == BANG) {
            if (currentTime - field[i].lastFire >= BangTime) {
                if (&field[i] == player) {
                    player->type = PLAYER_SHIP;
                    player->x    = PlayerStartX;
                    player->y    = PlayerStartY;
                    player->lastMoveX = 0;
                    player->lastFire  = 0;
                } else {
                    field[i].dead = true;
                }

                updateFlag = true;
            }
        }
    }
    if (updateFlag) {
        updateField();
        updateStatusText();
    }
}

void Game::rocketAction(FieldItem *rocket)
{
    if (rocket->type == PLAYER_ROCKET ||
        rocket->type == PLAYER_HOMING_ROCKET ) {
        if (rocket->y < 0) {
            rocket->dead = true;
            return;
        }
        for (int i = 0; i < fieldItemCount; i++) {
            if (&field[i] != rocket && !field[i].dead) {
                if (field[i].type == WALL) {
                    if (rocket->x >= field[i].x && rocket->x <= field[i].x + 1 \
                        && rocket->y == field[i].y) {
                        field[i].type = BANG;
                        field[i].x -= 2; field[i].y--;
                        field[i].lastFire = GetTickCount();
                        rocket->dead = true;
                        return;
                    }
                }
                if (field[i].type == ENEMY_SHIP_1 || 
                    field[i].type == ENEMY_SHIP_2 ||
                    field[i].type == ENEMY_SHIP_3) {
                    if (rocket->x >= field[i].x && rocket->x <= field[i].x + 2 &&
                        (rocket->y >= field[i].y && rocket->y <= field[i].y + 3)) {
                        switch (field[i].type) {
                            case ENEMY_SHIP_1:
                                field[i].type = BANG;
                                field[i].xSpeed = field[i].ySpeed = 0;
                                field[i].lastFire = GetTickCount();
                                rocket->dead = true;
                                enemyCount--;
                                if (enemyCount == 0) {
                                    player->ySpeed = -PlayerShipSpeed;
                                    homingRockets += HomingRocketsLevelBonus;
                                    for (int p = 0; p < fieldItemCount; p++) {
                                        if (!field[p].dead && 
                                            (field[p].type == ENEMY_ROCKET || 
                                             field[p].type == PLAYER_ROCKET)) {
                                            field[p].type = BANG;
                                            field[p].xSpeed = field[p].ySpeed = 0;
                                            field[p].lastFire = GetTickCount();
                                        }
                                    }
                                }
                                return;
                            case ENEMY_SHIP_2:
                                field[i].type = ENEMY_SHIP_1;
                                rocket->dead = true;
                                return;
                            case ENEMY_SHIP_3:
                                field[i].type = ENEMY_SHIP_2;
                                rocket->dead = true;
                                return;
                        }
                    }
                }
            }
        }
        if (rocket->type == PLAYER_HOMING_ROCKET) {
            FieldItem* target = getHomingTarget(rocket);
            if (target) {
                rocket->xSpeed = PlayerRocketSpeed;
                rocket->xSpeed *= (rocket->x > target->x)  ? (-1) : ((rocket->x < target->x) ? 1 :0);
                rocket->ySpeed = PlayerRocketSpeed;
                rocket->ySpeed *= (rocket->y > target->y)  ? (-1) : ((rocket->y < target->y) ? 1 :0); 
            }
        }
    } else if (rocket->type == ENEMY_ROCKET) {
        if (rocket->y > FieldHeight) {
            rocket->dead = true;
            return;
        }
        for (int i = 0; i < fieldItemCount; i++) {
            if (field[i].type == WALL) {
                if (rocket->x >= field[i].x && rocket->x <= field[i].x + 1 \
                    && rocket->y >= field[i].y && rocket->y >= field[i].y) {
                    field[i].type = BANG;
                    field[i].x -= 2; field[i].y--;
                    field[i].lastFire = GetTickCount();
                    rocket->dead = true;
                    return;
                }
            }
            if (field[i].type == PLAYER_SHIP) {
                if (rocket->x >= player->x && rocket->x <= player->x + 2 &&
                   (rocket->y >= player->y && rocket->y <= player->y + 3)) {
                    lifes--;
                    if (lifes == 0) {
                        player->dead = true;
                        MessageBox(0, TEXT("Вы проиграли"), TEXT("sSpace Invaders"),
                                   MB_OK | MB_DEFAULT_DESKTOP_ONLY);
                        level = 1;
                        loadLevel();
                        lifes = StartLifes;
                        homingRockets = StartHomingRockets;
                        return;
                    } else {
                        player->type = BANG;
                        player->xSpeed = player->ySpeed = 0;
                        player->lastFire = GetTickCount();
                    }
                    rocket->dead = true;
                    return;
                }
            }
        }
    }
}

inline time_t Game::Speed2Time(int speed)
{
    return time_t((1. / speed) * 1000);
}

void Game::updateStatusText()
{
    std::wstring text = std::wstring(TEXT("УРОВЕНЬ: "))       + \
                        std::to_wstring((long long)level)     + \
                        (level < 10 ? TEXT("  ") : TEXT(" ")) + \
                        std::wstring(TEXT("КОРАБЛИ: "))       + \
                        std::to_wstring((long long)lifes)     + \
                        (lifes < 10 ? TEXT("  ") : TEXT(" ")) + \
                        std::wstring(TEXT("СН-РАКЕТЫ: "))     + \
                        std::to_wstring((long long)homingRockets);
    graph.setStatusText(text);
}

Game::FieldItem* Game::getHomingTarget(FieldItem *rocket)
{
    float minDist = -1.0f, curDist;
    FieldItem *target = 0;
    for (int i = 0; i < fieldItemCount; i++) {
        if (!field[i].dead && 
            (field[i].type == ENEMY_SHIP_1 || 
             field[i].type == ENEMY_SHIP_2 || 
             field[i].type == ENEMY_SHIP_3)) {
            curDist = sqrt(pow((float)rocket->x - field[i].x, 2.0f) + 
                           pow((float)rocket->y - field[i].y, 2.0f));
            if (minDist < 0 || curDist < minDist) {
                minDist = curDist;
                target = &field[i];
            }
        }
    }
    return target;
}