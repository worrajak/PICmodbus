/////////////////////////////////////////////////////////////////////////
////                         ex_modbus_slave.c                       ////
////                                                                 ////
////    This is a simple test program for a modbus slave device.     ////
/////////////////////////////////////////////////////////////////////////
////                                                                 ////
//// Notes:                                                          ////
////                                                                 ////
//// Testing Example:                                                ////
//// To test the slave code you will need a master device loaded     ////
//// with the ex_modbus_master.c program (Note that you can test it  ////
//// with PC software as well.  See Communicating with PC Software.) ////
//// After making the proper connections (refer to wiring diagram)   ////
//// you should power on the slave device and then power on the      ////
//// master device.  The master device should begin sending commands ////
//// out and showing the replies from the slave through the PC       ////
//// connection.                                                     ////
////                                                                 ////
//// Hardware UART:                                                  ////
//// If you use a hardware UART, make sure MODBUS_SERIAL_INT_SOURCE  ////
//// is set to MODBUS_INT_RDA, MODBUS_INT_RDA2, MODBUS_INT_RDA3, or  ////
//// MODBUS_INT_RDA4 depending on which UART you are using.  Also,   ////
//// when using a hardware UART you do not need to specify the TX    ////
//// and RX pins.  The driver figures them out for you.              ////
////                                                                 ////
//// Communicating with PC software:                                 ////
//// To communicate with a PC, connect the board to your computer    ////
//// using the serial cable provided with your board.  Then          ////
//// uncomment the #define USE_WITH_PC 1 line.  In this              ////
//// configuration the PC is the master and the PIC is the slave.    ////
////                                                                 ////
//// Troubleshooting:                                                ////
//// If the device seems unresponsive, power cycle the board and     ////
//// wait 5 seconds.                                                 ////
////                                                                 ////
//// Wiring Diagram:                                                 ////
//// This is the diagram for the default configuration.  Note that   //// 
//// the external interrupt (INT), PIN B0, is the only pin that can  ////
//// be used for receiving with software RS232 for PCM and PCH, and  ////
//// for PCD the exteranl interrupt (INT0), usually PIN F6, is the   ////
//// only pin that can be used for receiving with software RS232.    ////
////                                                                 ////
////            PCH and PCM                        PCD               ////
////    ----------     ----------       ----------     ----------    ////
////    |        |     |        |       |        |     |        |    ////
////    | Master |     | Slave  |       | Master |     | Slave  |    ////
////    |        |     |        |       |        |     |        |    ////
////    |      B1|---->|B0      |       |      D8|---->|F6      |    ////
////    |      B0|<----|B1      |       |      F6|<----|D8      |    ////
////    ----------     ----------       ----------     ----------    ////
////                                                                 ////
/////////////////////////////////////////////////////////////////////////
////        (C) Copyright 1996, 2006 Custom Computer Services        ////
//// This source code may only be used by licensed users of the CCS  ////
//// C compiler.  This source code may only be distributed to other  ////
//// licensed users of the CCS C compiler.  No other use,            ////
//// reproduction or distribution is permitted without written       ////
//// permission.  Derivative programs created using this software    ////
//// in object code form are not restricted in any way.              ////
/////////////////////////////////////////////////////////////////////////

//#define USE_WITH_PC 1

#if defined(__PCD__)
#include <24FJ128GA006.h>
#fuses PR,HS,NOWDT
#use delay(clock=20M)
#elif defined(__PCH__)
#include <18f458.h>
#DEVICE  *=16 ADC=10
#fuses HS, NOWDT
#use delay(clock=20M)
#else
#include <16F690.h>
#DEVICE  *=16 ADC=10
#fuses   NOPROTECT,INTRC,NOWDT,PUT,NOBROWNOUT,MCLR
//#fuses XT,NOWDT
#use delay(clock=4M)
#endif


//#include "lcd.c"

#define MODBUS_TYPE MODBUS_TYPE_SLAVE
#define MODBUS_SERIAL_TYPE MODBUS_RTU     //use MODBUS_ASCII for ASCII mode
#define MODBUS_SERIAL_RX_BUFFER_SIZE 64
#define MODBUS_SERIAL_BAUD 2400

