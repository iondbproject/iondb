#include "lfsr.h"

void
lfsr_init_start_state(
	uint16_t seed, 
	lfsr_t * instance
)
{
	instance->start_state = seed;
	instance->lfsr_value = seed;
}

uint16_t
lfsr_get_next(
	lfsr_t * instance
)
{
	unsigned lsb = instance->lfsr_value & 1;  /* Get LSB (i.e., the output bit). */
        instance->lfsr_value >>= 1;               /* Shift register */
        if (lsb == 1)             /* Only apply toggle mask if output bit is 1 */
            instance->lfsr_value ^= 0xB400u;      /* Apply toggle mask, value has 1 at bits corresponding
                                   * to taps, 0 elsewhere. */
	return instance->lfsr_value;
}

void
lfsr_reset(
	lfsr_t * instance
)
{
	instance->lfsr_value = instance->start_state;
}
