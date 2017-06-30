/******************************************************************************/
/**
@file
@author		Eric Huang, Wade Penson
@brief		Resources used by the file and in-memory sorting implementations.
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

#if !defined(ION_SORT_H_)
#define ION_SORT_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "../../key_value/kv_system.h"

typedef void *ion_sort_comparator_context_t;

typedef ion_comparison_e (*ion_sort_comparator_t)(
	ion_sort_comparator_context_t	context,
	void							*value_a,
	void							*value_b
);

#if defined(__cplusplus)
}
#endif

#endif /* ION_SORT_H_ */
