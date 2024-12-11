
#include "solicitud.h"
#include <fcntl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>

#define NUMARG 7


void EnviarInformacion(char *nombre, char *archivo, char *nompipe, int fdSubida, FILE *arch, int fdBajada){
  int r, conexion = 1, envio = 0, horaactual, horaleida = FALSO, respuestaleida;
  char respuesta[MAXRES], nombreagente[MAXNOMBRE];
  solicitud sol, con;
  con.conexion = 1;
  strcpy(con.nombreagente, nombre);
  strcpy(nombreagente, nombre);
  printf("ConexiÃ³n del agente: %s \n", nombreagente);
  r = write(fdSubida, &con, sizeof(solicitud));
  while(horaleida==FALSO){
    if(read(fdBajada, &horaactual, sizeof(int))>0)
      horaleida++;
  }
  printf("Hora actual: %d\n", horaactual);
  while (fscanf(arch, "%[^,],%d,%d\n", sol.nombrefamilia, &sol.hora, &sol.numpersonas,&sol.ambiente) == 3) {//modificacion 
    respuestaleida = FALSO;
    printf("PeticiÃ³n: \n");
    printf("Nombre: %s, Hora: %d, Personas: %d, ambiente: %d \n", sol.nombrefamilia, sol.hora, sol.numpersonas,sol.ambiente);//modificacion 
    printf("Respuesta: \n");
    if(sol.hora<horaactual){
      printf("PeticiÃ³n por debajo de la hora actual \n");
    }else{
      strcpy(sol.nombreagente, nombre);
      sol.conexion = 0;
      r = write(fdSubida, &sol, sizeof(solicitud));
      while(respuestaleida==FALSO){
        if(read(fdBajada, &respuesta, MAXRES)>0){
          respuestaleida++;
          printf("%s \n", respuesta);
        }
      }
    }
    sleep(2);
  }
}


void Inicializar(char *nombre, char *archivo, char *nompipe) {//modificacion 
  int fd, creado = 0, fdBajada;
  mode_t fifo_mode = S_IRUSR | S_IWUSR;
  FILE *arch;

  do {
    fd = open(nompipe, O_WRONLY);
    if (fd == -1) {
      perror("pipe");
      printf(" Se volvera a intentar despues\n");
      sleep(5);
    } else
      creado = 1;
  } while (creado == 0);

  arch = fopen(archivo, "r");
  if (arch == NULL) {
    perror("Error al abrir el archivo");
    exit(1);
  }
  unlink(nombre);
  if (mkfifo(nombre, fifo_mode) == -1) {
    perror("mkfifo");
    exit(1);
  }
  if ((fdBajada = open(nombre, O_RDONLY| O_NONBLOCK)) == -1) {
    perror("Agente abriendo el pipe: ");
    exit(1);
  }
  EnviarInformacion(nombre, archivo, nompipe, fd, arch, fdBajada);

  fclose(arch);
  close(fdBajada);
  printf("Agente %s termina\n", nombre);
}


int main(int argc, char *argv[]) {
   int parametro;
   char *nompipe = NULL, *nombre, *archivo;

   if (argc != NUMARG) {
     printf("Error en los parametros\n");
     printf("Uso correcto: %s â€“s nombre â€“a archivosolicitudes â€“p pipecrecibe\n",
            argv[0]);
     return 1;
   }
   while ((parametro = getopt(argc, argv, "s:a:p:")) != -1) {
     if (parametro == 's') {
       nombre = optarg;
     } else if (parametro == 'a') {
       archivo = optarg;
     } else if (parametro == 'p') {
       nompipe = optarg;
     } else {
       printf("Error en los parametros\n");
       printf("Uso correcto: %s â€“s nombre â€“a archivosolicitudes â€“p pipecrecibe\n", argv[0]);
       return 1;
     }
   }

  Inicializar(nombre, archivo, nompipe);
  return 0;
}