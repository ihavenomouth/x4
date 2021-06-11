
#ifndef _CONFIGURACION_H_   /* Include guard */
#define _CONFIGURACION_H_


//To look for fonts, you can use the command xlsfont or alternatively xfontsel -scaled
//Para buscar las fuentes disponibles en el sistema puedes usar xlsfont o xfontsel -scaled
typedef struct Config{
   int nivelesDeshacer;     //undo levels
   int tamFlecha;           //arrow size
   int anchoBorde;          //border size
   char  fuente[1024];      //font
} config; 

void leerCofiguracion(config *c);

#endif