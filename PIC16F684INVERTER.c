const unsigned char sin_table[416]={
0, 16, 32, 47, 62, 77, 91, 103, 115, 126, 136, 144, 151, 156, 160, 162, 163, 162, 160, 156, 151, 144, 136, 126, 115, 103, 91, 77, 62, 47, 32, 16, //65%

0, 17, 33, 49, 65, 80, 94, 107, 120, 131, 141, 149, 156, 162, 166, 168, 169, 168, 166, 162, 156, 149, 141, 131, 120, 107, 94, 80, 65, 49, 33, 17, //67.5%

0, 17, 34, 51, 67, 82, 97, 111, 124, 135, 146, 154, 162, 167, 172, 174, 175, 174, 172, 167, 162, 154, 146, 135, 124, 111, 97, 82, 67, 51, 34, 17, //70%

0, 18, 35, 53, 69, 85, 101, 115, 128, 140, 150, 160, 167, 173, 178, 180, 181, 180, 178, 173, 167, 160, 150, 140, 128, 115, 101, 85, 69, 53, 35, 18, //72.5%

0, 18, 37, 55, 72, 89, 104, 119, 133, 145, 156, 166, 174, 180, 184, 187, 188, 187, 184, 180, 174, 166, 156, 145, 133, 119, 104, 89, 72, 55, 37, 18, //75%

0, 19, 38, 56, 74, 91, 108, 123, 137, 150, 161, 171, 179, 186, 190, 193, 194, 193, 190, 186, 179, 171, 161, 150, 137, 123, 108, 91, 74, 56, 38, 19, //77.5%

0, 20, 39, 58, 77, 94, 111, 127, 141, 155, 166, 176, 185, 191, 196, 199, 200, 199, 196, 191, 185, 176, 166, 155, 141, 127, 111, 94, 77, 58, 39, 20, //80%

0, 20, 40, 60, 79, 97, 114, 131, 146, 159, 171, 182, 190, 197, 202, 205, 206, 205, 202, 197, 190, 182, 171, 159, 146, 131, 114, 97, 79, 60, 40, 20, //82.5%

0, 21, 42, 62, 82, 100, 118, 135, 151, 165, 177, 188, 197, 204, 209, 212, 213, 212, 209, 204, 197, 188, 177, 165, 151, 135, 118, 100, 82, 62, 42, 21, //85

0, 21, 43, 64, 84, 103, 122, 139, 155, 169, 182, 193, 202, 210, 215, 218, 219, 218, 215, 210, 202, 193, 182, 169, 155, 139, 122, 103, 84, 64, 43, 21, //87.5%

0, 22, 44, 65, 86, 106, 125, 143, 159, 174, 187, 198, 208, 215, 221, 224, 225, 224, 221, 215, 208, 198, 187, 174, 159, 143, 125, 106, 86, 65, 44, 22, //90%

0, 23, 45, 67, 88, 109, 128, 147, 163, 179, 192, 204, 213, 221, 227, 230, 231, 230, 227, 221, 213, 204, 192, 179, 163, 147, 128, 109, 88, 67, 45, 23, //92.5%

0, 23, 46, 69, 91, 112, 132, 151, 168, 184, 198, 210, 220, 228, 233, 237, 238, 237, 233, 228, 220, 210, 198, 184, 168, 151, 132, 112, 91, 69, 46, 23
};

unsigned int TBL_POINTER_NEW, TBL_POINTER_OLD, TBL_POINTER_SHIFT, SET_FREQ;
unsigned int TBL_temp, FBV, BATTERY, output;
short fdivisor;
unsigned char DUTY_CYCLE, FB_Step, adder=320,pong;
short inv = 0;

int error;
#define REF_AMPLITUDE 512


unsigned int measure_voltage() {
    unsigned int adc_value;
    // Configure ADC
    ADCON0.ADON = 1;     // Enable ADC module
    ADCON0.ADFM = 1;
    ADCON0.VCFG = 0;
    ADCON0.CHS2 = 0; // Select channel 0
    ADCON0.CHS1 = 0; // Select channel 0
    ADCON0.CHS0 = 0; // Select channel 0
    ADCON1.ADCS2 = 1;
    ADCON1.ADCS1 = 0;
    ADCON1.ADCS0 = 0;
    // Start ADC conversion
    delay_us(20);
    ADCON0.GO = 1;       // Start conversion

    while (ADCON0.GO);   // Wait for conversion to complete
    // Read ADC result
    adc_value = ADRESH << 8 | ADRESL;
    // Disable ADC
    ADCON0.ADON = 0;     // Disable ADC module
    return adc_value;
}

void interrupt() {
    if (TMR2IF_bit == 1 && TMR2IE_bit == 1) {
        TBL_POINTER_NEW = TBL_POINTER_OLD + SET_FREQ;
        if (TBL_POINTER_NEW < TBL_POINTER_OLD) {
            CCP1CON.P1M1 = ~CCP1CON.P1M1; // Reverse direction of full-bridge
            PORTA.F2 = 1;
        }
        TBL_POINTER_SHIFT = TBL_POINTER_NEW >> 11;
        DUTY_CYCLE = TBL_POINTER_SHIFT;
        CCPR1L = sin_table[DUTY_CYCLE] + adder;
        TBL_POINTER_OLD = TBL_POINTER_NEW;
        TMR2IF_bit = 0;
    }
}

void main() {
    CMCON0 = 7; // Disable Comparator
    SET_FREQ = 410;
    TBL_POINTER_SHIFT = 0;
    TBL_POINTER_NEW = 0;
    TBL_POINTER_OLD = 0;
    DUTY_CYCLE = 0;
    PR2 = 249;
    TRISC = 0x00111101;
    TRISA0_BIT = 1;
    TRISA1_BIT = 1;
    TRISA2_BIT = 0;
    ANSEL.F0 = 1;
    ANSEL.F1 = 0;
    CCP1CON = 0x4C;
    T2CON.F2 = 1;
    TMR2IF_bit = 0;
    TMR2IE_bit = 1;
    GIE_bit = 1;
    PEIE_bit = 1;
    TRISC = 0b00000000;
    ADC_Init();
    PORTC.F0 = 0;
    for (;;) {
        output = measure_voltage();
        if ( output <490) {
if(adder < 416) adder += 32;
}
else if ( output > 510){
if(adder> 192) adder -= 32;
}
        /*
    if ( FBV > 250) pong -= 32;
else if ( FBV < 230) pong += 32;
if (pong <= 32) pong = 32;
else if (pong >= 384) pong = 384;
adder=pong<<5;
*/
    }
}