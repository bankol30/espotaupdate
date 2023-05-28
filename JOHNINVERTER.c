
unsigned char sin_table[32]={0, 25, 49, 73, 96, 118, 139, 159, 177, 193, 208, 220, 231, 239, 245, 249, 250, 249, 245, 239, 231, 220, 208, 193, 177, 159, 139, 118, 96, 73, 49, 25};
int duty=0;
float fb_constant=1.0;
float voltage_feedback=0.0,  DutyCycle=0.9;
bit alt;
bit inv_mask;
unsigned int output;
int k;
#define SHORTCIRCUIT RB4_BIT
#define ChangeoverPin RB0_BIT
#define RELAY RB7_BIT
#define BUZZER RC4_BIT
#define Button RB1_BIT
#define FAN RC6_BIT
#define INVLED RB6_BIT    //INVERTER LED
#define CHGLED RB5_BIT    //CHARGER LED
#define MNSLED RB3_BIT    //MAINS LED
#define OVLLED RB2_BIT    //OVERLOAD LED
#define LOWBAT RC5_BIT    //LOW BATTERY LED
int  maxduty=90, ii;
int minduty=50;
int  lowduty=10;
bit mainscheck;
bit checkflag;
short batterylow_flag=0;
int blink;
short ERROR_FLAG=0;
unsigned int battery,input=0,current=0, temperature=0;
int TimeSec, FaultTag, ChargeDuty, ForLoop, Timerec;
unsigned int InverterPhase, BatPercent, TimerCounter, GridTimer;
bit PowerON, Use1,Use2,GridON,ChargingState, FaultOccur,GridCharge,GridPhase;
short ChargePhase;
int a,b;
void ADC_Initialize()
{
ADCON0 = 0b01000001; //ADC ON and Fosc/4 is selected
ADCON1 = 0b11000000; // Internal reference voltage is selected
}
unsigned int ADC_GET(int channel)
{
ADCON0 &= 0x11000101; //Clearing the Channel Selection Bits
ADCON0 |= channel<<3; //Setting the required Bits
delay_us(20); //Acquisition time to charge hold capacitor
ADCON0.F2 = 1; //Initializes A/D Conversion
while(ADCON0.F2); //Wait for A/D Conversion to complete
return (ADRES); //Returns Result
}
void LoadCheck(){

if(battery<=40 && PowerON){  //battery low
FaultOccur=1;
inv_mask=0;
RELAY=0;
BUZZER=0;
LOWBAT=0;
FaultTag=1;
}
 if(temperature<=80 && PowerON ){  //fan start
 FAN=1;
 }
  if(temperature>100 && PowerON ){  //fan start
 FAN=0;
 }

 if(temperature<=50 && PowerON ){  //over temperature
 FaultOccur=1;
 BUZZER=1;
 LOWBAT=1;
 RELAY=0;
 FAN=1;
 inv_mask=0;
 FaultTag=2;
 }
  if(temperature>=200 && PowerON){  //no thermistor detected
 FaultOccur=1;
 BUZZER=1;
 LOWBAT=1;
 RELAY=0;
 inv_mask=0;
 FaultTag=2;
 }
 if(ERROR_FLAG==1 && PowerON){
 BUZZER=1;
 LOWBAT=1;
 }
  /*
 if(overload_check >=512 && PowerON){  //over current or over load
 FaultOccur=1;  //set fault to 1
 inv_mask=0;    //inverter shutdown
 RELAY=0;       //relay deactivated
 FaultTag=3;      //faulttag for serial set to 3
   }
   */
 }
void SystemControl(){

 if(Button==0){
 delay_us(2);
 if(Button==0 && !PowerON){
 delay_us(1);
 if(ChangeoverPin ==1 && !FaultOccur){
 RELAY=1;
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
 }

void Interrupt() iv 0x0004 ics ICS_AUTO
{
   if (TMR2IF_bit == 1)
   {
    output=ADC_GET(0);
    battery=ADC_GET(1);
    SystemControl();
    LoadCheck();
      duty++;
      if(duty>=32)
      {
        duty=0;
        alt=~alt;
        CCPR1L = 0;//inverter shutdown
        CCPR2L = 0;
        RC0_bit=0;
        RC3_bit=0;
        asm nop;
        asm nop;
      }

      if(inv_mask)
      {
          if(alt)
          {
             CCPR1L = sin_table[duty]*DutyCycle;
             CCPR2L=0;
             RC0_bit=1;
             RC3_bit=0;
          }
          else
          {
             CCPR2L = sin_table[duty]*DutyCycle;
             CCPR1L=0;
             RC0_bit=0;
             RC3_bit=1;
          }


if ( output <100) {
if(DutyCycle < 0.9)  DutyCycle += 0.001;
}
else if ( output > 115){
if(DutyCycle> 0.5)DutyCycle -= 0.001;
}  

      }
      else
      {
         CCPR1L = 0;//inverter shutdown
         CCPR2L = 0;
         RC0_bit=0;
         RC3_bit=0;
      }

TMR2IF_bit = 0;
   }
}


   int j;
void main()
{
   TRISA=0xFF;//all input
   TRISC = 0x00;//all output
   PORTC=0x00;
     for( j=0; j<6; j++){
BUZZER=~BUZZER;
delay_ms(400);
}
RC4_BIT=0;
   TRISB1_BIT=1;
   TRISB6_BIT=0;
   RB6_bit = 0;//for LCD backlight
  ADC_Initialize();
   PR2 = 249;
   CCP1CON = 0x4C;
   CCP2CON = 0x4C;
   TMR2IF_bit = 0;
   T2CON = 0x2C;
   TMR2IF_bit = 0;
   TRISC = 0;
   TMR2IE_bit = 1;
   GIE_bit = 1;
   PEIE_bit = 1;

   while(1)
   {
  // LoadCheck();
   }
}// end