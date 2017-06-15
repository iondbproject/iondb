/******************************************************************************/
/**
@file		behaviour_dictionnary.h
@author		Eric Huang
@brief		Entry point for dictionary level behaviour tests.
@details	Note that the tests assume a minimum capacity of 160 simultaneous
			records being present in the dictionary at any given time. A max
			capacity of 200 is suggested.
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

#if !defined(TEST_BEHAVIOUR_H)
#define TEST_BEHAVIOUR_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../planck-unit/src/planck_unit.h"
#include "../../../dictionary/dictionary_types.h"
#include "../../../dictionary/dictionary.h"
#include "../../../dictionary/ion_master_table.h"

#define ION_BHDCT_INT_INT					(1 << 0)
#define ION_BHDCT_STRING_INT				(1 << 1)
#define ION_BHDCT_DUPLICATES				(1 << 2)
#define ION_BHDCT_ALL_TESTS					(0xFFFFFFFF)

#define ION_BHDCT_STRING_KEY_BUFFER_SIZE	9
#define ION_BHDCT_STRING_KEY_PAYLOAD		"k%d"

typedef struct {
	ion_handler_initializer_t	init_fcn;		/**< A pointer to a handler initialization function. */

	ion_dictionary_size_t		dictionary_size;/**< Some configuration for the dictionary, so that we can change parameters
												   on a per-implementation basis. */
	uint32_t					test_classes;	/**< A bit mask that determines which sets of tests to run. */
} ion_bhdct_context_t;

/**
@brief		Executes the behaviour test suite, given the testing parameters.
@param		init_fcn
				A function pointer that designates the initializer for a specific dictionary implementation.
@param		dictionary_size
				The specified dictionary size to use for tests.
@param		test_classes
				A supplied bit mask used to determine which tests to run.
*/
void
bhdct_run_tests(
	ion_handler_initializer_t	init_fcn,
	ion_dictionary_size_t		dictionary_size,
	uint32_t					test_classes
);

#if defined(__cplusplus)
}
#endif

#endif
