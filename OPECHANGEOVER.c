#define RELAY GP1_BIT
#define MAINSLED GP5_BIT
#define GRIDVOLTAGE GP0_BIT
unsigned int VOLTAGE;
unsigned int counter, count;
short kk=0;
void Interrupt(){
if (TMR0IF_bit==1 && TMR0IE_bit==1){  
VOLTAGE=ADC_Read(0);
if(VOLTAGE>300 && VOLTAGE<600){
counter++;
if(counter==350){
RELAY=1;
MAINSLED=1;
//counter=0;
}
}
else if(VOLTAGE<300 || VOLTAGE >600){
counter=0;
RELAY=0;
MAINSLED=0;
}
TMR0IF_bit =0;
TMR0= 22;
}
}
void InitTimer0(){//generating 30ms interrupt
OPTION_REG=0x86;
TMR0=22;
INTCON=0xA0;
}
void main() {
CMCON0|=0x07;
ANSEL=1;
ADCON0=0x01;
VRCON=0;
TRISIO=0b00000001;
GPIO=0;
for(kk=0; kk<14; kk++){
 MAINSLED=~MAINSLED;
 delay_ms(500);
}
MAINSLED=0;
ADC_Init();
InitTimer0();
while(1){

}
}