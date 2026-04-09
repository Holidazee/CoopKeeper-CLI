#include <iostream>
#ifdef _WIN32
#include <windows.h>
#endif
#include "CoopTracker.h"

using namespace std;

int main() {
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut != INVALID_HANDLE_VALUE) {
        DWORD dwMode = 0;
        if (GetConsoleMode(hOut, &dwMode)) {
            SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
        }
    }
#endif

    CoopTracker tracker;

    cout << "\033[35m        ,~.\033[0m\n";
    cout << "\033[35m       ('v')\033[0m\n";
    cout << "\033[35m      ((___))\033[0m\n";
    cout << "\033[35m       ^   ^\033[0m\n";
    cout << "\033[90m============================================================\033[0m\n";
    cout << "\033[1;96m                     COOPKEEPER CLI\033[0m\n";
    cout << "\033[1;36m              Chicken Coop Management System\033[0m\n";
    cout << "\033[90m============================================================\033[0m\n\n";
    cout << "\033[2mTrack chickens, eggs, feed, expenses, health, and cleaning.\033[0m\n";

    tracker.run();

    cout << "\n\033[1;32mThanks for using CoopKeeper!\033[0m\n";
    return 0;
}