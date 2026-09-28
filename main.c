/*
 * L'únic que s'ha de tocar per canviar el mode de funcionament (emulador o real) és la
 * constant MODE_FUNCIONAMENT d'aquest mateix main.
 */

#include "msp.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "lib_PAE.h"
//#include "lib_transmetreDades.h"
#include "lib_initTimers.h"
#include "lib_moviment.h"


//MODE_FUNCIONAMENT 0 és l'emulador
//MODE_FUNCIONAMENT 1 és el real
#define MODE_FUNCIONAMENT 1

#define VELOCITAT 600
#define VELOCITAT_INICI 250

//Si està a 0 està parat, sinó s'està movent
volatile int estat_robot = 0;

void init_interrupciones_emulador()
{
    //Activem les interrupcions pel eUSCI_A0
    NVIC->ICPR[0] |= BIT(16);
    NVIC->ISER[0] |= BIT(16);

    //Activem les interrupcions pel timer A1
    NVIC->ICPR[0] |= BITA;
    NVIC->ISER[0] |= BITA;
}
void init_interrupciones_real()
{
    //Activem les interrupcions pel eUSCI_A2
    NVIC->ICPR[0] |= BIT(18);
    NVIC->ISER[0] |= BIT(18);

    //Activem les interrupcions pel timer A1
    NVIC->ICPR[0] |= BITA;
    NVIC->ISER[0] |= BITA;
}



void init_UART_emulador(void)
{
    //Posem en el primer bit 1 per activat el reset
    UCA0CTLW0 |= UCSWRST; //Fem un reset de la USCI, desactiva la USCI
    //Posem en el bit 7-6 10b per tenir el SMCLK com a font de rellotge
    UCA0CTLW0 |= UCSSEL__SMCLK;

    //Volem mode asíncron perquè treballem amb el UART
    //Seleccionem el mode UART, volem el bit 10-9 a 00b
    //Volem 1 stop bit
    //UC7BIT=0 8 bits de dades
    //Com que no tenim bit de paritat, no importa el valor del registre UCPAR
    //UCPEN=0 sense bit de paritat

    UCA0CTLW0  &= ~(UCSYNC+UC7BIT+UCMODE0+UCMODE1+UCSPB+UCMSB+UCPEN);

    UCA0MCTLW = UCOS16; // Necessitem sobre-mostreig => bit 0 = UCOS16 = 1
    UCA0BRW = 3; //Prescaler de BRCLK fixat a 3. Com SMCLK va a 24MHz,
    //volem un baud rate de 500kb/s i fem sobre-mostreig de 16
    //el rellotge de la UART ha de ser de ~8MHz (24MHz/3).
    //No cal canviar el valor del UCBRFx ni el de UCBRSx, ja que és 0 (la divisió és entera)

    //Configurem els pins de la UART
    //Configurem els pins com a primera funció alternativa
    P1SEL0 |= BIT2 | BIT3; //I/O funció: P1.3 = UART0TX, P1.2 = UART0RX
    P1SEL1 &= ~ (BIT2 | BIT3);
    UCA0CTLW0 &= ~UCSWRST; //Reactivem la línia de comunicacions sèrie
    //Treballem amb l'UART, que és asíncron, per tant és l'interfície A (número 0).
    EUSCI_A0->IFG &= ~EUSCI_A_IFG_RXIFG; // Clear eUSCI RX interrupt flag
    EUSCI_A0->IE |= EUSCI_A_IE_RXIE; // Enable USCI_A0 RX interrupt, nomes quan tinguem la recepcio
}

void init_UART_real(void)
{
    //Posem en el primer bit 1 per activat el reset
    UCA2CTLW0 |= UCSWRST; //Fem un reset de la USCI, desactiva la USCI
    //Posem en el bit 7-6 10b per tenir el SMCLK com a font de rellotge
    UCA2CTLW0 |= UCSSEL__SMCLK;

    //Volem mode asíncron perquè treballem amb el UART
    //Seleccionem el mode UART, volem el bit 10-9 a 00b
    //Volem 1 stop bit
    //UC7BIT=0 8 bits de dades
    //Com que no tenim bit de paritat, no importa el valor del registre UCPAR
    //UCPEN=0 sense bit de paritat

    UCA2CTLW0  &= ~(UCSYNC+UC7BIT+UCMODE0+UCMODE1+UCSPB+UCMSB+UCPEN);

    UCA2MCTLW = UCOS16; // Necessitem sobre-mostreig => bit 0 = UCOS16 = 1
    UCA2BRW = 3; //Prescaler de BRCLK fixat a 3. Com SMCLK va a 24MHz,
    //volem un baud rate de 500kb/s i fem sobre-mostreig de 16
    //el rellotge de la UART ha de ser de ~8MHz (24MHz/3).
    //No cal canviar el valor del UCBRFx ni el de UCBRSx, ja que és 0 (la divisió és entera)

    //Configurem els pins de la UART
    //Configurem els pins com a primera funció alternativa
    P3SEL0 |= BIT2 | BIT3; //I/O funció: P1.3 = UART0TX, P1.2 = UART0RX
    P3SEL1 &= ~ (BIT2 | BIT3);
    UCA2CTLW0 &= ~UCSWRST; //Reactivem la línia de comunicacions sèrie
    //Treballem amb l'UART, que és asíncron, per tant és l'interfície A (número 0).
    EUSCI_A2->IFG &= ~EUSCI_A_IFG_RXIFG; // Clear eUSCI RX interrupt flag
    EUSCI_A2->IE |= EUSCI_A_IE_RXIE; // Enable USCI_A0 RX interrupt, nomes quan tinguem la recepcio
}



