
#define VoltageFB 1
#define VoltageBat 0
#define Button PORTBbits.RB7
#define Fan PORTBbits.RB2
#define Relay PORTBbits.RB1
#define SPWMA PORTCbits.RC5
#define SPWMB PORTCbits.RC0
#define ChangeoverPin PORTBbits.RB0
#define ShortcctPin PORTBbits.RB6

const uint8_t SineValue[64] = {
      0, 8, 15, 23, 30, 38, 45, 52, 59, 66, 73, 80, 86, 92, 98, 104, 110, 115, 120, 124, 129, 133, 137, 140,
      143, 146, 148, 150, 152, 153, 154, 155, 155, 155, 154, 153, 152, 150, 148, 146, 143, 140, 137, 133, 129,
      124, 120, 115, 110, 104, 98, 92, 86, 80, 73, 66, 59, 52, 45, 38, 30, 23, 15, 8};

int TimeSec, FaultTag, ChargeDuty, ForLoop ;
unsigned int InitADC, ADCValueHold, CurrentFB, NTC, Temp;
int InverterPhase,DutyPoints, BatPercent, TimerCounter, GridTimer, CheckCycle;
bit PowerState =0, Use1 = 0, Use2 = 0, GridON = 0, ChargingState = 0,  FaultState = 0, GridCharge = 0, ChargePhase = 0;
float FeedBackVoltage,  LoadPower, DischargeCurrent,  BatVolt, Battery_Resolution, Percent_Resolution, Current_Resolution;
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

void Timer1Start(){
  T1CON    = 0x01;
  TMR1IF_bit = 0;
  TMR1H    = 0xB1;
  TMR1L    = 0xE0;
  INTCON = 0xC0;
  TMR1IE_bit = 1;
}
void INT0_Init(){
    INTCON.INTF = 0;
    OPTION_REG.INTEDG = 1;
    INTCON.INTE = 1;
}

void SystemON(){
    InverterPhase = 1;
    SPWMA = 0, SPWMB = 0;
    T2CON = 0x1D;
    PR2 = 38;
    TMR2IF_bit = 0;
    CCP1CON = 0X0C; // Enable PWM1
    CCP2CON = 0X00; // Enable PWM2
    CCPR1H = 0;
    CCPR2H = 0;
    TMR2IE_bit = 1;
}

void SystemOFF(){
    ShortcctPin = 0, InverterPhase = 0, DutyPoints = 0, CheckCycle = 0;
    SPWMA = 0, SPWMB = 0;
    T2CON = 0;
    PR2 = 0;
    TMR2IF_bit = 0;
    TMR2IE_bit = 0;
    CCP1CON = 0; // Disable PWM1
    CCP2CON = 0; // Disable PWM2
    PORTC.RC1 = 0;
    PORTC.RC2 = 0;
    delay_us(10);
    Relay = 0;
}

void PWM1_LoadDutyValue(unsigned int dutyValue)
{
    dutyValue &= 0x03FF;
    CCPR1H = dutyValue;
    CCPR1L = dutyValue >> 2;
}

void PWM2_LoadDutyValue(unsigned int dutyValue)
{
    dutyValue &= 0x03FF;
    CCPR2H = dutyValue ;
    CCPR2L = dutyValue >> 2;
}



void FBCalc(){
    ADCValueHold = 0;
    for(ForLoop = 0; ForLoop < 10; ForLoop++){
        ADCValueHold  += ADC_GET(VoltageFB);
    }
    (ADCValueHold > 640)? (FeedBackVoltage = ADCValueHold * 1024) : (FeedBackVoltage = 0);
}

void LoadCheck(){
    ADCValueHold = 0;
    for(ForLoop = 0; ForLoop < 10; ForLoop++){
     ADCValueHold  += ADC_GET(VoltageBat);
    }
    (ADCValueHold > 640)? (BatVolt = ADCValueHold * Battery_Resolution) : (BatVolt = 0);

    ADCValueHold = 0;
    for(ForLoop = 0; ForLoop < 10; ForLoop++){
        ADCValueHold += ADC_GET(CurrentFB);
    }
    ADCValueHold = (ADCValueHold * 0.1) + 8;
    if (ADCValueHold < InitADC){
        DischargeCurrent =  (InitADC - ADCValueHold) * Current_Resolution;
        LoadPower = BatVolt * DischargeCurrent;
    }
    if (LoadPower > 50) LoadPower = LoadPower - 25;
    else LoadPower = 0;

    if (DischargeCurrent >= 25){
        FaultState = true;
        SystemOFF();
        FaultTag = 2;
    }

    (BatVolt > 10)? (BatPercent = (BatVolt - 25) * Percent_Resolution) : (BatPercent = 0);

    if (LoadPower >= 1500){ /// overload
        FaultState = true;
        SystemOFF();
        FaultTag = 4;
    }

    if (BatVolt <= 24.8 && PowerState){
        FaultState = true;
        SystemOFF();
        FaultTag = 1;
    }

    ///// Temperature Measure
    ADCValueHold  = 0;
    for(ForLoop = 0; ForLoop < 10; ForLoop++){
      ADCValueHold  += ADC_GET(NTC);
//      __delay_us(1);
    }
    ADCValueHold = ADCValueHold * 0.1;
    if (ADCValueHold > 100){
        Temp = (102300/ADCValueHold) - 100;
        if (Temp >= 55 && Fan == 1) Fan = 0;
        if (Temp <= 45 && Fan == 0) Fan = 1;
        if (Temp <= 24 && !FaultState){
            SystemOFF();
            FaultState = true;
            FaultTag = 3;
            if (GridCharge)GridCharge = false;
        }
        if(Temp >= 30 && FaultState && FaultTag == 3) FaultState = false;
    }
    else if (PowerState || ChargingState) {
        SystemOFF();
        FaultState = true;
        FaultTag = 3;
        Fan = 0;
    }
}

