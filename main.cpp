#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>

#include "input.h"
#include "utils.h"
#include "signal.h"

using namespace std;

void process_external(Command& cmd, int (&read_pipe)[2], int (&write_pipe)[2]) {
    pid_t c_pid = fork();

    if (c_pid == 0) {
        /* el hijo lee la pipe anterior y escribe en la siguiente */
        if (read_pipe[1] != -1) close(read_pipe[1]);
        if (write_pipe[0] != -1) close(write_pipe[0]);
        
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

        /* ejecutar programa externo */
        execvp(argv[0], argv.data());
        if (errno == ENOENT) {
            cerr << "Command \"" << cmd.name << "\" doesn't exist." << endl;
        }
        _exit(EXIT_ERROR);
    }
    else {
        /* pipes no usadas */
        if (read_pipe[0] != STDIN_FILENO) close(read_pipe[0]);
        if (read_pipe[1] != -1) close(read_pipe[1]);

        int status;
        waitpid(c_pid, &status, 0);
    }
}

void process_miprof(Command& cmd, int (&read_pipe)[2], int (&write_pipe)[2]) {
    pid_t c_pid = fork();

    if (c_pid == 0) {
        /* el hijo lee la pipe anterior y escribe en la siguiente */
        if (read_pipe[1] != -1) close(read_pipe[1]);
        if (write_pipe[0] != -1) close(write_pipe[0]);
        
        /* redireccionar a I/O estandar */
        if (read_pipe[0] != STDIN_FILENO) {
            dup2(read_pipe[0], STDIN_FILENO);
            close(read_pipe[0]);
        }
        if (write_pipe[1] != STDOUT_FILENO) {
            dup2(write_pipe[1], STDOUT_FILENO);
            close(write_pipe[1]);
        }

        (void)cmd;
        print_the_creature();
        _exit(EXIT_OK);
    }
    else {
        /* pipes no usadas */
        if (read_pipe[0] != STDIN_FILENO) close(read_pipe[0]);
        if (read_pipe[1] != -1) close(read_pipe[1]);

        int status;
        waitpid(c_pid, &status, 0);
    }
}

/*
decide que hacer con un comando
*/
void process(Command& cmd, bool& exit_called,
             int (&read_pipe)[2], int (&write_pipe)[2]) {
    if (cmd.name.empty() && cmd.first) {
    }
    else if (cmd.name == "exit") {
        exit_called = true;
    }
    else if (cmd.name == "|" || cmd.name.empty()) {
        cerr << "Pipe operator used incorrectly. Use the following syntax:" << endl
        << "[command 1] | [command 2] | ... | [command n]" << endl;
    }
    else if (cmd.name == "miprof") {
        /* wip */
        process_miprof(cmd, read_pipe, write_pipe);
    }
    else {
        /* ejecutables externos (ejemplo: ls) */
        process_external(cmd, read_pipe, write_pipe);
    }
}

/*
gestiona las pipes entre comandos
*/
void pipeline(vector<Command>& cmds, bool& exit_called) {
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

        process(cmds[i], exit_called, read_pipe, write_pipe);
        if (exit_called) break;

        /* siguiente iteración */
        if (i < cmds.size() - 1) {
            read_pipe[0] = write_pipe[0];
            read_pipe[1] = write_pipe[1];
        }
    }
}

/* ciclo principal */
int main() {
    struct sigaction sa;
    disable_ctrl_c(sa);

    bool exit_called = false;
    while (!exit_called) {
        string input = read_input();
        auto cmds = parser(input);
        pipeline(cmds, exit_called);
    }
    cout << "bye bye!" << endl;
}