/////////////////////////////////////////////////////////////////////////
////                        ex_modbus_master.c                       ////
////                                                                 ////
////    This is a simple test program for a modbus master device.    ////
/////////////////////////////////////////////////////////////////////////
////                                                                 ////
//// Notes:                                                          ////
////                                                                 ////
//// Testing Example:                                                ////
//// To test the master code you will need a slave device loaded     ////
//// with the ex_modbus_slave.c program (Note that you can test it   ////
//// with PC software as well.  See Communicating with PC Software.) ////
//// After making the proper connections (refer to wiring diagram)   ////
//// you should power on the slave device and then power on the      ////
//// master device.  The master device should begin sending commands ////
//// out and showing the replies from the slave through the PC       ////
//// connection.                                                     ////
////                                                                 ////
//// Hardware UART:                                                  ////
//// If you use a hardware UART, make sure MODBUS_SERIAL_INT_SOURCE  ////
//// is set to MODBUS_INT_RDA or MODBUS_INT_RDA2 depending on which  ////
//// UART you are using.  Also, when using a hardware UART you do    //// 
//// not need to specify the TX and RX pins.  The driver figures     //// 
//// them out for you.                                               ////
////                                                                 ////
//// Communicating with PC software:                                 ////
//// To communicate with a PC, connect the board to your computer    ////
//// using the serial cable provided with your board.  Then          ////
//// uncomment the #define USE_WITH_PC 1 line.  In this              ////
//// configuration the PIC is the master and the PC is the slave.    ////
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
#fuses HS, NOWDT
#use delay(clock=4M)
#else
#include <16f690.h>
#fuses HS,NOWDT
#use delay(clock=20M)
#endif

#define MODBUS_TYPE MODBUS_TYPE_MASTER
#define MODBUS_SERIAL_TYPE MODBUS_RTU     //use MODBUS_ASCII for ASCII mode
#define MODBUS_SERIAL_RX_BUFFER_SIZE 64
#define MODBUS_SERIAL_BAUD 2400

#ifndef USE_WITH_PC
#use rs232(baud=9600, UART1, stream=PC, errors)
#define MODBUS_SERIAL_INT_SOURCE MODBUS_INT_EXT
#if defined(__PCD__)
#define MODBUS_SERIAL_TX_PIN PIN_D8   // Data transmit pin
#define MODBUS_SERIAL_RX_PIN PIN_F6   // Data receive pin
//The following should be defined for RS485 communication
//#define MODBUS_SERIAL_ENABLE_PIN   0   // Controls DE pin for RS485
//#define MODBUS_SERIAL_RX_ENABLE    0   // Controls RE pin for RS485
#else
#define MODBUS_SERIAL_TX_PIN PIN_B1   // Data transmit pin
#define MODBUS_SERIAL_RX_PIN PIN_B0   // Data receive pin
//The following should be defined for RS485 communication
#define MODBUS_SERIAL_ENABLE_PIN   PIN_B2   // Controls DE pin for RS485
#define MODBUS_SERIAL_RX_ENABLE    0   // Controls RE pin for RS485
#endif
#define DEBUG_MSG(msg) fprintf(PC, msg)
#define DEBUG_DATA(msg,data) fprintf(PC, msg, data)
#else
#define MODBUS_SERIAL_INT_SOURCE MODBUS_INT_EXT
#define DEBUG_MSG(msg) if(0)
#define DEBUG_DATA(msg,data) if(0)
#endif

#include <modbus.c>
#include "lcd.c"

#define MODBUS_SLAVE_ADDRESS 0x80

int i;

/*This function may come in handy for you since MODBUS uses MSB first.*/
int8 swap_bits(int8 c)
{
   return ((c&1)?128:0)|((c&2)?64:0)|((c&4)?32:0)|((c&8)?16:0)|((c&16)?8:0)
          |((c&32)?4:0)|((c&64)?2:0)|((c&128)?1:0);
}

