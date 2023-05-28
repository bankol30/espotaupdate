#include <i2cccs.h>
#use I2C(master, I2C1, FAST = 100000, STREAM = I2C_LCD)

// CCS C driver code for I2C LCDs (HD44780 compliant controllers)
// https://simple-circuit.com/

#define BAT_VOLTS_SCALE 0.1123047
#define SOL_VOLTS_SCALE 0.1123047
unsigned int battery, solar, current, temperature;
float bat, sol, cur, temp;
///////////////////////////////////////////////////////////////////////////////
#define LCD_BACKLIGHT          0x08
#define LCD_NOBACKLIGHT        0x00
#define LCD_FIRST_ROW          0x80
#define LCD_SECOND_ROW         0xC0
#define LCD_THIRD_ROW          0x94
#define LCD_FOURTH_ROW         0xD4
#define LCD_CLEAR              0x01
#define LCD_RETURN_HOME        0x02
#define LCD_ENTRY_MODE_SET     0x04
#define LCD_CURSOR_OFF         0x0C
#define LCD_UNDERLINE_ON       0x0E
#define LCD_BLINK_CURSOR_ON    0x0F
#define LCD_MOVE_CURSOR_LEFT   0x10
#define LCD_MOVE_CURSOR_RIGHT  0x14
#define LCD_TURN_ON            0x0C
#define LCD_TURN_OFF           0x08
#define LCD_SHIFT_LEFT         0x18
#define LCD_SHIFT_RIGHT        0x1E
#ifndef LCD_TYPE
   #define LCD_TYPE 2           // 0=5x7, 1=5x10, 2=2 lines
#endif
int1 RS;
unsigned int8 i2c_addr, backlight_val = LCD_BACKLIGHT;
void LCD_Write_Nibble(unsigned int8 n);
void LCD_Cmd(unsigned int8 Command);
void LCD_Goto(unsigned int8 col, unsigned int8 row);
void LCD_Out(unsigned int8 LCD_Char);
void LCD_Begin(unsigned int8 _i2c_addr);
void Backlight();
void noBacklight();
void Expander_Write(unsigned int8 value);
void LCD_Write_Nibble(unsigned int8 n) {
  n |= RS;
  Expander_Write(n);
  Expander_Write(n | 0x04);
  delay_us(1);
  Expander_Write(n & 0xFB);
  delay_us(50);
}

void LCD_Cmd(unsigned int8 Command) {
  RS = 0;
  LCD_Write_Nibble(Command & 0xF0);
  LCD_Write_Nibble((Command << 4) & 0xF0);
  if((Command == LCD_CLEAR) || (Command == LCD_RETURN_HOME))
  delay_ms(2);
}
void LCD_Goto(unsigned int8 col, unsigned int8 row) {
  switch(row) {
    case 2:
      LCD_Cmd(0xC0 + col-1);
      break;
    case 3:
      LCD_Cmd(0x94 + col-1);
      break;
    case 4:
      LCD_Cmd(0xD4 + col-1);
    break;
    default:      // case 1:
      LCD_Cmd(0x80 + col-1);
  }
}

void LCD_Out(unsigned int8 LCD_Char){
  RS = 1;
  LCD_Write_Nibble(LCD_Char & 0xF0);
  LCD_Write_Nibble((LCD_Char << 4) & 0xF0);
}

void LCD_Begin(unsigned int8 _i2c_addr) {
  i2c_addr = _i2c_addr;
  Expander_Write(0);
  delay_ms(40);
  LCD_Cmd(3);
  delay_ms(5);
  LCD_Cmd(3);
  delay_ms(5);
  LCD_Cmd(3);
  delay_ms(5);
  LCD_Cmd(LCD_RETURN_HOME);
  delay_ms(5);
  LCD_Cmd(0x20 | (LCD_TYPE << 2));
  delay_ms(50);
  LCD_Cmd(LCD_TURN_ON);
  delay_ms(50);
  LCD_Cmd(LCD_CLEAR);
  delay_ms(50);
  LCD_Cmd(LCD_ENTRY_MODE_SET | LCD_RETURN_HOME);
  delay_ms(50);
}

void Backlight() {
  backlight_val = LCD_BACKLIGHT;
  Expander_Write(0);
}

void noBacklight() {
  backlight_val = LCD_NOBACKLIGHT;
  Expander_Write(0);
}

void Expander_Write(unsigned int8 value) {
  I2C_Start(I2C_LCD);
  I2C_Write(I2C_LCD, i2c_addr);
  I2C_Write(I2C_LCD, value | backlight_val);
  I2C_Stop(I2C_LCD);
}

