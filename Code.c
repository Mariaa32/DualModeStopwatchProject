

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


#define ResetButton PD2
#define PauseButton PD3
#define ResumeButton PB2
#define ToggleModeButton PB7
#define IncHrsButton PB1
#define DecHrsButton PB0
#define IncMinButton PB4
#define DecMinButton PB3
#define IncSecButton PB6
#define DecSecButton PB5
#define CountingUpLed PD4   // Red LED
#define CountingDownLed PD5 // Yellow LED
#define Buzzer PD0

#define SegmentControl PORTA
#define SegmentPort PORTC
#define ButtonPressed 0

uint8_t hours = 0, minutes = 0, seconds = 0;
uint8_t mode = 0;

uint8_t buzzerFlag = 0;

void Timer1SetUp() {
	TCNT1 = 0;
	OCR1A = 62500;  // Compare match value for 1-second delay
	TCCR1B |= (1 << WGM12); // Set CTC mode in TCCR1B
	TCCR1B |= (1 << CS12);  // Set prescaler to 256
	TIMSK |= (1 << OCIE1A); // Enable compare interrupt
	sei(); // Enable global interrupts
}

void InterruptsSetUp() {
	// Reset button (INT0)
	DDRD &= ~(1 << ResetButton);
	PORTD |= (1 << ResetButton); // Enable pull-up resistor
	MCUCR |= (1 << ISC01);        // Falling edge on INT0
	GICR |= (1 << INT0);

	// Pause button (INT1)
	DDRD &= ~(1 << PauseButton);
	PORTD |= (1 << PauseButton);  // Enable pull-up resistor
	MCUCR |= (1 << ISC11) | (1 << ISC10); // Rising edge on INT1
	GICR |= (1 << INT1);

	// Resume button (INT2)
	DDRB &= ~(1 << ResumeButton);
	PORTB |= (1 << ResumeButton); // Enable pull-up resistor
	MCUCSR &= ~(1 << ISC2);       // Falling edge on INT2
	GICR |= (1 << INT2);
}

void IncrementTime() {
	PORTD |= (1 << CountingUpLed);
	PORTD &= ~(1 << CountingDownLed);
	seconds++;
	if (seconds == 60) {
		seconds = 0;
		minutes++;
		if (minutes == 60) {
			minutes = 0;
			hours++;
			if (hours == 24) {
				hours = 0;
			}
		}
	}
}

void DecrementTime() {
	PORTD |= (1 << CountingDownLed);
	PORTD &= ~(1 << CountingUpLed);
	if (hours == 0 && minutes == 0 && seconds == 0) {
		PORTD |= (1 << Buzzer);

		}
		else {
		if (seconds == 0) {
			if (minutes == 0) {
				hours--;
				minutes = 59;
				seconds = 59;
				}
				else {
				minutes--;
				seconds = 59;
			}
			}
			else {
			seconds--;
		}
	}
}