#ifndef USE_WITH_PC
#define MODBUS_SERIAL_INT_SOURCE MODBUS_INT_RDA	//MODBUS_INT_EXT
#if defined(__PCD__)
#define MODBUS_SERIAL_TX_PIN PIN_D8   // Data transmit pin
#define MODBUS_SERIAL_RX_PIN PIN_F6   // Data receive pin
//The following should be defined for RS485 communication
//#define MODBUS_SERIAL_ENABLE_PIN   0   // Controls DE pin for RS485
//#define MODBUS_SERIAL_RX_ENABLE    0   // Controls RE pin for RS485
#else
#define MODBUS_SERIAL_TX_PIN PIN_B7   // Data transmit pin
#define MODBUS_SERIAL_RX_PIN PIN_B5   // Data receive pin
//The following should be defined for RS485 communication
#define MODBUS_SERIAL_ENABLE_PIN   PIN_B4   // Controls DE pin for RS485
#define MODBUS_SERIAL_RX_ENABLE    0    // Controls RE pin for RS485
#endif
#else
#define MODBUS_SERIAL_INT_SOURCE MODBUS_INT_RDA
#endif

#include <modbus.c>

#define MODBUS_ADDRESS 0x80

/*This function may come in handy for you since MODBUS uses MSB first.*/
int8 swap_bits(int8 c)
{
   return ((c&1)?128:0)|((c&2)?64:0)|((c&4)?32:0)|((c&8)?16:0)|((c&16)?8:0)
          |((c&32)?4:0)|((c&64)?2:0)|((c&128)?1:0);
}

