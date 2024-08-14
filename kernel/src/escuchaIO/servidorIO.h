#ifndef CPU_KERN_SER_H
#define CPU_KERN_SER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <commons/log.h>
#include <commons/config.h>

#include <./main/kernel.h>
#include <utilsServidor/utilsServidor.h>
#include <utilsCliente/utilsCliente.h>
#include <contextoEjecucion/contextoEjecucion.h>
#include <commons/collections/queue.h>
extern t_log* loggerError; 
typedef struct {
    char *nombre_interfaz;
    char *tipo_interfaz;
    int socket_interfaz;
     t_queue *cola_procesos_io;
    sem_t semaforo_cola_procesos;
     sem_t liberacion_semaforo;
    pthread_mutex_t mutex_cola_procesos;
    char* parametro1;
    char *parametro2;
    char* parametro3;
    char* parametro4;
    int funcion;
} Interfaz;

// Estructura para gestionar el arreglo dinÃ¡mico de interfaces
typedef struct {
    t_list *interfaces;

} Kernel_io;
extern Interfaz* io_global;
extern t_list * lista_global_io;
void escucharAlIO(); 
void destruirStructsIO(Kernel_io *kernel);
void inicializarStructsIO(Kernel_io *kernel);
int existeLaInterfaz(char *nombreInterfaz, Kernel_io *kernel);
int verificarConexionInterfaz(Kernel_io *kernel, const char *nombre_interfaz);
int obtener_socket(const Kernel_io *kernel, const char *nombre_interfaz);
int validarTipoInterfaz(const Kernel_io *kernel, char *nombreInterfaz, char *tipoRequerido);
Interfaz *obtener_interfaz(const Kernel_io *kernel, const char *nombre_interfaz);
extern t_contexto* contextoEjecucion;
void ejecutar_io(Interfaz *interfaz, t_pcb *proceso);
void ejecutar_io_stdin_read(Interfaz *interfaz, t_pcb *proceso);
void ejecutar_io_fs_truncate(Interfaz *interfaz, t_pcb *proceso);
void ejecutar_io_fs_write(Interfaz *interfaz, t_pcb *proceso);
void ejecutar_io_fs_read(Interfaz *interfaz, t_pcb *proceso);
void ejecutar_io_stdout_write(Interfaz *interfaz, t_pcb *proceso);
void ejecutar_io_fs_create(Interfaz *interfaz, t_pcb *proceso);
void ejecutar_io_fs_delete(Interfaz *interfaz, t_pcb *proceso);
extern Kernel_io kernel;
#endif