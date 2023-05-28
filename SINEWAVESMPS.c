const int look_up[]={0, 20, 39, 58, 77, 94, 111, 127, 141, 155, 166, 176, 185, 191, 196, 199, 200, 199, 196, 191, 185, 176, 166, 155, 141, 127, 111, 94, 77, 58, 39, 20};
unsigned int duty=0;
float DutyCycle,FeedBackVoltage;
short alt=0,test=0;
short inv_mask=0, buzzer_test;
unsigned int counter, count, chargerwaittime, chargerbuzzer;
unsigned int charge_counter, inverter_buzzer, temperature_alarm, error_alarm;
short buzzercount=0;
unsigned int uart_data=0;
long uart_counter=0;
char aux1;
#define SHORTCIRCUIT RB4_BIT
#define BUZZER RB3_BIT
#define Button RB1_BIT
#define FAN RC4_BIT
#define ERRORLED RB7_BIT
#define INVLED RB6_BIT  //INVERTER LED
#define LOWBAT RB5_BIT  //LOW BATTERY LED
#define SPWMA RC0_BIT
#define SPWMB RC3_BIT
#define  SPWMA_HIGH RC1_BIT
#define SPWMB_HIGH RC2_BIT
#define SG3525N RB2_BIT
unsigned int battery=0,output=0,mains=0, temperature=0, current=0;
int a,b,ii;
long j;
unsigned int inverter_time_counter=0;// 2 secs on time
unsigned long alarm=0;
unsigned long battery_counter=0;
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
void ExternalInit_activate(){
INTF_BIT=0;
OPTION_REG.INTEDG=0;
}
void ExternalInit_deactivate(){
INTE_BIT=0;
}

 void SystemControl(){
 if(Button==0){
 battery_counter++;
if(battery_counter==1000){
battery_counter=1000;
inv_mask=1;
 }
 }
 else if(Button==1){
 battery_counter=0;
 inv_mask=0;
 BUZZER=0;
 }
 if((temperature<70 || temperature>200) && inv_mask){
 inv_mask=0;
 BUZZER=1;
 }
 if(battery>143 && battery<155 && inv_mask){
 alarm++;
 if(alarm>=1000){
 BUZZER=~BUZZER;
 alarm=0;
 }
 }
 if(battery>155 && inv_mask){
 BUZZER=0;
 }
 if(battery<143 && inv_mask){
 inv_mask=0;
 BUZZER=0;
 }
 }
void Interrupt()
{
if (TMR2IF_bit == 1 && TMR2IE_bit==1)
{
TMR2IF_bit = 0;
output=ADC_GET(0);
battery=ADC_GET(1);     
temperature=ADC_GET(3);
SystemControl();
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
if(inv_mask)
{
SG3525N=0;
INVLED=1;
FAN=1;
if(alt){
CCPR1L=0; //spwm mosfet1 off
CCPR2L = look_up[duty] * DutyCycle;  //spwm mosfet2 on
SPWMA=0; //low side mosfet1 off
SPWMB=1; //low side mosfet2 on
}
else{
CCPR2L=0;  //spwm mosfet2 off
CCPR1L = look_up[duty] * DutyCycle; //spwm mosfet1 on
SPWMB=0;
SPWMA=1;  //low side mosfet1 on
}
///////////////////////////////////////////feedback control for inverter output
//inverter mode
if ( output > 120) DutyCycle -= 0.001;
else if ( output < 115) DutyCycle += 0.001;
if (DutyCycle <= 0.1) DutyCycle = 0.1;
else if (DutyCycle >= 1.0) DutyCycle = 1.0;
}
else  if(!inv_mask){
inverter_time_counter=0;
DutyCycle=0;
CCPR1L = 0;//inverter shutdown
CCPR2L = 0;
SG3525N=1;
SPWMA=0;
SPWMB=0;
INVLED=0;
LOWBAT=0;
BUZZER=0;
FAN=0;
}
}
}
void main(){
trisB3_bit=0;
trisB6_bit=0;
trisB5_bit=0;
trisB2_bit=0;
SG3525N=1;
BUZZER=0;
FAN=0;
LOWBAT=0;
for(ii=0;ii<10;ii++){
FAN=1;
INVLED=1;
BUZZER=~BUZZER;
LOWBAT=~LOWBAT;
delay_ms(300);
}
INVLED=0;
BUZZER=0;
FAN=0;
LOWBAT=0;
TRISA=0xFF;
TRISB=0b00010011;
INVLED=0;
LOWBAT=0;
SG3525N=1;
DutyCycle=0;
TRISC = 0b00001111;//all output
PORTC=0x00;
ADC_Initialize();
inv_mask=0;
PR2 = 249;
CCP1CON = 0x4C;
CCP2CON = 0x4C;
TMR2IF_bit = 0;
T2CON = 0x2C;
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