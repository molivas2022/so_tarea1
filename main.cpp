#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <csignal>
#include <fstream>
#include <sys/resource.h>

#include "input.h"
#include "utils.h"
#include "signal.h"
#include "tests.h"

using namespace std;

pid_t miprof_child_pid = -1; /*esto para el alarm handler lo puedo utilizar*/

/*
para llamar a programas externos (ej: ls, grep), importante manejar las pipes
notar que comandos como 'cd' no son programas externos
*/
pid_t process_external(Command& cmd, int (&read_pipe)[2], int (&write_pipe)[2]) {
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

        return c_pid;
    }
}


/* funcion utilizada por process_miprof, si en la variable global miprof_child_pid_local es
un pid valido, se ASESINA ese proceso*/
void miprof_alarm_handler(int sig) {
    (void) sig;
    if (miprof_child_pid > 0) {
        kill(miprof_child_pid, SIGKILL);
        cerr << "El comando supero el tiempo máximo de ejecución. El proceso ha sido ASESINADO!.\n" << endl;
    }
}

/*
programa tipo profiler integrado en la shell, este permite ejecutar cualquier comando o programa y
capturar la información respecto al tiempo de ejecución en: tiempos de usuario, sistema y real mas
la información acerca del peak de memoria máxima residente. Los comandos disponibles son:

- miprof ejec <comando> [args]: ejecuta y despliega en pantalla la información al ejecutar un
comando.
- miprof ejecsave <filepath> <comando> [args]: ejecuta y escribe en un archivo la información al
ejecutar un comando.
- miprof ejecmaxtime <segundos> <comando> [args]: ejecuta y despliega en pantalla la información 
al ejecutar un comando. En este caso la ejecución del comando tiene un tiempo máximo de ejecución,
en caso no se cumpla este tiempo el proceso se termina.
*/
void process_miprof(Command& cmd, int (&read_pipe)[2], int (&write_pipe)[2]) {
    
    /* comando sin argumentos ._. */
    if (cmd.args.size() == 0) {
        cerr << "Uso: miprof [ejec|ejecsave <filepath>|ejecmaxtime <segundos>|help] <comando> [args]" << endl;
        return;
    }

    string mode = cmd.args[0];
    size_t arg_start_index = 1; /* quizas no la mejor forma jejeje */
    string save_file;
    int max_seconds = 0;

    /* en caso de ser modo ejecsave, apartamos ruta de archivo*/
    if (mode == "ejecsave") {
        if (cmd.args.size() < 3) {
            cerr << "Uso: miprof ejecsave <filepath> <comando> [args]" << endl;
            return;
        }
        save_file = cmd.args[1];
        arg_start_index = 2;
    } 
    /*en caso de ser ejecmaxtime, guardamos los segundos limite*/
    else if (mode == "ejecmaxtime") {
        if (cmd.args.size() < 3) {
            cerr << "Uso: miprof ejecmaxtime <segundos> <comando> [args]" << endl;
            return;
        }
        try {
            max_seconds = stoi(cmd.args[1]);
        } catch (...) {
            cerr << "Error al convertir segundos: " << cmd.args[1] << endl;
            return;
        }
        arg_start_index = 2;
    } 
    /*Aqui se deberian imprimir las intruciones*/
    else if (mode == "help") {
        cout 
            << "Programa tipo profiler integrado en la shell, el cual permite ejecutar cualquier comando \n"
            << "o programa y capturar la información respecto al tiempo de ejecución en: tiempos de usuario,\n"
            << "sistema y real junto con la información acerca del peak de memoria máxima residente.\n"
            << "Los comandos disponibles son:\n"
            << "- miprof ejec <comando> [args]: ejecuta y despliega en pantalla la información de tiempo del comando.\n"
            << "- miprof ejecsave <filepath> <comando> [args]: ejecuta y escribe en un archivo la información\n"
            << "de tiempo del comando.\n"
            << "- miprof ejecmaxtime <segundos> <comando> [args]: ejecuta y despliega en pantalla la información\n"
            << "al ejecutar un comando. En este caso la ejecución del comando tiene un tiempo máximo de ejecución\n"
            << "que termina abruptamente el programa si lo excede.\n";
        return;
    } 
    else if (mode != "ejec") {
        cerr << "Comando desconocido: " << mode << "\n Uso: miprof [ejec|ejecsave <filepath>|ejecmaxtime <segundos>|help] <comando> [args]" <<endl;
        return;
    }

    if (cmd.args.size() <= arg_start_index) {
        cerr << "No hay un comando especificado." << endl;
        return;
    }

    /* reconstruir los argumentos c-style, esta vez desde arg_start_index o.O*/
    vector<char *> argv;
    for (size_t i = arg_start_index; i < cmd.args.size(); i++) {
        argv.push_back(const_cast<char *>(cmd.args[i].c_str()));
    }
    argv.push_back(nullptr);

    struct rusage prev_usage;
    getrusage(RUSAGE_CHILDREN, &prev_usage); /*obtiene memoria sobre hijo*/

    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start); /*TODO: nose si monotonic es el mejor para este caso*/

    pid_t c_pid = fork();

    if (c_pid == 0) {
        /* PROCESO HIJO */
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

        /* ejecutar programa externo */
        execvp(argv[0], argv.data());

        if (errno == ENOENT) {
            cerr << "Command \"" << cmd.name << "\" doesn't exist." << endl;
        }
        _exit(EXIT_ERROR);

        // print_the_creature(); /*Algún dia la creatura revivira*/
    }
    else {
        /* PROCESO PADRE*/
        /* pipes no usadas */
        if (read_pipe[0] != STDIN_FILENO) close(read_pipe[0]);
        if (read_pipe[1] != -1) close(read_pipe[1]);

        miprof_child_pid = c_pid;

        /*temporizador para matar proceso*/
        if (mode == "ejecmaxtime" && max_seconds > 0) {
            signal(SIGALRM, miprof_alarm_handler); /*si se manda la señal de alarma, MATAR MATAR MATAR*/
            alarm(max_seconds); /*10, 9, 8,....*/
        }

        int status;
        waitpid(c_pid, &status, 0);

        /*salvado por la campana...*/
        if (mode == "ejecmaxtime"){
            alarm(0); 
        }
        miprof_child_pid = -1;

        clock_gettime(CLOCK_MONOTONIC, &end); /*TODO: nose si monotonic es el mejor para este caso*/

        /*ahora se viene lo chido (mide uso)*/
        struct rusage usage;
        getrusage(RUSAGE_CHILDREN, &usage); /*obtiene memoria sobre hijo*/

        /*calculo de tiempos y MAXRSS*/
        double real_time = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec)/1e9;
        double user_time = (usage.ru_utime.tv_sec + usage.ru_utime.tv_usec / 1e6) - (prev_usage.ru_utime.tv_sec + prev_usage.ru_utime.tv_usec / 1e6);
        double sys_time  = (usage.ru_stime.tv_sec  + usage.ru_stime.tv_usec  / 1e6) - (prev_usage.ru_stime.tv_sec  + prev_usage.ru_stime.tv_usec  / 1e6);
        long max_rss = usage.ru_maxrss;


        /*concatenamos comando y sus argumentos en un string*/
        string command_str;
        for (size_t i = 0; i < argv.size(); ++i) {
            if (argv[i] == nullptr) {
                break;
            }
            command_str += argv[i];
            if (i != argv.size() - 1 && argv[i + 1] != nullptr) {
                command_str += " ";
            }
        }

        string results =
                        "----------- Resultados -----------\n"
                        "Comando: " + command_str + "\n" + /*quizas deberia ser todo el comando aquí jeje*/
                        "Tiempo de usuario: " + to_string(user_time) + " s\n" +
                        "Tiempo del sistema: " + to_string(sys_time) + " s\n" +
                        "Tiempo real: " + to_string(real_time) + " s\n" +
                        "Peak de memoria máxima: " + to_string(max_rss) + " KB\n"
                        "----------------------------------\n";

        cout << results << endl;

        if (mode == "ejecsave") {
            ofstream outcsv("last_profile.csv", ios::app);
            if (!outcsv) {
                cerr << "Log CSV no se pudo abrir." << endl;
            } 
            else {
                outcsv << to_string(user_time) + "," + to_string(sys_time) + "," + to_string(real_time) + "," + to_string(max_rss) + "\n";
            }

            ofstream outfile(save_file, ios::app);
            if (!outfile) {
                cerr << "No se pudo abrir el archivo: " << save_file << endl;
            } 
            else {
                outfile << results;
            }
        }
    }
}

