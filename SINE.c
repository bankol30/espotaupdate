
unsigned char sin_table[32]={0, 25, 49, 73, 96, 118, 139, 159, 177, 193, 208, 220, 231, 239, 245, 249, 250, 249, 245, 239, 231, 220, 208, 193, 177, 159, 139, 118, 96, 73, 49, 25};
int duty=0;
float fb_constant=0.8;
bit alt;
bit inv_mask;
#define SPWMA RC0_BIT
#define SPWMB RC3_BIT
#define  SPWMA_HIGH RC1_BIT
#define SPWMB_HIGH RC2_BIT
void Interrupt() iv 0x0004 ics ICS_AUTO
{
   if (TMR2IF_bit == 1)
   {
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
          if(alt)
          {
		  	CCPR2L = sin_table[duty] * fb_constant; //spwm2 DutyCycle set
		    CCPR1L=0;    //spwm1 off
			SPWMB=1;
			SPWMA=0;

          }
          else
          {
		    CCPR1L = sin_table[duty] *fb_constant;    //spwm1 DutyCycle set
		  CCPR2L=0;      //spwm2 off
          SPWMA=1;
          SPWMB=0;

          }
      }
      else
      {
         CCPR1L = 0;//inverter shutdown
         CCPR2L = 0;
          SPWMA=0;
          SPWMB=0;
      }
      TMR2IF_bit = 0;
   }
}
int k=0;
unsigned int battery=0,output=0;
bit lcd_clr;
char msg[]="000";
void main()
{
   TRISA=0xFF;//all input
   TRISC = 0x00;//all output
   TRISB0_BIT=1;
   PORTC=0x00;
   RC4_bit = 1;//for LCD backlight
   ADC_Init();
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
   UART1_Init(9600);
   while(1)
   {
      //switch operation
      if(!RB1_bit)
      {
        inv_mask=1;
        lcd_clr=1;
      }
      else
      {
        inv_mask=0;
        if(lcd_clr)
        {
            lcd_clr=0;
            Delay_ms(1000);
        }
      }

      //read output
      output=0;
      for(k=0;k<20;k++)
      {
         output+=ADC_Get_Sample(0);
         Delay_ms(1);
      }
      output/=20;
     if(output<100)
     {
        if(fb_constant<0.7)fb_constant+=0.1;
     }
     if(output>115)
     {
        if(fb_constant>0.5)fb_constant-=0.1;
     }
   }
}// end