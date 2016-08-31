/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Header declarations for the flat file store unit tests.
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

#if !defined(TEST_FLAT_FILE_H)
#define TEST_FLAT_FILE_H

#if defined(__cplusplus)
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "../../../planckunit/src/planck_unit.h"
#include "../../../../dictionary/flat_file/flat_file.h"

void
runalltests_flat_file(
);

#if defined(__cplusplus)
}
#endif

#endif
