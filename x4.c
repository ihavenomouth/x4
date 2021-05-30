#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MAXPOS 2

void dibujarFlecha(Display *d, Window w, GC gc, int x0, int y0, int x1, int y1) {
  XDrawLine(d, w, gc, x0, y0, x1, y1);

  int tamFlecha = 15;
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

void dibujarRectangulo(Display *d, Window w, GC gc, int x0, int y0, int x1, int y1) {
  int alto  = abs(y1 - y0);
  int ancho = abs(x1 - x0);

  if (x0 <= x1) {
    if (y0 <= y1) {
      XDrawRectangle(d, w, gc, x0, y0, ancho, alto);
    } else {
      XDrawRectangle(d, w, gc, x0, y1, ancho, alto);
    }
  } else {
    if (y0 <= y1) {
      XDrawRectangle(d, w, gc, x1, y0, ancho, alto);
    } else {
      XDrawRectangle(d, w, gc, x1, y1, ancho, alto);
    }
  }
}

int main() {
  Display *d;
  Window w;
  XEvent e;
  GC gc;

  XPoint puntos[MAXPOS];  // 2 puntos para dibujar un rectángulo
  int p = 0;              // contador de los puntos

  // Para predibujar la forma antes de pintarla
  // valores incorrectos iniciales, para no pintarlo sin tener el primer punto
  // elegido
  GC gcPreDraw;
  XPoint pointPreDraw;
  pointPreDraw.x = -1;
  pointPreDraw.y = -1;

  char herramienta = 'c';
  long color = 0xFF3333;

  d = XOpenDisplay(NULL);

  w = XCreateWindow(d, DefaultRootWindow(d), 200, 100, 400, 300, CopyFromParent,
                    CopyFromParent, CopyFromParent, CopyFromParent,
                    CopyFromParent, CopyFromParent);

  // Cambiamos a pantalla completa
  Atom atoms[2] = {XInternAtom(d, "_NET_WM_STATE_FULLSCREEN", False), None};
  XChangeProperty(d, w, XInternAtom(d, "_NET_WM_STATE", False), XA_ATOM, 32,
                  PropModeReplace, (unsigned char *)atoms, 1);

  // Creamos los GC (Contexto gráfico) que usaremos
  gc = XCreateGC(d, w, 0, NULL);

  XSetForeground(d, gc, color);
  XSetLineAttributes(d, gc,
                     5,  // ancho línea
                     LineSolid, CapRound, JoinMiter);

  XGCValues gcValuesPreDraw;
  gcValuesPreDraw.function = GXxor;
  gcValuesPreDraw.foreground = 0x0000FF;
  gcPreDraw = XCreateGC(d, w, GCForeground + GCFunction, &gcValuesPreDraw);

  XSetLineAttributes(d, gcPreDraw, 5, LineDoubleDash, CapRound, JoinMiter);

  // Los eventos que vamos a escuchar
  XSelectInput(d, w,
               ExposureMask | ButtonPressMask | KeyPressMask |
               ButtonReleaseMask | ButtonMotionMask);

  // Mostramos la pantalla
  XMapWindow(d, w);

  while (True) {
    XNextEvent(d, &e);
    switch (e.type) {
      case ButtonPress:
        puntos[p].x = e.xbutton.x;
        puntos[p].y = e.xbutton.y;
        p++;
      case Expose:
        break;
      case ButtonRelease:
        puntos[p].x = e.xbutton.x;
        puntos[p].y = e.xbutton.y;

        switch (herramienta) {
          case 'c':
            dibujarRectangulo(d, w, gc, 
                  puntos[0].x, puntos[0].y, puntos[1].x, puntos[1].y);
            break;
          case 'a':
            dibujarFlecha(d, w, gc, 
                  puntos[0].x, puntos[0].y, puntos[1].x, puntos[1].y);
            break;
        }

        // reiniciamos los puntos y la herramienta de predibujado
        p = 0;
        pointPreDraw.x = -1;
        pointPreDraw.y = -1;
        break;
      case MotionNotify:
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
                  puntos[0].x, puntos[0].y,pointPreDraw.x, pointPreDraw.y);
            break;
        }
        XFlush(d);
        break;
      case KeyPress:
        if (e.xkey.keycode == 54 || e.xkey.keycode == 37) {  // 54 =c,37=control
          herramienta = 'c';
          p = 0;
        } else if (e.xkey.keycode == 38 || e.xkey.keycode == 50) {  // 38 =a, 50=shift
          herramienta = 'a';
          p = 0;
        } else if (e.xkey.keycode == 29) {  // 29 =y
          color = 0xFFFF33;
          XSetForeground(d, gc, color);
        } else if (e.xkey.keycode == 27) {  // 27 =r
          color = 0xFF3333;
          XSetForeground(d, gc, color);
        } else if (e.xkey.keycode == 56) {  // 56 =b
          color = 0x3333FF;
          XSetForeground(d, gc, color);
        } else if (e.xkey.keycode == 25) {  // 25 =w
          color = 0xFFFFFF;
          XSetForeground(d, gc, color);
        } else if (e.xkey.keycode == 42) {  // 42 =g
          color = 0x00FF00;
          XSetForeground(d, gc, color);
        } else {
          printf("%d", e.xkey.keycode);
          exit(0);
        }
    }
  }
  return 0;
}