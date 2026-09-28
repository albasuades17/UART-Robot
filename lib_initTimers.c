#include "lib_initTimers.h"

void Activa_TimerA1_TimeOut(void)
{
    /*
    //Timer A1
    //Fem un divider de 1. Netegem el counter i posem mode up.
    TIMER_A1->CTL = TIMER_A_CTL_ID__1 | TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_CLR
                | TIMER_A_CTL_MC__UP;
    //SMCLK va a 24 MHz, volem 1 MHz, per tant, amb el divider de 1, tindrem un total de 24 cicles.
    TIMER_A1->CCR[0] = 240-1;     // 1 MHz
    TIMER_A1->CCTL[0] |= TIMER_A_CCTLN_CCIE; //Interrupciones activadas en CCR0
    */

    cont_A1=0;

}

void init_TimerA1(){
    //Timer A1
    //Fem un divider de 1. Netegem el counter i posem mode up.
    TIMER_A1->CTL = TIMER_A_CTL_ID__1 | TIMER_A_CTL_SSEL__SMCLK | TIMER_A_CTL_CLR
                | TIMER_A_CTL_MC__UP;
    //SMCLK va a 24 MHz, volem 100 KHz, per tant, amb el divider de 1, tindrem un total de 24 cicles.
    TIMER_A1->CCR[0] = 240-1;     // 100 KHz
    TIMER_A1->CCTL[0] |= TIMER_A_CCTLN_CCIE; //Interrupciones activadas en CCR0
}

void Reset_Timeout(){
    cont_A1=0;
}

//Mirem si ja ha passat el temps fixat
byte TimeOut(uint16_t time){
    //Si superem el temps establert tenim time out
    if(cont_A1 >= time){
        return 1;
    }
    return 0;
}

void TA1_0_IRQHandler(void)
{
    TA1CCTL0 &= ~TIMER_A_CCTLN_CCIFG; //Clear interrupt flag
    cont_A1++;

}