void SystemControl(){
    if (Button == 1){
        DelayTimer(10);
        if(Button == 1 && !PowerState ){
            if (ChangeoverPin == 1 && !FaultState) {
                Relay = 1;
              DelayTimer(10);
              SystemON();
            }
            DelayTimer(5);
            PowerState = true;
            ShortcctPin = 1;
        }
        else if(Button == 1 && (PowerState || FaultState)){
            ShortcctPin = 0, FaultTag = 0;
            SystemOFF();
            PowerState = false;
            if (Temp >= 30) FaultState = false;
        }
    }

    if (ChangeoverPin == 0 && !GridON){
        DelayTimer(100);
        if (ChangeoverPin == 0){
            if (PowerState) SystemOFF();
            GridON = true;
            INT0_Init();
        }
    }

    if (GridTimer == 2) ShortcctPin = 1;

    if (GridON && !GridCharge && BatVolt < 32 && !FaultState){
        ChargePhase = true;
        T2CON = 0x1D;
        PR2 = 38;
        GridCharge = true;
    }

}


void __interrupt()INTERRUPT_InterruptManager (void)
{
    if (PIE1bits.TMR2IE == 1 && PIR1bits.TMR2IF == 1){
        PIR1bits.TMR2IF = 0;
        DutyPoints++;
        if (DutyPoints == 64){
            DutyPoints = 0;
            PWM1_LoadDutyValue(0);
            PWM2_LoadDutyValue(0);
            PORTCbits.RC1 = 0;
            PORTCbits.RC2 = 0;
            CCP1CON = 0X00; // Enable PWM1
            CCP2CON = 0X00;
            SPWMA = 0;
            SPWMB = 0;
            __delay_us(1);
            if (InverterPhase == 2){
               InverterPhase = 1;
               SPWMA = 1;
               CCP1CON = 0X0C;
            }
            else if (InverterPhase == 1){
                InverterPhase = 2;
                SPWMB = 1;
                CCP2CON = 0X0C;
            }
            CheckCycle++;
        }

        if (CheckCycle == 2){
            CheckCycle = 0;
            if (FeedBackVoltage > 1.6) DutyCycle -= 0.2;
            else if (FeedBackVoltage < 1.55) DutyCycle += 0.2;
            if (DutyCycle <= 60) DutyCycle = 60;
            else if (DutyCycle >= 99) DutyCycle = 99;
            if (FeedBackVoltage < 0.05) DutyCycle = 65;
        }

        if (InverterPhase == 1){
            if (TableSelect == 1)PWM1_LoadDutyValue (SineValue[DutyPoints]);
            else if (TableSelect == 2)PWM1_LoadDutyValue(SineValue[DutyPoints]);
        }
        else if (InverterPhase == 2){
            if (TableSelect == 1)PWM2_LoadDutyValue (SineValue[DutyPoints]);
            else if (TableSelect == 2)PWM2_LoadDutyValue(SineValue[DutyPoints]);
        }

    }
    else if (PIE1bits.TMR1IE == 1 && PIR1bits.TMR1IF == 1){
        PIR1bits.TMR1IF = 0;
        TMR1H    = 0xB1;
        TMR1L    = 0xE0;
        TimerCounter++;
        ////////////////
        if (GridON && GridCharge && BatVolt < 32){
            ChargePhase = !ChargePhase;
            if (!ChargePhase){
                CCP2CON = 0;
                PORTCbits.RC1 = 0;
                CCP1CON = 0X0C;
                PWM1_LoadDutyValue(90);
            }
            else if (ChargePhase){
                CCP1CON = 0;
                PORTCbits.RC2 = 0;
                CCP2CON = 0X0C;
                PWM2_LoadDutyValue(90);
            }
        }
        else if (GridON && GridCharge && BatVolt > 33){
            T2CON = 0;
            PR2 = 0;
            GridCharge = false;
        }
        /////////
        if (TimerCounter == 200){
             TimerCounter = 0;
             TimeSec++;
             PORTCbits.RC4 = ~PORTCbits.RC4;
             (PowerState && !GridON && ShortcctPin == 0 && !FaultState)? (GridTimer++):(GridTimer = 0);
        }
    }
    else if (INTCONbits.INTE && INTCONbits.INTF){
        if (PowerState){
            Relay = 1;
            SystemON();
        }
        GridON = false;
        GridCharge = false;
        INTCONbits.INTF = 0;
        INTCONbits.INTE = 0;
    }
}

void main() {
    DelayTimer(30);
    UART_Init(9600);
    TRISC = 0x80;
    PORTC = 0;
    TRISB = 0X81;
    PORTB = 0x0C;
    TRISA = 0XEF;
    PORTA = 0;
    Timer1Start();
    ADC_Init();
    DelayTimer(40);
    FBCalc();
    TableCheck();
    Timer1Start();
    PORTB = 0;

    while(1){
        FBCalc();
        SystemControl();
        LoadCheck();
    }
}