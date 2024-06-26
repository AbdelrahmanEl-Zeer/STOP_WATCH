/*
 * STOP__WATCH.C
 *
 *  Created on: May 18, 2024
 *      Author: El-Zeer
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

// variable to count the number of timer ticks (compare match)
unsigned char tick = 0;

// flag will be set when the timer count 1 second
unsigned char flag = 0;

// variables to hold the clock time
unsigned char seconds = 0;
unsigned char minutes = 0;
unsigned char hours = 0;

// Interrupt Service Routine for timer1 compare mode channel A
ISR(TIMER1_COMPA_vect)
{
	flag = 1;
}

void Timer1_Init_CTC_Mode(unsigned short tick)
{
	TCNT1 = 0; //timer initial value
	OCR1A  = tick; //compare value
	TIMSK |= (1<<OCIE1A); //enable compare interrupt for channel A
	/* Configure timer1 control registers
	 * 1. Non PWM mode FOC1A=1 and FOC1B=1
	 * 2. No need for OC1A & OC1B in this example so COM1A0=0 & COM1A1=0 & COM1B0=0 & COM1B1=0
	 * 3. CTC Mode and compare value in OCR1A WGM10=0 & WGM11=0 & WGM12=1 & WGM13=0
	 */
	TCCR1A = (1<<FOC1A) | (1<<FOC1B);
	/*
	 * 4. Clock = F_CPU/1024 CS10=1 CS11=0 CS12=1
	 */
	TCCR1B = (1<<WGM12) | (1<<CS10) | (1<<CS12);
}

// External INT0 Interrupt Service Routine
ISR(INT0_vect)
{
	seconds = 0;
	minutes = 0;
	hours = 0;
}

// External INT1 Interrupt Service Routine
ISR(INT1_vect)
{
	// Pause the stop watch by disable the timer
	// Clear the timer clock bits (CS10=0 CS11=0 CS12=0) to stop the timer clock.
	TCCR1B &= 0xF8;
}

// External INT2 Interrupt Service Routine
ISR(INT2_vect)
{
	// resume the stop watch by enable the timer through the clock bits.
	TCCR1B |= (1<<CS10) | (1<<CS12);
}

// External INT0 Enable and Configuration function
void INT0_Init(void)
{
	// Configure INT0/PD2 as input pin
	DDRD  &= (~(1<<PD2));
	// Enable internal pull up resistor at INT0/PD2 pin
	PORTD |= (1<<PD2);
	// Trigger INT0 with the falling edge
	MCUCR &= ~(1<<ISC00);
	MCUCR |= (1<<ISC01);
	// Enable external interrupt pin INT0
	GICR  |= (1<<INT0);
}

// External INT1 Enable and Configuration function
void INT1_Init(void)
{
	// Configure INT1/PD3 as input pin
	DDRD  &= (~(1<<PD3));
	// Trigger INT1 with the raising edge
	MCUCR |= (1<<ISC10);
	MCUCR |= (1<<ISC11);
	// Enable external interrupt pin INT1
	GICR  |= (1<<INT1);
}

// External INT2 Enable and Configuration function
void INT2_Init(void)
{
	// Configure INT2/PB2 as input pin
	DDRB   &= (~(1<<PB2));
	// Enable internal pull up resistor at INT2/PB2 pin
    PORTB |= (1<<PB2);
	// Trigger INT2 with the falling edge
	MCUCSR &= !(1<<ISC2);
	// Enable external interrupt pin INT2
	GICR   |= (1<<INT2);
}

int main(void)
{
	// Enable all the 7-Segments and initialize all of them with zero value
	// configure first 6 pins in PORTA as output pins
    DDRA |= 0x3F;
    PORTA |= 0x3F;



    // configure first four pins of PORTC as output pins
 	DDRC |= 0x0F;
	PORTC &= 0xF0;

	// Enable global interrupts in MC.
	SREG  |= (1<<7);

	// Start timer1 to generate compare interrupt every 1000 MiliSeconds(1 Second)
	Timer1_Init_CTC_Mode(1000);

	// Activate external interrupt INT0
	INT0_Init();

	// Activate external interrupt INT1
	INT1_Init();

	// Activate external interrupt INT2
	INT2_Init();

    while(1)
    {
    	if(flag == 1)
    	{
    		//enter here every one second
    		//increment seconds count
    		seconds++;

    		if(seconds == 60)
    		{
    			seconds = 0;
    			minutes++;
    		}
    		if(minutes == 60)
    		{
    			minutes = 0;
    			hours++;
    		}
    		if(hours == 24)
    		{
    			hours = 0;
    		}
    	    // reset the flag again
    	    flag = 0;
    	}
    	else
    	{
    		// out the number of seconds
    		PORTA = (PORTA & 0xC0) | 0x01;
    		PORTC = (PORTC & 0xF0) | (seconds%10);

    		// make small delay to see the changes in the 7-segment
    		// 2Miliseconds delay will not effect the seconds count
    		_delay_ms(2);

    		PORTA = (PORTA & 0xC0) | 0x02;
    		PORTC = (PORTC & 0xF0) | (seconds/10);

    		_delay_ms(2);

    		// out the number of minutes
    		PORTA = (PORTA & 0xC0) | 0x04;
    		PORTC = (PORTC & 0xF0) | (minutes%10);

    		_delay_ms(2);

    		PORTA = (PORTA & 0xC0) | 0x08;
    	    PORTC = (PORTC & 0xF0) | (minutes/10);

    	    _delay_ms(2);

    	    // out the number of hours
    	    PORTA = (PORTA & 0xC0) | 0x10;
    	    PORTC = (PORTC & 0xF0) | (hours%10);

    	    _delay_ms(2);

    	    PORTA = (PORTA & 0xC0) | 0x20;
    	    PORTC = (PORTC & 0xF0) | (hours/10);

    	    _delay_ms(2);
    	}
    }
}


