/*
 * AMIT Project.c
 *
 * Created: 2/1/2018 5:11:34 PM
 * Author : Raafat Abualazm
 */ 

#define F_CPU 1000000UL
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile int16_t adcVal = 0;
volatile int8_t processed_receive = 0;
volatile int8_t processed_send = 0;
ISR(INT1_vect) {
	PORTC ^= (1 << PC5);
}

void start_INT1() {
	DDRA = 0x00;
	DDRA |= (1 << PA6) | (1 << PA7);
	DDRB = 0xFF;
	DDRC |= (1 << PC0) | (1 << PC5);
	PORTD |= (1 << PD3);
	GICR |= (1 << INT1);
	MCUCR |= (1 << ISC11);
	sei();
}

void start_ADC() {
	ADCSRA |= (1 << ADEN);
	ADCSRA |= (1 << ADPS0) | (1 << ADPS1) | (1 << ADPS2);
	ADMUX  |=(1 << ADLAR);
}
void convert() {
	ADCSRA |= (1 << ADSC);
	while (bit_is_set(ADCSRA, ADSC));
	adcVal = (ADCL >> 6) | (ADCH << 2);
}

void start_UART() {
	uint16_t ubrr_val = 6;
	UBRRH = (uint8_t)(ubrr_val >> 8);
	
	UBRRL = (uint8_t)ubrr_val;
	UCSRB |= (1 << TXEN) | (1 << RXEN);
	UCSRC |= (1 << UCSZ1) | (1 << UCSZ0);
	
}
/*ISR(USART_TXC_vect) {
	while (((UCSRA >> UDRE) & 1) == 0);
	UDR = adcVal_send;
}

ISR(USART_RXC_vect) {
	adcVal_receive = UDR;

}
*/
void send() {
	while (bit_is_clear(UCSRA, UDRE));
	 UDR = (int8_t)processed_send;
	
}

void receive() {
	while(bit_is_clear(UCSRA, RXC));
	processed_receive =(int8_t)UDR;
}

void showOn7SEG() 
{
	uint8_t first_digit = processed_receive % 10;
	uint8_t second_digit = (processed_receive - first_digit) / 10;
	
	PORTA |= (1 << PA6);
	PORTA &= ~(1 << PA7);
	PORTB = first_digit;
	_delay_ms(50);
	PORTA |= (1 << PA7);
	PORTA &= ~(1 << PA6);
	PORTB = second_digit;

}
int main(void)
{
	
	start_INT1();
	start_ADC();
	start_UART();
    while (1) 
    {
		convert();
		processed_send = (adcVal / 10) - 24;
		send();
		receive();
		if (processed_receive >= 50) PORTC |= (1 << PC0);
		else if (processed_receive < 50) PORTC &= ~(1 << PC0);
		if (processed_receive < 0)
		{
			processed_receive = 0;
		}
		showOn7SEG();
		
    }
}

