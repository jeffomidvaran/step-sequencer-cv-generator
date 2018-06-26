
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include <math.h>
#include "lcd.h"
#include "avr.h"
#include "notes.h"

#define TRUE 1
#define FALSE 0
#define DEFAULT_LEN 60000
char buf[100];

typedef enum NOTE_LENGTH{
	whole, half, quarter, eighth, sixteenth, thirtySecond
} NoteLength;

typedef struct SONG{
	float tempo;
	float currentDivision;
	NoteLength noteLength; 
} Song;

Song song;

void printInfo(Song *song){
	clr_lcd();
	
	switch(song->noteLength){
		case whole:
			sprintf(buf, "bpm: %d whole", (int)round(song->tempo));
			break;
		case half:
			sprintf(buf, "bpm: %d 1/2", (int)round(song->tempo));
			break;
		case quarter:
			sprintf(buf, "bpm: %d 1/4", (int)round(song->tempo));
			break;
		case eighth:
			sprintf(buf, "bpm: %d 8th", (int)round(song->tempo));
			break;
		case sixteenth:
			sprintf(buf, "bpm: %d 16th", (int)round(song->tempo));
			break;
		case thirtySecond:
			sprintf(buf, "bpm: %d 32nd", (int)round(song->tempo));
			break;
		default:
			break;
	}
	
	puts_lcd2(buf);
}
void cycleThroughSteps(Song *song){
	SET_BIT(PORTC, 0);
	printInfo(song);
	wait_avr(song->currentDivision);
	CLR_BIT(PORTC, 0);
	SET_BIT(PORTC, 1);
	
	printInfo(song);
	wait_avr(song->currentDivision);
	CLR_BIT(PORTC, 1);
	SET_BIT(PORTC, 2);
	
	printInfo(song);
	wait_avr(song->currentDivision);
	CLR_BIT(PORTC, 2);
	SET_BIT(PORTC, 3);
	
	printInfo(song);
	wait_avr(song->currentDivision);
	CLR_BIT(PORTC, 3);
	SET_BIT(PORTC, 4);
	
	printInfo(song);
	wait_avr(song->currentDivision);
	CLR_BIT(PORTC, 4);
	SET_BIT(PORTC, 5);
	
	printInfo(song);
	wait_avr(song->currentDivision);
	CLR_BIT(PORTC, 5);
	SET_BIT(PORTC, 6);
	
	printInfo(song);
	wait_avr(song->currentDivision);
	CLR_BIT(PORTC, 6);
	SET_BIT(PORTC, 7);
	
	printInfo(song);
	wait_avr(song->currentDivision);
	CLR_BIT(PORTC, 7);
}




int main(void){
	ini_avr();
	ini_lcd();
    SET_BIT(DDRB, 3); // make B3 an output
	// set all a ports to output   
	SET_BIT(DDRC, 0);
	SET_BIT(DDRC, 1);
	SET_BIT(DDRC, 2);
	SET_BIT(DDRC, 3);
	SET_BIT(DDRC, 4);
	SET_BIT(DDRC, 5); 
	SET_BIT(DDRC, 6);
	SET_BIT(DDRC, 7);
	
	ADCSRA |= 1<<ADPS2;
	ADMUX |= (1<<REFS0) | (1<<REFS1);
	ADCSRA |= 1<<ADIE;
	ADCSRA |= 1<<ADEN;
	sei(); // enable interrupts
	
	ADCSRA |= 1<<ADSC;

	while(1){
		cycleThroughSteps(&song);  
	}
}

ISR(ADC_vect){
	uint8_t low = ADCL; 
	float result = ADCH<<8 | low; 
	
	switch(ADMUX){
		case 0xC0: // tempo updates
			song.tempo = result * 0.29296875;
			if(song.tempo <= 20){
				song.tempo = 20;
			}
			ADMUX = 0xC1; 
			break;
		case 0xC1: // division update
			
			if(result <170){
				song.currentDivision = (DEFAULT_LEN/song.tempo)*4; // whole note
				song.noteLength = whole; 
			}else if (result > 170 && result < 340){
				song.currentDivision = (DEFAULT_LEN/song.tempo)*2; // half note
				song.noteLength = half; 
			}else if(result >340 && result < 510){
				song.currentDivision =  (DEFAULT_LEN/song.tempo); // quarter note
				song.noteLength = quarter; 
			}else if(result >510 && result < 680){
				song.currentDivision = DEFAULT_LEN/(song.tempo*2); // eighth note
				song.noteLength = eighth;  
			}else if(result >680 && result < 850){
				song.currentDivision = DEFAULT_LEN/(song.tempo*4); // 16th note
				song.noteLength = sixteenth;
			}else if(result > 850){
				song.currentDivision = DEFAULT_LEN/(song.tempo*8); // 32nd note
				song.noteLength = thirtySecond;
			}
			
			ADMUX = 0xC0;
			break;
		default:
			break;
	}

	ADCSRA |= 1<<ADSC;
}