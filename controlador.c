
#include "solicitud.h"
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define NUMARG 11
#define MAXHOURS 13
#define MAXAGENTS 20
#define HORAPERTURA 7
#define HORACIERRE 19
#define TIPOSRESP 3
#define ACEPTADAS 0
#define REPROGRAMADAS 1
#define NEGADAS 2

typedef struct {
  int *aforo;
  int fdSubida;
  int totalpersonas;
  int horainicio;
  int *respuestas;
} DatosHilo;

int horaactual = 0, horafinal = 0, segundos, maxpersonas, playacolumn;
solicitud **playa;


void Reporte(int aforo[MAXHOURS], int respuestas[TIPOSRESP],solicitud sol){
  int pico = 0, valle = 1000, i, horapico1, horapico2, horavalle1, horavalle2;
  for(i=0; i<MAXHOURS-1;i++){
    if(aforo[i]+aforo[i+1] > pico){
      pico = aforo[i]+aforo[i+1];
      horapico1 = i;
      horapico2 = i+1;
    }
    if(aforo[i]+aforo[i+1] < valle && aforo[i]!=0 && aforo[i+1]!=0){
      valle = aforo[i]+aforo[i+1];
      horavalle1 = i;
      horavalle2 = i+1;
    }
  }
  printf("Horas pico: %d a %d\n", horapico1+HORAPERTURA, horapico2+HORAPERTURA);
  printf("Horas con menor afluencia: %d a %d\n", horavalle1+HORAPERTURA, horavalle2+HORAPERTURA);
  printf("NÃºmero de solicitudes negadas: %d\n", respuestas[NEGADAS]);
  printf("NÃºmero de solicitudes aceptadas en su hora: %d\n", respuestas[ACEPTADAS]);
  printf("Numero de solicitudes reprogramadas: %d\n", respuestas[REPROGRAMADAS]);
  //modificacion
  printf("Numero de personas en A: %d\n",sol.numpersonasambiA);
  printf("Numero de personas en B: %d\n",sol.numpersonasambiB);

  
  

  printf("Fin simulacion\n");
}

void SalidasyEntradas(){
  int i, hora;
  solicitud sol, sol2;
  hora = horaactual - playacolumn;//es la columna de la matriz que se examinara
  for(i=0; i<maxpersonas;i++){
    sol = playa[i][hora];
    if(hora!=0){
      sol2 = playa[i][hora-1];//es el valor de la anterior hora, para verificar si ya salio la familia
      if(sol2.salida==VERDADERO)
        printf("Sale la familia %s, personas: %d\n", sol2.nombrefamilia, sol2.numpersonas);
    }    
    if(sol.salida==FALSO && horaactual!=horafinal)
      printf("Entra la familia %s, personas: %d\n", sol.nombrefamilia, sol.numpersonas);
  }
}


void Alarma(int signum) {
  horaactual++;
  printf("Hora actual: %d\n", horaactual);
  if(horaactual<horafinal){
    alarm(segundos);
  }
  SalidasyEntradas();
}


void Conexion(char nombreAgente[MAXNOMBRE]){// modificacion
  int fd, r;
  printf("ConexiÃ³n del agente: %s \n", nombreAgente);
  if ((fd = open(nombreAgente, O_WRONLY| O_NONBLOCK)) == -1) {
    perror("Conexion abriendo el pipe: ");
    exit(1);
  }
  r = write(fd, &horaactual, sizeof(int));
  close(fd);
}


void AsignarFamilia(solicitud sol, int horainicio, int maxpersonas, int pos){
  int i;
  solicitud solSalida = sol;
  solSalida.salida = VERDADERO;//la ultima hora de la famiia
  sol.salida = FALSO;//hora de llegada de la familia
  for(i = 0; i<maxpersonas;i++){//busca en la matriz donde poner la solicitud en las horas indicadas por "pos"
    if(playa[i][pos].numpersonas == FALSO &&playa[i][pos+1].numpersonas == FALSO){
      playa[i][pos] = sol;
      playa[i][pos+1] = solSalida;
      break;
    }
  }
}


