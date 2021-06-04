#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXPOS 2
#define MAX_NIVELES_DESHACER 5 //TODO: leer este valor de un fichero de configuración


/**
 * Dibuja una flecha en la pantalla
 * x0, y0 : punto que marca la punta de la flecha
 * x1, y1 : punto que marca el final de la flecha
 */
void dibujarFlecha(Display* d, Window w, GC gc, int x0, int y0, int x1, int y1) {
  XDrawLine(d, w, gc, x0, y0, x1, y1);

  int tamFlecha = 15;  //TODO: leer este valor de un fichero de configuración
  float angulo = M_PI / 4;
  angulo = atan2(y0 - y1, x0 - x1) + M_PI;
  float anguloFlecha = M_PI / 4;

  int f_x1 = x0 + tamFlecha * cos(angulo + anguloFlecha);
  int f_y1 = y0 + tamFlecha * sin(angulo + anguloFlecha);
  int f_x2 = x0 + tamFlecha * cos(angulo - anguloFlecha);
  int f_y2 = y0 + tamFlecha * sin(angulo - anguloFlecha);
  XDrawLine(d, w, gc, x0, y0, f_x1, f_y1);
  XDrawLine(d, w, gc, x0, y0, f_x2, f_y2);
}

/**
 * dibuja un rectángulo en la pantalla
 * x0, y0 : punto que marca una esquina del rectángulo
 * x1, y1 : punto que marca la esquina opuesta del rectángulo
 * */
void dibujarRectangulo(Display* d, Window w, GC gc, int x0, int y0, int x1, int y1) {
  int alto = abs(y1 - y0);
  int ancho = abs(x1 - x0);

  if (x0 <= x1) {
    if (y0 <= y1) {
      XDrawRectangle(d, w, gc, x0, y0, ancho, alto);
    }
    else {
      XDrawRectangle(d, w, gc, x0, y1, ancho, alto);
    }
  }
  else {
    if (y0 <= y1) {
      XDrawRectangle(d, w, gc, x1, y0, ancho, alto);
    }
    else {
      XDrawRectangle(d, w, gc, x1, y1, ancho, alto);
    }
  }
}


/**
 * Inicializa los niveles de deshacer que contendrán "capturas de pantalla" del estado de la misma
 */
void inicializaNivelesDeshacer(Pixmap* fondo, Display* d, Window w, int screen, unsigned int ancho, unsigned int alto) {
  for (int i = 0;i < MAX_NIVELES_DESHACER;i++) {
    fondo[i] = XCreatePixmap(d, w, ancho, alto, XDefaultDepth(d, screen));
  }
}



/**
 * rota el array de fondos, de manera que el fondo que estuviera en la pos 0 pasa a la 1, el 1 a la 2, etc...
 * el 0 es el nivel más actual, el 1 el anterior, etc.
 */
void rotaNivelDeshacerAlaDer(Pixmap* fondo, Display* d, GC gc, unsigned int ancho, unsigned int alto) {
  for (int i = MAX_NIVELES_DESHACER - 1;i > 0;i--) {
    XCopyArea(d, fondo[i - 1], fondo[i], gc, 0, 0, ancho, alto, 0, 0); //el 0 es el fondo actual
  }
}

/** 
 * rota el array de fondos, de manera inversa a la anterior
 */
void rotaNivelDeshacerAlaIzq(Pixmap* fondo, Display* d, GC gc, unsigned int ancho, unsigned int alto) {
  for (int i = 0;i < MAX_NIVELES_DESHACER - 1;i++) {
    XCopyArea(d, fondo[i + 1], fondo[i], gc, 0, 0, ancho, alto, 0, 0);
  }
}


