
const unsigned int look_up[]={0, 25, 49, 73, 96, 118, 139, 159, 177, 193, 208, 220, 231, 239, 245, 249, 250, 249, 245, 239, 231, 220, 208, 193, 177, 159, 139, 118, 96, 73, 49, 25};
 int duty=0;
unsigned int feed_back;         //feedback for reference
unsigned int voltage;
         short toggle=0;         //to toggle alternate state
  unsigned int i;
   int blik;
    short en=0;
    short inv_mask=0;
    unsigned int battery,feedback,temperature;
    unsigned long counter=0;
    float DutyCycle;
    
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
    
void Interrupt(){
  if (TMR1IF_bit){
  TMR1IF_bit = 0;                 //clear timer1 flag
  TMR1H	 = 0xF9;
  TMR1L	 = 0xE8;
  feedback=ADC_GET(0);
  battery=ADC_GET(1);
  temperature=ADC_GET(2);
    ++i;                               //increment i,i=555us
    if(i>=32)                            //if i==32  555*32=10000us=10ms
    {
    toggle=~toggle;
    i=0;
PWM1_Set_Duty(0);
PWM2_Set_Duty(0);
rc0_bit=0;                //lowside2 =low
rc3_bit=0;
asm nop;
asm nop;
    }
    if(rb1_bit==0){
    counter++;
    if(counter==2000){
    counter=2000;
      inv_mask=1;
      }
    }
    else if(rb1_bit==1){
    counter=0;
     inv_mask=0;
    }
    if(inv_mask){
    if(toggle){
    rc0_bit=0;                //lowside2 =low
    rc3_bit=1;            // lowside 1 =high
    PWM1_Set_Duty(0);        //duty cycle 1 to 0
       PWM2_Set_Duty(look_up[i]* DutyCycle);
    }
    else
    {
      rc3_bit=0;         //set side1 low
        rc0_bit=1;        //side 2 high

    PWM2_Set_Duty(0);
     PWM1_Set_Duty(look_up[i]* DutyCycle);
   }
   if ( feedback > 120) DutyCycle -= 0.01;
else if ( feedback < 115) DutyCycle += 0.01;
if (DutyCycle <= 0.1) DutyCycle = 0.1;
else if (DutyCycle >= 1.0) DutyCycle = 1.0;
   }
   else
   {
       rc0_bit=0;        //side 2 high
        rc3_bit=0;        //side 2 high
        PWM1_Set_Duty(0);
    PWM2_Set_Duty(0);
   
   }
  }
}
int a;
void main() {
  TRISA=0xff;
  TRISC=0x00;
  TRISB=0b00000011;
  PORTC=0x00;
  PORTB=0x00;
  ADC_Initialize();
  PWM1_Init(20000);      //20khz
  PWM1_Start();
  PWM2_Init(20000);
  PWM2_Start();
  T1CON	 = 0x01;
  TMR1IF_bit	 = 0;
  TMR1H	 = 0xF9;
  TMR1L	 = 0xE8;
  TMR1IE_bit	 = 1;
  INTCON	 = 0xC0;
     while(1)
     {
     }
}