void main()
{
   int8 coils = 0b00000111;
   int8 inputs = 0b00001001;
   int16 hold_regs[] = {0x8800,0x7700,0x6600,0x5500,0x4400,0x3300,0x2200,0x1100};
   int16 input_regs[] = {0x1100,0x2200,0x3300,0x4400,0x5500,0x6600,0x7700,0x8800};
   int16 event_count = 0;

   long int ADC_value0,ADC_value1,ADC_value2;

//   lcd_init();
//   	delay_ms(6);   

   	SETUP_ADC_PORTS(sAN2|sAN6|sAN7|sAN8|sAN9);
//   	setup_adc(ADC_CLOCK_INTERNAL);
//   setup_adc_ports( RA0_RA1_RA3_ANALOG );
   setup_adc(adc_clock_internal);
   
   modbus_init();

//   lcd_gotoxy(1,1);	
//   printf(lcd_putc,"Modbus Slave                  ");
//   lcd_gotoxy(1,2);	  
//   printf(lcd_putc,"addr 0x%X CH0-1                 ",MODBUS_ADDRESS);
//		delay_ms(2000);

   while(TRUE)
   {
	 set_adc_channel(2);
			delay_us(20);
     ADC_value0=read_adc();
     input_regs[0]=ADC_value0;

	 set_adc_channel(6);
			delay_us(20);
     ADC_value1=read_adc();
     input_regs[1]=ADC_value1;

	 set_adc_channel(7);
			delay_us(20);
     ADC_value2=read_adc();
     input_regs[2]=ADC_value2;

    while(!modbus_kbhit());
     
      delay_us(50);
      
      //check address against our address, 0 is broadcast
      if((modbus_rx.address == MODBUS_ADDRESS) || modbus_rx.address == 0)
      {
         switch(modbus_rx.func)
         {
            case FUNC_READ_COILS:    //read coils
            case FUNC_READ_DISCRETE_INPUT:    //read inputs
               if(modbus_rx.data[0] || modbus_rx.data[2] ||
                  modbus_rx.data[1] >= 8 || modbus_rx.data[3]+modbus_rx.data[1] > 8)
                  modbus_exception_rsp(MODBUS_ADDRESS,modbus_rx.func,ILLEGAL_DATA_ADDRESS);
               else
               {
                  int8 data;
                  
                  if(modbus_rx.func == FUNC_READ_COILS)
                     data = coils>>(modbus_rx.data[1]);      //move to the starting coil
                  else
                     data = inputs>>(modbus_rx.data[1]);      //move to the starting input

                  data = data & (0xFF>>(8-modbus_rx.data[3]));  //0 out values after quantity

                  if(modbus_rx.func == FUNC_READ_COILS)
                     modbus_read_discrete_input_rsp(MODBUS_ADDRESS, 0x01, &data);
                  else
                     modbus_read_discrete_input_rsp(MODBUS_ADDRESS, 0x01, &data);
                     
                  event_count++;
               }
               break;
            case FUNC_READ_HOLDING_REGISTERS:
            case FUNC_READ_INPUT_REGISTERS:
               if(modbus_rx.data[0] || modbus_rx.data[2] ||
                  modbus_rx.data[1] >= 8 || modbus_rx.data[3]+modbus_rx.data[1] > 8)
                  modbus_exception_rsp(MODBUS_ADDRESS,modbus_rx.func,ILLEGAL_DATA_ADDRESS);
               else
               {
                  if(modbus_rx.func == FUNC_READ_HOLDING_REGISTERS)
                     modbus_read_holding_registers_rsp(MODBUS_ADDRESS,(modbus_rx.data[3]*2),hold_regs+modbus_rx.data[1]);
                  else
                     modbus_read_input_registers_rsp(MODBUS_ADDRESS,(modbus_rx.data[3]*2),input_regs+modbus_rx.data[1]);
                  
                  event_count++;
               }
               break;
            case FUNC_WRITE_SINGLE_COIL:      //write coil
               if(modbus_rx.data[0] || modbus_rx.data[3] || modbus_rx.data[1] > 8)
                  modbus_exception_rsp(MODBUS_ADDRESS,modbus_rx.func,ILLEGAL_DATA_ADDRESS);
               else if(modbus_rx.data[2] != 0xFF && modbus_rx.data[2] != 0x00)
                  modbus_exception_rsp(MODBUS_ADDRESS,modbus_rx.func,ILLEGAL_DATA_VALUE);
               else
               {
                  //coils are stored msb->lsb so we must use 7-address
                  if(modbus_rx.data[2] == 0xFF)
                     bit_set(coils,modbus_rx.data[1]);
                  else
                     bit_clear(coils,modbus_rx.data[1]);

                  modbus_write_single_coil_rsp(MODBUS_ADDRESS,modbus_rx.data[1],((int16)(modbus_rx.data[2]))<<8);
                  
                  event_count++;
               }
               break;
            case FUNC_WRITE_SINGLE_REGISTER:
               if(modbus_rx.data[0] || modbus_rx.data[1] >= 8)
                  modbus_exception_rsp(MODBUS_ADDRESS,modbus_rx.func,ILLEGAL_DATA_ADDRESS);
               else
               {
                  //the registers are stored in little endian format
                  hold_regs[modbus_rx.data[1]] = make16(modbus_rx.data[3],modbus_rx.data[2]);

                  modbus_write_single_register_rsp(MODBUS_ADDRESS,
                               make16(modbus_rx.data[0],modbus_rx.data[1]),
                               make16(modbus_rx.data[2],modbus_rx.data[3]));
               }
               break;
            case FUNC_WRITE_MULTIPLE_COILS:
               if(modbus_rx.data[0] || modbus_rx.data[2] ||
                  modbus_rx.data[1] >= 8 || modbus_rx.data[3]+modbus_rx.data[1] > 8)
                  modbus_exception_rsp(MODBUS_ADDRESS,modbus_rx.func,ILLEGAL_DATA_ADDRESS);
               else
               {
                  int i,j;

                  modbus_rx.data[5] = swap_bits(modbus_rx.data[5]);

                  for(i=modbus_rx.data[1],j=0; i < modbus_rx.data[1]+modbus_rx.data[3]; ++i,++j)
                  {  
                     if(bit_test(modbus_rx.data[5],j))
                        bit_set(coils,7-i);
                     else
                        bit_clear(coils,7-i);
                  }

                  modbus_write_multiple_coils_rsp(MODBUS_ADDRESS,
                                 make16(modbus_rx.data[0],modbus_rx.data[1]),
                                 make16(modbus_rx.data[2],modbus_rx.data[3]));
                  
                  event_count++;
               }
               break;
            case FUNC_WRITE_MULTIPLE_REGISTERS:
               if(modbus_rx.data[0] || modbus_rx.data[2] ||
                  modbus_rx.data[1] >= 8 || modbus_rx.data[3]+modbus_rx.data[1] > 8)
                  modbus_exception_rsp(MODBUS_ADDRESS,modbus_rx.func,ILLEGAL_DATA_ADDRESS);
               else
               {
                  int i,j;

                  for(i=0,j=5; i < modbus_rx.data[4]/2; ++i,j+=2)
                     hold_regs[i] = make16(modbus_rx.data[j+1],modbus_rx.data[j]);

                  modbus_write_multiple_registers_rsp(MODBUS_ADDRESS,
                                 make16(modbus_rx.data[0],modbus_rx.data[1]),
                                 make16(modbus_rx.data[2],modbus_rx.data[3]));
               
                  event_count++;
               }
               break;           
            default:    //We don't support the function, so return exception
               modbus_exception_rsp(MODBUS_ADDRESS,modbus_rx.func,ILLEGAL_FUNCTION);
         }
      }
//   	lcd_gotoxy(1,1);	
//   	printf(lcd_putc,"ADC0: %LX              ",ADC_value0);
//   	lcd_gotoxy(1,2);	
//  	printf(lcd_putc,"ADC1: %LX              ",ADC_value1);
  }
}
