#pragma once
#include <stdint.h>

// Nominal PLL peripheral clock. Calibrate this value against a scope if needed.
#ifndef PWM_CLOCK_HZ
#define PWM_CLOCK_HZ 64000000UL
#endif

struct PwmSettings {
  uint8_t clockSelect;
  uint8_t top;
  uint8_t compare;
};

static PwmSettings calculatePWM(uint32_t frequency, uint16_t dutyPermille)
{
  if (frequency < 100) frequency = 100;
  if (frequency > 500000) frequency = 500000;
  if (dutyPermille < 50) dutyPermille = 50;
  if (dutyPermille > 950) dutyPermille = 950;

  uint8_t select = 1;
  uint16_t divider = 1;
  // Round period upwards: nominal frequency never exceeds the request.
  uint32_t counts;
  for (;;) {
    const uint32_t denominator = frequency * divider;
    counts = (PWM_CLOCK_HZ + denominator - 1) / denominator;
    if (counts <= 256 || select == 15) break;
    divider <<= 1;
    ++select;
  }
  if (counts < 2) counts = 2;
  if (counts > 256) counts = 256;

  // Timer1 edge timing: high PLL cycles = (OCR1A + 1) * divider - 1.
  // Round inward at the endpoints to keep the nominal IN duty in 5..95%.
  // Actual HO and MOSFET conduction duty must be measured separately.
  const uint32_t period = counts * divider;
  const uint32_t scale = 1000UL * divider;
  uint16_t compare =
      (period * dutyPermille + 1000UL + scale / 2) / scale - 1;
  const uint16_t lower =
      (period * 50UL + 1000UL + scale - 1) / scale - 1;
  const uint16_t upper =
      (period * 950UL + 1000UL) / scale - 1;
  if (compare < lower) compare = lower;
  if (compare > upper) compare = upper;

  PwmSettings result = {select, (uint8_t)(counts - 1), (uint8_t)compare};
  return result;
}
