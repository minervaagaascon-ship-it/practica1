#include <stdio.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>

void despierta(){}

/*
	./hijos 4 3
	
	hijos 
	|                     0   1   2   3
	x1		vx = [x1, x2, x3, x4]
	|                     0   1   2
	x2		vy = [y1, y2, y3]
	|
	x3
	|
	x4
      /  | \
    y1  y2  y3	=> sleep(10)
	
	
*/
int main(int argc, char *argv[]){
	int i, x, y, shmidx, shmidy;
	int *vx, *vy; // guardo la direccion del primer elemento del vector de ids x e y.
	pid_t pid, pid_hijos;
	
	if(argc != 3){
		printf("error en argumentos\n");
	}
	else{
		pid_hijos = getpid();
		x = atoi(argv[1]);
		y = atoi(argv[2]);
		
		// vector para almacenar los identificadores de los procesos x.
		//	sizeof(int) * x  =>  x enteros donde almaceno los pids de los x procesos verticales.  	
		shmidx = shmget(IPC_PRIVATE, sizeof(int) * x, IPC_CREAT | 0666);
		vx = (int *) shmat (shmidx, 0, 0);
		
		// vector para almacenar los identificadores de los procesos y.
		shmidy = shmget(IPC_PRIVATE, sizeof(int) * y, IPC_CREAT | 0666);
		vy = (int *) shmat (shmidy, 0, 0);

		// Creamos la vertical 
		for(i = 1; i <= x; i++){
			pid = fork();
			//signal(SIGUSR2, despierta);
			if(pid != 0){
				
				// pause();
				// kill(vx[i], SIGUSR2);
				wait(NULL); // el padre de todos sale de aqui con i = 1
				break;
			}
			else{
				// el nuevo proceso deja su pid en el vector de pids.
				vx[i - 1] = getpid();	// el primer hijo (x1) lo almacena en v[1 - 1] = getpid()
				// El nuevo proceso muestra el vector hasta la posicion en la que estoy.
				printf("Soy el proceso %d. Mis padres son: ", getpid());
				printf("%d", pid_hijos); // Dios todo poderoso!	
				// Muestra los identificadores de todos los hijos creados anteriormente.
				
				// si voy por la i = 3, tengo que mostrar los pids de sus padres que estan en las posiciones 0 y 1
				// Porque el 3 lo almacen en la 2.
				for(int j = 0; j < i - 1; j++){
					printf(", %d", vx[j]);
				}
				printf("\n");	
			}
		}
		if(i == 1){ // es el super padre
			printf("Soy el super padre %d, mis hijos finales son: ", getpid());
			for(i = 0; i < y; i++){
				printf("%d" ,vy[i]);
				if(i != y - 1){
					printf(", ");
				}
			}
			printf("\n");
			// FALTA AQUI LIBERAR LA MEMORIA COMPARTIDA!!
		}
		else{
			// el ultimo hijo vertical, es el que tiene hijos horizontales.
			if(i == x + 1){
				for(i = 1; i <= y; i++){
					pid = fork();
					if(pid == 0){
						vy[i-1] = getpid();
						// sleep(10);
						signal(SIGALRM, despierta);
						alarm(10);
						pause();
						break;
					}
				}
				if(i == y + 1){	// esto lo ejecuta el que rompe el bucle de forma natural, todos lo hijos se han salido con break.
					for(i = 1; i <= y; i++){
						wait(NULL);
					}
				}
			}
		}
	}
	return 0;
}


