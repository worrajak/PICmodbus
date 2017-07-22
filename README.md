# PICmodbus PIC-CCS code 
PIC Microcontroller Master and Slave Modbus RTU Protocol 

ISIS Proteus file [https://github.com/worrajak/PICmodbus/blob/master/ModBus.DSN]

![ScreenShot](https://github.com/worrajak/PICmodbus/blob/master/Modbus002.jpg?raw=true)

1.Modbus master 

PIC18F458 configuration 

```
#include <18f458.h>
#fuses HS, NOWDT
#use delay(clock=20M)

#define MODBUS_TYPE MODBUS_TYPE_MASTER
#define MODBUS_SERIAL_TYPE MODBUS_RTU     //use MODBUS_ASCII for ASCII mode
#define MODBUS_SERIAL_RX_BUFFER_SIZE 64
```
SDM120 or modbus BuadRate to 2400bps 

```
#define MODBUS_SERIAL_BAUD 2400
```

using EXT INT RB0 (PIC18F458) 

```
#define MODBUS_SERIAL_INT_SOURCE MODBUS_INT_EXT
```
and config pin 

```
#define MODBUS_SERIAL_TX_PIN PIN_B1   // Data transmit pin
#define MODBUS_SERIAL_RX_PIN PIN_B0   // Data receive pin

#define MODBUS_SERIAL_ENABLE_PIN   PIN_B2   // Controls DE pin for RS485
#define MODBUS_SERIAL_RX_ENABLE    0   // Controls RE pin for RS485
```
![ScreenShot](https://github.com/worrajak/PICmodbus/blob/master/ModBus003.jpg?raw=true)

![ScreenShot](https://github.com/worrajak/PICmodbus/blob/master/ModBus005.jpg?raw=true)

2. Slave modbus PIC16F690 

```
#include <16F690.h>
#DEVICE  *=16 ADC=10
#fuses   NOPROTECT,INTRC,NOWDT,PUT,NOBROWNOUT,MCLR
#use delay(clock=4M)

#define MODBUS_TYPE MODBUS_TYPE_SLAVE
#define MODBUS_SERIAL_TYPE MODBUS_RTU     //use MODBUS_ASCII for ASCII mode
#define MODBUS_SERIAL_RX_BUFFER_SIZE 64
#define MODBUS_SERIAL_BAUD 2400
```
using EXT INT RDA (UART) (PIC18F458) 

```
#define MODBUS_SERIAL_INT_SOURCE MODBUS_INT_RDA
```
and config pin 

```
#define MODBUS_SERIAL_TX_PIN PIN_B7   // Data transmit pin
#define MODBUS_SERIAL_RX_PIN PIN_B5   // Data receive pin

#define MODBUS_SERIAL_ENABLE_PIN   PIN_B4   // Controls DE pin for RS485
#define MODBUS_SERIAL_RX_ENABLE    0    // Controls RE pin for RS485
```
![ScreenShot](https://github.com/worrajak/PICmodbus/blob/master/ModBus004.jpg?raw=true)

![ScreenShot](https://github.com/worrajak/PICmodbus/blob/master/Modbus001.jpg?raw=true)

