unsigned int look_up[]={0, 25, 49, 73, 96, 118, 139, 159, 177, 193, 208, 220, 231, 239, 245, 249, 250, 249, 245, 239, 231, 220, 208, 193, 177, 159, 139, 118, 96, 73, 49, 25};
unsigned int duty=0;
float DutyCycle,FeedBackVoltage;
short alt=0,test=0;
short inv_mask=0, buzzer_test;
unsigned int counter, count, chargerwaittime, chargerbuzzer;
unsigned int charge_counter, inverter_buzzer;
unsigned int serial_counter = 0;
#define ChangeoverPin RB0_BIT
#define RELAY RB7_BIT
#define BUZZER RC4_BIT
#define Button RB1_BIT
#define INVLED RC5_BIT
#define CHGLED RB6_BIT
#define OVLLED RB3_BIT
#define MNSLED RB5_BIT
#define LOWBAT RB2_BIT
#define SPWMA RC0_BIT
#define SPWMB RC3_BIT
#define  SPWMA_HIGH RC1_BIT
#define SPWMB_HIGH RC2_BIT
unsigned char receiveddata;
unsigned char FlagReg;
sbit ZC at FlagReg.B0;
unsigned int overload_check=0;
float  maxduty=120.0, minduty=30.0, lowduty=20.0;
unsigned int battery=0,output=0,mains=0, temperature=0, current=0;
int TimeSec, FaultTag, ChargeDuty, ForLoop, Timerec;
unsigned int InverterPhase=0, BatPercent=0, TimerCounter=0, GridTimer=0, GridCount=0;
short PowerON=0, GridON=0, ChargingState=0, FaultOccur=0, GridCharge=0, GridPhase=0, checkflag=0,  chargestart=0, mainscheck=0, mainsdetect=0, chargePhase=0;
int a,b,ii;
unsigned int ADC_Value, DisplayVolt;
char *volt = "00.0";
  long j;
  short batterylow_flag=0;
  int blink;
  short ERROR_FLAG=0;
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
return (ADRES); //Returns Result
}
void ExternalInit_activate(){
 INTF_BIT=0;
OPTION_REG.INTEDG=0;
}

void ExternalInit_deactivate(){
INTE_BIT=0;
}

