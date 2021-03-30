/*
   AkwardClock.c

   Author  : Benjamin Sølberg
   Email   : benjamin.soelberg@gmail.com
   Github  : https://github.com/benjaminsoelberg/AkwardClock
   Version : 1.01

   Notes   :
        #1 : Compile with -O2 to optimize for speed, not size to save battery
        #2 : P1.4 & P1.6 must be connected to the watch coil
        #3 : If coil uses more current than the pins can supply then bridge P1.4 with P1.5 and P1.6 with P1.7
        #4 : Define NORMAL_CLOCK to disable randomization
        #5 : Change random_iv to any value between 1 and 0xFFFFFFFF to set prng starting point
*/ 

#include<msp430.h>

#define F_CPU 1200000L

//#define NORMAL_CLOCK

#define SETBIT(A,k)     ( A[(k / 8)] |=  (1 << (k % 8)) )
#define XORBIT(A,k)     ( A[(k / 8)] ^=  (1 << (k % 8)) )
#define CLEARBIT(A,k)   ( A[(k / 8)] &= ~(1 << (k % 8)) )
#define TESTBIT(A,k)    ((A[(k / 8)] &   (1 << (k % 8))) > 0)

// Not really changeable without changing the code too
#define tick_per_sec 4
#define ticks_pr_round 64
#define num_of_tick_bits (ticks_pr_round * tick_per_sec)

#define random_iv 0x1234

uint8_t tick_bits[num_of_tick_bits / 8];

uint32_t xorshift32() {
    /* The state must be initialized to non-zero */
    static uint32_t state = random_iv;
    
    /* Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs" */
    uint32_t x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;

    return state = x;
}

inline void bitswap(uint8_t x, uint8_t y) {
    if (TESTBIT(tick_bits, x) ^ TESTBIT(tick_bits, y)) {
        XORBIT(tick_bits, x);
        XORBIT(tick_bits, y);
    }
}

void permutate() {
    #ifdef NORMAL_CLOCK
    return;
    #endif
    uint8_t i = 0;
    uint32_t rnd = 0;
    
    do {
        if ((i % 4) == 0) {
            rnd = xorshift32();
        } else {
            rnd >>= 8;
        }
        bitswap(i++, rnd % num_of_tick_bits);
    } while (i != 0);
}

static volatile bool flipflop = false;
void generate_puls() {
    if (flipflop) {
        P1OUT |= BIT6 | BIT7; // P1.6-7 high
    } else {
        P1OUT |= BIT4 | BIT5; // P1.4-5 high
    }
    __delay_cycles(25 * (F_CPU/1000L)); // Around 30 ms (clock specific)
    P1OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7); // P1.4-7 low
 
    flipflop = !flipflop;
}

// Timer A0 interrupt service routine
#pragma vector = TIMER0_A0_VECTOR
__interrupt void Timer_A(void) {
    static volatile uint8_t tick = 0;

    if (TESTBIT(tick_bits, tick)) {
        generate_puls();
    }

    if (tick == num_of_tick_bits - 1) {
        tick = 0;
        permutate();
    } else {
        tick++;
    }
}

int main(void) {
    // Stop WDT
    WDTCTL = WDTPW + WDTHOLD;

    // NMI on OSC fault
    //IE1 |= OFIE;

    // Setup xtal caps and clock routing
    BCSCTL1 |= DIVA_3; // ACLK/8
    BCSCTL3 |= XCAP_3; //12.5pF cap- setting for 32.768 kHz crystal

    // Set P1.4-7 as input and low
    P1OUT &= ~(BIT4 | BIT5 | BIT6 | BIT7); // P1.4-7 low
    P1DIR |= BIT4 | BIT5 | BIT6 | BIT7; // set P1.4-7 as output

    // Set 64 of the tick bits to 1
    memset(tick_bits, 0x11, sizeof(tick_bits));
    
    // Generate initial permutation of tick bits
    permutate();

    // Enable 32.768 kHz timer interrupt clock to once every 250 ms
    CCTL0 = CCIE;                   // CCR0 interrupt enabled
    CCR0 = 127;                     // 250 ms
    TACTL = TASSEL_1 + ID_3 + MC_1; // ACLK, /8, upmode

    // Go low power and wait for timer interrupt
    _BIS_SR(LPM3_bits + GIE); // Enter LPM3 w/ interrupt
}
