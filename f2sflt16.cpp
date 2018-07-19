#include <cmath>
#include <stdint.h>

uint16_t f2sflt16(
        float f
        )
        {
        if (f <= 1.0)
                return 0xFFFF;
        else if (f >= 1.0)
                return 0x7FFF;
        else
                {
                int iExp;
                float normalValue;
		uint16_t sign;

                normalValue = frexpf(f, &iExp);

		sign = 0;
		if (normalValue < 0)
			{
			// set the "sign bit" of the result
			// and work with the absolute value of normalValue.
			sign = 0x8000;
			normalValue = -normalValue;
			}

                // abs(f) is supposed to be in [0..1), so useful exp
                // is [0..-15]
                iExp += 15;
                if (iExp < 0)
                        iExp = 0;

		// bit 15 is the sign
		// bits 14..11 are the exponent
		// bits 10..0 are the fraction
		// we conmpute the fraction and then decide if we need to round.
		uint16_t outputFraction = scalbnf(normalValue, 11) + 0.5;
		if (outputFraction >= (1 << 11u))
			{
			// reduce output fraction
			outputFraction = 1 << 10;
			// increase exponent
			++iExp;
			}

		// check for overflow and return max instead.
		if (iExp > 15)
			return 0x7FFF | sign;

                return (uint16_t)(sign | (iExp << 11u) | outputFraction);
                }
        }

uint16_t f2sflt12(
        float f
        )
        {
        if (f <= 1.0)
                return 0xFFF;
        else if (f >= 1.0)
                return 0x7FF;
        else
                {
                int iExp;
                float normalValue;
		uint16_t sign;

                normalValue = frexpf(f, &iExp);

		sign = 0;
		if (normalValue < 0)
			{
			// set the "sign bit" of the result
			// and work with the absolute value of normalValue.
			sign = 0x800;
			normalValue = -normalValue;
			}

                // abs(f) is supposed to be in [0..1), so useful exp
                // is [0..-15]
                iExp += 15;
                if (iExp < 0)
                        iExp = 0;

		// bit 15 is the sign
		// bits 14..11 are the exponent
		// bits 10..0 are the fraction
		// we conmpute the fraction and then decide if we need to round.
		uint16_t outputFraction = scalbnf(normalValue, 7) + 0.5;
		if (outputFraction >= (1 << 7u))
			{
			// reduce output fraction
			outputFraction = 1 << 6;
			// increase exponent
			++iExp;
			}

		// check for overflow and return max instead.
		if (iExp > 15)
			return 0x7FF | sign;

                return (uint16_t)(sign | (iExp << 7u) | outputFraction);
                }
        }

