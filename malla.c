#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

void nada(){
}

int main(int argc, char *argv[]){
	int x, y, i, j, pid;
	
	if(argc == 3){
		x = atoi(argv[1]);
		y = atoi(argv[2]);
		if(x > 0 && y > 0){
			for(i = 1; i <= y; i++){
				pid = fork();
				if(pid == 0){
					for(j = 1; j <= x - 1; j++){
						pid = fork();
						if(pid !=  0){
							wait(NULL);
							exit(0);
						}
					}
					if(j == x){
						signal(SIGALRM, nada);
						alarm(10);
						pause();
						exit(0);
					}
				}
			}
			if(i == y + 1){
				for(i = 1; i <= y; i++){
					wait(NULL);
				}
			}
		}
	}

	return 0;
}


