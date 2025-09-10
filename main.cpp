#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>

/*
    TODO: comando 'cd' no funciona
*/

bool DEBUG_PARSER = false;

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
    EXIT_END_CALLED = 2,
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
    bool first = true;
    for (auto word: words) {
        if (mode == 0) {
            cmd.name = word;
            cmd.first = first;
            first = false;
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

/* decide que hacer con un comando dado */
void process_command(Command& cmd, int (&read_pipe)[2], int (&write_pipe)[2]) {

    /* redireccionar a I/O estandar */
    if (read_pipe[0] != STDIN_FILENO) {
        dup2(read_pipe[0], STDIN_FILENO);
        close(read_pipe[0]);
    }
    if (write_pipe[1] != STDOUT_FILENO) {
        dup2(write_pipe[1], STDOUT_FILENO);
        close(write_pipe[1]);
    }

    /* reconstruir los argumentos c-style */
    vector<char *> argv;
    argv.push_back(const_cast<char *>(cmd.name.c_str()));
    for (string &arg : cmd.args) {
        argv.push_back(const_cast<char *>(arg.c_str()));
    }
    argv.push_back(nullptr);

    /*
    interpretación del comando
    */
    /* entrada vacia */
    if (cmd.name.empty() && cmd.first) _exit(EXIT_OK);
    /* exit */
    else if (cmd.name == "exit") _exit(EXIT_END_CALLED);
    /* uso incorrecto de pipes */
    else if (cmd.name == "|" || cmd.name.empty()) {
        cerr << "Pipe operator used incorrectly. Use the following syntax:" << endl
        << "[command 1] | [command 2] | ... | [command n]" << endl;
        _exit(EXIT_ERROR);
    }
    /* comando custom: miprof */
    else if (cmd.name == "miprof") {
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
        _exit(EXIT_OK);
    }
    /* comandos built-in */
    else {
        execvp(argv[0], argv.data());
        if (errno == ENOENT) {
            cerr << "Command \"" << cmd.name << "\" doesn't exist." << endl;
        }
        _exit(EXIT_ERROR);
    }
}

/*
gestiona que hacer con toda la secuencia de comandos entregada en una sola entrada
gestiona las pipes entre comandos
*/
void process_all(vector<Command>& cmds, bool& exit_called) {
    int read_pipe[2] = { STDIN_FILENO, -1 };
    int write_pipe[2];

    for (size_t i = 0; i < cmds.size(); i++) {

        /* nueva pipe de escritura */
        if (i < cmds.size() - 1) {
            pipe(write_pipe);
        } else { /* la ultima pipe es la salida estandar */
            write_pipe[0] = -1;
            write_pipe[1] = STDOUT_FILENO;
        }

        pid_t c_pid = fork();

        if (c_pid == 0) {
            /* el hijo lee la pipe anterior y escribe en la siguiente */
            if (read_pipe[1] != -1) close(read_pipe[1]);
            if (write_pipe[0] != -1) close(write_pipe[0]);
            process_command(cmds[i], read_pipe, write_pipe);
        } 

        else {
            /* pipes obsoletas */
            if (read_pipe[0] != STDIN_FILENO) close(read_pipe[0]);
            if (read_pipe[1] != -1) close(read_pipe[1]);

            int status;
            waitpid(c_pid, &status, 0);

            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                if (exit_code == EXIT_ERROR) break;
                if (exit_code == EXIT_END_CALLED) {
                    exit_called = true;
                    break;
                }
            }

            /* siguiente iteración */
            if (i < cmds.size() - 1) {
                read_pipe[0] = write_pipe[0];
                read_pipe[1] = write_pipe[1];
            }
        }
    }
}

/*
al inicializarse configura todos los handlers de señales usados en el programa
¿pq una clase y no una función? tiene variables que debe guardar, aunque no se usen
*/
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
        process_all(cmds, exit_called);
    }
    cout << "bye bye!" << endl;
}