int DeterminarRespuesta(solicitud sol, int aforo[MAXHOURS], int horainicio, int maxpersonas){
  int pos, i, disponible = FALSO, inicio;
  pos = sol.hora - HORAPERTURA;//posicion dentro del arreglo de aforo
  if(aforo[pos]+sol.numpersonas <= maxpersonas && aforo[pos+1]+sol.numpersonas <= maxpersonas && sol.hora>=horaactual){
    AsignarFamilia(sol, horainicio, maxpersonas, sol.hora-horainicio);
    aforo[pos]+=sol.numpersonas;
    aforo[pos+1]+=sol.numpersonas;
    return RESERVAOK;
  }
  else{
    inicio = horaactual;
    for(i = sol.hora+1; i<horafinal;i++){//busca otra hora en la cual programar la solicitud
      pos = i - HORAPERTURA;
      if(aforo[pos]+sol.numpersonas <= maxpersonas && aforo[pos+1]+sol.numpersonas <= maxpersonas&& i>=horaactual){
        AsignarFamilia(sol, horainicio, maxpersonas, i - horainicio);
        aforo[pos]+=sol.numpersonas;
        aforo[pos+1]+=sol.numpersonas;
        disponible++;
        break;
      }
    }
    if(disponible!=FALSO)
      return i;
  }
  return RESERVANON;
}


void Peticion(solicitud sol, int maxpersonas, int aforo[MAXHOURS], int horainicio, int respuestas[TIPOSRESP]){
  int fd, r, determinado;
  char res[MAXRES];
  char *mensaje;
  if(sol.hora >= horafinal || sol.numpersonas > maxpersonas){
    snprintf(res, sizeof(res), "Reserva negada, debe volver otro dÃ­a");
  }else{
    determinado = DeterminarRespuesta(sol, aforo, horainicio, maxpersonas);
    //modificacion 
    if(sol.ambiente=="A"){
      snprintf(res, sizeof(res), "numero de personas:",sol.numpersonas);
       sol.numpersonas= sol.numpersonasambiA;
    }
    else if (sol.ambiente=="B"){
       snprintf(res, sizeof(res), "numero de personas:",sol.numpersonas);   
      sol.numpersonas= sol.numpersonasambiB;
    }
    //modoficacion 
  
    if(determinado == RESERVAOK){
      snprintf(res, sizeof(res), "Reserva ok");
      respuestas[ACEPTADAS]++;
    }
    
    else if (sol.hora<horaactual && determinado>RESERVAREPROG){
      snprintf(res, sizeof(res), "Reserva negada por tarde, nueva hora: %d", determinado);
      respuestas[REPROGRAMADAS]++;
    }
    else if (determinado>RESERVAREPROG){
      snprintf(res, sizeof(res), "Reserva garantizada para otras horas, nueva hora: %d", determinado);
      respuestas[REPROGRAMADAS]++;
    }
    else{
      snprintf(res, sizeof(res), "Reserva negada, debe volver otro dÃ­a");
      respuestas[NEGADAS]++;
    }
  }
  if ((fd = open(sol.nombreagente, O_WRONLY| O_NONBLOCK)) == -1) {
    perror("Peticion abriendo el pipe: ");
    exit(1);
  }
  r = write(fd, &res, MAXRES);
  close(fd);
}


void *LeerPipe (void *parametros){//modificacion 
  DatosHilo *datos = (DatosHilo *)parametros;
  solicitud sol;
  int fd = datos->fdSubida, tipo, r;
  char nombreAgente[MAXNOMBRE];
  while (horaactual < horafinal) {
    if(read(fd, &sol, sizeof(solicitud))>0){
      if(sol.conexion == VERDADERO){
        Conexion(sol.nombreagente);
      }else{
        printf("Agente: %s, familia: %s, hora: %d, personas: %d, ambiente: %d \n", sol.nombreagente, sol.nombrefamilia, sol.hora, sol.numpersonas,sol.ambiente);//modificacion 
        Peticion(sol, datos->totalpersonas, datos->aforo, datos->horainicio, datos->respuestas);
      }
    }
  }
  return NULL;
}


