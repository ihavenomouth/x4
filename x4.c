#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "configuracion.h"

#include <signal.h>
// #include <unistd.h> //por el usleep

#define MAXPOS 2


/**
 * Dibuja una flecha en la pantalla
 * x0, y0 : punto que marca la punta de la flecha
 * x1, y1 : punto que marca el final de la flecha
 */
void dibujarFlecha(Display* d, Window w, GC gc, int x0, int y0, int x1, int y1, int tamFlecha) {
  XDrawLine(d, w, gc, x0, y0, x1, y1);

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
void inicializaNivelesDeshacer(Pixmap* fondo, Display* d, Window w, int screen, unsigned int ancho, unsigned int alto, int nivelesDeshacer, int depth) {
  for (int i = 0;i < nivelesDeshacer;i++) {
    // fondo[i] = XCreatePixmap(d, w, ancho, alto, XDefaultDepth(d, screen));
    fondo[i] = XCreatePixmap(d, w, ancho, alto, depth);  // Usar depth específico
  }
}




////////////////////////
// MAIN
////////////////////////
int main() {
  // Ignorar la señal de SIGQUIT Ctrol+4
  signal(SIGQUIT, SIG_IGN);


  Display* d;
  Window w;
  XEvent e;
  GC gc;
  int screen;

  XPoint puntos[MAXPOS];
  int p = 0;

  GC gcPreDraw;
  XPoint pointPreDraw;
  pointPreDraw.x = -1;
  pointPreDraw.y = -1;







  char herramienta = 'r';  //rectángulo
  long color = 0xFFFF3333; //Rojo translúcido = 0xFF3333; Rojo opaco = 0xFFFF3333, El primer número es la opacidad
  int numeroDePaso=1;

  config c;
  leerCofiguracion(&c);

  d = XOpenDisplay(NULL);
  screen = XDefaultScreen(d);
  unsigned int alto = DisplayHeight(d, screen);
  unsigned int ancho = DisplayWidth(d, screen);

  // printf("Creando ventana de %dx%d (pantalla completa)\n", ancho, alto);

  // === AQUÍ VA EL CÓDIGO NUEVO ===
  XVisualInfo vinfo;
  int visual_encontrado = 0;
  
  if (XMatchVisualInfo(d, screen, 32, TrueColor, &vinfo)) {
      // printf("Encontrado visual de 32 bits con alpha\n");
      visual_encontrado = 1;
      
      // Crear colormap
      Colormap colormap = XCreateColormap(d, DefaultRootWindow(d), vinfo.visual, AllocNone);
      
      // Atributos de ventana
      XSetWindowAttributes attrs;
      attrs.colormap = colormap;
      attrs.background_pixel = 0;  // Negro transparente
      attrs.border_pixel = 0;
      attrs.override_redirect = True;
      
      // Crear ventana con visual de 32 bits
      w = XCreateWindow(d, DefaultRootWindow(d), 0, 0, ancho, alto, 0,
                       vinfo.depth, InputOutput, vinfo.visual,
                       CWColormap | CWBackPixel | CWBorderPixel | CWOverrideRedirect, &attrs);
  } else {
      // printf("Visual de 32 bits no disponible, usando visual por defecto\n");
      // Fallback: crear ventana normal
      w = XCreateSimpleWindow(d, DefaultRootWindow(d), 0, 0, ancho, alto, 0, 0, 0);
  }
  // === FIN DEL CÓDIGO NUEVO ===

  // Configurar propiedades para pantalla completa
  Atom net_wm_state = XInternAtom(d, "_NET_WM_STATE", False);
  Atom net_wm_state_fullscreen = XInternAtom(d, "_NET_WM_STATE_FULLSCREEN", False);

  if (net_wm_state != None && net_wm_state_fullscreen != None) {
      XChangeProperty(d, w, net_wm_state, XA_ATOM, 32,
                     PropModeReplace, (unsigned char*)&net_wm_state_fullscreen, 1);
      // printf("Configurada pantalla completa\n");
  }

  // Configurar ventana siempre encima
  Atom net_wm_state_above = XInternAtom(d, "_NET_WM_STATE_ABOVE", False);
  if (net_wm_state_above != None) {
      Atom states[2] = {net_wm_state_fullscreen, net_wm_state_above};
      XChangeProperty(d, w, net_wm_state, XA_ATOM, 32,
                     PropModeReplace, (unsigned char*)states, 2);
      // printf("Configurada para estar siempre encima\n");
  }

  // Quitar decoraciones
  Atom motif_hints = XInternAtom(d, "_MOTIF_WM_HINTS", False);
  if (motif_hints != None) {
      unsigned long hints[5] = {2, 0, 0, 0, 0};
      XChangeProperty(d, w, motif_hints, motif_hints, 32,
                     PropModeReplace, (unsigned char*)hints, 5);
      // printf("Decoraciones quitadas\n");
  }

  // Configurar como overlay/dock
  Atom net_wm_window_type = XInternAtom(d, "_NET_WM_WINDOW_TYPE", False);
  Atom net_wm_window_type_dock = XInternAtom(d, "_NET_WM_WINDOW_TYPE_DOCK", False);
  if (net_wm_window_type != None && net_wm_window_type_dock != None) {
      XChangeProperty(d, w, net_wm_window_type, XA_ATOM, 32,
                     PropModeReplace, (unsigned char*)&net_wm_window_type_dock, 1);
      // printf("Configurada como dock/overlay\n");
  }

  // Configurar eventos
  XSelectInput(d, w,
      ExposureMask | ButtonPressMask | KeyPressMask |
      ButtonReleaseMask | ButtonMotionMask | StructureNotifyMask);

  // Mapear ventana
  XMapWindow(d, w);
  // Después de XMapWindow, forzar el foco
  XSetInputFocus(d, w, RevertToParent, CurrentTime);
  XRaiseWindow(d, w);  // Asegurar que está encima
  XFlush(d);
  // printf("Ventana mapeada\n");


  // === CREAR GCs DESPUÉS de mapear ===
  gc = XCreateGC(d, w, 0, NULL);
  
  // Configurar color OPACO para el GC principal
  if (visual_encontrado) {
      // Para visual de 32 bits, usar color con alpha opaco
      XSetForeground(d, gc, color);  // Rojo completamente opaco
  } else {
      // Para visual normal
      XSetForeground(d, gc, color);
  }
  
  XSetLineAttributes(d, gc, c.anchoBorde, LineSolid, CapRound, JoinMiter);

  // GC para predibujado
  XGCValues gcValuesPreDraw;
  gcValuesPreDraw.function = GXxor;
  gcValuesPreDraw.foreground = 0xFF77AADD; //Color con el que se predibuja la flecha o rectángulo
  gcPreDraw = XCreateGC(d, w, GCForeground + GCFunction, &gcValuesPreDraw);
  XSetLineAttributes(d, gcPreDraw, c.anchoBorde, LineDoubleDash, CapRound, JoinMiter);

  // printf("GCs creados - aplicación lista\n");

  // ... el resto de tu código (nivelesDeshacer, while loop, etc.)
/** FIN WAYLAND */

  // Mostramos la ventana
  // XMapWindow(d, w);

  // Obtener el depth de la ventana actual
  Window root;
  int x, y;
  unsigned int width, height, border_width, depth;
  XGetGeometry(d, w, &root, &x, &y, &width, &height, &border_width, &depth);
  
  // printf("Depth de la ventana: %d\n", depth);

  // Preparamos los niveles de deshacer
  int nivelesDeshacerDisponibles = 0;
  int indiceNivelDeshacer = 0;
  Pixmap fondo[c.nivelesDeshacer];
  inicializaNivelesDeshacer(fondo, d, w, screen, ancho, alto, c.nivelesDeshacer, depth);




  while (True) {
    XNextEvent(d, &e);

    // Debug: imprimir todos los eventos
    // printf("Evento recibido: tipo=%d\n", e.type);
        // DEBUG: Imprimir TODOS los eventos
    // printf("=== EVENTO ===\n");
    // printf("Tipo: %d\n", e.type);
    // printf("Serial: %lu\n", e.xany.serial);
    // printf("Send_event: %d\n", e.xany.send_event);
    // printf("Display: %p\n", e.xany.display);
    // printf("Window: %lu\n", e.xany.window);
    
    /*if (e.type == KeyPress || e.type == KeyRelease) {
        printf("Evento de tecla: %s\n", (e.type == KeyPress) ? "PRESS" : "RELEASE");
        printf("Keycode: %d\n", e.xkey.keycode);
        printf("State: %d\n", e.xkey.state);
        // printf("Misma pantalla: %d\n", e.xkey.same_screen);
    }*/

    switch (e.type) {
    case ButtonPress: /* Posición inicial de la herramienta */
      puntos[p].x = e.xbutton.x;
      puntos[p].y = e.xbutton.y;
      p++;
      break;
    case Expose:
      break;
    case ButtonRelease: /* Posición final de la herramienta */
      puntos[p].x = e.xbutton.x;
      puntos[p].y = e.xbutton.y;

      switch (herramienta) {
      case 'r':
        //se borra el predibujado antes de guardar el nivel de deshacer
        if (pointPreDraw.x >= 0 && pointPreDraw.y >= 0) {
          dibujarRectangulo(d, w, gcPreDraw, puntos[0].x, puntos[0].y, pointPreDraw.x, pointPreDraw.y);
        }

        //guardamos el fondo en la posición actual y la incrementamos 
        XCopyArea(d, w, fondo[indiceNivelDeshacer], gc, 0, 0, ancho, alto, 0, 0); //guardamos el fondo actual
        
        indiceNivelDeshacer = (indiceNivelDeshacer >= c.nivelesDeshacer - 1) ? 0 : ++indiceNivelDeshacer;
        nivelesDeshacerDisponibles = (nivelesDeshacerDisponibles >= c.nivelesDeshacer) ? c.nivelesDeshacer : ++nivelesDeshacerDisponibles;

        dibujarRectangulo(d, w, gc, puntos[0].x, puntos[0].y, puntos[1].x, puntos[1].y);
        break;
      case 'a':
        //se borra el predibujado antes de guardar el nivel de deshacer
        if (pointPreDraw.x >= 0 && pointPreDraw.y >= 0) {
          dibujarFlecha(d, w, gcPreDraw, puntos[0].x, puntos[0].y, pointPreDraw.x, pointPreDraw.y, c.tamFlecha);
        }

        //guardamos el fondo en la posición actual y la incrementamos 
        XCopyArea(d, w, fondo[indiceNivelDeshacer], gc, 0, 0, ancho, alto, 0, 0); //guardamos el fondo actual

        indiceNivelDeshacer = (indiceNivelDeshacer >= c.nivelesDeshacer - 1) ? 0 : ++indiceNivelDeshacer;
        nivelesDeshacerDisponibles = (nivelesDeshacerDisponibles >= c.nivelesDeshacer) ? c.nivelesDeshacer : ++nivelesDeshacerDisponibles;

        dibujarFlecha(d, w, gc, puntos[0].x, puntos[0].y, puntos[1].x, puntos[1].y, c.tamFlecha);
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
        case 'r':
          dibujarRectangulo(d, w, gcPreDraw,
            puntos[0].x, puntos[0].y, pointPreDraw.x, pointPreDraw.y);
          break;
        case 'a':
          dibujarFlecha(d, w, gcPreDraw,
            puntos[0].x, puntos[0].y, pointPreDraw.x, pointPreDraw.y, c.tamFlecha);
          break;
        }
      }

      pointPreDraw.x = e.xmotion.x;
      pointPreDraw.y = e.xmotion.y;

      switch (herramienta) {
      case 'r':
        dibujarRectangulo(d, w, gcPreDraw,
          puntos[0].x, puntos[0].y, pointPreDraw.x, pointPreDraw.y);
        break;
      case 'a':
        dibujarFlecha(d, w, gcPreDraw,
          puntos[0].x, puntos[0].y, pointPreDraw.x, pointPreDraw.y, c.tamFlecha);
        break;
      }
      XFlush(d);
      break;

    case KeyPress:
      /*Keycodes:
      24 = q
      25 = w
      26 = e
      27 = r
      28 = t
      29 = y
      38 = a
      42 = g
      54 = c
      56 = b
      57 = n
      37 = control
      50 = shift

      10 = 1
      11 = 2
      12 = 3
      13 = 4

      */
      if (e.xkey.keycode == 37) {  // 37=control
        herramienta = 'r';
        p = 0;
      }
      else if (e.xkey.keycode == 50) {  // 50=shift
        herramienta = 'a';
        p = 0;
      }
      else if (e.xkey.keycode == 10) {  // 1
        color = 0xFFFF3333;
        XSetForeground(d, gc, color);
      }
      else if (e.xkey.keycode == 11) {  // 2
        color = 0xFF3333FF;
        XSetForeground(d, gc, color);
      }
      else if (e.xkey.keycode == 12) {  // 3
        color = 0xFFFFFF33;
        XSetForeground(d, gc, color);
      }
      else if (e.xkey.keycode == 13) {  // 4
        color = 0xFF33FF33;
        XSetForeground(d, gc, color);
      }
      else if (e.xkey.keycode == 14) {  // 5
        color = 0xFFFFFFFF;
        XSetForeground(d, gc, color);
      }
      else if (e.xkey.keycode == 57) {  // n = numeros

        XFontStruct * font = XLoadQueryFont(d, c.fuente);
        XSetFont(d, gc, font->fid);
        
        char numCadena[4];
        numCadena[0]='(';
        numCadena[1]= numeroDePaso +'0';;
        numCadena[2]=')';
        numCadena[3]='\0';
        numeroDePaso++;
        if(numeroDePaso>=9)numeroDePaso=0;
        XDrawString (d, w, gc, e.xbutton.x, e.xbutton.y, numCadena, strlen (numCadena));
      }
      else if (e.xkey.keycode == 30) { //30 =u
        if (nivelesDeshacerDisponibles > 0) { //si hay niveles que se pueden deshacer
          indiceNivelDeshacer = (indiceNivelDeshacer == 0) ? c.nivelesDeshacer - 1 : --indiceNivelDeshacer;
          nivelesDeshacerDisponibles = (nivelesDeshacerDisponibles < 0) ? 0 : --nivelesDeshacerDisponibles;

          //restauramos el fondo guardado
          XCopyArea(d, fondo[indiceNivelDeshacer], w, gc, 0, 0, ancho, alto, 0, 0);
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
