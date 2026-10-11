
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

pid_t pid_ejec;
pid_t pid_a;
pid_t pid_b;
pid_t pid_x;
pid_t pid_y;
pid_t pid_z;

char target_process_char;
int seconds;


void args(int argc, char *argv[]);

void destrucion(int s);

void crearA(void);
void A(void);
void tarea_pstree_A(int s);
void destruccion_A(int s);

void crearB(void);
void B(void);
void tarea_pstree_B(int s);
void destruccion_B(int s);

void crearX(void);
void X(void);
void tarea_ls_X(int s);
void destruccion_X(int s);

void crearY(void);
void Y(void);
void tarea_ls_Y(int s);
void destruccion_Y(int s);

void crearZ(void);
void Z(void);
void alarma_Z(int s);
void destruccion_Z(int s);




void args(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <proceso> <segundos>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    target_process_char = argv[1][0];
    seconds = atoi(argv[2]);

    if (target_process_char != 'A' && target_process_char != 'B' &&
        target_process_char != 'X' && target_process_char != 'Y') {
        fprintf(stderr, "Proceso objetivo invalido.\n");
        exit(EXIT_FAILURE);
    }
}



int main(int argc, char *argv[]) {
    args(argc, argv);

    pid_ejec = getpid();
    printf("Soy el proceso ejec: mi pid es %d\n", pid_ejec);

    /* SIGUSR2 indica que la tarea ha terminado y comienza el apagado. */
    signal(SIGUSR2, destrucion);    // ejec no ejecuta tareas, solo espera para la destruccion.

    /* Crea A y espera a que toda su rama termine. */
    crearA();

    printf("Soy ejec (%d) y muero\n", pid_ejec);
    return 0;
}



void destrucion(int s) {
    (void)s;
    kill(pid_a, SIGUSR2);
}


void crearA(void) { // este codigo lo hace ejec
    switch (pid_a = fork()) {
        case 0:
            A();
            exit(0);

        default:
            wait(NULL); // aqui espera ejec
    }
}



void A(void) {
    pid_a = getpid();
    printf("Soy el proceso A: mi pid es %d. Mi padre es %d\n", pid_a, pid_ejec);
    // me preparo para dos formas de despertarme.
    signal(SIGUSR1, tarea_pstree_A);
    signal(SIGUSR2, destruccion_A);
    crearB();
}



void tarea_pstree_A(int s) {
    pid_t pid;

    (void)s;
    pid = fork();
    if (pid == 0) {
        execlp("pstree", "pstree", (char *)NULL);
        exit(EXIT_FAILURE); // esto realmente es flexeo
    }
    // en mi codigo original aqui habria un else!
    wait(NULL); // espero a que termine mi hijo (reconvertido a comando)
    kill(pid_ejec, SIGUSR2); // le digo al proceso ejec que empiece la destruccion!!
}



void destruccion_A(int s) {
    (void)s;

    kill(pid_b, SIGUSR2);
    wait(NULL);

    printf("Soy A (%d) y muero\n", pid_a);
    exit(0);
}



void crearB(void) {
    switch (pid_b = fork()) {
        case 0:
            B();
            exit(0);

        default:
            wait(NULL);
    }
}



void B(void) {
    pid_b = getpid();

    printf("Soy el proceso B: mi pid es %d. Mi padre es %d. Mi abuelo es %d\n",
           pid_b, pid_a, pid_ejec);

    signal(SIGUSR1, tarea_pstree_B);
    signal(SIGUSR2, destruccion_B);

    crearX();
    crearY();
    crearZ();

    while (1) {
        pause();
    }
}



void tarea_pstree_B(int s) {
    pid_t pid;

    (void)s;


    pid = fork();

    if (pid == 0) {
        execlp("pstree", "pstree", (char *)NULL);
        exit(EXIT_FAILURE);
    }

    wait(NULL);
    kill(pid_ejec, SIGUSR2);
}


void destruccion_B(int s) {
    (void)s;

    kill(pid_z, SIGUSR2);
    wait(NULL);

    kill(pid_y, SIGUSR2);
    wait(NULL);

    kill(pid_x, SIGUSR2);
    wait(NULL);

    printf("Soy B (%d) y muero\n", pid_b);
    exit(0);
}


void crearX(void) {
    switch (pid_x = fork()) {
        case 0:
            X();
            exit(0);
    }
}


void X(void) {
    pid_x = getpid();

    printf("Soy el proceso X: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo es %d\n",
       pid_x, pid_b, pid_a, pid_ejec);

    signal(SIGUSR1, tarea_ls_X);
    signal(SIGUSR2, destruccion_X);

    while (1) {
        pause();
    }
}



void tarea_ls_X(int s) {
    pid_t pid;

    (void)s;

    pid = fork();

    if (pid == 0) {
        execlp("ls", "ls", (char *)NULL);
        exit(EXIT_FAILURE);
    }

    wait(NULL);
    kill(pid_ejec, SIGUSR2);
}



void destruccion_X(int s) {
    (void)s;

    printf("Soy X (%d) y muero\n", pid_x);
    exit(0);
}



void crearY(void) {
    switch (pid_y = fork()) {
        case 0:
            Y();
            exit(0);
    }
}



void Y(void) {
    pid_y = getpid();

    printf("Soy el proceso Y: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo es %d\n",
       pid_y, pid_b, pid_a, pid_ejec);

    signal(SIGUSR1, tarea_ls_Y);
    signal(SIGUSR2, destruccion_Y);

    while (1) {
        pause();
    }
}



void tarea_ls_Y(int s) {
    pid_t pid;

    (void)s;


    pid = fork();

    if (pid == 0) {
        execlp("ls", "ls", (char *)NULL);
        exit(EXIT_FAILURE);
    }

    wait(NULL);
    kill(pid_ejec, SIGUSR2);
}



void destruccion_Y(int s) {
    (void)s;

    printf("Soy Y (%d) y muero\n", pid_y);
    exit(0);
}



void crearZ(void) {
    switch (pid_z = fork()) {
        case 0:
            Z();
            exit(0);
    }
}


void Z(void) {
    pid_z = getpid();

    printf("Soy el proceso Z: mi pid es %d. Mi padre es %d. Mi abuelo es %d. Mi bisabuelo es %d\n",
            pid_z, pid_b, pid_a, pid_ejec);

    signal(SIGALRM, alarma_Z);
    signal(SIGUSR2, destruccion_Z);

    alarm(seconds);

    while (1) {
        pause();
    }
}


void alarma_Z(int s) {
    (void)s;

    switch (target_process_char) {
        case 'A':
            kill(pid_a, SIGUSR1);
            break;

        case 'B':
            kill(pid_b, SIGUSR1);
            break;

        case 'X':
            kill(pid_x, SIGUSR1);
            break;

        case 'Y':
            kill(pid_y, SIGUSR1);
            break;
    }
}

void destruccion_Z(int s) {
    (void)s;

    printf("Soy Z (%d) y muero\n", pid_z);
    exit(0);
}
