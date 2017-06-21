/******************************************************************************/
/**
@file	   lfsr.h
@author		Scott Fazackerley
@brief		A linear-feedback shift register pseudo-random number generator.
@details	This code implements a simple random number generator that
			generates a subset of possible integral values (16 bit unsigned)
			such that there are no collisions within the cycle generated.
@copyright	Copyright 2017
			The University of British Columbia,
			IonDB Project Contributors (see AUTHORS.md)
@par Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are met:

@par 1.Redistributions of source code must retain the above copyright notice,
	this list of conditions and the following disclaimer.

@par 2.Redistributions in binary form must reproduce the above copyright notice,
	this list of conditions and the following disclaimer in the documentation
	and/or other materials provided with the distribution.

@par 3.Neither the name of the copyright holder nor the names of its contributors
	may be used to endorse or promote products derived from this software without
	specific prior written permission.

@par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
	AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
	IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
	ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
	LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
	CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
	SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
	INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
	CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
	ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
/******************************************************************************/

#if !defined(_LFSR_H_)
#define _LFSR_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdint.h>

/**
@brief		An instance type for the random number generator.
*/
typedef struct lfsr {
	uint16_t	start_state;/**< Any nonzero start state for the generator. */
	uint16_t	lfsr_value;	/**< The current state of the generator. */
} lfsr_t;

/**
@brief		Initialize the seed for the random number generator.
@param		seed
				The initial start state represented as an integer
				for the random number generator.
@param		instance
				A pointer to the random number generator object to be
				initialized. This is caller allocated.
*/
void
lfsr_init_start_state(
	uint16_t	seed,
	lfsr_t		*instance
);

/**
@brief		Generate the next random number.
@param		instance
				A pointer to the random number generator object that we
				wish to use to generate the next random item.
@returns	The next random integer from the random number generator.
*/
uint16_t
lfsr_get_next(
	lfsr_t *instance
);

/**
@brief		Reset the random number generator to it's initial state.
@param		instance
				A pointer to the random number generator object
				to reset.
*/
void
lfsr_reset(
	lfsr_t *instance
);

#if defined(__cplusplus)
}
#endif

#endif
