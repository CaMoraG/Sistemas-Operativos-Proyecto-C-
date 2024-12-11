/*
Archivo: solicitud.c
Realizado por: Camilo Mora y Sergio Herrera
Contiene: estructura usada en el proyecto ademÃ¡s de algunos macros usados por tanto el controlador como el agente
Fecha Ãºltima modificaciÃ³n: 18/11/2023
*/
#define MAXRES 80
#define MAXNOMBRE 40
#define VERDADERO 1
#define FALSO 0
#define RESERVAOK 1
#define RESERVAREPROG 2
#define RESERVANON 0
#define VACIO 2

typedef struct solicitud {
  char nombrefamilia[MAXNOMBRE];
  int hora;
  int numpersonas;
  char nombreagente[MAXNOMBRE];
  int salida;
  int conexion;
  char ambiente;//modificacion 
  int numpersonasambiA;//modificacion 
  int numpersonasambiB;//modificacion 
} solicitud;

solicitud IniciarSolicitud() {
    solicitud nuevoStruct;
    nuevoStruct.numpersonas = FALSO;
    nuevoStruct.salida = VACIO;
    return nuevoStruct;
}