#if !defined(_LFSR_H_)
#define _LFSR_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

typedef struct lfsr
{
	uint16_t start_state;  /* Any nonzero start state will work. */
	uint16_t lfsr_value;
} lfsr_t;

void
lfsr_init_start_state(
	uint16_t seed,
	lfsr_t * instance
);

uint16_t
lfsr_get_next(
	lfsr_t * instance
);

void
lfsr_reset(
	lfsr_t * instance
);

#if defined(__cplusplus)
}
#endif

#endif


