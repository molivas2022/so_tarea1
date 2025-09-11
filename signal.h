#pragma once

#include <signal.h>

/* intercepta CTRL+C para que no haga nada */
void handle_sigint(int signum) {
    (void)signum;
    return;
}

/* desactiva CTRL+C */
void disable_ctrl_c(struct sigaction& sa) {
    sa.sa_handler = handle_sigint;
    sa.sa_flags = 0; 
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
}