#ifndef GLOBAL_H_
#define GLOBAL_H_

    #include <signal.h>
    #include <commons/log.h>
    #include <commons/config.h>

    typedef enum {
        SET, MOV_IN, MOV_OUT, SUM, SUB,
        JNZ, RESIZE, COPY_STRING, WAIT, SIGNAL,
        IO_GEN_SLEEP, IO_STDIN_READ, IO_STDOUT_WRITE, IO_FS_CREATE,
        IO_FS_DELETE, IO_FS_TRUNCATE, IO_FS_WRITE, IO_FS_READ,
        EXIT, FIN_DE_QUANTUM, USER_INTERRUPTION, VOID,
    } t_comando; 
    typedef struct {
        t_comando comando; 
        char* parametros[5]; 
    }t_instruccion; 
    
#endif
