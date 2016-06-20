#include "usyscall.h"

/* This code was done by the PEC professors */

/**************************************************************
* Espera a que se pulse un pulsador (KEY) en la placa         *
* y devuelve el numero del pulsador apretado cuando se suelta *
**************************************************************/
int read_key_bloq (void)
{
  // espera a que se pulse un pulsador y devuelve el numero del pulsador apretado.
  int res,aux1,aux2,aux3;

    /*
    polling1: in   r3, 7        ; leemos el estado de los pulsadores
              not  r3, r3       ; invertimos los bits ya que los pulsadores son activos a 0
              movi r1, 0x0F     ; mascara para quedarnos con los 4 bits de menor peso
              and  r1, r3, r1   ; nos quedamos con los 4 bits de los pulsadores
              bz   r1, polling1 ; si todos valen 0 nos quedamos esperando (bloqueando) a que se pulse uno.
    pooling2: in   r3, 7        ; volvemos a leer el estado de los pulsadores
              not  r3, r3       ;
              movi r2, 0x0F     ;
              and  r2, r3, r2   ; nos quedamos con los 4 bits ya que no sabemos como esta implementado el registro hardware de los pulsadores
              bnz  r2, polling2 ; y esperamos hasta que no se pulse ninguno
    */

__asm__ (
        "__pol1: in %1, %4\n\t"
        "not %1, %1\n\t"
        "movi %0, 15\n\t"
        "and %0, %1, %0\n\t"
        "bz %0, __pol1\n\t"
        "__pol2: in %1, %4\n\t"
        "not %1, %1\n\t"
        "movi %2, 15\n\t"
        "and %2, %1, %2\n\t"
        "bnz %2, __pol2\n\t"
        : "=r" (res)
        : "r" (aux1),
          "r" (aux2),
          "r" (aux3),
          "i" (7));              // Puerto 7 => Registro con el estado de los pulsadores (KEY)

    return res;
}


/************************************************************
* Devuelve el estado actual de los interruptores (switch)   *
* de la placa DE1                                           *
************************************************************/
unsigned int read_switchs(void)
{
  unsigned int res;

  __asm__ ( "in %0, %1"
          : "=r" (res)
          : "i" (8));         // Puerto 8 => registro con el estado de los interruptores (SWITCH)

  return res;
}


/*************************************************************
* Escribe en los visores 7 segmentos un valor hexadecimal    *
* y determina que visores están encendidos y cuales apagados *
*************************************************************/
int write_7segments(int valor, char control)
{
    int res;

    __asm__ (
        "out %0, %1\n\t"      // apagamos/encendemos los visores
        "out %2, %3\n\t"      // mostramos el valor
        : /* sin salidas*/
        : "i" (9),            // Puerto 9 => Control visores
          "r" (control),
          "i" (10),           // Puerto 10 => Valor visores
          "r" (valor));

    return res;
}


/* key codes */
#define KEY3      0x08
#define KEY2      0x04
#define KEY1      0x02
#define KEY0      0x01


unsigned int fibonacci_recursiu(unsigned int n)
{
    if (n<2)
        return n;
    else
        return fibonacci_recursiu(n-1) + fibonacci_recursiu(n-2);
}

/*
 Muestra el valor de la secuencia de fibonaccio (en hexadecimal) por los visores de la placa
 a) calcula el valor de fibonacci de forma recursiva:
    Si se pulsa KEY1 se llama al fibonacci recursivo con el valor natural que este codificado en los 4 SWITCH de menor peso.

 b) calcula el valor del fibonacci de forma iterativa paso a paso
    Si se pulsa KEY3 se ponen la sucecions al inicio (primer valor)
    cada vez que se pulsa KEY2 muestra el siguiente valor de la sucesion
*/

int main2 (void)
{
    int pulsador;
    unsigned int interruptores;
    int fibonacci=0, anterior=0, actual=1;

    write_7segments(fibonacci, 0x0F);
    for (;;) {
        pulsador=read_key_bloq();
        if (pulsador==KEY3) { //reset para el fibonacci interativo
            fibonacci=0;
            anterior=0;
            actual=1;
        }
        if (pulsador==KEY2) { //siguiente sucesion del fibonacci iterativo
            fibonacci=actual+anterior;
            anterior=actual;
            actual=fibonacci;
        }
        if (pulsador==KEY1) { //llamada al fibonacci recursivo
            interruptores=read_switchs();
            interruptores=interruptores & 0x000F; // limitamos el valor a 15 para que los numeros grandes no desborden la pila en las llamadas recursivas
            fibonacci=fibonacci_recursiu(interruptores);
        }
        write_7segments(fibonacci, 0x0F);
    }
    return 0;
}
