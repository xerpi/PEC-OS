#include "usyscall.h"

/* This code was done by the PEC professors */

/**************************************************************
* Borra la pantalla: Escribe 80x30 caracteres (espacios con   *
* fondo negro) en la memoria de la pantalla (0xA000)          *
**************************************************************/
void clear_screen(void)
{
    int aux1, aux2, aux3;

    __asm__ (
            "movi  %0, lo(0xA000)\n\t"   //0xA000 direccion de inicio de la memoria de video
            "movhi %0, hi(0xA000)\n\t"
            "movi  %1, lo(2400)\n\t"     //(80*30=2400=0x0960) numero caracteres de la pantalla
            "movhi %1, hi(2400)\n\t"
            "movi  %2, lo(0x0020)\n\t"
            "movhi %2, hi(0x0020)\n\t"   //un espacio en color negro
            "__repe: st 0(%0), %2\n\t"
            "addi  %0, %0,2\n\t"
            "addi  %1, %1,-1\n\t"
            "bnz   %1, __repe\n\t"
            : /* sin salidas*/
            : "r" (aux1),
              "r" (aux2),
              "r" (aux3));
}


/**************************************************************
* Borra un caracter en la pantalla: Escribe un espacio con    *
* fondo negro en la fila y columnas indicadas                 *
**************************************************************/
void borrar_letra(int fila, int col)
{
    int valor;
    int pos;

    valor=0x0020;               //un espacio en color negro
    pos=(80*fila+col)*2;        // cada caracter ocupa 2 bytes de memoria de video
    pos=pos + 0xA000;           // la memoria de video empieza en la direccion 0xA000 (40960)

    __asm__ (
            "st 0(%0), %1"      // %0 corresponde a la variable "pos" que habra sido cargada en un registro
                                // %1 corresponde a la variable "c" que habra sido cargada en otro registro
            : /* sin salidas */
            : "a" (pos),
              "b" (valor));
}


/*******************************************************************
* Muestra un caracter en la pantalla: Escribe el caracter recibido *
* con el color y fondo indicados en la fila y columnas indicadas   *
*******************************************************************/
void mostrar_nueva_posicion(int fila, int col, char c, char color)
{
    int caracter_compuesto;
    int pos;

    caracter_compuesto=color*256+c;
    pos=(80*fila+col)*2;           // cada caracter ocupa 2 bytes de memoria de video
    pos=pos + 0xA000;              // la memoria de video empieza en la direccion 0xA000 (40960)

    __asm__ (
            "st 0(%0), %1"         // %0 corresponde a la variable "pos" que habra sido cargada en un registro
                                   // %1 corresponde a la variable "c" que habra sido cargada en otro registro
            : /* sin salidas */
            : "a" (pos),
              "b" (caracter_compuesto));
}


/*****************************************************************
* Devuelve la longitud de una cadena de caracteres *
******************************************************************/
int strlen_s(char *str)
{
  int len;
  for (len = 0; *str++; len++);
  return len;
}


/*****************************************************************
* Muestra una cadena de caracteres en la pantalla con el color y *
* el fondo indicados empezando la fila y columnas indicadas      *
******************************************************************/
int write(int fila, int col, char *buf, int size, char color)
{
  int i, t;
  int f, c, car;
  t = 0; f = fila; c = col;
  for (i=0; i<size; i++)
    {
      car = buf[i];
      mostrar_nueva_posicion(f, c, car, color);
      t++;
      if (car=='\n') f++;
      else if (car=='\r') c=1;
      else if (car!=0) c++;
    }

  return t;
}




#define ANCHO_PANTALLA  80
#define LIMITE_LETRAS   13

/* colores */
#define NEGRO           0x00
#define BLANCO          0x3F
#define ROJO            0x03
#define VERDE           0x0C
#define AZUL            0x30
#define GRIS_OSCURO     0x15
#define AZUL_CIELO      0x3C
#define AMARILLO        0x0F
#define LILA            0x2A
#define GRIS_CLARO      0x2A
#define GRIS_MUY_CLARO  0x3F

char letras[] = {'A','B','C','D','E','F','G','H','I','J','K','L','M'};
char colores[] = {BLANCO, ROJO, VERDE, AZUL, GRIS_OSCURO, AZUL_CIELO, AMARILLO, LILA, GRIS_CLARO, GRIS_MUY_CLARO, 0x1A, 0X21, 0X2B};
int divisores[] = {1,2,3,4,5,6,7,8,9,10,11,12,13};
char mensaje[] = "pulsa 'R' para reiniciar o 'P' para pausa.";

unsigned int tics_timer=0;     //variable global cuyo valor es modificado por la interrupcion de reloj
unsigned int tecla_pulsada=0;  //variable global cuyo valor es modificado por la interrupcion de teclado

struct letra_t
{
    int posicion;    // posicion dentro la linea
    char caracter;   // caracter a mostrar
    char color;      // color caracter

};

#define NUMERO_LETRAS  12
struct letra_t  letra[NUMERO_LETRAS];


void reset_corre_letras(void)
{
    clear_screen();      // borrar pantalla

    write(15, 19, mensaje, strlen_s(mensaje), ROJO);
    int i;
    for (i=0; i<NUMERO_LETRAS; i++) {
        letra[i].posicion=0;
        letra[i].caracter=letras[i];
        letra[i].color=colores[i];
        mostrar_nueva_posicion(i,letra[i].posicion,letra[i].caracter,letra[i].color);
    }
}


int main1 (void) {

    unsigned int tics_anterior=0;
    int pause=0;

    if (NUMERO_LETRAS<=LIMITE_LETRAS) {

        reset_corre_letras();

        while (1) {
	    tics_timer = getticks();
	    tecla_pulsada = readkb();

            switch (tecla_pulsada)
            {
            case 'r':    case 'R':
                reset_corre_letras();
                tecla_pulsada=0;
                break;

            case 'p':    case 'P':
                if (pause==0) pause=1; else pause=0;
                tecla_pulsada=0;
                break;

            default:
                break;
            }

            if (pause==0) {
                // para DEBUG: mostramos los valores del timer por los leds verdes
                __asm__ ( "out %0, %1"
                        : /* sin salidas*/
                        : "i" (5), "r" (tics_timer));
                // fin DEBUG

                if (tics_timer!=tics_anterior) {
                    tics_anterior=tics_timer;

                    int fila;
                    for (fila=0; fila<NUMERO_LETRAS; fila++) {
                        if (tics_timer%divisores[fila]==0) {
                            borrar_letra(fila,letra[fila].posicion);
                            if (letra[fila].posicion<80)
                                letra[fila].posicion++;
                            else
                                letra[fila].posicion=0;

                            mostrar_nueva_posicion(fila,letra[fila].posicion,letra[fila].caracter,letra[fila].color);
                        }
                    }
                }
            }
        }
    }

    return 0;
}
