/******************************************************************************/
/**
@file		test_cpp_wrapper2.h
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

#include "../test1/test_cpp_wrapper1.h"

#ifndef TEST_CPP_WRAPPER2_H_
#define TEST_CPP_WRAPPER2_H_

void
runalltests_cpp_wrapper2(
);

void
test_cpp_wrapper_equality_duplicates(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int key
);

void
test_cpp_wrapper_equality_duplicates(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int key
);

void
test_cpp_wrapper_range(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int min_key,
	int max_key
);

void
test_cpp_wrapper_equality_no_duplicates(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int key
);

void
test_cpp_wrapper_all_records(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int random_positive_num
);

void
cpp_wrapper_range(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int min_key,
	int max_key,
	int expected_records[],
	int expected_num_records,
	ion_boolean_t records_exist
);

void
cpp_wrapper_equality(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int key,
	int expected_values[],
	int expected_num_records,
	ion_boolean_t key_exists
);

void
test_cpp_wrapper_equality(
	planck_unit_test_t	*tc,
	int					key
);

void
cpp_wrapper_all_records(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict,
	int expected_records[],
	int expected_num_records,
	ion_boolean_t records_exist
);

void
cpp_wrapper_update(
	planck_unit_test_t *tc,
	Dictionary<int, int>	*dict,
	int key,
	int value,
	ion_err_t expected_status,
	ion_result_count_t expected_count,
	ion_boolean_t check_result
);

void
test_cpp_wrapper_get_nonexist_many(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
);

void
test_cpp_wrapper_update_exist_in_many(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
);

void
test_cpp_wrapper_get_lots(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
);

void
test_cpp_wrapper_setup(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
);

void
test_cpp_wrapper_insert_multiple(
	planck_unit_test_t *tc,
	Dictionary<int, int> *dict
);

#endif
