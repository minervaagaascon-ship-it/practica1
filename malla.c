#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

void nada();
void crear_malla(int x, int y);
void crear_cadena(int x);
void proceso_final();
void esperar_cadenas(int y);

void nada(){
}

int main(int argc, char *argv[]){
	int x, y;
	
	if(argc == 3){
		x = atoi(argv[1]);
		y = atoi(argv[2]);
		if(x > 0 && y > 0){
			crear_malla(x, y);
		}
	}

	return 0;
}



void crear_malla(int x, int y){
	int i, pid;

	for(i = 1; i <= y; i++){
		pid = fork();
		if(pid == 0){
			crear_cadena(x);
		}
	}
	if(i == y + 1){
		esperar_cadenas(y);
	}
}



void crear_cadena(int x){
	int j, pid;

	for(j = 1; j <= x - 1; j++){
		pid = fork();
		if(pid !=  0){
			wait(NULL);
			exit(0);
		}
	}
	if(j == x){
		proceso_final();
	}
}



void proceso_final(){
	signal(SIGALRM, nada);
	alarm(10);
	pause();
	exit(0);
}



void esperar_cadenas(int y){
	int i;

	for(i = 1; i <= y; i++){
		wait(NULL);
	}
}
