/******************************************************************************/
/**
@file		iinq_user.h
@author		Dana Klamut
@brief		This code contains definitions for iinq user functions
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

#if !defined(IINQ_USER_H_)
#define IINQ_USER_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "../../dictionary/dictionary_types.h"

/**
@brief		A supertype for query iterator objects.
@details	This is a super type. The state information
			must be stored within a subtype that makes
			sense to the particular dictionary implementation.

			There are different types of cursors for different types of
			dictionary operations.
*/
struct query_iterator {
	ion_cursor_status_t status;	/**< Status of last cursor call. */
	ion_record_t		record;		/**< The current record returned by the
												iterator. */

	ion_record_t		(*next)(
	);
	/**< A pointer to the next function,
		 which returns the next record). */
	void (*init)(
	);
	/**< A pointer to the next function,
		 which returns the next record). */
	void (*destroy)(
	);
	/**< A pointer to the function used
		 to destroy the cursor (frees
		 internal memory). */
};

/**
@brief		The iterator for a query.
@see		query_iterator
*/
typedef struct query_iterator ion_query_iterator_t;

ion_query_iterator_t
SQL_query(
	char *sql_string
);

#if defined(__cplusplus)
}
#endif

#endif