void init_direction_port(){
    //Configurem el DIRECTION_PORT
    //Configurem el P3.0 com a GPIO
    P3SEL0 &= ~(BIT0);
    P3SEL1 &= ~(BIT0);

    P3DIR |= (BIT0);      //El DIRECTION_PORT es una sortida
    P3OUT &= ~(BIT0); //Podem el mode "segur", l'UART està rebent dades
}

void init_joysticks(){
    /*
    //Configurem els joystick esquerra i dreta (P4.5 I P4.7)
    P4SEL0 &= ~(BIT5 + BIT7 );    //Els joystick son GPIOs
    P4SEL1 &= ~(BIT5 + BIT7 );    //Els joystick son GPIOs

    P4DIR &= ~(BIT5 + BIT7);    //Un joystick es una entrada
    P4REN |= (BIT5 + BIT7);     //Pull-up/pull-down pel joystick
    P4OUT |= (BIT5 + BIT7); //Donat que l'altra costat es GND, volem una pull-up
    P4IE |= (BIT5 + BIT7);      //Interrupcions activades
    P4IES &= ~(BIT5 + BIT7);    // amb transicio L->H
    P4IFG = 0;                  // Netegem les interrupcions anteriors
    */

    //Configurem els joystick amunt i avall (P5.4 I P5.5)
    P5SEL0 &= ~(BIT4);    //Els joystick son GPIOs
    P5SEL1 &= ~(BIT4);    //Els joystick son GPIOs

    P5DIR &= ~(BIT4);    //Un joystick es una entrada
    P5REN |= (BIT4);     //Pull-up/pull-down pel joystick
    P5OUT |= (BIT4); //Donat que l'altra costat es GND, volem una pull-up
    P5IE |= (BIT4);      //Interrupcions activades
    P5IES &= ~(BIT4);    // amb transicio L->H
    P5IFG = 0;                  // Netegem les interrupcions anteriors

    //Int. port 4 i 5, que correspon al bit 6 i bit 7 del segon registre ISER1:
    NVIC->ICPR[1] |= BIT7; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente,
    NVIC->ISER[1] |= BIT7; //y habilito las interrupciones del puerto

}

void funcionament_robot(){
    uint16_t i = 0, timeOut = 175;
    int trobada = 0;
    while(estat_robot == 1 && i < timeOut && !trobada){
        trobada = busquem_paret_girant(VELOCITAT_INICI);
        i++;
    }
    while(estat_robot ==1 && !trobada && !busquem_paret(VELOCITAT) ){
        delay(10);
    }

    while(estat_robot ==1 && !orientem_robot_esquerra(VELOCITAT)){
        delay(10);
    }


    while(estat_robot == 1){
        moviment_robot(VELOCITAT);

        delay(10);
    }
}

/**
 * main.c
 */
void main(void)
{
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;		// stop watchdog timer
	//Inicialitzacions
	init_ucs_24MHz();
	//Si el mode de funcionament és el real
	if(MODE_FUNCIONAMENT){
	    init_interrupciones_real();
	    init_UART_real();
	    init_joysticks();

	}
	else{
	    init_interrupciones_emulador();
        init_UART_emulador();
        canvi_mode_emulador();
	}

	halLcdInit();
	halLcdClearScreenBkg();
	init_direction_port();
	__enable_interrupt();



	//llegir_sensors();
	while(true){
        parar();
        llegir_sensors();
        if(estat_robot == 1){
            funcionament_robot();
        }
        delay(10);
	}




}
/*
//ISR para las interrupciones del puerto 4:
void PORT4_IRQHandler(void)
{
    uint8_t flag = P4IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P4IE &= ~(BIT5 + BIT7); //interrupcions dels joystick dreta i esquerra en port 4 desactivades

    interrupcioActivada = true;

    switch (flag)
    {
    case 0x0C://interrupció del P4.5
        estat = 4;
        break;
    case 0x10: //interrupció del P4.7
        estat = 3;
        break;
    default:
        break;
    }

    P4IE |= (BIT5 + BIT7);   //interrupciones reactivadas
}
*/

//ISR para las interrupciones del puerto 5:
void PORT5_IRQHandler(void)
{
    uint8_t flag = P5IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P5IE &= ~(BIT4); //interrupcions dels joystick amunt i avall en port 5 desactivades

    switch (flag)
    {
    case 0x0A://interrupció del P5.4 (el joystick amunt)
        estat_robot = (estat_robot+1)%2;
        break;

    default:
        break;
    }

    P5IE |= (BIT4);   //interrupciones reactivadas
}