void Inicializar(int horainicio, int segundoshora, int totalpersonas,
                 char *nompipe) {
  mode_t fifo_mode = S_IRUSR | S_IWUSR;
  int result, bytes, aforo[MAXHOURS], i, j, fdCont, respuestas[TIPOSRESP];
  pthread_t hilo;
  DatosHilo datosParque;

  maxpersonas = totalpersonas;
  horaactual = horainicio;
  segundos = segundoshora;
  playacolumn = horainicio;
  for(i=0; i<TIPOSRESP; i++)
    respuestas[i] = 0;
  for(i=0; i<MAXHOURS; i++)
    aforo[i] = 0;
  playa = malloc(totalpersonas * sizeof(solicitud *));
  for (i = 0; i < totalpersonas; i++){
    playa[i] = malloc((horafinal-horainicio) * sizeof(solicitud));
    for(j = 0; j<horafinal-horainicio; j++){
      playa[i][j] = IniciarSolicitud();
    }
  }
  signal(SIGALRM, Alarma);

  unlink(nompipe);
  if (mkfifo(nompipe, fifo_mode) == -1) {
    perror("mkfifo");
    exit(1);
  }
  printf("Hora actual: %d\n", horaactual);
  alarm(segundoshora);
  if ((fdCont = open(nompipe, O_RDONLY| O_NONBLOCK)) == -1) {
    perror(" Servidor abriendo el pipe: ");
    exit(1);
  }
  datosParque.aforo = aforo;
  datosParque.fdSubida = fdCont;
  datosParque.totalpersonas = totalpersonas;
  datosParque.horainicio = horainicio;
  datosParque.respuestas = respuestas;

  if (pthread_create(&hilo, NULL, LeerPipe, (void *)&datosParque) != 0) {
      perror("Error al crear el hilo");
      exit(EXIT_FAILURE);
  }
  pthread_join(hilo, NULL);
  close(fdCont);
  Reporte(aforo, respuestas);
}


int main(int argc, char *argv[]) {
  int horainicio = 0, segundoshora = 0, totalpersonas = 0, parametro = 0;
  char *nompipe = NULL;

    if (argc != NUMARG) {
      printf("Error en los parametros\n");
      printf("Uso correcto: %s â€“i horaInicio â€“f horafinal â€“s segundoshora â€“t "
             "totalpersonas â€“p pipecrecibe\n",
             argv[0]);
      return 1;
    }
    while ((parametro = getopt(argc, argv, "i:f:s:t:p:")) != -1) {
      if (parametro == 'i') {
        horainicio = atoi(optarg);
      } else if (parametro == 'f') {
        horafinal = atoi(optarg);
      } else if (parametro == 'p') {
        nompipe = optarg;
      } else if (parametro == 't') {
        totalpersonas = atoi(optarg);
      } else if (parametro == 's') {
        segundoshora = atoi(optarg);
      } else {
        printf("Error en los parametros\n");
        printf("Uso correcto: %s â€“i horaInicio â€“f horafinal â€“s segundoshora â€“t "
               "totalpersonas â€“p pipecrecibe\n",
               argv[0]);
        return 1;
      }
    }
    if (horainicio < HORAPERTURA || horainicio >= HORACIERRE || horafinal < HORAPERTURA || horafinal > HORACIERRE || horafinal <= horainicio) {
      printf("Error en los parametros\n");
      printf("La horaInicio y horaFinal deben estar entre los valores 7 y 19, "
             "ademÃ¡s horaFinal debe ser mayor a horaInicio\n");
      return 1;
    }
  Inicializar(horainicio, segundoshora,totalpersonas, nompipe);
  return 0;
}