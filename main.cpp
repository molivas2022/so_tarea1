#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <signal.h>
#include <unistd.h>

bool DEBUG_PARSER = true;

using namespace std;

struct Command {
    string name;
    vector<string> args;
};

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
TODO: cuando se ingresan dos | | seguidos
TODO ?: cuando se ingresa | seguido del string vacio
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

/* intercepta CTRL+C */
void handle_sigint(int signum) {
    (void)signum;
    return;
}

int main() {
    struct sigaction sa;
    sa.sa_handler = handle_sigint;
    sa.sa_flags = 0; 
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);

    bool exit_called = false;
    while (!exit_called) {
        string input = read_input();
        auto cmds = parser(input);
        for (auto cmd: cmds) {
            if (cmd.name == "exit") {
                exit_called = true;
            }
        }
    }
    cout << "bye bye!" << endl;
}