void AdjustTime() {
	//STATIC VARIABLES bec i want their values to be retained across multiple calls;
	// SO: the previous button state is remembered when the function is called again.

	static uint8_t prevIncHrsState = !ButtonPressed;
	static uint8_t prevDecHrsState = !ButtonPressed;
	static uint8_t prevIncMinState = !ButtonPressed;
	static uint8_t prevDecMinState = !ButtonPressed;
	static uint8_t prevIncSecState = !ButtonPressed;
	static uint8_t prevDecSecState = !ButtonPressed;


	// one button press = one hour increment
if ((PINB & (1 << IncHrsButton)) != ButtonPressed) { // Button is pressed
		if (prevIncHrsState != ButtonPressed) { // Button was not previously pressed
			// Increment hours
			hours++;
		}
		prevIncHrsState = ButtonPressed; // Update the state
		}
		else {
		prevIncHrsState =! ButtonPressed; // Button is not pressed
	}

if ((PINB & (1 << DecHrsButton)) != ButtonPressed) {
		if (prevDecHrsState != ButtonPressed)
		{
			if(hours>0)
			{
			hours--;
			}
			else
			{
				hours=0;
			}
		}
		prevDecHrsState = ButtonPressed;
		}
		else
		{
		prevDecHrsState =! ButtonPressed;
	}

if ((PINB & (1 << IncMinButton)) != ButtonPressed) {
		if (prevIncMinState != ButtonPressed) {

			minutes++;
		}
		prevIncMinState = ButtonPressed;
	}
	else {
		prevIncMinState = !ButtonPressed;
	}

if ((PINB & (1 << DecMinButton)) != ButtonPressed) {
		if (prevDecMinState != ButtonPressed)
		{
			if(minutes>0)
			{
				minutes--;
			}
			else
			{
				minutes=0;
			}
		}
		prevDecMinState = ButtonPressed;
	}
	else
	{
		prevDecMinState = !ButtonPressed;
	}


if ((PINB & (1 << IncSecButton)) != ButtonPressed) {
		if (prevIncSecState != ButtonPressed) {

			seconds++;
		}
		prevIncSecState = ButtonPressed;
	}
	else {
		prevIncSecState = !ButtonPressed;
	}


if ((PINB & (1 << DecSecButton)) != ButtonPressed) {
			if (prevDecSecState != ButtonPressed)
			{
				if(seconds>0)
				{
					seconds--;
				}
				else
				{
					seconds=0;
				}
			}
			prevDecSecState = ButtonPressed;
		}
		else
		{
			prevDecSecState = !ButtonPressed;
		}
}


void TimeDisplay(uint8_t hours, uint8_t minutes, uint8_t seconds) {
	for (int i = 0; i < 6; i++) {
		SegmentControl |= (1 << i);
		switch (i) {
			case 0:
			SegmentPort = hours / 10;
			break;
			case 1:
			SegmentPort = hours % 10;
			break;
			case 2:
			SegmentPort = minutes / 10;
			break;
			case 3:
			SegmentPort = minutes % 10;
			break;
			case 4:
			SegmentPort = seconds / 10;
			break;
			case 5:
			SegmentPort = seconds % 10;
			break;
		}
		_delay_ms(1);
		SegmentControl &= ~(1 << i);
	}
}

ISR(INT0_vect) {
	hours = 0;
	minutes = 0;
	seconds = 0;
	PORTD &= ~(1 << Buzzer);

}


ISR(INT1_vect) {
	TIMSK &= ~(1 << OCIE1A);
}


ISR(INT2_vect) {
	TIMSK |= (1 << OCIE1A);
}

ISR(TIMER1_COMPA_vect) {

		if (mode == 0) {
			IncrementTime();
			} else if (mode == 1) {
				PORTD |= (1 << CountingDownLed);
				PORTD &= ~(1 << CountingUpLed);
			DecrementTime();
		}
}

int main(void) {
uint8_t	prevToggleModeState =! ButtonPressed;
	DDRC |= 0xFF;
	DDRA |= 0xFF;
	DDRD |= (1 << CountingUpLed) | (1 << CountingDownLed) | (1 << Buzzer);

	InterruptsSetUp();
	Timer1SetUp();
	sei();
	while (1) {
		TimeDisplay(hours, minutes, seconds);
		AdjustTime();
		if ((PINB & (1 << ToggleModeButton)) != ButtonPressed) {
			if (prevToggleModeState != ButtonPressed) {
				mode = !mode;
				if (mode == 0) {
					PORTD |= (1 << CountingUpLed);
					PORTD &= ~(1 << CountingDownLed);
							}
				else if (mode == 1) {
								PORTD |= (1 << CountingDownLed);
								PORTD &= ~(1 << CountingUpLed);

						}
			}
			prevToggleModeState = ButtonPressed;
			}
		else {
			prevToggleModeState = !ButtonPressed;
		}
	}
}

