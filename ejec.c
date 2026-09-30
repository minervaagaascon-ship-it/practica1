#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>


int main(){
	pid_t pidarb, pida, pidb, pidx, pidy, pidz, pid;
	int i;
	
	pidarb = getpid();
	printf("Soy el proceso ejec: mi pid es %d\n", pidarb);
	pid = fork();
	if(pid != 0){
		// Arb
		wait(NULL);
		printf("Soy ejec (%d) y muero\n", getpid());
	}
	else{
		// A
		pida = getpid();
		printf("Soy el proceso A: mi pid es %d. Mi padre es %d\n", getpid(), pidarb);
		pid = fork();
		if(pid != 0){
			wait(NULL);
			printf("Soy A (%d) y muero\n", getpid());
		}
		else{
			// B
			pidb = getpid();
			printf("Soy el proceso B. mi pid es %d. Mi padre es %d. Mi abuelo %d\n", getpid(), pida, pidarb);
			for(i = 1; i <= 3; i++){
				pid = fork();
				if(pid != 0){
					switch(i){
						case 1:
							pidx = pid;
						break;
						case 2:
							pidy = pid;
						break;
						case 3:
							pidz = pid;
						break;
					}
				}
				else{
					switch(i){
						case 1:
							printf("Soy el proceso X. mi pid es %d, Mi padre es %d. Mi abuela es %d. Mis bis %d\n", getpid(),
							pidb, pida, pidarb);
							sleep(5);
							printf("Soy X (%d)\n", getpid());
						break;
						case 2:
							printf("Soy el proceso Y. mi pid es %d, Mi padre es %d. Mi abuela es %d. Mis bis %d\n", getpid(),
							pidb, pida, pidarb);
							sleep(5);
							printf("Soy Y (%d)\n", getpid());
						break;
						case 3:
							printf("Soy el proceso Z. mi pid es %d, Mi padre es %d. Mi abuela es %d. Mis bis %d\n", getpid(),
							pidb, pida, pidarb);
							sleep(5);
							printf("Soy Z (%d)\n", getpid());
						break;
					}
					break;
				}			
			}
			if(i == 4){
				// B
				for(int i = 1; i <= 3; i++){
					wait(NULL);
				}
				printf("Soy B (%d) y muero\n", getpid());
			}
		}
	}
	

	return 0;
}