void pipeline(vector<Command>&, bool&);

/*
decide que hacer con un comando
*/
pid_t process(Command& cmd, bool& exit_called,
             int (&read_pipe)[2], int (&write_pipe)[2]) {
    if (cmd.name.empty() && cmd.first) {
        return -1;
    }
    else if (cmd.name == "exit") {
        exit_called = true;
        return -1;
    }
    else if (cmd.name == "|" || cmd.name.empty()) {
        cerr << "Pipe operator used incorrectly. Use the following syntax:" << endl
        << "[command 1] | [command 2] | ... | [command n]" << endl;
        return -1;
    }
    else if (cmd.name == "miprof") {
        /* wip */
        process_miprof(cmd, read_pipe, write_pipe);
        return -1;
    }
    else if (cmd.name == "batchrun") {
        ifstream commands;
        commands.open(cmd.args[0]);
        char buffer[256];
        if (commands) {
            while (!commands.eof()) {
                commands.getline(buffer, 256);
                auto cmds = parser(buffer);
                pipeline(cmds, exit_called);
            }
        }
    }
    else if (cmd.name == "test") {
        save_new_texts();  
    }
    else {
        /* ejecutables externos (ejemplo: ls) */
        return process_external(cmd, read_pipe, write_pipe);
    }
    return 0;
}

/*
   gestiona las pipes entre comandos
   */
void pipeline(vector<Command>& cmds, bool& exit_called) {
    int read_pipe[2] = { STDIN_FILENO, -1 };
    int write_pipe[2];

    vector<pid_t> child_pids;

    for (size_t i = 0; i < cmds.size(); i++) {

        /* nueva pipe de escritura */
        if (i < cmds.size() - 1) {
            pipe(write_pipe);
        } else { /* la ultima pipe es la salida estandar */
            write_pipe[0] = -1;
            write_pipe[1] = STDOUT_FILENO;
        }

        pid_t pid = process(cmds[i], exit_called, read_pipe, write_pipe);
        if (pid > 0) child_pids.push_back(pid);
        if (exit_called) break;

        /* siguiente iteración */
        if (i < cmds.size() - 1) {
            read_pipe[0] = write_pipe[0];
            read_pipe[1] = write_pipe[1];
        }
    }

    /* esperamos a todos los cabros chicos */
    for (pid_t pid : child_pids) {
        int status;
        waitpid(pid, &status, 0);
    }
}

/* ciclo principal */
int main() {
    //print_the_creature();
    struct sigaction sa;
    disable_ctrl_c(sa);
    /*TODO: ver porque no funciona el ALRM de los cojoneees*/
    activate_miprof_alarm(sa);

    bool exit_called = false;
    while (!exit_called) {
        string input = read_input();
        auto cmds = parser(input);
        pipeline(cmds, exit_called);
    }
    cout << "bye bye!" << endl;
}
