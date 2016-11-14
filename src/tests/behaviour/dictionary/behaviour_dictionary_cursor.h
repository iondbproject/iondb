#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <alloca.h>
#include "../../planckunit/src/planck_unit.h"
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
@brief		Executes the behaviour test suite for cursor behaviour, given the testing parameters.
@param		init_fcn
				A function pointer that designates the initializer for a specific dictionary implementation.
@param		dictionary_size
				The specified dictionary size to use for tests.
@param		test_classes
				A supplied bit mask used to determine which tests to run.
*/
void
bhdct_run_cursor_tests(
	ion_handler_initializer_t	init_fcn,
	ion_dictionary_size_t		dictionary_size,
	uint32_t					test_classes
);

#if defined(__cplusplus)
}
#endif
