#include "lib_initTimers.h"
#include "lib_transmetreDades.h"


#define HORARI 1
#define ANTIHORARI 0
#define DISTANCE 5
#define N 100000


void canvi_mode_emulador();

byte girar_motor(byte id_motor, byte direction, uint16_t speed);

void endavant(uint16_t speed);

void endarrere(uint16_t speed);

void girar_dreta(uint16_t speed);

void girar_esquerra(uint16_t speed);

void parar();

RxReturn llegir_sensors();

void moviment_robot(uint16_t speed);

int busquem_paret(uint16_t speed);

int orientem_robot_dreta(uint16_t speed);

int orientem_robot_esquerra(uint16_t speed);






