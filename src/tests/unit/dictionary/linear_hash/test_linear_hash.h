/******************************************************************************/
/**
@file
@author		Spencer MacBeth
@brief		Header declarations for the linear hash unit tests.
@copyright	Copyright 2016
				The University of British Columbia,
				IonDB Project Contributors (see AUTHORS.md)
@par
			Licensed under the Apache License, Version 2.0 (the "License");
			you may not use this file except in compliance with the License.
			You may obtain a copy of the License at
					http://www.apache.org/licenses/LICENSE-2.0
@par
			Unless required by applicable law or agreed to in writing,
			software distributed under the License is distributed on an
			"AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
			either express or implied. See the License for the specific
			language governing permissions and limitations under the
			License.
*/
/******************************************************************************/

#if !defined(TEST_LINEAR_HASH_H)
#define TEST_LINEAR_HASH_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "../../../planckunit/src/planck_unit.h"
#include "../../../../dictionary/linear_hash/linear_hash_p.h"

void
runalltests_linear_hash(
);

#if defined(__cplusplus)
}
#endif

#endif
