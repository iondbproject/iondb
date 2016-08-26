/******************************************************************************/
/**
@file
@author		Wade Penson
@brief		Implementation of the external merge sort algorithm.
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

#include "external_merge_sort.h"

ion_err_t
ion_external_merge_sort_init(
	ion_external_sort_t			*es,
	ion_external_sort_cursor_t	*cursor
) {
	return err_ok;
}

ion_err_t
ion_external_merge_sort_next(
	ion_external_sort_cursor_t *cursor,
	void *output_value
) {
	return err_ok;
}
