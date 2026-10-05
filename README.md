# Robot Control System

Embedded C project for controlling an autonomous robot using an MSP432 microcontroller. The program supports two operating modes:

- **Real mode:** communicates with and controls the physical robot.
- **Emulator mode:** communicates with a robot emulator for development and testing.

The project uses UART communication, GPIO interrupts, timers, sensors, and movement-control routines to operate the robot.

## Features

- Selection between **real robot** and **emulator** operation.
- UART communication at **500 kbaud**.
- Robot start/stop control using a joystick.
- Sensor reading and wall detection.
- Automatic robot orientation and movement.
- Interrupt-driven joystick input.
- Timer-based delays and movement control.
- MSP432 clock configured at **24 MHz**.

## Project Structure

The main program depends on several project-specific libraries:

```text
.
├── main.c
├── lib_PAE.h
├── lib_initTimers.h
└── lib_moviment.h
```

## Libraries

The project is divided into three main libraries. Each library is responsible for a different part of the robot's operation:

- `lib_initTimers` → manages timers and timeouts.
- `lib_transmetreDades` → manages communication with the motors and sensors.
- `lib_moviment` → controls the robot's movement and autonomous behaviour.

The relationship between the libraries can be summarized as:

```text
                     main.c
                       │
                       ▼
                lib_moviment
             Movement and sensors
                       │
                       ▼
             lib_transmetreDades
          UART communication/packets
                       │
                       ▼
                 Robot devices
             ┌─────────┴─────────┐
             │                   │
           Motors             Sensors
             
                ▲
                │
         lib_initTimers
       Timing and timeouts
