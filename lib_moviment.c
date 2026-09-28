#include "lib_moviment.h"

//Valors per defecte
volatile int motor_esquerre=3;
volatile int motor_dret=2;
volatile int sensor = 100;

void canvi_mode_emulador(){
    motor_esquerre=1;
    motor_dret=2;
    sensor=3;
    mode_emulador();

}

byte girar_motor(byte id_motor, byte direction, uint16_t speed){
    byte goal_speed[3];
    //Volem començar a escriure a la posició 32
    goal_speed[0] = 32;
    //Escrivim la velocitat desitjada (té 16 bits per tant necessitem 2 bytes)
    goal_speed[1] = (uint8_t) speed & 0xFF;
    goal_speed[2] = ((direction << 2) & 0x04) | ((speed >> 8) & 0x03);
    //Fem que l'UART enviï la instrucció write (id 3), que té 3 paràmetres
    TxPacket(id_motor,3,3,goal_speed);

    //Definim un RxReturn per veure si s'ha enviat bé la instrucció
    RxReturn r = RxPacket();
    //Anem enviant el TxPacket fins que rebem bé el Status Packet
    while(r.timeOut){
        TxPacket(id_motor,3,3,goal_speed);
        r = RxPacket();
    }

}

void endavant(uint16_t speed){
    //Fem que la roda esquerra es mogui en sentit antihorari
    girar_motor(motor_esquerre,ANTIHORARI,speed);

    //Fem que la roda dreta es mogui en sentit horari
    girar_motor(motor_dret,HORARI,speed);

}

void endarrere(uint16_t speed){
    //Fem el mateix que endavant() però amb els sentits contraris
    //Fem que la roda esquerra es mogui en sentit horari
    girar_motor(motor_esquerre,HORARI,speed);
    //Fem que la roda dreta es mogui en sentit antihorari
    girar_motor(motor_dret,ANTIHORARI,speed);

}



void girar_dreta(uint16_t speed){
    //Fem que les dues rodes es moguin en el mateix sentit
    girar_motor(motor_esquerre,ANTIHORARI,speed);

    girar_motor(motor_dret,ANTIHORARI,speed);

}

void girar_esquerra(uint16_t speed){
    //Fem que les dues rodes es moguin en el mateix sentit, al revés que girar_dreta
    girar_motor(motor_esquerre,HORARI,speed);

    girar_motor(motor_dret,HORARI,speed);

}

void parar(){
    endavant(0);
}

RxReturn llegir_sensors(){
    //Llegim els sensors, per tant hem de posar el id del sensor, la instrucció READ té 2 paràmetres, i el id de funció Read és 2
    //Hem de començar a llegir des de la posició 1A, i seran en total 3 posicions (3 bytes), ja que volem llegir els 3 sensors alhora.
    byte Parametros[16];
    Parametros[0] = 26; //1C en hexadecimal
    Parametros[1] = 3;
    TxPacket(sensor,2,2,Parametros);
    RxReturn resultat = RxPacket();

    //Anem enviant el TxPacket fins que rebem bé el Status Packet
    while(resultat.timeOut){
        TxPacket(sensor,2,2,Parametros);
        resultat = RxPacket();
    }
    char sensor1[20];
    char sensor2[20];
    char sensor3[20];
    //halLcdClearScreenBkg();
    halLcdClearLine(1);
    halLcdClearLine(3);
    halLcdClearLine(5);
    sprintf(sensor1, "Sensor e: %u", resultat.StatusPacket[5]);
    sprintf(sensor2, "Sensor m: %u", resultat.StatusPacket[6]);
    sprintf(sensor3, "Sensor d: %u", resultat.StatusPacket[7]);



    halLcdPrintLine(sensor1, 1, NORMAL_TEXT);

    halLcdPrintLine(sensor2, 3, NORMAL_TEXT);
    halLcdPrintLine(sensor3, 5, NORMAL_TEXT);

    return resultat;

}

