#define clear_display                           0x01
#define goto_home                               0x02
#define cursor_direction_inc                    (0x04 | 0x02)
#define cursor_direction_dec                    (0x04 | 0x00)
#define display_shift                           (0x04 | 0x01)
#define display_no_shift                        (0x04 | 0x00)
#define display_on                              (0x08 | 0x04)
#define display_off                             (0x08 | 0x02)
#define cursor_on                               (0x08 | 0x02)
#define cursor_off                              (0x08 | 0x00)
#define blink_on                                (0x08 | 0x01)
#define blink_off                               (0x08 | 0x00)
#define _8_pin_interface                        (0x20 | 0x10)
#define _4_pin_interface                        (0x20 | 0x00)
#define _2_row_display                          (0x20 | 0x08)
#define _1_row_display                          (0x20 | 0x00)
#define _5x10_dots                              (0x20 | 0x40)
#define _5x7_dots                               (0x20 | 0x00)
#define dly                                     1
#define LCD_SDI   RC4_BIT //pin declearations   DS
#define LCD_SCK   RC5_BIT      // SH
#define LCD_LCK   RC6_BIT       //ST
#define LCD_SDI_Direction   TRISC4_bit
#define LCD_SCK_Direction   TRISC5_bit
#define LCD_LCK_Direction   TRISC6_bit
unsigned char data_value;
void SIPO();
void LCD_init();
void LCD_command(unsigned char value);
void LCD_send_data(unsigned char value);
void LCD_4bit_send(unsigned char lcd_data);
void LCD_putstr(char *lcd_string);
void LCD_putchar(char char_data);
void LCD_clear_home();
void LCD_goto(unsigned char x_pos, unsigned char y_pos);
void SIPO()
{
    unsigned char clk = 8;
    unsigned char temp = 0;
    temp = data_value;
    LCD_LCK = 0;
    while(clk > 0)
    {
        LCD_SDI = ((temp & 0x80) >> 0x07);
        LCD_SCK = 1;
        temp <<= 1;
        LCD_SCK = 0;
        clk--;
    }
    LCD_LCK = 1;
}
void LCD_initialize()
{
    unsigned char t = 0x0A;
    data_value = 0x08;
    SIPO();
    while(t > 0x00)
    {
      Delay_ms(dly);
      t--;
    };
    data_value = 0x30;
    SIPO();
    data_value |= 0x08;
    SIPO();
    Delay_ms(dly);
    data_value &= 0xF7;
    SIPO();
    Delay_ms(dly);
    data_value = 0x30;
    SIPO();
    data_value |= 0x08;
    SIPO();
    Delay_ms(dly);
    data_value &= 0xF7;
    SIPO();
    Delay_ms(dly);
    data_value = 0x30;
    SIPO();
    data_value |= 0x08;
    SIPO();
    Delay_ms(dly);
    data_value &= 0xF7;
    SIPO();
    Delay_ms(dly);
    data_value = 0x20;
    SIPO();
    data_value |= 0x08;
    SIPO();
    Delay_ms(dly);
    data_value &= 0xF7;
    SIPO();
    Delay_ms(dly);
    LCD_command(_4_pin_interface | _2_row_display | _5x7_dots);
    LCD_command(display_on | cursor_off | blink_off);
    LCD_command(clear_display);
    LCD_command(cursor_direction_inc | display_no_shift);
}
void LCD_command(unsigned char value)
{
    data_value &= 0xFB;
    SIPO();
    LCD_4bit_send(value);
}
void LCD_send_data(unsigned char value)
{
    data_value |= 0x04;
    SIPO();
    LCD_4bit_send(value);
}
void LCD_4bit_send(unsigned char lcd_data)
{
    unsigned char temp = 0x00;
    temp = (lcd_data & 0xF0);
    data_value &= 0x0F;
    data_value |= temp;
    SIPO();
    data_value |= 0x08;
    SIPO();
    Delay_ms(dly);
    data_value &= 0xF7;
    SIPO();
    Delay_ms(dly);
    temp = (lcd_data & 0x0F);
    temp <<= 0x04;
    data_value &= 0x0F;
    data_value |= temp;
    SIPO();
    data_value |= 0x08;
    SIPO();
    Delay_ms(dly);
    data_value &= 0xF7;
    SIPO();
    Delay_ms(dly);
}
void LCD_putstr(char *lcd_string)
{
    while (*lcd_string != '\0')
    {
        LCD_send_data(*lcd_string);
        lcd_string++;
    };
}
void LCD_putchar(char char_data)
{
    LCD_send_data(char_data);
}
void LCD_clear_home()
{
    LCD_command(clear_display);
    LCD_command(goto_home);
}
void LCD_goto(unsigned char y_pos,unsigned char x_pos)
{
    if((y_pos-1) == 0)
    {
        LCD_command(0x80 | (x_pos-1));
    }
    else
    {
        LCD_command(0x80 | 0x40 | (x_pos-1));
    }
}