////////////////////////////////////////////
const float VCC   = 5.00;// supply voltage 5V or 3.3V. If using PCB, set to 5V only.
const int model = 8;   // enter the model (see below)
float cutOffLimit = 1.00;// reading cutt off current. 1.00 is 1 Amper
float sol_amps, sol_watts;
float sensitivity[] ={
          40.0,// for ACS758LCB-050B model 0
          60.0,// for ACS758LCB-050U model 1
          20.0,// for ACS758LCB-100B model 2
          40.0,// for ACS758LCB-100U model 3
          13.3,// for ACS758KCB-150B model 4
          16.7,// for ACS758KCB-150U model 5
          10.0,// for ACS758ECB-200B model 6
          20.0,// for ACS758ECB-200U model 7
          66.0,// for ACS712-30A model 8
          100.0,//for ACS712-20A model 9
          180.0//for ACS712-5A model 10
         };
float quiescent_Output_voltage [] ={
          0.5,// for ACS758LCB-050B
          0.12,// for ACS758LCB-050U
          0.5,// for ACS758LCB-100B
          0.12,// for ACS758LCB-100U
          0.5,// for ACS758KCB-150B
          0.12,// for ACS758KCB-150U
          0.5,// for ACS758ECB-200B
          0.12,// for ACS758ECB-200U   
          0.5,// for ACS712-30A 
          0.5,//for ACS712-20A 
          0.5// for ACS712-5A          
          };
 float FACTOR = sensitivity[model]/1000;// set sensitivity for selected model
 float QOV =   quiescent_Output_voltage [model] * VCC;// set quiescent Output voltage for selected model
float voltage;// internal variable for voltage
float cutOff = FACTOR/cutOffLimit;// convert current cut off to mV
int16 getAnalog(char channel){
   set_adc_channel(channel);
   delay_us(20);
   int16 analogValue=read_adc();
   while(!adc_done());
   return analogValue;
}

void adc_data(){
sol=getAnalog(0) * SOL_VOLTS_SCALE;
bat=getAnalog(1) * BAT_VOLTS_SCALE;
}

int8 k=0;
void UI_INTERFACE(){  
if(k==0){
LCD_Goto(1, 1);
  LCD_Out("S:");
sol=getAnalog(0) * SOL_VOLTS_SCALE;
LCD_Goto(3, 1);
 printf(lcd_out, "%.1fV", sol);

  
   LCD_Goto(9, 1);
  LCD_Out("B:");
  bat=getAnalog(1) * BAT_VOLTS_SCALE;
LCD_Goto(11, 1);
 printf(lcd_out, "%.1fV", bat);
  
   LCD_Goto(1, 2);
  LCD_Out("A:");
  cur=getAnalog(2)*  (5.0 / 1023.0);
sol_amps=(cur-2.50)/0.066;
if(sol_amps<0){
sol_amps=(2.50-cur)/0.066;
}  
  LCD_Goto(3, 2);
 printf(lcd_out, "%.1fA", sol_amps);
  
   LCD_Goto(9, 2);
  LCD_Out("T:");
}



}
  int jj=0;
#INT_TIMER1
void TIMER1(void){
k++;
if(k>5)k=0;


///////
set_timer1(15536);
clear_interrupt(INT_TIMER1);
}

void main()
{

unsigned int i = 0;
  setup_adc(ADC_CLOCK_INTERNAL);
   setup_adc_ports(ALL_ANALOG);
  setup_oscillator(OSC_8MHZ);         // Set internal oscillator to 8MHz
     LCD_Begin(0x4E);                    // Initialize LCD module with I2C address = 0x4E 

  clear_interrupt(INT_TIMER1);
  enable_interrupts(INT_TIMER1);
  enable_interrupts(GLOBAL);
 setup_timer_1(T1_INTERNAL | T1_DIV_BY_4);
 set_timer1(15536);
 setup_ccp1(CCP_OFF);
  //setup_ccp2(CCP_PWM);
 setup_timer_2(T2_DIV_BY_1,79,1);
 
 
 

 
   while(TRUE)
   {
    UI_INTERFACE();
    if(sol>bat){
    jj++;
    if(jj>1000)jj=1000;
     setup_ccp1(CCP_PWM);
    set_pwm1_duty(jj);
    }
    if(sol<bat)
    {
    jj--;
    if(jj==0)jj=0;
     setup_ccp1(CCP_OFF);
     set_pwm1_duty(jj);
    }
   
   }

}
