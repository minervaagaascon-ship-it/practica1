#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

pid_t g_pid_ejec;
pid_t g_pid_a;
pid_t g_pid_b;
pid_t g_pid_x;
pid_t g_pid_y;
pid_t g_pid_z;

char g_target_process_char;
int g_seconds;


void parse_args(int argc, char *argv[]);

void handler_start_destruction(int s);

void create_A_process(void);
void run_process_A(void);
void handler_A_exec_task(int s);
void handler_A_destroy_and_propagate(int s);

void create_B_process(void);
void run_process_B(void);
void handler_B_exec_task(int s);
void handler_B_destroy_and_propagate(int s);

void create_X_process(void);
void run_process_X(void);
void handler_X_exec_task(int s);
void handler_X_destroy_leaf(int s);

void create_Y_process(void);
void run_process_Y(void);
void handler_Y_exec_task(int s);
void handler_Y_destroy_leaf(int s);

void create_Z_process(void);
void run_process_Z(void);
void handler_Z_alarm(int s);
void handler_Z_destroy_leaf(int s);




void parse_args(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <proceso> <segundos>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    g_target_process_char = argv[1][0];
    g_seconds = atoi(argv[2]);

    if (g_target_process_char != 'A' && g_target_process_char != 'B' &&
        g_target_process_char != 'X' && g_target_process_char != 'Y') {
        fprintf(stderr, "Proceso objetivo invalido.\n");
        exit(EXIT_FAILURE);
    }
}



int main(int argc, char *argv[]) {
    parse_args(argc, argv);

    g_pid_ejec = getpid();
    printf("Soy el proceso ejec: mi pid es %d\n", g_pid_ejec);

    /* SIGUSR2 indica que la tarea ha terminado y comienza el apagado. */
    signal(SIGUSR2, handler_start_destruction);	// ejec no ejecuta tareas, solo espera para la destruccion.

    /* Crea A y espera a que toda su rama termine. */
    create_A_process();

    printf("Soy ejec (%d) y muero\n", g_pid_ejec);
    return 0;
}



void handler_start_destruction(int s) {
    (void)s;
    kill(g_pid_a, SIGUSR2);
}


void create_A_process(void) {	// este codigo lo hace ejec
    switch (g_pid_a = fork()) {
        case 0:
            run_process_A();
            exit(0);

        default:
            wait(NULL);	// aqui espera ejec
    }
}



void run_process_A(void) {
    g_pid_a = getpid();
    printf("Soy el proceso A: mi pid es %d. Mi padre es %d\n", g_pid_a, g_pid_ejec);
    // me preparo para dos formas de despertarme.
    signal(SIGUSR1, handler_A_exec_task);
    signal(SIGUSR2, handler_A_destroy_and_propagate);
    create_B_process();
}



void handler_A_exec_task(int s) {
    pid_t pid;

    (void)s;
    printf("Soy el proceso A con %d, he recibido la senyal.\n", g_pid_a);
    pid = fork();
    if (pid == 0) {
        execlp("pstree", "pstree", (char *)NULL);
        exit(EXIT_FAILURE); // esto realmente es flexeo
    }
    // en mi codigo original aqui habria un else!
    wait(NULL); // espero a que termine mi hijo (reconvertido a comando)
    kill(g_pid_ejec, SIGUSR2); // le digo al proceso ejec que empiece la destruccion!!
}



void handler_A_destroy_and_propagate(int s) {
    (void)s;

    kill(g_pid_b, SIGUSR2);
    wait(NULL);

    printf("Soy A (%d) y muero\n", g_pid_a);
    exit(0);
}



void create_B_process(void) {
    switch (g_pid_b = fork()) {
        case 0:
            run_process_B();
            exit(0);

        default:
            wait(NULL);
    }
}



void run_process_B(void) {
    g_pid_b = getpid();

    printf("Soy el proceso B: mi pid es %d. Mi padre es %d. Mi abuelo es %d\n",
           g_pid_b, g_pid_a, g_pid_ejec);

    signal(SIGUSR1, handler_B_exec_task);
    signal(SIGUSR2, handler_B_destroy_and_propagate);

    create_X_process();
    create_Y_process();
    create_Z_process();

    while (1) {
        pause();
    }
}



