#include "p24fj64ga002.h"
#include "lcd.h"
#include "timer.h"
#include "adc.h"
#include "pwm.h"
#include "sw1.h"
#include <stdio.h>

#define DEBOUNCE_DELAY_US 5000


_CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF & BKBUG_ON & COE_OFF & ICS_PGx1 &
          FWDTEN_OFF & WINDIS_OFF & FWPSA_PR128 & WDTPS_PS32768 )

_CONFIG2( IESO_OFF & SOSCSEL_SOSC & WUTSEL_LEG & FNOSC_PRIPLL & FCKSM_CSDCMD & OSCIOFNC_OFF &
          IOL1WAY_OFF & I2C1SEL_PRI & POSCMOD_XT )

typedef enum stateTypeEnum{
    idle_forward,
    idle_backward,
    forward,
    backward,
    displayLCDMessage,
    debouncePress,
    debounceRelease,
    wait,

}stateType;

volatile stateType currState=idle_forward;
volatile stateType mode=backward;
volatile int ad0val;
volatile double voltage=0;

int main(void)
{
    char v[10];
    int percent1=0;
    int percent2=0;
    
    initSW1();
    initLCD();
    initADC();
    //initTimer2(); //enabling this breaks everything for some reason?
    initMotorOnePWM();
    initMotorTwoPWM();
    T3CONbits.TON = 1;//PUT THIS AFTER PWM INITS

    while(1)
    {
        voltage = 3.3*((double)ad0val)/1023; //3.3 volts times the ratio
        moveCursorLCD(0,1);//move the cursor to center the text
        sprintf(v, "%.3f V", voltage); //make a new string out of the voltage float
        printStringLCD(v); //print the new string to the LCD
        
        switch (currState)
        {
            case wait:
                
                break;

            case idle_forward:
                setDutyCycle(MOTOR_ONE,0);
                setDutyCycle(MOTOR_TWO,0);
                moveCursorLCD(1,0); //move cursor second row.
                printStringLCD("        ");
                moveCursorLCD(1,0); //move cursor second row.
                printStringLCD("IDLE");
                currState=wait;
                break;

            case idle_backward:
                setDutyCycle(MOTOR_ONE,0);
                setDutyCycle(MOTOR_TWO,0);
                moveCursorLCD(1,0); //move cursor second row.
                printStringLCD("        ");
                moveCursorLCD(1,0); //move cursor second row.
                printStringLCD("IDLE");
                currState=wait;
                break;

            case debouncePress:
                delayUs(DEBOUNCE_DELAY_US);
                currState=wait;
                break;

            case debounceRelease:
                delayUs(DEBOUNCE_DELAY_US);
                currState=displayLCDMessage;
                break;

            case displayLCDMessage:
                if(mode==idle_forward)
                {
                    currState=idle_forward;
                }
                else if(mode==forward)
                {
                    moveCursorLCD(1,0);
                    printStringLCD("        ");
                    moveCursorLCD(1,0);
                    printStringLCD("FORWARD");
                    currState=forward;
                }
                else if(mode==idle_backward)
                {
                    currState=idle_backward;
                }
                else if(mode==backward)
                {
                    moveCursorLCD(1,0);
                    printStringLCD("        ");
                    moveCursorLCD(1,0);
                    printStringLCD("BACKWARD");
                    currState=backward;
                }
                break;

            case forward:
                setDirection(MOTOR_ONE, FORWARD);
                setDirection(MOTOR_TWO, FORWARD);
                
                if(ad0val>511)
                {
                    percent1=((1023-ad0val)/512)*100;
                    percent2=100;
                }
                else if(ad0val<=511)
                {
                    percent1=100;
                    percent2=(ad0val/511)*100;
                }

                setDutyCycle(MOTOR_ONE, percent1);
                setDutyCycle(MOTOR_TWO, percent2);

        

                break;

            case backward:
                setDirection(MOTOR_ONE, REVERSE);
                setDirection(MOTOR_TWO, REVERSE);

                if(ad0val>511)
                {
                    percent1=((1023-ad0val)/512)*100;
                    percent2=100;
                }
                else if(ad0val<=511)
                {
                    percent1=100;
                    percent2=(ad0val/511)*100;
                }

                setDutyCycle(MOTOR_ONE, percent1);
                setDutyCycle(MOTOR_TWO, percent2);

           

                break;


        }
        
    }


    return 0;
}

void _ISR _CNInterrupt()
{
    IFS1bits.CNIF=0;
    if(SW1PORT==PRESSED)
    {
        currState=debouncePress;
        if(mode==idle_forward)
        {
            mode=forward;
        }
        else if(mode==forward)
        {
            mode=idle_backward;
        }
        else if(mode==idle_backward)
        {
            mode=backward;
        }
        else if(mode==backward)
        {
            mode=idle_forward;
        }
    }
    else if(SW1PORT==RELEASED)
    {
        currState=debounceRelease;
    }
}

//will be called after 10 samples
void _ISR _ADC1Interrupt(void){
    IFS0bits.AD1IF = 0;

    int i = 0;
    unsigned int *adcPtr;
    ad0val = 0;
    adcPtr = (unsigned int *) (&ADC1BUF0);

    for(i = 0; i < 16; i++){
        ad0val = ad0val + *adcPtr/16;
        adcPtr++;
    }
}