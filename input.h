#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "utils.h"

bool DEBUG_PARSER = false;

using namespace std;

/* lee entrada... wow */
string read_input() {
    cout << "$ ";
    string input;
    while (!getline(cin, input)) {
        /* esta parte es pq recibir señales (como interceptar CTRL+C)
        cancela la I/O de los procesos */
        cin.clear();  
        input.clear();
    }
    return input;
}

/*
parsea la entrada devolviendo un vector de comandos (vease struct Command)
*/
vector<Command> parser(string inputstr) {
    vector<Command> output;

    /* separar por espacios */
    vector<string> words;
    {
    istringstream iss(inputstr);
    string word;
    while (iss >> word) words.push_back(word);
    }

    /* leer cada comando y sus argumentos */
    {
    Command cmd;
    cmd.first = true;
    int mode = 0;
    for (auto word: words) {
        if (mode == 0) {
            cmd.name = word;
            mode = 1;
        }
        else if (mode == 1) {
            if (word == "|") {
                output.push_back(cmd);
                cmd = Command();
                cmd.first = false;
                mode = 0;
                continue;
            }
            cmd.args.push_back(word);
        }
    }
    output.push_back(cmd);
    }

    /* debug */
    if (DEBUG_PARSER) {
        for (auto cmd: output) {
            cout << "command:\n\t" << cmd.name << endl;
            cout << "args:" << endl;
            for (auto arg: cmd.args) {
                cout << "\t" << arg << endl;
            }
            cout << endl;
        }
    }

    return output;
}