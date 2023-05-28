//programmer: Akingunsola Caleb
//date: july 7 2019
//microcontroller used:pic16f876a but can be ported to any microcontroller
//topology:H-BRIDGE OR FULL BRIDGE

unsigned int i=0;
const unsigned char sign_table_0[32]={0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
16, 15, 14, 13, 12, 11, 10, 9,8, 7,6, 5, 4, 3, 2, 1};
const unsigned char sign_table_1[32]={0, 17, 33, 49, 65, 80, 94, 107, 120, 131, 141, 149, 156, 162, 166, 168, 169, 168,
166, 162, 156, 149, 141, 131, 120, 107, 94, 80, 65, 49, 33, 17};
const unsigned char sign_table_2[32]={0, 17, 34, 51, 67, 82, 97, 111, 124, 135, 146, 154, 162, 167, 172, 174, 175, 174,
172, 167, 162, 154, 146, 135, 124, 111, 97, 82, 67, 51, 34, 17};
const unsigned char sign_table_3[32]={0, 18, 35, 53, 69, 85, 101, 115, 128, 140, 150, 160, 167, 173, 178, 180, 181,
180, 178, 173, 167, 160, 150, 140, 128, 115, 101, 85, 69, 53, 35, 18};
const unsigned char sign_table_4[32]={0, 18, 37, 55, 72, 89, 104, 119, 133, 145, 156, 166, 174, 180, 184, 187, 188,
187, 184, 180, 174, 166, 156, 145, 133, 119, 104, 89, 72, 55, 37, 18};
const unsigned char sign_table_5[32]={0, 20, 39, 58, 77, 94, 111, 127, 141, 155, 166, 176, 185, 191, 196, 199, 200,
199, 196, 191, 185, 176, 166, 155, 141, 127, 111, 94, 77, 58, 39, 20};
const unsigned char sign_table_6[32]={0, 21, 42, 62, 82, 100, 118, 135, 151, 165, 177, 188, 197, 204, 209, 212, 213,
212, 209, 204, 197, 188, 177, 165, 151, 135, 118, 100, 82, 62, 42, 21};
const unsigned char sign_table_7[32]={0, 22, 44, 65, 86, 106, 125, 143, 159, 174, 187, 198, 208, 215, 221, 224, 225,
224, 221, 215, 208, 198, 187, 174, 159, 143, 125, 106, 86, 65, 44, 22};
const unsigned char sign_table_8[32]={0, 23, 46, 69, 91, 112, 132, 151, 168, 184, 198, 210, 220, 228, 233, 237, 238,
237,233,228,220,210,198,184,168,151,132,112,91,69,46,23};
const unsigned char sign_table_9[32]={0, 24, 48, 72, 94, 117, 138, 158, 175, 190, 200, 219, 230, 240, 245, 250, 254,
250,245,240,230,219,200,190,175,158,138,117,94,72,48,24};
//note:constant unsigned was used so as not to overload the ram the sinewave value will be saved inside the rom of the mcu since the ram memory is very limited.note the ram memory is always volatile but the rom memory is static since the word constant is a static word.i.e it will never change value that was the concept behind it now with the tips have given you i believe you can eliminate ram full memory error in all your coding

unsigned int feed_back;         //feedback for reference
unsigned int voltage;
         short toggle=0;         //to toggle alternate state




   int blik;
    short en=0;
void Interrupt(){

  if (TMR1IF_bit){
    TMR1IF_bit = 0;                 //clear timer1 flag
 TMR1H         = 0xFB;                //set higher byte
  TMR1L         = 0xAA;               //set lower byte
    i++;                               //increment i,i=555us
    if(i==32)                            //if i==32  555*32=10000us=10ms
    {
    toggle=~toggle;
    i=0;
    if(toggle)
    rc2_bit=0;             //set pin low
    else
    rc1_bit=0;              //set pin low
    }
    if(toggle){
    rc0_bit=0;                //lowside2 =low
    rc3_bit=1;            // lowside 1 =high
    PWM1_Set_Duty(0);        //duty cycle 1 to 0
    switch(feed_back)             //output correction for feed-back
    {
     case 0:PWM2_Set_Duty(sign_table_0[i]);    //duty cyle=5%
     break;
     case 1:PWM2_Set_Duty(sign_table_1[i]);     //50%
     break;
     case 2:PWM2_Set_Duty(sign_table_2[i]);     //60%
     break;
     case 3:PWM2_Set_Duty(sign_table_3[i]);      //65%
     break;
     case 4:PWM2_Set_Duty(sign_table_4[i]);       //70
     break;
      case 5:PWM2_Set_Duty(sign_table_5[i]);    //  75
     break;
      case 6:PWM2_Set_Duty(sign_table_6[i]);      //80
     break;
      case 7:PWM2_Set_Duty(sign_table_7[i]);    //85
     break;
      case 8:PWM2_Set_Duty(sign_table_8[i]);     //90
     break;
      case 9:PWM2_Set_Duty(sign_table_9[i]);     //95
     break;
      case 10:PWM2_Set_Duty(sign_table_0[i]);    //5% for proper connection
     break;
    }
    }
    else
    {
      rc3_bit=0;         //set side1 low
        rc0_bit=1;        //side 2 high

    PWM2_Set_Duty(0);
     switch(feed_back)
    {
    case 0:PWM1_Set_Duty(sign_table_0[i]);
     break;
     case 1:PWM1_Set_Duty(sign_table_1[i]);
     break;
     case 2:PWM1_Set_Duty(sign_table_2[i]);
     break;
     case 3:PWM1_Set_Duty(sign_table_3[i]);
     break;
     case 4:PWM1_Set_Duty(sign_table_4[i]);
     break;
      case 5:PWM1_Set_Duty(sign_table_5[i]);
     break;
      case 6:PWM1_Set_Duty(sign_table_6[i]);
     break;
      case 7:PWM1_Set_Duty(sign_table_7[i]);
     break;
      case 8:PWM1_Set_Duty(sign_table_8[i]);
     break;
      case 9:PWM1_Set_Duty(sign_table_9[i]);
     break;
     case 10:PWM1_Set_Duty(sign_table_0[i]);
     break;

    }
   }
  }



}
                 int a;
void main() {

  CMCON|=0x07;
  TRISA=0xff;
  TRISC=0x00;
  PORTC=0x00;
  trisb0_bit=1;
  ADCON1=0x00;
     ADCON0=0x01;
  PWM1_Init(20000);      //20khz
  PWM1_Start();
    PWM2_Init(20000);
  PWM2_Start();
      T1CON         = 0x01;      //start timer1
  TMR1IF_bit         = 0;
TMR1H         = 0xFB;
  TMR1L         = 0xAA;
  TMR1IE_bit         = 1;
  //INTCON.PEIE=1;

  //INTCON         = 0xC0;

     ADC_Init();

     while(1)
     {
     if(!rb1_bit){
         INTCON.PEIE=1;   INTCON.GIE=1;
         }
         if(rb1_bit)
         {
           INTCON.PEIE=0;   INTCON.GIE=0;
         rc1_bit=0;
         rc2_bit=0;
           RC0_BIT=RC3_BIT=0;
           }
         //feed back take place here
         voltage=ADC_Read(0);
         voltage=voltage/100;

         feed_back=voltage;
        delay_ms(100);

     }
}