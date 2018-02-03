/******************************************************************************/
/**
@file		test_cpp_wrapper1.h
@author		Eric Huang, Dana Klamut
@brief		Unit test for the C++ Wrapper
@todo		These tests need to be fully fleshed out.
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

#include "../../../planck-unit/src/planck_unit.h"
#include "../../../../cpp_wrapper/Dictionary.h"
#include "../../../../cpp_wrapper/MasterTable.h"

#ifndef TEST_CPP_WRAPPER_H_
#define TEST_CPP_WRAPPER_H_

/* This is used to define how complicated to pre-fill a dictionary for testing. */
typedef enum ION_BEHAVIOUR_FILL_LEVEL {
	ion_fill_none, ion_fill_low, ion_fill_medium, ion_fill_high, ion_fill_edge_cases
} ion_behaviour_fill_level_e;

#define ION_FILL_LOW_LOOP(var) \
	for (var = 0; var<10;var++)
#define ION_FILL_MEDIUM_LOOP(var) \
	for (var = 50; var<100;var += 2)
#define ION_FILL_HIGH_LOOP(var) \
	for (var = 500; var<1000;var += 5)
#define ION_FILL_EDGE_LOOP(var) \
	for (var = -100; var<-50;var += 2)

void
runalltests_cpp_wrapper1(
);

void
runalltests_cpp_wrapper2(
);

void
master_table_delete_from_master_table(
	planck_unit_test_t	*tc,
	MasterTable			*master_table,
	ion_dictionary_id_t id
);

void
cpp_wrapper_get(
	planck_unit_test_t *tc,
	Dictionary<int, int>	*dict,
	int key,
	int expected_value,
	ion_err_t expected_status,
	ion_result_count_t expected_count
);

void
master_table_init(
	planck_unit_test_t	*tc,
	MasterTable			*master_table
);

void
master_table_setup(
	planck_unit_test_t	*tc,
	MasterTable			*master_table
);

void
master_table_close(
	planck_unit_test_t	*tc,
	MasterTable			*master_table
);

void
master_table_lookup_dictionary(
	planck_unit_test_t				*tc,
	MasterTable						*master_table,
	ion_dictionary_id_t				id,
	ion_key_type_t					key_type,
	ion_key_size_t					key_size,
	ion_value_size_t				value_size,
	ion_dictionary_size_t			dictionary_size,
	ion_dictionary_type_t			dictionary_type,
	ion_dictionary_config_info_t	*config,
	ion_boolean_t					expect_found
);

void
master_table_delete_dictionary(
	planck_unit_test_t *tc,
	MasterTable *master_table,
	Dictionary<int, int>	*dictionary
);

void
master_table_create_dictionary(
	planck_unit_test_t *tc,
	MasterTable *master_table,
	Dictionary<int, int>	*dictionary,
	ion_dictionary_id_t id,
	ion_key_type_t key_type,
	ion_key_size_t key_size,
	ion_value_size_t value_size,
	ion_dictionary_size_t dictionary_size,
	ion_dictionary_type_t dictionary_type
);

void
master_table_dictionary_add(
	planck_unit_test_t *tc,
	MasterTable *master_table,
	Dictionary<int, int>	*dictionary,
	ion_key_type_t key_type,
	ion_key_size_t key_size,
	ion_value_size_t value_size,
	ion_dictionary_size_t dictionary_size,
	ion_dictionary_type_t dictionary_type
);

void
cpp_wrapper_insert(
	planck_unit_test_t *tc,
	Dictionary<int, int>	*dict,
	int key,
	int value,
	ion_boolean_t check_result
);

void
cpp_wrapper_setup(
	planck_unit_test_t *tc,
	Dictionary<int, int>			*dict,
	ion_behaviour_fill_level_e fill_level
);

#endif