void handler_B_exec_task(int s) {
    pid_t pid;

    (void)s;

    printf("Soy el proceso B con %d, he recibido la senyal.\n", g_pid_b);

    pid = fork();

    if (pid == 0) {
        execlp("pstree", "pstree", (char *)NULL);
        exit(EXIT_FAILURE);
    }

    wait(NULL);
    kill(g_pid_ejec, SIGUSR2);
}


void handler_B_destroy_and_propagate(int s) {
    (void)s;

    kill(g_pid_z, SIGUSR2);
    wait(NULL);

    kill(g_pid_y, SIGUSR2);
    wait(NULL);

    kill(g_pid_x, SIGUSR2);
    wait(NULL);

    printf("Soy B (%d) y muero\n", g_pid_b);
    exit(0);
}


void create_X_process(void) {
    switch (g_pid_x = fork()) {
        case 0:
            run_process_X();
            exit(0);
    }
}


void run_process_X(void) {
    g_pid_x = getpid();

    printf("Soy el proceso X: mi pid es %d. Mi padre es %d. Mi abuelo es %d...\n",
           g_pid_x, g_pid_b, g_pid_a);

    signal(SIGUSR1, handler_X_exec_task);
    signal(SIGUSR2, handler_X_destroy_leaf);

    while (1) {
        pause();
    }
}



void handler_X_exec_task(int s) {
    pid_t pid;

    (void)s;

    printf("Soy el proceso X con %d, he recibido la senyal.\n", g_pid_x);

    pid = fork();

    if (pid == 0) {
        execlp("ls", "ls", (char *)NULL);
        exit(EXIT_FAILURE);
    }

    wait(NULL);
    kill(g_pid_ejec, SIGUSR2);
}



void handler_X_destroy_leaf(int s) {
    (void)s;

    printf("Soy X (%d) y muero\n", g_pid_x);
    exit(0);
}



void create_Y_process(void) {
    switch (g_pid_y = fork()) {
        case 0:
            run_process_Y();
            exit(0);
    }
}



void run_process_Y(void) {
    g_pid_y = getpid();

    printf("Soy el proceso Y: mi pid es %d. Mi padre es %d. Mi abuelo es %d...\n",
           g_pid_y, g_pid_b, g_pid_a);

    signal(SIGUSR1, handler_Y_exec_task);
    signal(SIGUSR2, handler_Y_destroy_leaf);

    while (1) {
        pause();
    }
}



void handler_Y_exec_task(int s) {
    pid_t pid;

    (void)s;

    printf("Soy el proceso Y con %d, he recibido la senyal.\n", g_pid_y);

    pid = fork();

    if (pid == 0) {
        execlp("ls", "ls", (char *)NULL);
        exit(EXIT_FAILURE);
    }

    wait(NULL);
    kill(g_pid_ejec, SIGUSR2);
}



void handler_Y_destroy_leaf(int s) {
    (void)s;

    printf("Soy Y (%d) y muero\n", g_pid_y);
    exit(0);
}



void create_Z_process(void) {
    switch (g_pid_z = fork()) {
        case 0:
            run_process_Z();
            exit(0);
    }
}


void run_process_Z(void) {
    g_pid_z = getpid();

    printf("Soy el proceso Z: mi pid es %d. Mi padre es %d. Mi abuelo es %d...\n", g_pid_z, g_pid_b, g_pid_a);

    signal(SIGALRM, handler_Z_alarm);
    signal(SIGUSR2, handler_Z_destroy_leaf);

    alarm(g_seconds);

    while (1) {
        pause();
    }
}



void handler_Z_alarm(int s) {
    (void)s;

    switch (g_target_process_char) {
        case 'A':
            kill(g_pid_a, SIGUSR1);
            break;

        case 'B':
            kill(g_pid_b, SIGUSR1);
            break;

        case 'X':
            kill(g_pid_x, SIGUSR1);
            break;

        case 'Y':
            kill(g_pid_y, SIGUSR1);
            break;
    }
}

void handler_Z_destroy_leaf(int s) {
    (void)s;

    printf("Soy Z (%d) y muero\n", g_pid_z);
    exit(0);
}
