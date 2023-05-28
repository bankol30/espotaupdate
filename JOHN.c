const int look_up[32]=  { 0, 25, 49, 73, 96, 118, 139, 159, 177, 193, 208, 220, 231, 239, 245, 249, 250,
 249, 245, 239, 231, 220, 208, 193, 177, 159, 139, 118, 96, 73, 49, 25
};
unsigned int duty=0;
float DutyCycle=0.9,FeedBackVoltage;
short alt=0,test=0;
short inv_mask=0, buzzer_test;
unsigned int counter, count, chargerwaittime, chargerbuzzer;
unsigned int charge_counter, inverter_buzzer, temperature_alarm, error_alarm;
short buzzercount=0;
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
#define SPWMA RC0_BIT
#define SPWMB RC3_BIT
#define  SPWMA_HIGH RC1_BIT
#define SPWMB_HIGH RC2_BIT
unsigned char receiveddata;
unsigned char FlagReg;
sbit ZC at FlagReg.B0;
unsigned int overload_check=0;
float  maxduty=150.0, minduty=0.0, lowduty=60.0;
unsigned int battery=0,output=0,mains=0, temperature=0, current=0;
int TimeSec, FaultTag, ChargeDuty, ForLoop, Timerec;
unsigned int InverterPhase=0, BatPercent=0, TimerCounter=0, GridTimer=0, GridCount=0;
short PowerON=0, GridON=0, ChargingState=0, FaultOccur=0, GridCharge=0, GridPhase=0, checkflag=0,  chargestart=0, mainscheck=0, mainsdetect=0, chargePhase=0;
int a,b,ii;
unsigned int ADC_Value, DisplayVolt,alarm;
long j;
short batterylow_flag=0;
int blink;
short ERROR_FLAG=0;
unsigned int charging_delay=0, charging_timer=0;
unsigned int inverter_time_counter=0;// 2 secs on time
unsigned int auto_transfer=0;
unsigned long grid_check=25000;
unsigned long grid_counter=0;
unsigned long battery_check=2000;
unsigned long battery_counter=0;
unsigned long battery_count=0;
unsigned long button_counter=0;

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
return (ADRES);
}
void ExternalInit_activate(){
INTF_BIT=0;
OPTION_REG.INTEDG=0;
}
void ExternalInit_deactivate(){
INTE_BIT=0;
}
 unsigned long recovery_counter=0;
 unsigned long recovery_delay=400;
 short hysteresis=0;
