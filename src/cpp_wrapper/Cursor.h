/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		A simple implementation of an iterator for the C++ Wrapper interface
			of IonDB.
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

#if !defined(CURSOR_H)
#define CURSOR_H

template<typename K, typename V>
class Cursor {
public:
Cursor(
	ion_dictionary_t	*dictionary,
	ion_predicate_t		*predicate
) {
	dictionary_find(dictionary, predicate, &cursor);
	record.key		= malloc(dictionary->instance->record.key_size);
	record.value	= malloc(dictionary->instance->record.value_size);
}

~Cursor(
) {
	cursor->destroy(&cursor);
	free(record.key);
	free(record.value);
}

bool
hasNext(
) {
	return cursor->status == cs_cursor_initialized || cursor->status == cs_cursor_active;
}

bool
next(
) {
	ion_cursor_status_t status = cursor->next(cursor, &record);

	return status == cs_cursor_initialized || status == cs_cursor_active;
}

K
getKey(
) {
	return *((K *) record.key);
}

V
getValue(
) {
	return *((V *) record.value);
}

private:

ion_dictionary_t	*dictionary;
ion_dict_cursor_t	*cursor;
ion_record_t	record;
};

#endif
