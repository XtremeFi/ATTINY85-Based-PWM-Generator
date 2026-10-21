/*
 * Digispark / ATtiny85: 100Hz..500kHz, requested duty 5%..95%.
 * Board: Digispark 16MHz - No USB, 5V supply, original PLL CPU fuse.
 * P1/PB1: OC1A -> manual RUN switch -> 1k -> IRS20752 pin 3 IN.
 * P2/PB2/ADC1: 10k frequency potentiometer wiper.
 * P3/PB3/ADC3: duty pot through S2; isolate ALL onboard USB branches.
 * S2 UPLOAD: PB3 to original USB branch. S2 RUN: PB3 to duty pot.
 * P5 unused; no fuse changes. Disconnect USB before selecting RUN.
 * Keep RUN open during boot/upload. No DigiUSB, millis(), delay(), Servo.
 * 500kHz refers to IN, not guaranteed MOSFET gate fidelity at short pulses.
 */
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/wdt.h>
#include <util/delay.h>
#include "PwmSettings.h"

#if !defined(__AVR_ATtiny85__)
#error "Select an ATtiny85 Digispark board."
#endif
#if F_CPU != 16000000UL
#error "Select Digispark 16MHz - No USB (not default 16.5MHz)."
#endif

static uint16_t adcRead(uint8_t channel)
{
  ADMUX = channel;  // VCC reference; ADC1 or ADC3
  ADCSRA |= _BV(ADSC);
  while (ADCSRA & _BV(ADSC)) {}
  (void)ADC;       // discard after switching multiplexer
  ADCSRA |= _BV(ADSC);
  while (ADCSRA & _BV(ADSC)) {}
  return ADC;
}

static uint16_t difference(uint16_t a, uint16_t b)
{
  return a > b ? a - b : b - a;
}

static uint16_t endpointMap(uint16_t value)
{
  if (value <= 8) return 0;
  if (value >= 1015) return 1023;
  return (uint32_t)(value - 8) * 1023UL / 1007UL;
}

// Three approximately logarithmic decades, implemented without floating point.
// 0..340 -> 100..1000Hz, 341..681 -> 1..10kHz,
// 682..1023 -> 10..500kHz. Piecewise linear for simple knob operation.
static uint32_t frequencyFromADC(uint16_t value)
{
  if (value < 341) return 100UL + 900UL * value / 341UL;
  if (value < 682) return 1000UL + 9000UL * (value - 341) / 341UL;
  return 10000UL + 490000UL * (value - 682) / 341UL;
}

static void applyPWM(PwmSettings next)
{
  static bool running = false;
  static PwmSettings previous = {0, 0, 0};
  if (running && next.clockSelect == previous.clockSelect &&
      next.top == previous.top) {
    // Hardware buffers duty updates until the period boundary.
    if (next.compare != previous.compare) OCR1A = next.compare;
  } else {
    // Frequency updates deliberately restart the timer; transitional cycles
    // may be shortened. Steady-state PWM is generated entirely in hardware.
    PORTB &= ~_BV(PB1);
    TCCR1 = 0;
    TCNT1 = 0;
    GTCCR = _BV(PSR1);
    OCR1C = next.top;
    OCR1A = next.compare;
    TIFR = _BV(TOV1) | _BV(OCF1A) | _BV(OCF1B);
    TCCR1 = _BV(PWM1A) | _BV(COM1A1) | next.clockSelect;
    running = true;
  }
  previous = next;
}

void setup()
{
  cli();
  MCUSR &= ~_BV(WDRF);
  wdt_disable();
  TIMSK = 0;
  GIMSK = 0;
  TCCR0A = 0;
  TCCR0B = 0;
  TCCR1 = 0;
  GTCCR = 0;
  PORTB &= ~(_BV(PB1) | _BV(PB2) | _BV(PB3));
  DDRB |= _BV(PB1);
  DDRB &= ~(_BV(PB2) | _BV(PB3));

  ADMUX = 0;
  ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);
  DIDR0 |= _BV(ADC3D) | _BV(ADC1D);

  // Do not turn off PLL: it also clocks the CPU on a stock Digispark.
  PLLCSR |= _BV(PLLE);
  _delay_us(150);
  while (!(PLLCSR & _BV(PLOCK))) {}
  PLLCSR &= ~_BV(LSM);
  PLLCSR |= _BV(PCKE);
  _delay_ms(20);
}

void loop()
{
  static bool initialized = false;
  static uint16_t frequencyFilter, dutyFilter;
  static uint16_t acceptedFrequency, acceptedDuty;

  uint16_t frequency = adcRead(1);
  uint16_t duty = adcRead(3);
  if (!initialized) {
    frequencyFilter = frequency << 4;
    dutyFilter = duty << 4;
  }
  frequencyFilter = frequencyFilter - (frequencyFilter >> 4) + frequency;
  dutyFilter = dutyFilter - (dutyFilter >> 4) + duty;
  frequency = endpointMap(frequencyFilter >> 4);
  duty = endpointMap(dutyFilter >> 4);

  if (!initialized || difference(frequency, acceptedFrequency) >= 3 ||
      ((frequency == 0 || frequency == 1023) && frequency != acceptedFrequency))
    acceptedFrequency = frequency;
  if (!initialized || difference(duty, acceptedDuty) >= 3 ||
      ((duty == 0 || duty == 1023) && duty != acceptedDuty))
    acceptedDuty = duty;

  applyPWM(calculatePWM(frequencyFromADC(acceptedFrequency),
                       50 + (uint32_t)acceptedDuty * 900UL / 1023UL));
  initialized = true;
  _delay_ms(5);
}
