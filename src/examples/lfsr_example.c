#include "../util/lfsr/lfsr.h"
#include <stdint.h>
#include <stdio.h>

int
main(
	void
) {
	lfsr_t num_gen;

	lfsr_init_start_state(0xACE1u, &num_gen);	/* seed can be any non-zero value */

	uint16_t period = 0;

	do {
		++period;
		printf("%d \n", lfsr_get_next(&num_gen));
	} while (period != 0);

	return 0;
}
