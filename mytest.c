#include "tinythreads.h"
#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/iom169p.h>
#define true	1
#define false	0
int pp;							// Global variable.
mutex mute = MUTEX_INIT;		// Initialize the mutex.

int ASCII_TABLE[10] = 
{
		0x1551,		// 0 = 1551, A = 0F51
		0x0110,		// 1
		0x1e11,		// 2
		0x1B11,		// 3
		0x0B50,		// 4
		0x1B41,		// 5
		0x1F41,		// 6
		0x0111,		// 7
		0x1F51,		// 8
		0x0B51		// 9
};
void writeChar(char ch, int pos){
	/* ch = character we want to write, pos = position on the LCD: 0->5 digits. */
	char mask;
	int karaktar = 0x0000;
	char nibbles = 0x00;
	int x;
	char lcddr = 0xec;

	int bokstav = atoi(&ch); // Converts ch, which the variable is at the form 'x' and not x, into an int!

	if(pos > 5){
		return;
	}
	if (pos & 0x01){
        mask = 0x0F;                // Position 1, 3, 5
	}		
    else{
        mask = 0xF0;                // Position 0, 2, 4 
	}
	if(bokstav <= 9 && bokstav >= 0){
	karaktar = ASCII_TABLE[(int)bokstav]; 
	}	
	else{
	karaktar = 0x0000;
	}	
	lcddr  += (pos >>1); // (0,0,1,1,2,2) (LCDDR0, LCDDR0) (LCDDR1, LCDDR1) (LCDDR2, LCDDR2)

	for(x = 0; x < 4; x++){
		nibbles = karaktar & 0x000f;
		karaktar = karaktar >> 4;
		if(pos & 0x01){					// True om 1,3,5
			nibbles = nibbles << 4;
		}

		_SFR_MEM8(lcddr) = ((_SFR_MEM8(lcddr) & mask) | nibbles);
		lcddr += 5;
	}
}
int is_prime(long i){
	/*	Checks if it's a prime number.
	*	Done by computing i % n, 
	*	(i.e., the remainder from division i/n) for all 2 <= n < i, 
	*	and returning false (0) if any such expression is 0, 
	*	true (1) otherwise.
	*/
	int start;
	for(start = 2; start < i; start++){
		if ((i % start) == 0) {
			return false;
		}		
	}
	return true;
}
void printAt(long num, int pos){
	lock(&mute);
    pp = pos;
    writeChar( (num % 100) / 10 + '0', pp);
	int loop_var;
	for(int loop; loop < 1000; loop++){
		loop_var++;
	}
    pp++;
    writeChar( num % 10 + '0', pp);
	unlock(&mute);
}
void computePrimes(int pos){
    long n;
    for(n = 1; ; n++) {
        if (is_prime(n)) {
            printAt(n, pos);
        }
    }
}

//SIG_PIN_CHANGE1
ISR(PCINT1_vect){
	if((PINB >> 7) == 0){ 
		yield();
	}
}
//SIG_OUTPUT_COMPARE1A
ISR(TIMER1_COMPA_vect) {
	yield();
}

int main(){
	// Clock prescaler 
    CLKPR = 0x80;	//(1<<CLKPCE);
	CLKPR =	0x00;	//(0<<CLKPS3);
	
	// Device Initialization values:
	LCDCRA = 0xC0;	//(1<<LCDEN)  | (1<<LCDAB);						//0xC0;
	LCDCRB = 0xB7;	//(1<<LCDCS)  | (3<<LCDMUX0) | (7<<LCDPM0);		//0xB7;
	LCDFRR = 0x07;	//(0<<LCDPS0) | (7<<LCDCD0);					//0x07;
	LCDCCR = 0x0F;	//(15<<LCDCC0);									//0x0F;

    spawn(computePrimes, 0);
	computePrimes(3);
}