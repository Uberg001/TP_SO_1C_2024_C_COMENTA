#ifndef CONFIGURACIONES_H
#define CONFIGURACIONES_H

    #include <stdlib.h>
    #include <stdarg.h>
    #include <string.h>
    #include <unistd.h>
    #include <sys/socket.h>
    #include <commons/log.h>
    #include <commons/config.h>
    #include <commons/string.h>

    #define confGet(key) config_get_string_value(config, key)
    #define confGetInt(key) config_get_int_value(config, key)

    extern t_log *logger;
    extern t_log *loggerError;
    extern t_config *config;

    extern bool imprimirLogs;
    extern t_log_level nivelDeLog;

    
    t_config* iniciarConfiguracion(char* ruta);
    t_log* iniciarLogger(char* nombreArchivo, char* nombreLogger);
    t_log *cambiarNombre(t_log* logger, char *nuevoNombre);

    char * duplicarNombre(t_log *logger);
    void terminarPrograma();
    void debug (char * mensajeFormato, ...);
    void error (char *mensajeFormato, ...); 

#endif /* CONFIGURACIONES_H_ */