#define ChangeoverPin RB0_BIT
#define RELAY RB7_BIT
#define BUZZER RC7_BIT
#define Button RB2_BIT
#define SPWMA_LOW RC0_BIT
#define SPWMB_LOW RC3_BIT
unsigned int look_up[]={0, 27, 53, 78, 103, 127, 149, 170, 189, 205, 220, 232, 242, 248, 253, 254,253, 248, 242, 232, 220, 205, 189, 170, 149, 127, 103, 78, 53, 27};
unsigned int duty=0;
float DutyCycle,FeedBackVoltage;
short alt=0,test=0;
short inv_mask=0, buzzer_test;
unsigned int counter, count, chargerwaittime, chargerbuzzer;
unsigned int charge_counter, inverter_buzzer;
unsigned char FlagReg;
sbit ZC at FlagReg.B0;
float  maxduty=160.0, minduty=25.0, lowduty=30.0;
unsigned int battery=0,output=0,mains=0, temperature=0;
int TimeSec, FaultTag, ChargeDuty, ForLoop, Timerec;
unsigned int InverterPhase=0, BatPercent=0, TimerCounter=0, GridTimer=0, GridCount=0;
short PowerON=0, GridON=0, ChargingState=0, FaultOccur=0, GridCharge=0, GridPhase=0, checkflag=0,  chargestart=0, mainscheck=0, mainsdetect=0, chargePhase=0;
int a,b,ii;
unsigned int ADC_Value, DisplayVolt;
char *volt = "00.0";
long j;
short batterylow_flag=0;

void ADC_Initialize()
{
ADCON0 = 0b01000001; //ADC ON and Fosc/4 is selected
ADCON1 = 0b11000000; // Internal reference voltage is selected
}
unsigned int ADC_GET( int channel)
{
ADCON0 &= 0x11000101; //Clearing the Channel Selection Bits
ADCON0 |= channel<<3; //Setting the required Bits
delay_us(20); //Acquisition time to charge hold capacitor
ADCON0.F2 = 1; //Initializes A/D Conversion
while(ADCON0.F2); //Wait for A/D Conversion to complete
return ((ADRESH<<8)+ADRESL); //Returns Result
}
void ExternalInit_activate(){
INTF_BIT=0;
OPTION_REG.INTEDG=0;
}
void ExternalInit_deactivate(){
INTE_BIT=0;
}
void LoadCheck(){
if(battery<614 && PowerON){  //battery low
FaultOccur=1;
inv_mask=0;
RELAY=0;
FaultTag=1;
}
/*uncomment if you wanna use current sensor and temperature sensor
 if(current>=512 && PowerON){  //overcurrent
 FaultOccur=1;
 inv_mask=0;
 RELAY=0;
 FaultTag=2;
 }
 if(temperature <=512 && PowerON){  //high temperature
 FaultOccur=1;
 inv_mask=0;
 RELAY=0;
 FaultTag=3;
 if(GridCharge)GridCharge = 0;
 }
 */
}
//////////////////////////////////INVERTER CONTROL AND MAINS TRANSFER CONTROL LOOP
void SystemControl(){
if(Button==0){
delay_us(2);
if(Button==0 && !PowerON){
delay_us(1);
if(ChangeoverPin ==1 && !FaultOccur){
delay_us(1);
inv_mask=1;
}
delay_us(1);
PowerON=1;
}
}
else if(Button==0 && (PowerON || FaultOccur)){
inv_mask=0;
PowerON=0;
RELAY=0;
FaultOccur=0;
}
else if(Button==1 && PowerON){
inv_mask=0;
PowerON=0;
RELAY=0;
FaultOccur=0;
}
 /////////////////////////////interrupt charge control
if(ChangeoverPin ==0){
INTE_BIT=1;
}
else if(ChangeoverPin ==1){
INTE_BIT=0;
//MNSLED=0;  OR DISPLAY MAINS  NOT AVAILABLE
mainsdetect=0;
}
/////////////////////////////////////
}