void LoadCheck(){
 if(battery<143 && PowerON){  //battery low
  BUZZER=1;
 FaultOccur=1;
 inv_mask=0;
 RELAY=0;
 FaultTag=1;
 }
 if(temperature<=87 && PowerON){  //over temperature
 FaultOccur=1;
  BUZZER=1;
  RELAY=0;
 inv_mask=0;

 FaultTag=2;
 }
  if(temperature<=87 && mainsdetect){  //over temperature
 FaultOccur=1;
 BUZZER=1;
 minduty=lowduty;
 mainsdetect=0;
 FaultTag=3;
 }
 if(ERROR_FLAG==1 && PowerON){
 BUZZER=1;
 }
 /*
 if(overload_check <=512 && PowerON){  //high temperature
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
 ERROR_FLAG=0;
 BUZZER=0;
 OVLLED=0;
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
 MNSLED=0;
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
current=ADC_GET(4);
SystemControl();

//LoadCheck();
duty++;
if(duty>=32)
{
duty=0;
alt=~alt;
CCPR1L = 0;//inverter shutdown
CCPR2L = 0;
SPWMA=0;
SPWMB=0;
asm nop;
asm nop;
}
if(inv_mask && !mainsdetect)
{
RELAY=1;
INVLED=1;
MNSLED=0;
CHGLED=0;
OVLLED=0;
if(alt)
{
CCPR1L=0;
CCPR2L = look_up[duty]*DutyCycle;
SPWMA=0;
SPWMB=1;
}
else
{
CCPR2L=0;
CCPR1L = look_up[duty]*DutyCycle;
SPWMA=1;
SPWMB=0;
}
///////////////////////////////////////////feedback control for inverter output
//ups mode
if(ra4_bit==1){
if ( output > 128) DutyCycle -= 0.01;
else if ( output < 124) DutyCycle += 0.01;
if (DutyCycle <= 0.1) DutyCycle = 0.1;
else if (DutyCycle >= 1.0) DutyCycle = 1.0;
}
////inverter mode
 else if(ra4_bit==0){
   if ( output > 128) DutyCycle -= 0.0001;
else if ( output < 124) DutyCycle += 0.0001;
if (DutyCycle <= 0.1) DutyCycle = 0.1;
else if (DutyCycle >= 1.0) DutyCycle = 1.0;
 }
}
//////////////////////////////////////////charging phase  when inverter is on
else if(mainsdetect  && (mains >= 102 && mains <= 150) && (inv_mask || !inv_mask)){     //when grid available  and inverter is on
RELAY=0;
T1CON=0;
if(inv_mask){INVLED=1;} if(!inv_mask){INVLED=0;}
if(chargestart==1){
GridTimer++;
if(GridTimer==100) {
GridCount++;
GridTimer=0;
if(GridCount==10){
CHGLED=~CHGLED;
GridCount=0;
}
}
}
if(battery > 115 && battery < 184){
minduty+=0.001;
if(minduty>=maxduty)
minduty=maxduty;
}
else if(battery>188 && battery<=195){
minduty-=0.001;
if(minduty==lowduty)
minduty=lowduty;
}
else if(battery > 195){
minduty=0.0;
}
if(chargePhase==0 &&  chargestart==1 ){    //positive half cycle
CCPR1L=0;
CCPR2L=minduty;
SPWMA=0;
SPWMB=0;
chargePhase=1;
}
else if(chargePhase==1 &&  chargestart==1) {  //negative half cycle
CCPR2L=0;
CCPR1L=minduty;
SPWMA=0;
SPWMB=0;
chargePhase=0;
}
}
  //////////////////////////////////////////////////////////////////
else  if(!inv_mask  && !mainsdetect){
DutyCycle=0;
CCPR1L = 0;//inverter shutdown
CCPR2L = 0;
T1CON=0;
SPWMA=0;
SPWMB=0;
INVLED=0;
CHGLED=0;
MNSLED=0;
LOWBAT=0;
//OVLLED=0;
BUZZER=0;
chargerwaittime=0;
chargestart=0;
buzzer_test=0;
}
}
 if(INTF_BIT==1 && INTE_BIT==1)
{
MNSLED=1;
 T1CON=0x31;  //start timer1 clock
mainsdetect=1;
 //INTF_BIT=0;
}
if(RBIF_BIT==1 && RBIE_BIT==1)
{
if(RB4_BIT==0){
  OVLLED=1;
ERROR_FLAG=1;
FaultOccur=1;
inv_mask=0;
RELAY=0;
}
 RBIF_BIT=0;
}
if(TMR1IF_BIT==1 && TMR1IE_BIT==1){ //0.1s Interrupt time multiply by number of seconds before charging start
chargerwaittime++;
if(chargerwaittime==1){BUZZER=1;}
if(chargerwaittime>=10){BUZZER=0;}
if(chargerwaittime>=20){BUZZER=1;}
if(chargerwaittime>=30){BUZZER=0;}
if(chargerwaittime>=40){BUZZER=1;}
if(chargerwaittime>=50){BUZZER=0;}
if(chargerwaittime==150)  //10s delay before charging start
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
trisc4_bit=0;
BUZZER=0;
for(ii=0;ii<10;ii++){
BUZZER=~BUZZER;
delay_ms(300);
}
BUZZER=0;
TRISA=0xFF;
TRISB=0b00010011;
INVLED=0;
MNSLED=0;
LOWBAT=0;
OVLLED=0;
CHGLED=0;
RELAY=0;
DutyCycle=0;
TRISC = 0b00001111;//all output
PORTC=0x00;
ADC_Initialize();
inv_mask=0;
PR2 = 251;
CCP1CON = 0x4C;
CCP2CON = 0x4C;
TMR2IF_bit = 0;
T2CON = 0x2C;
TMR2IF_bit = 0;
TRISC = 0;
TMR2IE_bit = 1;
ExternalInit_activate();
InitTimer1();
RBIF_BIT=0;
RBIE_BIT=1;
GIE_bit = 1;
PEIE_bit = 1;
for(;;){
LoadCheck();
   }
}