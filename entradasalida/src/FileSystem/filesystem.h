#ifndef FS_MAIN_H_
#define FS_MAIN_H_

#include <stdio.h>
#include <stdlib.h>

#include <commons/log.h>
#include <commons/config.h>
#include <pthread.h>

#include <conexionKernel/conexionKernel.h>
#include <conexionMemoria/conexionMemoria.h>

#include <global.h>
#include <utilsServidor/utilsServidor.h>
#include <utilsCliente/utilsCliente.h>
#include <configuraciones/configuraciones.h>

#include <../include/iniciar_io.h>

#include <../include/io_kernel.h>
//FS
#include <commons/bitarray.h>
#include <fcntl.h>
//#include <unistd.h>
#include <sys/stat.h>
//#include <sys/types.h>
//#include <errno.h>
#include <sys/mman.h>
//#include <string.h>
#include <dirent.h>

extern t_log* loggerError;

extern int fd_kernel;
extern int fd_memoria;

extern char* TIPO_INTERFAZ;
extern int TIEMPO_UNIDAD_TRABAJO;
extern char* IP_KERNEL;
extern char* PUERTO_KERNEL;
extern char* IP_MEMORIA;
extern char* PUERTO_MEMORIA;
extern char* PATH_BASE_DIALFS;
extern int BLOCK_SIZE;
extern int BLOCK_COUNT;
extern int RETRASO_COMPACTACION;
//fs
void delete_file(const char *filename);
void crearArchivo2(char* nombre);
void escribir_metadata(char *nombre, int bloqueInicial, int tamanoArchivo); 
int obtenerPrimeraPosicionLibre(t_bitarray *bitmap);
void create_bitmap_file(const char *filename, size_t size);
void truncarArchivo2(char* nombre, int tamanio, int pid);
void create_bloques_file(const char *filename, size_t size);
char *obtenerNombreSinExtension(const char *nombreArchivo);
void crearMetadata(char *nombre, int bloqueInicial);
void borrarContenidoArchivo(const char *nombreArchivo, size_t cantidadBytes);
void escribirCadenaEnArchivo(const char *nombreArchivo, const char *cadena, off_t registroPunteroArchivo);
char* leerDatosDesdeArchivo(const char *nombreArchivo, off_t registroPunteroArchivo, size_t registroTamanio);

#endif