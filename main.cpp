#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

bool DEBUG_PARSER = false;

using namespace std;

/* para almacenar cada comando y sus argumentos */
struct Command {
    string name;
    vector<string> args;
};

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

/* intercepta CTRL+C para que no haga nada */
void handle_sigint(int signum) {
    (void)signum;
    return;
}

/* decide que hacer con cada comando */
void process_commands(vector<Command>& cmds, bool& exit_called) {
    for (auto cmd: cmds) {
        if (cmd.name.empty()) continue;
        if (cmd.name == "exit") {
            exit_called = true;
            break;
        }
        else {
            pid_t c_pid = fork();

            if (c_pid == 0) {
                std::vector<char *> argv; /* hay que construir el arreglo c-style */
                argv.push_back(const_cast<char *>(cmd.name.c_str()));
                for (string arg: cmd.args) {
                    argv.push_back(const_cast<char *>(arg.c_str()));
                }
                argv.push_back(nullptr);
                execvp(argv[0], argv.data());
            }
            wait(nullptr);
        }
    }
}

/* al inicializarse configura todos los handlers de señales usados en el programa */
/* ¿pq una clase y no una función? tiene variables que debe guardar, aunque no se usen */
class Setup_signals {
private:
    struct sigaction sa;
public:
    Setup_signals() {
        sa.sa_handler = handle_sigint;
        sa.sa_flags = 0; 
        sigemptyset(&sa.sa_mask);
        sigaction(SIGINT, &sa, NULL);
    }
};

/* ciclo principal */
int main() {
    Setup_signals();
    bool exit_called = false;
    while (!exit_called) {
        string input = read_input();
        auto cmds = parser(input);
        process_commands(cmds, exit_called);
    }
    cout << "bye bye!" << endl;
}