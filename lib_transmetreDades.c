#include "lib_transmetreDades.h"

//Mode_funcionament 1 és que treballem amb la versió real
//Mode_funcionament 0 és que treballem amb la versió de l'emulador
//Per defecte serà 1;
volatile int mode_funcionament=1;

void mode_emulador(){
    mode_funcionament = 0;
}

/* funcions per canviar el sentit de les comunicacions */
void Sentit_Dades_Rx(void)
{ //Configuració del Half Duplex dels motors: Recepció
    P3OUT &= ~BIT0; //El pin P3.0 (DIRECTION_PORT) el posem a 0 (Rx)
}

void Sentit_Dades_Tx(void)
{ //Configuració del Half Duplex dels motors: Transmissió
    P3OUT |= BIT0; //El pin P3.0 (DIRECTION_PORT) el posem a 1 (Tx)
}

/* funció TxUACx(byte): envia un byte de dades per la UART 0 */
void TxUACx(uint8_t bTxdData)
{
    //Cas en què treballem amb la versió real
    if(mode_funcionament){
        while(!TXD2_READY); // Espera a que estigui preparat el buffer de transmissió
        UCA2TXBUF = bTxdData; //posem el buffer la dada que volem enviar
    }
    else{
        while(!TXD0_READY); // Espera a que estigui preparat el buffer de transmissió
        UCA0TXBUF = bTxdData; //posem el buffer la dada que volem enviar
    }
}

//TxPacket() 3 paràmetres: ID del Dynamixel, Mida dels paràmetres, Instruction byte. torna la mida del "Return packet"
byte TxPacket(byte bID, byte bParameterLength, byte bInstruction, byte Parametros[16])
{
    byte bCount,bCheckSum,bPacketLength;
    byte TxBuffer[32];
    Sentit_Dades_Tx(); //El pin P3.0 (DIRECTION_PORT) el posem a 1 (Transmetre)
    TxBuffer[0] = 0xff; //Primers 2 bytes que indiquen inici de trama FF, FF.
    TxBuffer[1] = 0xff;
    TxBuffer[2] = bID; //ID del mòdul al que volem enviar el missatge
    TxBuffer[3] = bParameterLength+2; //Length(Parameter,Instruction,Checksum)
    TxBuffer[4] = bInstruction; //Instrucció que enviem al Mòdul
    char error[] = "adr. no permitida";

    //Afegim codi per no escriure en les primeres 6 posicions
    //Parametros[0] ens indica on volem començar a escriure a l'EEPROM
    if ((Parametros[0] < 6) && (bInstruction == 3)){//si se intenta escribir en una direccion <= 0x05,
        //emitir mensaje de error de direccion prohibida:
          halLcdPrintLine(error, 8, INVERT_TEXT);
          //y salir de la funcion sin mas:
          return 0;
    }

    for(bCount = 0; bCount < bParameterLength; bCount++) //Comencem a generar la trama que hem d’enviar
    {
        TxBuffer[bCount+5] = Parametros[bCount];
    }
    bCheckSum = 0;
    bPacketLength = bParameterLength+4+2;
    for(bCount = 2; bCount < bPacketLength-1; bCount++) //Càlcul del checksum
    {
        bCheckSum += TxBuffer[bCount];
    }
    TxBuffer[bCount] = ~bCheckSum; //Escriu el Checksum (complement a 1)
    for(bCount = 0; bCount < bPacketLength; bCount++) //Aquest bucle és el que envia la trama al Mòdul Robot
    {
        TxUACx(TxBuffer[bCount]);
    }
    while( (UCA2STATW & UCBUSY)); //Espera fins que s’ha transmès el últim byte
    Sentit_Dades_Rx(); //Posem la línia de dades en Rx perquè el mòdul Dynamixel envia resposta
    return(bPacketLength);
}

