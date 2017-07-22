/*
LCD 16x2 4bit communication

RD2 -> RS
RD3 -> E
RD4 -> D4
RD5 -> D5
RD6 -> D6
RD7 -> D7

*/

struct lcd_pin_map {
           boolean rw;
           boolean unused;
           boolean rs ;
	       boolean enable;
        int     data : 4;
        } lcd_4bit;


#byte lcd_4bit = 0xF83                          // on to port d (at address 8)

#define lcd_type 2           // 0=5x7, 1=5x10, 2=2 lines
#define lcd_line_two 0x40    // LCD RAM address for the second line


byte CONST LCD_INIT_STRING[4] = {0x20 | (lcd_type << 2), 0xc, 1, 6};
                             // These bytes need to be sent to the LCD
                             // to start it up.


                             // The following are used for setting
                             // the I/O port direction register.

STRUCT lcd_pin_map const LCD_WRITE = {0,0,0,0,0}; // For write mode all pins are out


void lcd_send_nibble( byte n ) {
      lcd_4bit.data = n;
      delay_cycles(100);
      lcd_4bit.enable = 1;
      delay_us(2);
      lcd_4bit.enable = 0;
}


void lcd_send_byte( byte address, byte n ) {
      lcd_4bit.rs = 0;
      lcd_4bit.rs = address;
      delay_cycles(100);
      lcd_4bit.rw = 0;
      delay_cycles(100);
      lcd_4bit.enable = 0;
      lcd_send_nibble(n >> 4);
      lcd_send_nibble(n & 0xf);
}


void lcd_init() {
    byte i;
   set_tris_d(LCD_WRITE);
    lcd_4bit.rs = 0;
    lcd_4bit.rw = 0;
    lcd_4bit.enable = 0;
    delay_ms(15);
    for(i=1;i<=3;++i) {
       lcd_send_nibble(3);
       delay_ms(5);
    }
    lcd_send_nibble(2);
    for(i=0;i<=3;++i)
       lcd_send_byte(0,LCD_INIT_STRING[i]);
}


void lcd_gotoxy( byte x, byte y) {
   byte address;

   if(y!=1)
     address=lcd_line_two;
   else
     address=0;
   address+=x-1;
   lcd_send_byte(0,0x80|address);
}

void lcd_putc( char c) {
   switch (c) {
     case '\f'   : lcd_send_byte(0,1);
                   delay_ms(2);
                                           break;
     case '\n'   : lcd_gotoxy(1,2);        break;
     case '\b'   : lcd_send_byte(0,0x10);  break;
     default     : lcd_send_byte(1,c);     break;
   }
}


