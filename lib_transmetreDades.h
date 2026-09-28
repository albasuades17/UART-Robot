#include <msp432p401r.h>
#include "lib_PAE.h"
#include "lib_initTimers.h"

#define TXD0_READY (UCA0IFG & UCTXIFG)
#define TXD2_READY (UCA2IFG & UCTXIFG)


static byte Byte_Recibido;
static byte DatoLeido_UART;

typedef struct
{
    byte StatusPacket[32];
    byte timeOut;
} RxReturn;

void mode_emulador();

void Sentit_Dades_Rx(void);

void Sentit_Dades_Tx(void);

void TxUACx(uint8_t bTxdData);

byte TxPacket(byte bID, byte bParameterLength, byte bInstruction, byte Parametros[16]);

RxReturn RxPacket(void);

void EUSCIA0_IRQHandler();

void EUSCIA2_IRQHandler();
