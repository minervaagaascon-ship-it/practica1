#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>





void vacio(int aviso){
}










/* Lo ejecuta cada hijo de primer nivel: construye su rama
   y el proceso del fondo espera 10 s. No retorna nunca. */
void rama(int x){
	int j, pid_hijo;

	for(j = 1; j <= x - 1; j++){
		pid_hijo = fork();
		if(pid_hijo != 0){
			wait(NULL);
			exit(0);
		}
	}
	signal(SIGALRM, vacio);
	alarm(10);
	pause();
	exit(0);
}

/* Crea las y columnas: cada hijo construye su propia rama */
void colum(int x, int y){
	int i, pid_hijo;

	for(i = 1; i <= y; i++){
		pid_hijo = fork();
		if(pid_hijo == 0){
			rama(x);
		}
	}
}

/* El proceso original espera a sus y hijos */
void esperar(int y){
	int i;

	for(i = 1; i <= y; i++){
		wait(NULL);
	}
}

int main(int argc, char *argv[]){
	int x, y;

	if(argc == 3){
		x = atoi(argv[1]);
		y = atoi(argv[2]);
		if(x > 0 && y > 0){
			colum(x, y);
			esperar(y);
		}
	}

	return 0;
}
