#include "mmu.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <limits.h>
TLB *tlb;
uint64_t tiempo_actual=0; // Contador de tiempo para LRU

int obtenerTamanoPagina2(char* path){
    char *pathConfig=malloc(PATH_MAX);
    pathConfig[0]='\0';
    realpath(path, pathConfig);
    strcat(pathConfig,"/memoria.config");
  // printf("Path: %s\n", pathConfig);
    t_config* config = config_create(pathConfig);
    int tamPagina = config_get_int_value(config, "TAM_PAGINA");
    config_destroy(config);
    free(pathConfig);
    return tamPagina;
}
//MMU
uint32_t mmu(uint32_t pid, uint32_t direccionLogica, int tamValor) {
    //uint32_t dirLogica = (uint32_t)strtoul(direccionLogica, NULL, 10);
    int tamPagina = obtenerTamanoPagina2("../memoria");
    uint32_t page_number = direccionLogica / tamPagina;
    uint32_t offset = direccionLogica % tamPagina;
    //log_info(logger,"PID: <%d> - DIRECCION LOGICA: <%d> - PAGINA: <%d> - OFFSET: <%d>", pid, direccionLogica, page_number, offset);
    int recibo;
    char* valorAInsertar;
    int consulta;
    uint32_t frame_number;
    int cantidadEntradasTLB = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
    if (cantidadEntradasTLB > 0)
        consulta=consultar_tlb(pid, page_number, &frame_number);
    else
        consulta=0;
    if (consulta==1) {
        // TLB Hit
        log_info(logger, "PID: <%d> - TLB HIT - Pagina: <%d>", pid, page_number);
        log_info(logger,"PID: <%d> - OBTENER MARCO - Página: <%d> - Marco: <%d>",pid,page_number,frame_number);
        return frame_number * tamPagina + offset;
    } else {
        // TLB Miss
        // Aquí deberías consultar la tabla de páginas en memoria y actualizar la TLB
        if (cantidadEntradasTLB > 0)
            log_info(logger,"PID: <%d> - TLB MISS - Pagina: <%d>", pid, page_number);
        solicitarDireccion((int) pid,(int)page_number,conexionAMemoria);
        recibo = recibirOperacion(conexionAMemoria);
        int frame;
        switch (recibo){
            case 0:
                valorAInsertar = recibirMensaje(conexionAMemoria);
                //log_info(logger,"Frame de la pagina %d es: %s\n", page_number,valorAInsertar);
                frame=atoi(valorAInsertar);
                log_info(logger,"PID: <%d> - OBTENER MARCO - Página: <%d> - Marco: <%d>",pid,page_number,frame);
                free(valorAInsertar);
                break;
            case -1:
                log_error(logger, "ERROR OPCODE");
                return EXIT_FAILURE;
                break;
        }
        // Agregar la nueva entrada a la TLB
        if (cantidadEntradasTLB > 0)
            agregar_a_tlb(pid, page_number, frame);

        return frame* tamPagina + offset;
    }
}

void solicitarDireccion(int pid, int pagina, int socket){
    t_paquete *paquete = malloc(sizeof(t_paquete));
	paquete->codigo_operacion = MMU;
	paquete->buffer = malloc(sizeof(t_buffer));

	paquete->buffer->size = 2*sizeof(int);
	paquete->buffer->stream = malloc(paquete->buffer->size);
	
	memcpy(paquete->buffer->stream, &pid, sizeof(int));
    memcpy(paquete->buffer->stream + sizeof(int), &pagina, sizeof(int));
    int bytes = sizeof(op_code) + sizeof(paquete->buffer->size) + paquete->buffer->size;
	
    void *a_enviar = serializarPaquete(paquete, bytes);

    if (send(socket, a_enviar, bytes, 0) != bytes) {
        perror("Error al enviar datos al servidor");
        exit(EXIT_FAILURE); // Manejo de error, puedes ajustarlo según tu aplicación
    }
    free(paquete->buffer->stream);
    free(paquete->buffer);
	free(a_enviar);
	free(paquete);
}
void inicializar_tlb(char* algoritmoTLB) {
    //iniciar tlb
    tlb = malloc(sizeof(TLB));
    tlb->size = config_get_int_value(config, "CANTIDAD_ENTRADAS_TLB");
    tlb->algoritmo = algoritmoTLB;
    tlb->entries = list_create();
    for(size_t i = 0; i < tlb->size; i++){
        TLBEntry *entry = malloc(sizeof(TLBEntry));
        entry->pid = -1;
        entry->page_number = -1;
        entry->frame_number = -1;
        entry->valid = false;
        entry->last_used = -1;
        entry->time_added = -1;
        list_add(tlb->entries, entry);
    }
   // log_info(logger, "TLB inicializada con algoritmo %s.", algoritmoTLB);
}

int consultar_tlb(uint32_t pid, uint32_t page_number, uint32_t *frame_number) {
    for (size_t i = 0; i < list_size(tlb->entries); i++) {
        TLBEntry *entry = list_get(tlb->entries, i);
        if (entry->valid == true && entry->pid == pid && entry->page_number == page_number) {
            // TLB Hit
            entry->last_used = tiempo_actual++;
            *frame_number = entry->frame_number;
            return 1; // Indica TLB Hit
        }
    }
    // TLB Miss
    return 0; // Indica TLB Miss
}

void agregar_a_tlb(uint32_t pid, uint32_t page_number, uint32_t frame_number) {
    // Busca una entrada inválida
    for (size_t i = 0; i < list_size(tlb->entries); i++) {
        TLBEntry *entry = list_get(tlb->entries, i);
        if (!entry->valid) {
            entry->pid = pid;
            entry->page_number = page_number;
            entry->frame_number = frame_number;
            entry->valid = true;
            entry->last_used = tiempo_actual;
            entry->time_added = tiempo_actual;
            tiempo_actual++;
            //log_info(logger, "Se agregó la entrada %ld", i);
            return;
        }
    }
    // Reemplazar una entrada existente según el algoritmo configurado
    size_t reemplazo = 0;
    if (strcmp(tlb->algoritmo, "FIFO") == 0) {
        // Reemplazo FIFO: Encuentra la entrada más antigua
        reemplazo = 0;
        for (size_t i = 1; i < list_size(tlb->entries); i++) {
            TLBEntry *entry = list_get(tlb->entries, i);
            TLBEntry *oldest = list_get(tlb->entries, reemplazo);
            if (entry->time_added < oldest->time_added) {
                reemplazo = i;
            }
        }
    } else if (strcmp(tlb->algoritmo, "LRU") == 0) {
        // Reemplazo LRU: Encuentra la entrada menos recientemente usada
        reemplazo = 0;
        for (size_t i = 1; i < list_size(tlb->entries); i++) {
            TLBEntry *entry = list_get(tlb->entries, i);
            TLBEntry *least_used = list_get(tlb->entries, reemplazo);
            if (entry->last_used < least_used->last_used) {
                reemplazo = i;
            }
        }
    }
    // Actualiza la entrada reemplazada
    TLBEntry *entry = list_get(tlb->entries, reemplazo);
    entry->pid = pid;
    entry->page_number = page_number;
    entry->frame_number = frame_number;
    entry->valid = true;
    entry->last_used = tiempo_actual;
    entry->time_added = tiempo_actual;
    tiempo_actual++;
    //log_info(logger, "Se reemplazó la entrada %ld", reemplazo);
}