void print_menu()
{
   DEBUG_MSG("\r\nPick command to send\r\n1. Read all coils.\r\n");
   DEBUG_MSG("2. Read all inputs.\r\n3. Read all holding registers.\r\n");
   DEBUG_MSG("4. Read all input registers.\r\n5. Turn coil 6 on.\r\n6. ");
   DEBUG_MSG("Write 0x4444 to register 0x03\r\n7. Set 8 coils using 0x50 as mask\r\n");
   DEBUG_MSG("8. Set 2 registers to 0x1111, 0x2222\r\n9. Send unknown command\r\n");
}

void read_all_coils()
{
   DEBUG_MSG("Coils:\r\n");
   if(!(modbus_read_coils(MODBUS_SLAVE_ADDRESS,0,8)))
   {
      DEBUG_MSG("Data: ");
      /*Started at 1 since 0 is quantity of coils*/
      for(i=1; i < (modbus_rx.len); ++i)
         DEBUG_DATA("%X ", modbus_rx.data[i]);
      DEBUG_MSG("\r\n\r\n");
   }
   else
   {
      DEBUG_DATA("<-**Exception %X**->\r\n\r\n", modbus_rx.error);
   }
}

void read_all_inputs()
{
   DEBUG_MSG("Inputs:\r\n");
   if(!(modbus_read_discrete_input(MODBUS_SLAVE_ADDRESS,0,8)))
   {
      DEBUG_MSG("Data: ");
      /*Started at 1 since 0 is quantity of coils*/
      for(i=1; i < (modbus_rx.len); ++i)
         DEBUG_DATA("%X ", modbus_rx.data[i]);
      DEBUG_MSG("\r\n\r\n");
   }
   else
   {
      DEBUG_DATA("<-**Exception %X**->\r\n\r\n", modbus_rx.error);
   }
}

void read_all_holding()
{
   DEBUG_MSG("Holding Registers:\r\n");
   if(!(modbus_read_holding_registers(MODBUS_SLAVE_ADDRESS,0,8)))
   {
      DEBUG_MSG("Data: ");
      /*Started at 1 since 0 is quantity of coils*/
      for(i=1; i < (modbus_rx.len); ++i)
         DEBUG_DATA("%X ", modbus_rx.data[i]);
      DEBUG_MSG("\r\n\r\n");
   } 
   else
   {
      DEBUG_DATA("<-**Exception %X**->\r\n\r\n", modbus_rx.error);
   }
}

void read_all_input_reg()
{
   DEBUG_MSG("Input Registers:\r\n");
   if(!(modbus_read_input_registers(MODBUS_SLAVE_ADDRESS,0,8)))
   {
      DEBUG_MSG("Data: ");
      /*Started at 1 since 0 is quantity of coils*/
   		lcd_gotoxy(1,1);	
   		printf(lcd_putc,"data array:             ");
   		lcd_gotoxy(1,2);
      for(i=1; i < (modbus_rx.len); ++i){
         DEBUG_DATA("%X ", modbus_rx.data[i]);
		 printf(lcd_putc,"%X",modbus_rx.data[i]);
      } 
       DEBUG_MSG("\r\n\r\n");

   }
   else
   {
      DEBUG_DATA("<-**Exception %X**->\r\n\r\n", modbus_rx.error);
   }
}

void write_coil()
{
   DEBUG_MSG("Writing Single Coil:\r\n");
   if(!(modbus_write_single_coil(MODBUS_SLAVE_ADDRESS,6,TRUE)))
   {
      DEBUG_MSG("Data: ");
      for(i=0; i < (modbus_rx.len); ++i)
         DEBUG_DATA("%X ", modbus_rx.data[i]);
      DEBUG_MSG("\r\n\r\n");
   }
   else
   {
      DEBUG_DATA("<-**Exception %X**->\r\n\r\n", modbus_rx.error);
   }   
}

