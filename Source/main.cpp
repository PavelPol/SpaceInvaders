#include "version.h"
#include <windows.h>
#include <string>

#include "game.h"

INT WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, INT nCmdShow)
{
    UNREFERENCED_PARAMETER(hInstance);
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    try {
        Game *game = Game::getInstance();
        while (game->run());
    } catch(std::string msg) {
        MessageBox(0, std::wstring(msg.begin(), msg.end()).c_str(), TEXT("sSpace Invaders"), MB_OK);
        return EXIT_FAILURE;
    } catch (...) {
        MessageBox(0, TEXT("Возникла ошибка при работе программы"), TEXT("sSpace Invaders"), MB_OK);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

/* Fixes

    - количество жизней принимает значение -1

*/