void Interrupt()
{
if (TMR2IF_bit == 1 && TMR2IE_bit==1)
{
TMR2IF_bit = 0;
output=ADC_GET(0);
battery=ADC_GET(1);
mains=ADC_GET(2);
temperature=ADC_GET(3);
SystemControl();
//LoadCheck();
duty++;
if(duty>=29)
{
duty=0;
alt=~alt;
CCPR1L = 0;//inverter shutdown
CCPR2L = 0;
SPWMA_LOW=0;
SPWMB_LOW=0;
asm nop;
asm nop;
}
if(inv_mask && !mainsdetect)
{
RELAY=1;
//INVLED=1; DISPLAY INVERTER ON
//MNSLED=0;
//CHGLED=0;
//ERRLED=0;
if(alt)
{
CCPR2L=0;
CCPR1L = look_up[duty]*DutyCycle;
SPWMA_LOW=0;
SPWMB_LOW=1;
}
else
{
CCPR1L=0;
CCPR2L = look_up[duty]*DutyCycle;
SPWMA_LOW=1;
SPWMB_LOW=0;
}
///////////////////////////////////////////feedback control for inverter output
if ( output > 512) DutyCycle -= 0.1;
else if ( output < 508) DutyCycle += 0.1;
if (DutyCycle <= 0.1) DutyCycle = 0.1;
else if (DutyCycle >= 0.9) DutyCycle = 0.9;
//delay_us(1);
//if ( DutyCycle < 0.05) DutyCycle = 0.1;
///////////////////////////////////////////
minduty=0;
}
//////////////////////////////////////////charging phase  when inverter is on
else if(mainsdetect  && (inv_mask || !inv_mask)){     //when grid available  and inverter is on

RELAY=0;
T1CON=0;
DutyCycle=0;
//if(inv_mask && mainsdetect){INVLED=1;} if(!inv_mask && mainsdetect){INVLED=0;}
if(chargestart==1){
GridTimer++;
if(GridTimer==100) {
GridCount++;
GridTimer=0;
if(GridCount==30){
//CHGLED=~CHGLED;
GridCount=0;
}
}
}
if(battery < 800){      //14.4
minduty+=0.001;
if(minduty>=maxduty)
minduty=maxduty;
}
else if(battery>800 && battery<850){     //14.41-14.8
minduty-=0.001;
if(minduty==lowduty)
minduty=lowduty;
}
else if(battery > 851){    //14.81
minduty=0;
}
if(chargePhase==0 &&  chargestart==1 ){    //positive half cycle
CCPR1L=0;
asm nop;
CCPR2L=minduty;
SPWMA_LOW=0;
SPWMB_LOW=0;
chargePhase=1;
}
else if(chargePhase==1 &&  chargestart==1) {  //negative half cycle
CCPR2L=0;
asm nop;
CCPR1L=minduty;
SPWMA_LOW=0;
SPWMB_LOW=0;
chargePhase=0;
}
}
//////////////////////////////////////////////////////////////////
else  if(!inv_mask  && !mainsdetect){
DutyCycle=0;
inv_mask=0;
CCPR1L = 0;//inverter shutdown
CCPR2L = 0;
T1CON=0;
SPWMA_LOW=0;
SPWMB_LOW=0;
//INVLED=0;
//CHGLED=0;
//MNSLED=0;
//LOWBAT=0;
//ERRLED=0;
BUZZER=0;
chargerwaittime=0;
chargestart=0;
buzzer_test=0;
minduty=0;
}
}
if(INTF_BIT==1 && INTE_BIT==1)
{
//MNSLED=1;    DISPLAY MAINS AVAILABLE
T1CON=0x31;  //start timer1 clock
mainsdetect=1;
}
if(TMR1IF_BIT==1 && TMR1IE_BIT==1){ //0.1s Interrupt time multiply by number of seconds before charging start
chargerwaittime++;
if(chargerwaittime==1){BUZZER=1;}
if(chargerwaittime>=5){BUZZER=0;}
if(chargerwaittime>=10){BUZZER=1;}
if(chargerwaittime>=15){BUZZER=0;}
if(chargerwaittime>=20){BUZZER=1;}
if(chargerwaittime>=25){BUZZER=0;}
if(chargerwaittime>=30){BUZZER=1;}
if(chargerwaittime>=35){BUZZER=0;}
if(chargerwaittime>=40){BUZZER=1;}
if(chargerwaittime>=45){BUZZER=0;}
if(chargerwaittime==120)  //10s delay before charging start
chargestart=1;
TMR1H = 0x0B;
TMR1L = 0xDC;
TMR1IF_BIT=0;
}
}
/////////////////////////////////////////////////////////////////////////////
void InitTimer1(){
T1CON = 0;
TMR1IF_bit = 0;
TMR1H = 0x0B;
TMR1L = 0xDC;
TMR1IE_bit = 1;
}


void main(){
CMCON|=0x07;
TRISC=0;
LCD_initialize();
//LCD_clear_home();
//LCD_goto(1,1);
//LCD_putstr(" HC595 LCD TEST");
//delay_ms(1000);
//LCD_clear_home();

////LCD_clear_home();

 for(;;){
 
 }

}