#include "configuracion.h"
#include <stdio.h>
#include <string.h>

void leerCofiguracion(config *c){
    FILE *f;
    f = fopen("x4.conf", "r" );

    //valores por defecto
    c->anchoBorde      = 5;
    c->nivelesDeshacer = 10;
    c->tamFlecha       = 20;
    strcpy(c->fuente,"-*-*-*-*-*-*-60-*-*-*-*-*-iso8859-*");


    if (f == NULL){
        fprintf(stderr,"Fichero config.conf no encontrado, se usarán valores por defecto\n");
        return;
    }

    char buffer[256];
    char fuente[1024];
    int valor = 0;
    while (fscanf(f, " %255s", buffer) == 1) {
        if (strcmp(buffer, "nivelesDeshacer") == 0) {
            fscanf(f, "%d", &valor);
            // printf("Valor leído: %d\n", valor);
            c->nivelesDeshacer = valor;
        }
        else if (strcmp(buffer, "anchoBorde") == 0) {
            fscanf(f, "%d", &valor);
            c->anchoBorde = valor;
        }
        else if (strcmp(buffer, "tamFlecha") == 0) {
            fscanf(f, "%d", &valor);
            c->tamFlecha = valor;
        }
        else if (strcmp(buffer, "fuente") == 0) {
            fscanf(f, "%1023s", fuente);
            strcpy(c->fuente, fuente);
        }

    }

    fclose(f);
}