/* Aquest exemple no és complert, en principi RxPacket() torna una estructura“Status packet" que bàsicament
consisteix en un array amb “Status Packet”+ un byte indicant si hi ha un TimeOut. Això s’ha fet així perquè en C no
es pot posar un array com paràmetre de tornada.
Per altre banda, la part mostrada només llegeix els primers 4 bytes del status packet. Això és perquè el quart byte
indica precisament quants bytes queden per llegir, el que vol dir que s’ha de fer un altre bucle “for” semblant per
llegir els bytes que falten....
Evidentment, es podria fer d’altres maneres, per exemple enviant com paràmetre el número de bytes a llegir...
*/
RxReturn RxPacket(void)
{
    RxReturn respuesta;
    byte bCount, bLenght, bChecksum=0;
    byte Rx_time_out=0;
    Sentit_Dades_Rx(); //Ponemos la linea half duplex en Rx
    Activa_TimerA1_TimeOut();
    respuesta.timeOut = 0;
    char error[] = "error en el Status Packet";

    for(bCount = 0; bCount < 2; bCount++) //bRxPacketLength; bCount++)
    {
        Reset_Timeout();
        Byte_Recibido=0; //No_se_ha_recibido_Byte();
        while (!Byte_Recibido) //Se_ha_recibido_Byte())
        {
            Rx_time_out=TimeOut(1000); // tiempo en decenas de microsegundos
            if (Rx_time_out)break;//sale del while
            }
        if (Rx_time_out)break; //sale del for si ha habido Timeout
        //Si no, es que todo ha ido bien, y leemos un dato:


        respuesta.StatusPacket[bCount] = DatoLeido_UART; //Get_Byte_Leido_UART();
    }//fin del for
    //Des d'aquí comencem a calcular el checksum
    if (!Rx_time_out){

        for(bCount = 2; bCount < 4; bCount++) //bRxPacketLength; bCount++)
        {
            Reset_Timeout();
            Byte_Recibido=0; //No_se_ha_recibido_Byte();
            while (!Byte_Recibido) //Se_ha_recibido_Byte())
            {
                Rx_time_out=TimeOut(1000); // tiempo en decenas de microsegundos
                if (Rx_time_out)break;//sale del while
                }
            if (Rx_time_out)break; //sale del for si ha habido Timeout
            //Si no, es que todo ha ido bien, y leemos un dato:


            respuesta.StatusPacket[bCount] = DatoLeido_UART; //Get_Byte_Leido_UART();
            bChecksum += respuesta.StatusPacket[bCount];
        }//fin del for

    }

    if (!Rx_time_out){
        Reset_Timeout();
        Byte_Recibido=0; //No_se_ha_recibido_Byte();
        while (!Byte_Recibido) //Se_ha_recibido_Byte())
        {
            Rx_time_out=TimeOut(1000); // tiempo en decenas de microsegundos
            if (Rx_time_out)break;//sale del while
        }
        //Si no, es que todo ha ido bien, y leemos un dato:
        respuesta.StatusPacket[4] = DatoLeido_UART;
        if(respuesta.StatusPacket[4]!=0){
            halLcdPrintLine(error, 8, INVERT_TEXT);
            Rx_time_out = 1;
        }
        bChecksum+=respuesta.StatusPacket[4];
    }
    if(!Rx_time_out){
        //Accedim a la llargada del paquet
        bLenght = respuesta.StatusPacket[3];
        //Accedim a la resta de posicions
        for(bCount = 5; bCount < bLenght + 4; bCount++)
        {
            Reset_Timeout();
            Byte_Recibido=0; //No_se_ha_recibido_Byte();
            while (!Byte_Recibido) //Se_ha_recibido_Byte())
            {
                Rx_time_out=TimeOut(1000); // tiempo en decenas de microsegundos
                if (Rx_time_out)break;//sale del while
                }
            if (Rx_time_out)break; //sale del for si ha habido Timeout
            //Si no, es que todo ha ido bien, y leemos un dato:

            respuesta.StatusPacket[bCount] = DatoLeido_UART; //Get_Byte_Leido_UART();
            bChecksum += respuesta.StatusPacket[bCount];
        }//fin del for
        //Si hi ha hagut time out
        if(Rx_time_out){
            respuesta.timeOut = 1;
        }
        else{
            //Restem el checksum que hem inclòs a la suma
            bChecksum -= respuesta.StatusPacket[bLenght+3];
            bChecksum = ~bChecksum;
            if(bChecksum != respuesta.StatusPacket[bLenght+3]){
                //Hi ha un error en el checksum
                respuesta.timeOut = 1;
            }

        }
        return respuesta;
    }

    respuesta.timeOut = 1;

    return respuesta;

}


void EUSCIA0_IRQHandler()
{ //interrupcion de recepcion en la UART A0
    EUSCI_A0->IFG &=~ EUSCI_A_IFG_RXIFG; // Clear interrupt
    UCA0IE &= ~UCRXIE; //Interrupciones desactivadas en RX
    DatoLeido_UART = UCA0RXBUF;
    Byte_Recibido=1;
    UCA0IE |= UCRXIE; //Interrupciones reactivadas en RX
}

void EUSCIA2_IRQHandler()
{ //interrupcion de recepcion en la UART A0
    EUSCI_A2->IFG &=~ EUSCI_A_IFG_RXIFG; // Clear interrupt
    UCA2IE &= ~UCRXIE; //Interrupciones desactivadas en RX
    DatoLeido_UART = UCA2RXBUF;
    Byte_Recibido=1;
    UCA2IE |= UCRXIE; //Interrupciones reactivadas en RX
}