void write_reg()
{
   DEBUG_MSG("Writing Single Register:\r\n");
   if(!(modbus_write_single_register(MODBUS_SLAVE_ADDRESS,3,0x4444)))
   {
      DEBUG_MSG("Data: ");
      for(i=0; i < (modbus_rx.len); ++i)
         DEBUG_DATA("%X ", modbus_rx.data[i]);
      DEBUG_MSG("\r\n\r\n");
   }
   else
   {
      DEBUG_DATA("<-**Exception %X**->\r\n\r\n", modbus_rx.error);
   }
}

void write_coils()
{
   int8 coils[1] = { 0x50 };
   DEBUG_MSG("Writing Multiple Coils:\r\n");
   if(!(modbus_write_multiple_coils(MODBUS_SLAVE_ADDRESS,0,8,coils)))
   {
      DEBUG_MSG("Data: ");
      for(i=0; i < (modbus_rx.len); ++i)
         DEBUG_DATA("%X ", modbus_rx.data[i]);
      DEBUG_MSG("\r\n\r\n");
   }
   else
   {
      DEBUG_DATA("<-**Exception %X**->\r\n\r\n", modbus_rx.error);
   }   
}

void write_regs()
{
   int16 reg_array[2] = {0x1111, 0x2222};
   DEBUG_MSG("Writing Multiple Registers:\r\n");
   if(!(modbus_write_multiple_registers(MODBUS_SLAVE_ADDRESS,0,2,reg_array)))
   {
      DEBUG_MSG("Data: ");
      for(i=0; i < (modbus_rx.len); ++i)
         DEBUG_DATA("%X ", modbus_rx.data[i]);
      DEBUG_MSG("\r\n\r\n");
   }
   else
   {
      DEBUG_DATA("<-**Exception %X**->\r\n\r\n", modbus_rx.error);
   }   
}

void unknown_func()
{
   DEBUG_MSG("Trying unknown function\r\n");
   DEBUG_MSG("Diagnostic:\r\n");
   if(!(modbus_diagnostics(MODBUS_SLAVE_ADDRESS,0,0)))
   {
      DEBUG_MSG("Data:");
      for(i=0; i < (modbus_rx.len); ++i)
         DEBUG_DATA("%X ", modbus_rx.data[i]);
      DEBUG_MSG("\r\n\r\n");
   }
   else
   {
      DEBUG_DATA("<-**Exception %X**->\r\n\r\n", modbus_rx.error);
   }
}

void parse_read(char c)
{
   switch(c)
   {
      case '1':
         read_all_coils();
         break; 
      case '2':
         read_all_inputs();
         break; 
      case '3':
         read_all_holding();
         break; 
      case '4':
         read_all_input_reg();
         break; 
   }
}

void parse_write(char c)
{
   switch(c)
   {
      case '5':
         write_coil();
         break;            
      case '6':
         write_reg();
         break;
      case '7':
         write_coils();
         break; 
      case '8':
         write_regs();
         break; 
      case '9':
         unknown_func();
         break;
   }
}

void main()
{
   char c;
   lcd_init();
   	delay_ms(6);
   setup_adc_ports(NO_ANALOGS);
   
   DEBUG_MSG("\r\nInitializing...");
   modbus_init();
   DEBUG_MSG("...ready\r\n");

   lcd_gotoxy(1,1);	
   printf(lcd_putc,"Master                ");
   lcd_gotoxy(1,2);	
   printf(lcd_putc,"PIC Modbus            ");
 		delay_ms(5000);

#ifndef USE_WITH_PC
   do{
      print_menu();
      c = getc(PC);
      
      fprintf(PC,"\r\n");
      parse_read(c);
      parse_write(c);  //Split between two functions due to segment size issues
      
   } while(TRUE);
#else
   read_all_coils();
   read_all_inputs();
   read_all_holding();
   read_all_input_reg();
   write_coils();
   write_regs();
   write_coil();
   write_reg();
   read_all_coils();
   read_all_holding();
   unknown_func();
#endif

}
