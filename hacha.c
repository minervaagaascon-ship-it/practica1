#include <stdio.h> 
#include <sys/wait.h>
#include <sys/stat.h> 
#include <unistd.h> 
#include <stdlib.h> 
#include <string.h>
#include <fcntl.h>


void partir(char *nombre, int tamanyo);
int calcular_num_trozos(char *nombre, int tamanyo);
void crear_nombre_trozo(char *nombreFichero, char *nombre, int i);
void codigo_padre(int fEntrada, int tubo[2], char *trozoLeido, int tamanyo);
void codigo_hijo(int tubo[2], char *trozoLeido, int tamanyo, char *nombre, int i);
void esperar_hijos(int numTrozos);

int main(int argc, char *argv[]){
	if(argc != 3){	
		printf("Error. Uso: %s fichero tam_trozo\n", argv[0]);
	}
	else{
		partir(argv[1], atoi(argv[2]));
	}
	return 0;
}



void partir(char nombre[], int tamanyo){
	int i;	
	int tubo[2];
	int fEntrada; 
	int numTrozos;
 	char *trozoLeido;		
		
	trozoLeido = (char *) malloc(sizeof(char) * tamanyo);
	fEntrada = open(nombre, O_RDONLY);
	if(fEntrada < 0){
		printf("Error. No existe el fichero\n");
	}
	else{
		numTrozos = calcular_num_trozos(nombre, tamanyo);
		for(i = 0; i < numTrozos; i++){
			pipe(tubo);
			if(fork() != 0){
				codigo_padre(fEntrada, tubo, trozoLeido, tamanyo);
			}
			else{ 
				codigo_hijo(tubo, trozoLeido, tamanyo, nombre, i);
				break;
			}
		}
		if(i == numTrozos){
			free(trozoLeido);
			esperar_hijos(numTrozos);
		}
	}
}



int calcular_num_trozos(char *nombre, int tamanyo){
	struct stat propFichero;
	int numTrozos;

	stat(nombre, &propFichero);
	numTrozos = propFichero.st_size/tamanyo;
	if(propFichero.st_size % tamanyo != 0){
		numTrozos++;
	}
	return numTrozos;
}



void crear_nombre_trozo(char *nombreFichero, char *nombre, int i){
	if(i < 10){
		sprintf(nombreFichero, "%s.h0%d", nombre, i);
	}
	else{
		sprintf(nombreFichero, "%s.h%d", nombre, i);
	}
}



void codigo_padre(int fEntrada, int tubo[2], char *trozoLeido, int tamanyo){
	int numLeidos;

	numLeidos = read(fEntrada, trozoLeido, tamanyo);
	write(tubo[1], trozoLeido, numLeidos);
}



void codigo_hijo(int tubo[2], char *trozoLeido, int tamanyo, char *nombre, int i){
	char nombreFichero[50];
	int fSalida;
	int numLeidos;

	crear_nombre_trozo(nombreFichero, nombre, i);
	fSalida = creat(nombreFichero, 0666);	
	numLeidos = read(tubo[0], trozoLeido, tamanyo);
	write(fSalida, trozoLeido, numLeidos);
	close(fSalida);
	free(trozoLeido);
}



void esperar_hijos(int numTrozos){
	for(int i = 0; i < numTrozos; i++){
		wait(NULL);
	}
}
