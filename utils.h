#pragma once

#include <string>
#include <vector>

using namespace std;

/* para almacenar cada comando y sus argumentos */
struct Command {
    string name;
    vector<string> args;
    bool first;
};

/* codigos con que puede terminar (mediante _exit()) un proceso */
enum exit_codes {
    EXIT_OK         = 0,
    EXIT_ERROR      = 1,
};

/* la creatura */
void print_the_creature() {
    cout <<
    "⠀⠀⠀⠀⠀⠀⠀⠀⣠⣤⠀⠀⠀⠀⠀⠀⠀⢠⣴⡆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl <<
    "⠀⠀⠀⠀⠀⠀⠀⠀⢹⣿⠃⠀⠀⠀⠀⠀⠀⠀⣹⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl <<
    "⠀⠀⠀⠀⠀⠀⠀⠀⠸⣏⠀⠀⠀⠀⠀⠀⠀⠀⣿⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl <<
    "⠀⠀⠀⠀⠀⠀⠀⠀⣼⢻⣄⣀⣠⣀⣤⣤⣀⡀⣿⣆⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀" << endl <<
    "⠀⠀⠀⠀⠀⠀⢠⣴⢏⣾⣿⣿⣿⣿⣿⣿⣿⣷⣿⣿⣦⣄⡀⠀⠀⠀⠀⠀⠀⠀" << endl <<
    "⠀⠀⠀⠀⣠⣾⣿⣿⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣶⣄⡀⠀⠀⠀⠀" << endl <<
    "⠀⣤⣶⣶⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⣦⡀⠀⠀" << endl <<
    "⠀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡗⠀⠀" << endl <<
    "⠀⢻⣿⣿⣿⣿⣿⣿⣿⠟⠛⠛⠛⠿⣿⣿⣿⣿⣿⣿⣿⠟⠛⠻⣿⣿⣿⠀⠀⠀" << endl <<
    "⠀⠈⣿⣿⣿⢿⣿⣿⣿⣷⣦⣤⣀⣀⣽⣿⣿⣿⣿⣿⣧⣀⣀⣤⣿⣿⣿⣇⠀⠀" << endl <<
    "⠀⠘⣿⡿⣼⢿⣯⣿⢿⣿⣿⣿⣿⣿⡯⡝⣎⢿⣻⣿⣿⣿⣿⣿⣿⣿⣿⣿⠂⠀" << endl <<
    "⠀⠀⣿⣹⣭⡟⡿⣞⡿⣯⣟⡿⣳⢷⡹⣜⠠⣈⠵⣯⣿⣿⣿⣿⣿⣿⣿⣿⠃⠀" << endl <<
    "⠀⠀⠹⣯⣝⡳⣱⢛⡷⣻⢿⢷⣛⡬⢧⡙⠦⣉⠰⣹⢿⣿⣿⣿⣿⣿⣿⣿⠀⠀" << endl <<
    "⠀⠀⠀⢷⢮⡵⢣⢯⣜⣳⢻⣞⢧⡛⠴⣉⠰⣀⢆⡱⣺⣿⣿⣿⣿⣿⣿⣿⡇⠀" << endl <<
    "⠀⠀⠀⣾⡑⡎⣝⠲⡜⢆⡳⠜⣎⠿⣳⢭⡳⡵⢮⣷⣿⣿⣿⣿⣿⣿⣿⣿⣿⡀" << endl <<
    "⠀⠀⠼⠣⠜⠱⠌⠓⠜⠣⠜⠱⠈⠆⠡⠊⠱⠉⠗⠺⠳⠿⠿⠿⠿⠿⠿⠿⠿⠧" << endl;
}
