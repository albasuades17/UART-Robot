#include <msp432p401r.h>
#include "lib_PAE.h"

typedef uint8_t byte;
volatile uint16_t cont_A1;

void Activa_TimerA1_TimeOut(void);

void Reset_Timeout();

byte TimeOut(uint16_t time);

void TA1_0_IRQHandler(void);

void init_TimerA1();