////////////////////////
// MAIN
////////////////////////
int main() {
  Display* d;
  Window w;
  XEvent e;
  GC gc;
  int screen;
  int anchoBorde = 5;     //TODO: leer este valor de un fichero de configuración

  XPoint puntos[MAXPOS];  // 2 puntos para dibujar un rectángulo
  int p = 0;              // contador de los puntos

  GC gcPreDraw;           // Para predibujar la forma antes de pintarla
  XPoint pointPreDraw;    // valores incorrectos iniciales, para no pintarlo sin tener el primer punto elegido
  pointPreDraw.x = -1;
  pointPreDraw.y = -1;

  char herramienta = 'c';
  long color = 0xFF3333;  //TODO: leer los valores de los colores de un fichero de configuración

  d = XOpenDisplay(NULL);
  screen = XDefaultScreen(d);
  unsigned int alto = DisplayHeight(d, screen); //alto y ancho de la pantalla
  unsigned int ancho = DisplayWidth(d, screen);


  w = XCreateWindow(d, DefaultRootWindow(d), 200, 100, 400, 300, CopyFromParent,
    CopyFromParent, CopyFromParent, CopyFromParent,
    CopyFromParent, CopyFromParent);

  // Cambiamos a pantalla completa
  Atom atoms[2] = { XInternAtom(d, "_NET_WM_STATE_FULLSCREEN", False), None };
  XChangeProperty(d, w, XInternAtom(d, "_NET_WM_STATE", False), XA_ATOM, 32,
    PropModeReplace, (unsigned char*)atoms, 1);

  // Creamos los GC (Contexto gráfico) que usaremos junto a sus atributos
  gc = XCreateGC(d, w, 0, NULL);

  XSetForeground(d, gc, color);
  XSetLineAttributes(d, gc, anchoBorde, LineSolid, CapRound, JoinMiter);

  XGCValues gcValuesPreDraw;
  gcValuesPreDraw.function = GXxor;
  gcValuesPreDraw.foreground = 0xAACCFF;
  gcPreDraw = XCreateGC(d, w, GCForeground + GCFunction, &gcValuesPreDraw);
  XSetLineAttributes(d, gcPreDraw, anchoBorde, LineDoubleDash, CapRound, JoinMiter);

  // Los eventos que vamos a escuchar
  XSelectInput(d, w,
    ExposureMask | ButtonPressMask | KeyPressMask |
    ButtonReleaseMask | ButtonMotionMask);

  // Mostramos la ventana
  XMapWindow(d, w);

  // Preparamos los niveles de deshacer
  int nivelDeshacerDisponibles = 0;
  Pixmap fondo[MAX_NIVELES_DESHACER];
  inicializaNivelesDeshacer(fondo, d, w, screen, ancho, alto);




  while (True) {
    XNextEvent(d, &e);
    switch (e.type) {
    case ButtonPress: /* Posición inicial de la herramienta */
      puntos[p].x = e.xbutton.x;
      puntos[p].y = e.xbutton.y;
      p++;
    case Expose:
      break;
    case ButtonRelease: /* Posición final de la herramienta */
      puntos[p].x = e.xbutton.x;
      puntos[p].y = e.xbutton.y;

      switch (herramienta) {
      case 'c':
        //se borra el predibujado antes de guardar el nivel de deshacer
        if (pointPreDraw.x >= 0 && pointPreDraw.y >= 0) {
          dibujarRectangulo(d, w, gcPreDraw,
            puntos[0].x, puntos[0].y, pointPreDraw.x, pointPreDraw.y);
        }

        //rotamos los fondos para dejar espacio al nuevo
        rotaNivelDeshacerAlaDer(fondo, d, gc, ancho, alto);
        XCopyArea(d, w, fondo[0], gc, 0, 0, ancho, alto, 0, 0); //guardamos el fondo actual

        nivelDeshacerDisponibles++;
        if (nivelDeshacerDisponibles > MAX_NIVELES_DESHACER)
          nivelDeshacerDisponibles = MAX_NIVELES_DESHACER;

        dibujarRectangulo(d, w, gc,
          puntos[0].x, puntos[0].y, puntos[1].x, puntos[1].y);
        break;
      case 'a':
        //se borra el predibujado antes de guardar el nivel de deshacer
        if (pointPreDraw.x >= 0 && pointPreDraw.y >= 0) {
          dibujarFlecha(d, w, gcPreDraw,
            puntos[0].x, puntos[0].y, pointPreDraw.x, pointPreDraw.y);
        }

        //rotamos los fondos para dejar espacio al nuevo
        rotaNivelDeshacerAlaDer(fondo, d, gc, ancho, alto);
        XCopyArea(d, w, fondo[0], gc, 0, 0, ancho, alto, 0, 0); //guardamos el fondo actual

        nivelDeshacerDisponibles++;
        if (nivelDeshacerDisponibles > MAX_NIVELES_DESHACER)
          nivelDeshacerDisponibles = MAX_NIVELES_DESHACER;
          
        dibujarFlecha(d, w, gc,
          puntos[0].x, puntos[0].y, puntos[1].x, puntos[1].y);
        break;
      }

      // reiniciamos los puntos y la herramienta de predibujado
      p = 0;
      pointPreDraw.x = -1;
      pointPreDraw.y = -1;
      break;
    case MotionNotify: /* predibujado de la heramienta */
      // se borra la anterior figura de predibujado
      if (pointPreDraw.x >= 0 && pointPreDraw.y >= 0) {
        switch (herramienta) {
        case 'c':
          dibujarRectangulo(d, w, gcPreDraw,
            puntos[0].x, puntos[0].y, pointPreDraw.x, pointPreDraw.y);
          break;
        case 'a':
          dibujarFlecha(d, w, gcPreDraw,
            puntos[0].x, puntos[0].y, pointPreDraw.x, pointPreDraw.y);
          break;
        }
      }

      pointPreDraw.x = e.xmotion.x;
      pointPreDraw.y = e.xmotion.y;

      switch (herramienta) {
      case 'c':
        dibujarRectangulo(d, w, gcPreDraw,
          puntos[0].x, puntos[0].y, pointPreDraw.x, pointPreDraw.y);
        break;
      case 'a':
        dibujarFlecha(d, w, gcPreDraw,
          puntos[0].x, puntos[0].y, pointPreDraw.x, pointPreDraw.y);
        break;
      }
      XFlush(d);
      break;
    case KeyPress:
      if (e.xkey.keycode == 54 || e.xkey.keycode == 37) {  // 54 =c,37=control
        herramienta = 'c';
        p = 0;
      }
      else if (e.xkey.keycode == 38 || e.xkey.keycode == 50) {  // 38 =a, 50=shift
        herramienta = 'a';
        p = 0;
      }
      else if (e.xkey.keycode == 29) {  // 29 =y
        color = 0xFFFF33;
        XSetForeground(d, gc, color);
      }
      else if (e.xkey.keycode == 27) {  // 27 =r
        color = 0xFF3333;
        XSetForeground(d, gc, color);
      }
      else if (e.xkey.keycode == 56) {  // 56 =b
        color = 0x3333FF;
        XSetForeground(d, gc, color);
      }
      else if (e.xkey.keycode == 25) {  // 25 =w
        color = 0xFFFFFF;
        XSetForeground(d, gc, color);
      }
      else if (e.xkey.keycode == 42) {  // 42 =g
        color = 0x00FF00;
        XSetForeground(d, gc, color);
      }
      else if (e.xkey.keycode == 30) { //30 =u
        if (nivelDeshacerDisponibles > 0) {
          nivelDeshacerDisponibles = (nivelDeshacerDisponibles < 0) ? 0 : --nivelDeshacerDisponibles;

          //restauramos el fondo guardado
          XCopyArea(d, fondo[0], w, gc, 0, 0, ancho, alto, 0, 0);

          //perdemos un nivel de deshacer
          rotaNivelDeshacerAlaIzq(fondo, d, gc, ancho, alto);
        }
      }
      else {
        // printf("%d", e.xkey.keycode);
        exit(0);
      }
    }
  }
  return 0;
}
