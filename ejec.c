#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>


/* ================================================================
 * Variables globales
 * ================================================================ */

pid_t g_pid_ejec;
pid_t g_pid_a;
pid_t g_pid_b;
pid_t g_pid_x;
pid_t g_pid_y;
pid_t g_pid_z;

char g_target_process_char;
int g_seconds;


/* ================================================================
 * Prototipos
 * ================================================================ */

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


/* ================================================================
 * Funciones globales y de utilidad
 * ================================================================ */

/*
 * Comprueba que se reciben exactamente dos argumentos y guarda tanto
 * el proceso objetivo como el numero de segundos en variables globales.
 */
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


/* ================================================================
 * Proceso ejec (super-padre)
 * ================================================================ */

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


/*
 * ejec recibe SIGUSR2 cuando el proceso objetivo ha terminado de ejecutar
 * su comando. La destruccion se propaga comenzando por A.
 */
void handler_start_destruction(int s) {
    (void)s;
    kill(g_pid_a, SIGUSR2);
}


/* ================================================================
 * Proceso A
 * ================================================================ */

/*
 * Crea A. El proceso ejec queda esperando hasta que A haya terminado,
 * garantizando que el padre no muera antes que su hijo.
 */
void create_A_process(void) {	// este codigo lo hace ejec
    switch (g_pid_a = fork()) {
        case 0:
            run_process_A();
            exit(0);

        default:
            wait(NULL);	// aqui espera ejec
    }
}


/*
 * Inicializacion y vida de A. A instala los manejadores de ejecucion y
 * destruccion y continua la construccion del arbol creando B.
 */
void run_process_A(void) {
    g_pid_a = getpid();
    printf("Soy el proceso A: mi pid es %d. Mi padre es %d\n", g_pid_a, g_pid_ejec);
    // me preparo para dos formas de despertarme.
    signal(SIGUSR1, handler_A_exec_task);
    signal(SIGUSR2, handler_A_destroy_and_propagate);
    create_B_process();
}


/*
 * SIGUSR1 en A: crea un hijo temporal para ejecutar pstree.
 * A permanece intacto. Al terminar el comando se notifica a ejec mediante
 * SIGUSR2 para iniciar la fase de destruccion.
 */
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


/*
 * A propaga SIGUSR2 a B y no termina hasta que B haya finalizado.
 */
void handler_A_destroy_and_propagate(int s) {
    (void)s;

    kill(g_pid_b, SIGUSR2);
    wait(NULL);

    printf("Soy A (%d) y muero\n", g_pid_a);
    exit(0);
}


/* ================================================================
 * Proceso B
 * ================================================================ */

/*
 * Crea B. A espera a que B termine, igual que ejec espera a A.
 */
void create_B_process(void) {
    switch (g_pid_b = fork()) {
        case 0:
            run_process_B();
            exit(0);

        default:
            wait(NULL);
    }
}


/*
 * B instala sus manejadores y crea sus tres hijos X, Y y Z.
 * Despues queda suspendido esperando senales.
 */
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


/*
 * SIGUSR1 en B: ejecuta pstree mediante un hijo temporal.
 */
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


/*
 * B es padre de X, Y y Z. Para reproducir el orden de destruccion mostrado
 * en el enunciado, destruye y espera primero a Z, despues a Y y por ultimo
 * a X. Solo entonces termina B.
 */
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


/* ================================================================
 * Proceso X
 * ================================================================ */

/*
 * B crea X y continua inmediatamente para poder crear tambien Y y Z.
 * No se hace wait() aqui porque los tres hijos deben coexistir.
 */
void create_X_process(void) {
    switch (g_pid_x = fork()) {
        case 0:
            run_process_X();
            exit(0);
    }
}


/*
 * X queda a la espera de SIGUSR1 (ejecutar ls) o SIGUSR2 (terminar).
 */
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


/*
 * SIGUSR1 en X: ejecuta ls en un hijo temporal y, al finalizar,
 * notifica a ejec con SIGUSR2.
 */
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


/* X es una hoja: al recibir SIGUSR2 simplemente termina. */
void handler_X_destroy_leaf(int s) {
    (void)s;

    printf("Soy X (%d) y muero\n", g_pid_x);
    exit(0);
}


/* ================================================================
 * Proceso Y
 * ================================================================ */

/* B crea Y y continua inmediatamente para crear Z. */
void create_Y_process(void) {
    switch (g_pid_y = fork()) {
        case 0:
            run_process_Y();
            exit(0);
    }
}


/*
 * Y queda a la espera de SIGUSR1 (ejecutar ls) o SIGUSR2 (terminar).
 */
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


/* SIGUSR1 en Y: ejecuta ls mediante un hijo temporal. */
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


/* Y es una hoja: al recibir SIGUSR2 simplemente termina. */
void handler_Y_destroy_leaf(int s) {
    (void)s;

    printf("Soy Y (%d) y muero\n", g_pid_y);
    exit(0);
}


/* ================================================================
 * Proceso Z
 * ================================================================ */

/*
 * Z se crea despues de X e Y. De esta forma hereda de B los PID ya
 * almacenados de X e Y, ademas de los PID de A y B.
 */
void create_Z_process(void) {
    switch (g_pid_z = fork()) {
        case 0:
            run_process_Z();
            exit(0);
    }
}


/*
 * Z implementa la temporizacion sin sleep(). Registra SIGALRM, programa
 * alarm(g_seconds) y se suspende con pause() hasta recibir senales.
 */
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


/*
 * Cuando vence la alarma, Z envia SIGUSR1 al proceso indicado por el
 * primer argumento del programa.
 */
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


/* Z es una hoja: al recibir SIGUSR2 simplemente termina. */
void handler_Z_destroy_leaf(int s) {
    (void)s;

    printf("Soy Z (%d) y muero\n", g_pid_z);
    exit(0);
}