void LoadCheck() {
if(battery>158 && PowerON){
 BUZZER=0;
 LOWBAT=0;
}
   if(battery>143 && battery<158 && inv_mask){
 alarm++;
 if(alarm>=1000){
 BUZZER=~BUZZER;
 alarm=0;
 }
 }
if(battery<=143 && PowerON){  //battery low
recovery_counter++;
if(recovery_counter==10000){
recovery_counter=10000;
FaultOccur=1;
inv_mask=0;
RELAY=0;
BUZZER=0;
LOWBAT=0;
FaultTag=1;
}
}
 if(temperature<=90 && PowerON ){  //fan start
 FAN=1;
 }
  if(temperature>104 && PowerON ){  //fan stop
 FAN=0;
 }
 if(temperature<=30 && PowerON ){  //over temperature
 FaultOccur=1;
 BUZZER=1;
 LOWBAT=0;
 RELAY=0;
 FAN=1;
 inv_mask=0;
 FaultTag=2;
 }
  if(temperature>=200 && PowerON){  //no thermistor detected
 FaultOccur=1;
 BUZZER=1;
 LOWBAT=0;
 RELAY=0;
 inv_mask=0;
 FaultTag=2;
 }
 if(ERROR_FLAG==1 && PowerON){
 BUZZER=1;
 LOWBAT=1;
 }
 }
 //////////////////////////////////INVERTER CONTROL AND MAINS TRANSFER CONTROL LOOP
  void SystemControl(){
 if(Button==0){
 button_counter++;
 if(button_counter==6000){
 button_counter=6000;
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
 }
 else if(Button==0 && (PowerON || FaultOccur)){
recovery_counter=0;
 inv_mask=0;
 PowerON=0;
 RELAY=0;
 FaultOccur=0;
 FAN=0;
button_counter=0;
 }
 else if(Button==1 && PowerON){
 recovery_counter=0;
 ERROR_FLAG=0;
 FAN=0;
 LOWBAT=0;
 BUZZER=0;
 OVLLED=0;
 inv_mask=0;
 PowerON=0;
 RELAY=0;
 FaultOccur=0;
 button_counter=0;
 }
 /////////////////////////////interrupt charge control
if(ChangeoverPin ==0){
INTE_BIT=1;
}
else if(ChangeoverPin ==1){
INTE_BIT=0;
 MNSLED=0;
 mainscheck=0;
 grid_counter=0;
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
if ( output <100) {
if(DutyCycle < 0.85)  DutyCycle += 0.005;
}
else if ( output > 115){
if(DutyCycle> 0.5)DutyCycle -= 0.005;
}
battery=ADC_GET(1);
//mains=ADC_GET(2);
temperature=ADC_GET(3);
///////////change over control from inverter to charger
if(mainscheck){
grid_counter++;  //check if mains is available
if(grid_counter==1000){ BUZZER=1;}
if(grid_counter==2000){ BUZZER=0;}
if(grid_counter==3000){ BUZZER=1;}
if(grid_counter==4000){ BUZZER=0;}
if(grid_counter==5000){ BUZZER=1;}
if(grid_counter==6000){ BUZZER=0;}
if(grid_counter==grid_check){
grid_counter=grid_check;
RELAY=0;
mainsdetect=1;
chargestart=1;
}
}
//////////////////////////////////////////////////////
SystemControl();
LoadCheck();
++duty;
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
CHGLED=0;
OVLLED=0;
if(alt)
{
CCPR1L=0;    //spwm1 off
SPWMB=1;
CCPR2L = look_up[duty] *  DutyCycle ; //spwm2 DutyCycle set
SPWMA=0;
}
else
{
CCPR2L=0;      //spwm2 off
SPWMA=1;
CCPR1L = look_up[duty] *  DutyCycle;    //spwm1 DutyCycle set
SPWMB=0;
}
///////////////////////////////////////////feedback control for inverter output
///////////////////////////////////////////////battery low beep
/*
if(inv_mask && (battery>135 && battery<145)){
 battery_counter++;
 if(battery_counter==battery_check){
 BUZZER=~BUZZER;
 LOWBAT=~LOWBAT;
 battery_counter=0;
 }
}
*/
//////////////////////////////////////////////////////////
}
//////////////////////////////////////////charging phase  when inverter is on
else if(mainsdetect && (inv_mask || !inv_mask)){     //when grid available  and inverter is on
MNSLED=1;
if(inv_mask)INVLED=1;
else INVLED=0;
FAN=1;
T1CON=0;
if(chargestart==1 && minduty >= 20.0){
if(temperature>50){BUZZER=0;LOWBAT=0;}
if(temperature<50){BUZZER=1;LOWBAT=1;}
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
if(battery>120 && battery <190){
minduty+=0.001;
if(minduty>=maxduty) {
minduty=maxduty;
}
}
else if(battery > 191){
minduty-=0.001;
if(minduty<=0.0){
minduty=0.0;
}
}
if(chargePhase==0 &&  chargestart==1){    //positive half cycle
CCPR1L=0;
asm nop;
asm nop;
asm nop;
asm nop;
asm nop;
CCPR2L=minduty;
SPWMA=0;
SPWMB=0;
chargePhase=1;
}
else if(chargePhase==1 &&  chargestart==1) {  //negative half cycle
CCPR2L=0;
asm nop;
asm nop;
asm nop;
asm nop;
asm nop;
CCPR1L=minduty;
SPWMA=0;
SPWMB=0;
chargePhase=0;
}
}
else  if(!inv_mask  && !mainsdetect){
inverter_time_counter=0;
auto_transfer=0;
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
OVLLED=0;
BUZZER=0;
FAN=0;
RELAY=0;
chargerwaittime=0;
chargestart=0;
buzzer_test=0;
}
}
if(INTF_BIT==1 && INTE_BIT==1)
{
mainscheck=1;
}
if(RBIF_BIT==1 && RBIE_BIT==1)
{
if(SHORTCIRCUIT==0){
OVLLED=1;
ERROR_FLAG=1;
FaultOccur=1;
inv_mask=0;
RELAY=0;
}
RBIF_BIT=0;
}
}
void main(){
trisc4_bit=0;
trisc6_bit=0;
trisc5_bit=0;
BUZZER=0;
FAN=0;
LOWBAT=0;
for(ii=0;ii<6;ii++){
FAN=1;
MNSLED=1;
CHGLED=1;
INVLED=1;
BUZZER=~BUZZER;
LOWBAT=~LOWBAT;
delay_ms(500);
}
MNSLED=0;
CHGLED=0;
INVLED=0;
BUZZER=0;
FAN=0;
LOWBAT=0;
TRISA=0xFF;
TRISB=0b00010011;
INVLED=0;
MNSLED=0;
LOWBAT=0;
OVLLED=0;
CHGLED=0;
DutyCycle=0;
TRISC = 0b00001111;//all output
PORTC=0x00;
ADC_Initialize();
inv_mask=0;
PR2= 199;
CCP1CON = 0x0C;
CCP2CON = 0x0C;
TMR2IF_bit = 0;
T2CON = 0x2c;;
TMR2IF_bit = 0;
TRISC = 0;
TMR2IE_bit = 1;
ExternalInit_activate();
RBIF_BIT=0;
RBIE_BIT=1;
GIE_bit = 1;
PEIE_bit = 1;
for(;;){
}
}