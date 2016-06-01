//==============================================================================
#include "msp430.h"
#include "fonts.h"
//==============================================================================
unsigned char byteReorder(unsigned char aByte)
{
  unsigned char bOut = 0;
  
  if(aByte & (1 << 0)) bOut |= BIT7;
  if(aByte & (1 << 1)) bOut |= BIT6;
  if(aByte & (1 << 2)) bOut |= BIT0;
  if(aByte & (1 << 3)) bOut |= BIT5;
  if(aByte & (1 << 4)) bOut |= BIT1;
  if(aByte & (1 << 5)) bOut |= BIT3;
  if(aByte & (1 << 6)) bOut |= BIT2;
  if(aByte & (1 << 7)) bOut |= BIT4;  
  
//  if(aByte & (1 << 0)) bOut |= BIT4;
//  if(aByte & (1 << 1)) bOut |= BIT2;
//  if(aByte & (1 << 2)) bOut |= BIT3;
//  if(aByte & (1 << 3)) bOut |= BIT1;
//  if(aByte & (1 << 4)) bOut |= BIT5;
//  if(aByte & (1 << 5)) bOut |= BIT0;
//  if(aByte & (1 << 6)) bOut |= BIT6;
//  if(aByte & (1 << 7)) bOut |= BIT7;  
  
  return bOut;
}
//==============================================================================
#define ARRCNT (10)
unsigned int light_buffer[10];
//==============================================================================
void ADC_init(void)
{  
  P1DIR    &= ~(BIT0);
  P1SEL    |=  (BIT0);
  ADC10AE0 |=  (BIT0);
  ADC10CTL0 = ADC10ON + REFON +  ADC10SHT_3 + MSC +  SREF_0;
//  ADC10CTL1  = INCH_4 + CONSEQ_2;
  ADC10DTC1  = ARRCNT;  
}
//==============================================================================
unsigned int ADCmeasure(void)
{
  ADC10SA   = (unsigned int)light_buffer;    
  ADC10CTL1  = INCH_0 + CONSEQ_2;
  ADC10CTL0 |= ENC + ADC10SC;             
  
//  __disable_interrupt();
  
  while (!(ADC10CTL0 & ADC10IFG))   ;
    ADC10CTL0 &= ~ENC; 
  
  unsigned long UBat = 0;
  for (unsigned int i = 1; i < ARRCNT; i++) UBat += light_buffer[i];
  UBat /= (ARRCNT-1);    
  return (unsigned int)UBat;
}
//==============================================================================
unsigned int adc;
//==============================================================================
unsigned char string[] = {"- * Егор * - * Biker * -"};
//==============================================================================
void main( void )
{
  // Stop watchdog timer to prevent time out reset
  WDTCTL = WDTPW + WDTHOLD;
  
  P2OUT &= ~0xff;
  P2SEL &= ~0xff;
  P2DIR |=  0xff;  
  
  P1DIR &= ~BIT7;
  P1IFG &= ~BIT7;
  P1IE  |=  BIT7;
  
  TACCR0 = 1000;
  TACCTL0 |= CCIE;
  TACTL = TASSEL_2;
  TACTL |= MC_1;
  
  ADC_init();
  
  __enable_interrupt();
  while(1);
  {
    //__low_power_mode_4();
  }
}
//==============================================================================
#pragma vector = PORT1_VECTOR
__interrupt void sync_isr (void)
{
  P1IFG &= ~BIT7;
  TACTL |= MC_1;
  __low_power_mode_off_on_exit();
}
//==============================================================================
unsigned int blanc = 0;
unsigned int symNumber, symByteNumber; 
//==============================================================================
// Timer_A3 Interrupt Vector (TAIV) handler
#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A(void)
{
  blanc++;
  if(blanc > 1) { blanc = 0; P2OUT  = byteReorder(0);}
  else
  { 
    
    P2OUT  = byteReorder(Font8x6[string[symNumber] * 6 + symByteNumber] );
    symByteNumber++; 
    
    if(symNumber > (sizeof(string)- 1)) 
    {
      symNumber = 0;      
      symByteNumber = 0;      
      P2OUT = 0;
      adc = ADCmeasure();
      TACCR0 = adc*2 + 100;      
      TACTL &= ~MC_1; 
      __low_power_mode_4();
    }
    else
    {      
      if(symByteNumber > 5) 
      {
        symNumber++;
        symByteNumber = 0;
      }           
    }    
  } 
 
}