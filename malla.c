#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

void vacio(int senal){
}

int main(int argc, char *argv[]){
	int x, y, i, j, pid_hijo;

	if(argc == 3){
		x = atoi(argv[1]);
		y = atoi(argv[2]);
		if(x > 0 && y > 0){
			for(i = 1; i <= y; i++){
				pid_hijo = fork();
				if(pid_hijo == 0){
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
			}
			for(i = 1; i <= y; i++){
				wait(NULL);
			}
		}
	}

	return 0;
}
