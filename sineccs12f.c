#include <sineccs12f.h>
#define PWM1 PIN_A5 //
#define PWM2 PIN_A4 //
#define LOW1 PIN_A1 //
#define LOW2 PIN_A2 //
//#define NFET1 PIN_A1//
//#define NFET2 PIN_A4//
#define HIG output_high
#define LOW output_low
//char duty[40]={2,6,14,24,37,53,70,88,108,128,147,167,185,202,218,231,241,249,253,255,253,249,241,231,218,202,185,167,147,128,108,88,70,53,37,24,14,6,2,0};//50.5
//char duty[54]={0,2,4,7,11,16,22,28,35,42,50,58,67,83,92,100,108,115,122,128,134,139,143,146,148,150,150,150,148,146,143,139,134,128,122,115,108,100,92,83,75,67,58,50,42,35,28,22,16,11,7,4,2,0};
 const char duty[57]={0,1,3,7,13,20,
28,37,48,60,72,85,99,113,128,142,156,170,183,195,207,218,
227,235,242,248,252,254,255,254,
252,248,242,235,227,218,207,195,
183,170,156,142,128,113,99,85,
72,60,48,37,28,20,13,7,
3,1,0
};
unsigned int dutycycle=0;
int16 feedback;
int16 getAnalog(char channel){
   set_adc_channel(channel);
   delay_us(20);
   int16 analogValue=read_adc();
   while(!adc_done());
   return analogValue;
}

void main()
{
   setup_adc_ports(sAN0);
   setup_adc(ADC_CLOCK_DIV_8);
 LOW(PWM1);
  LOW(LOW1);
   LOW(LOW2);
 LOW(PWM2);
  int i;
  while(true)
  {
   feedback=getAnalog(0);
   if (  feedback > 512)dutycycle -= 1;
else if (  feedback < 450) dutycycle += 1;
if (dutycycle <= 10) dutycycle = 10;
else if (dutycycle >= 57) dutycycle = 57;

  
      for(i=0;i<dutycycle;i++)//PWM GP0
      {
    HIG(PWM1);
    delay_us(duty[i]);
    LOW(PWM1);
    delay_us(255-duty[i]);
      }
      for(i=0;i<dutycycle;i++)//PWM GP1
      {
    HIG(PWM2);
    delay_us(duty[i]);
    LOW(PWM2);
    delay_us(255-duty[i]);
}

  }

}