void moviment_robot(uint16_t speed){
    RxReturn resultat = llegir_sensors();

    //Si el sensor de davant ens marca que tenim una paret aprop
    if(resultat.StatusPacket[6] > DISTANCE+10 ){
        //&& resultat.StatusPacket[6]<(DISTANCE+15)
        //Anem girant fins que haguem esquivat l'obstacle
        /*
        girar_esquerra(speed);
        delay(800);
        endavant(speed);
        delay(500);
        */

        while(resultat.StatusPacket[6]>0){
            girar_esquerra(speed);
            delay(10);
            resultat = llegir_sensors();
        }

    }
    else if(resultat.StatusPacket[5]> 0){
        while(resultat.StatusPacket[5]>0){
            girar_esquerra(speed);
            delay(10);
            resultat = llegir_sensors();
        }
    }
    //Estem aprop de la paret
    else if(resultat.StatusPacket[7] > DISTANCE && resultat.StatusPacket[7]<(DISTANCE+15)){

        endavant(speed);

    }
    //Si ens allunyem de la paret
    else if(resultat.StatusPacket[7] < DISTANCE ){
        girar_dreta(speed);
        delay(150);
        endavant(speed);
        delay(50);
    }
    //Si estem apunt de xocar
    else{
        girar_esquerra(speed);
        delay(10);
        //orientem_robot_esquerra(speed);
    }


    //En funció de la lectura mirarem quin moviment hem de fer.
    //Farem que el robot vagi cap endavant fins que es trobi algun obstacle.
    //A continuació mirarem al sensor esquerre i després el dret.
    //Per tant, començar amb la lectura del sensor del centre (posició 1B).
    //Els paràmetres estan en l'Status Packet a partir de la posició 6
    /*
    if(resultat.StatusPacket[6] < DISTANCE){
        endavant(speed);
    }
    else if (resultat.StatusPacket[5] < DISTANCE){
        girar_esquerra(speed);
    }
    else if(resultat.StatusPacket[7] < DISTANCE){
        girar_dreta(speed);
    }
    else{
        //En cas que estigui atrapat a una cantonada
        endarrere(speed);
        //parar();
    }
    */
}
int busquem_paret_girant(uint16_t speed){
    RxReturn resultat = llegir_sensors();

    //Al sensor del centre li posem menys distància perquè quan orientem el robot no ens allunyem tant de la paret
    //Ens anem movent endavant fins que detectem una paret
    if(resultat.StatusPacket[6] > DISTANCE+20 || resultat.StatusPacket[5] > DISTANCE || resultat.StatusPacket[7] > DISTANCE){
        //parar();
        return 1;
    }
    girar_esquerra(speed);
    return 0;

}

int busquem_paret(uint16_t speed){
    RxReturn resultat = llegir_sensors();

    //Al sensor del centre li posem menys distància perquè quan orientem el robot no ens allunyem tant de la paret
    //Ens anem movent endavant fins que detectem una paret
    if(resultat.StatusPacket[6] > DISTANCE+20 || resultat.StatusPacket[5] > DISTANCE || resultat.StatusPacket[7] > DISTANCE){
        //parar();
        return 1;
    }
    endavant(speed);
    return 0;

}

int orientem_robot_esquerra(uint16_t speed){
    RxReturn resultat = llegir_sensors();
    //Ara ens orientarem per anar en sentit anithorari
    if(resultat.StatusPacket[7] > DISTANCE){
        //parar();
        return 1;
    }
    girar_esquerra(speed);
    return 0;

}

int orientem_robot_dreta(uint16_t speed){
    RxReturn resultat = llegir_sensors();
    //Ens apropem a la paret però girant cap a la dreta
    if(resultat.StatusPacket[7] > DISTANCE){
        //parar();
        return 1;
    }
    girar_dreta(speed);
    return 0;

}




