#include <cassert>
#include <cstdio>
#include "DigisparkPWM/PwmSettings.h"

int main()
{
  unsigned checked = 0;
  // All input knob frequency values; all requested 0.1% duty steps.
  for (unsigned adc = 0; adc <= 1023; ++adc) {
    uint32_t frequency = adc < 341 ? 100UL + 900UL * adc / 341UL :
        adc < 682 ? 1000UL + 9000UL * (adc - 341) / 341UL :
        10000UL + 490000UL * (adc - 682) / 341UL;
    uint8_t previous = 0;
    for (unsigned duty = 50; duty <= 950; ++duty) {
      PwmSettings p = calculatePWM(frequency, duty);
      uint32_t n = 1UL << (p.clockSelect - 1);
      uint32_t period = (uint32_t(p.top) + 1) * n;
      uint32_t high = (uint32_t(p.compare) + 1) * n - 1;
      assert(p.clockSelect >= 1 && p.clockSelect <= 15);
      assert(p.compare > 0 && p.compare < p.top);
      assert(PWM_CLOCK_HZ <= frequency * period);
      assert(high * 1000UL >= period * 50UL);
      assert(high * 1000UL <= period * 950UL);
      assert(p.compare >= previous);
      previous = p.compare;
      ++checked;
    }
  }
  const PwmSettings lo = calculatePWM(500000, 50);
  const PwmSettings hi = calculatePWM(500000, 950);
  assert(lo.clockSelect == 1 && lo.top == 127 && lo.compare == 7);
  assert(hi.clockSelect == 1 && hi.top == 127 && hi.compare == 121);
  std::printf("PASS: %u settings; 500kHz TOP=127, OCR1A=7..121\n", checked);
}
