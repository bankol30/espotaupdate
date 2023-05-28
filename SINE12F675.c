
const char duty[57]={0,1,3,7,13,20,
28,37,48,60,72,85,99,113,128,142,156,170,183,195,207,218,
227,235,242,248,252,254,255,254,
252,248,242,235,227,218,207,195,
183,170,156,142,128,113,99,85,
72,60,48,37,28,20,13,7,
3,1,0
};

#define PWM1  GP5_BIT
#define PWM2 GP0_BIT
#define HIG 1
#define LOW 0
   int i;

void main() {

for(;;){
   for(i=0;i<57;i++)//PWM GP0
      {
    PWM1=HIG;
    delay_us(duty[i]);
     PWM1=LOW;
    delay_us(255-duty[i]);
      }
      for(i=0;i<57;i++)//PWM GP1
      {
    PWM2=HIG;
    delay_us(duty[i]);
     PWM2=LOW;
    delay_us(255-duty[i]);
      }

}

}