/******************************************************************************/
/**
@file
@author		Graeme Douglas
@brief		A simple unit testing framework that emits JSON
		as well as human readable results, optionally.
@details	PlanckUnit is a simple, extensible, self-contained
		unit testing library for embedded devices that also
		works on the standard systems. It prints all output
		from tests immediately. By default PlanckUnit supports
		result output in JSON and human readable forms.
@copyright	Copyright 2015 Graeme Douglas
@license	Licensed under the Apache License, Version 2.0 (the "License");
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

#ifndef PLANCKUNIT_H
#define PLANCKUNIT_H

#ifdef  __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* If this is any sort of workstation system, don't include Arduino junk. */
//#if !(defined (__unix__) || (defined (__APPLE__) && defined (__MACH__)) || defined(WIN32) || defined(_WIN32) || defined(__WIN32) || defined(__CYGWIN))
/* If we are compiling for the arduino, include the serial interface. */
#ifdef ARDUINO
#include "serial_c_iface.h"
#endif

/**
@brief		NULL pointer constant, if not already defined.
*/
#ifndef NULL
#define NULL			((void *)0)
#endif

#define PLANCK_UNIT_MODE_JSON	0
#define PLANCK_UNIT_MODE_HUMAN	1

/**
@brief		An assertion result, either a success or a failure.
@details	See @ref PLANCK_UNIT_FAILURE, @ref PLANCK_UNIT_SUCCESS.
*/
typedef char			planck_unit_result_t;

/**
@brief		PlanckUnit failure result.
*/
#define PLANCK_UNIT_FAILURE	0

/**
@brief		PlanckUnit success result.
*/
#define PLANCK_UNIT_SUCCESS	1

/**
@brief		A single test case or unit test.
@details	This tracks the state of the execution of a single
		test function.
		
		See also @ref struct planck_unit_test.
*/
typedef struct planck_unit_test		planck_unit_test_t;

/**
@brief		A collection of unit tests / test cases.
@details	This tracks the state of the execution of many test functions.
		
		See also @ref struct planck_unit_suite.
*/
typedef struct planck_unit_suite	planck_unit_suite_t;

/**
@brief		A printing functions used to format the output
		of of a test suite.
*/
typedef struct planck_unit_print_functions
{
	/**> Print the result of a single test execution. This method
	     has access to all relevant information for a test suite. */
	void (*print_result)(planck_unit_test_t *);
	
	/**> If the output format requires some setup pre-execution
	     (such as the case for JSON), then this function can accomplish
	     this. Whatever is printed here is output before any tests are
	     executed. */
	void (*print_preamble)(void);
	
	/**> Following all tests have been executed, this function
	     can optionally print any final information. A summary
	     of the test results, for instance, can be printed,
	     as all information from the entire suite's execution is
	     available. */
	void (*print_postamble)(planck_unit_suite_t *);
} planck_unit_print_funcs_t;

/**
@brief		Print the result of a test as a JSON object.
@param		state
			A pointer to the test structure describing the result
			of the test's execution.
*/
void
planck_unit_print_result_json(
	planck_unit_test_t		*state
);

/**
@brief		Print the preamble for JSON document before suite
		execution.
@details	This just prints the start of the suite result JSON
		object.
*/
void
planck_unit_print_preamble_json(
	void
);

/**
@brief		Print any concluding information after the suite
		has been executed.
@param		suite
			A pointer to the suite that has just been executed.
*/
void
planck_unit_print_postamble_json(
	planck_unit_suite_t	*suite
);

/**
@brief		Print the result of a test's execution in human-readable
		format.
@param		state
			A pointer to the test structure describing the result
			of the test's execution.
*/
void
planck_unit_print_result_human(
	planck_unit_test_t		*state
);

/**
@brief		Literally print no preamble.
*/
void
planck_unit_print_preamble_none(
	void
);

/**
@brief		Print a human-readable summary of the result of a tests
		execution.
@param		suite
			A pointer to the suite that has just been executed.
*/
void
planck_unit_print_postamble_summary(
	planck_unit_suite_t	*suite
);

/**
@brief		JSON printing functions, for easy reference.
*/
extern planck_unit_print_funcs_t planck_unit_print_funcs_json;

/**
@brief		Human readable printing functions for easy reference.
*/
extern planck_unit_print_funcs_t planck_unit_print_funcs_human;

/**
@brief		A test suite for execution.
*/
struct planck_unit_suite
{
	/**> The print functions used to format the reuslt
	     of the suite's execution. */
	planck_unit_print_funcs_t	print_functions;
	/**> The total number of tests attempted. This number
	     includes both those passed and those failed. */
	int				total_tests;
	/**> The total number of tests passed. */
	int				total_passed;
	/**> The first test to execute. */
	planck_unit_test_t		*head;
	/**> The last test to execute. Used to append new tests
	     to execute to the suite. */
	planck_unit_test_t		*tail;
};

/**
@brief		The function pointer type for test functions to execute.
@param		test
			The state describing the result of the execution
			of the function this pointer is associated with.
*/
typedef void	(*planck_unit_test_func_t)(planck_unit_test_t *test);

/**
@brief		The state for a single unit test function.
@details	This tracks the result of the execution of single unit test
		or test case function. This structure tracks information
		related to the first failure point of a test's execution,
		at which point the function will cease execution.
*/
struct planck_unit_test
{
	/**> The result of the test function's execution. */
	planck_unit_result_t		result;
	/**> The line in the file at which the function failed.
	     If the test passes all assertions, this will be set
	     to @c -1. */
	int				line;
	/**> The name of the file to which a failing test belongs.
	     Otherwise will be set to the empty string, @c "". */
	const char			*file;
	/**> If the test failed, this will be set to the name of the
	     test function which failed. Otherwise this will be set
	     to the empty string, @c "". */
	const char			*func_name;
	/**> If the test failed, a failure message describing
	     the assertion that failed will be contained here.
	     Otherwise, this will point to the empty string, "". */
	char				*message;
	/**> The test function pointer that is to be or that was
	     executed. */
	planck_unit_test_func_t		test_func;
	/**> The pointer to the next suite in the test to be executed. */
	planck_unit_test_t		*next;
	/**> The suite to which this test belongs. */
	planck_unit_suite_t		*suite;
	/**> Whether or not the message was allocated. If the message is
	     allocated, the message will be immediately freed once the
	     execution has completed. */
	char				allocated_message;
};

/*** Do not call these methods directly, but instead use public macros below. */
planck_unit_result_t
planck_unit_assert_true(
	planck_unit_test_t		*state,
	int				condition,
	int				line,
	const char			*file,
	const char			*func,
	char				*message
);

planck_unit_result_t
planck_unit_assert_int_are_equal(
	planck_unit_test_t		*state,
	int				expected,
	int				actual,
	int				line,
	const char			*file,
	const char			*func
);

planck_unit_result_t
planck_unit_assert_int_are_not_equal(
	planck_unit_test_t		*state,
	int				expected,
	int				actual,
	int				line,
	const char			*file,
	const char			*func
);

planck_unit_result_t
planck_unit_assert_str_are_equal(
	planck_unit_test_t		*state,
	char				*expected,
	char				*actual,
	int				line,
	const char			*file,
	const char			*func
);

planck_unit_result_t
planck_unit_assert_str_are_not_equal(
	planck_unit_test_t		*state,
	char				*expected,
	char				*actual,
	int				line,
	const char			*file,
	const char			*func
);
/******************************************************************************/

/**
@brief		Initialize a suite to a pre-executed state.
@details	This method does not perform any allocations, only sets
		up the suite for execution. A default setting of JSON
		print functions is used by default when
		@ref planck_unit_new_suite is called, so calling
		this method is unncessary except to change the output
		method.
@param		suite
			A pointer to the suite to setup.
@param		print_funcs
			The collection of print functions to use
			to output the results of this suite.
*/
void
planck_unit_init_suite(
	planck_unit_suite_t		*suite,
	planck_unit_print_funcs_t	print_funcs
);

/**
@brief		Create a new test suite.
@details	This will create a new suite on the heap.
		To ensure no memory is leaked, make sure to call
		@ref planck_unit_destroy_suite once all testing has completed.
@returns	A pointer to the new suite that was allocated. If the
		the allocation failed, it will returns @c NULL.
*/
planck_unit_suite_t *
planck_unit_new_suite(
	void
);

/**
@brief		Add a test case / unit test to a test suite.
@details	This will allocate a test structure to track the execution
		of @p test_func. To avoid memory leakage, make sure
		@ref planck_unit_destroy_suite is called.
		Tests are executed in the same order they are added.
		Additionally, this method will setup the test structure
		and prepare it for execution.
@param		suite
			A pointer to the suite to add the test to.
@param		test_func
			A pointer to the function to execute as the test
			(the function containing the code and assertions
			forming the test).
*/
void
planck_unit_add_to_suite(
	planck_unit_suite_t		*suite,
	planck_unit_test_func_t		test_func
);

/**
@brief		Execute the test suite.
@param		suite
			A pointer to the suite to execute.
*/
void
planck_unit_run_suite(
	planck_unit_suite_t		*suite
);

/**
@brief		Free all memory associated with a suite (and all tests
		it contains).
@details	This needs to be called to avoid leaking memory from
		creating a test suite and any test cases added to said suite.
@param		suite
			A pointer to the suite to safely destroy.
*/
void
planck_unit_destroy_suite(
	planck_unit_suite_t		*suite
);

/**
@brief		Assert that a condition is true.
@param		state
			The test's state information tracking
			the result of the test.
@param		condition
			The condition expression to evaluate.
			If this condition is true, then the assertion
			passes. Otherwise, the assertion fails.
*/
#define PLANCK_UNIT_ASSERT_TRUE(state, condition)\
if (PLANCK_UNIT_FAILURE == planck_unit_assert_true((state), (condition), __LINE__, __FILE__, __func__, "condition was false, expected true")) {return;}

/**
@brief		Assert that a condition is false.

@param  	state
			The test's state information tracking
			the result of the test.
*/
#define PLANCK_UNIT_ASSERT_FALSE(state)\
if (PLANCK_UNIT_FAILURE == planck_unit_assert_true((state), 0, __LINE__, __FILE__, __func__, "asserted as false")) {return;}

/**
@brief		Assert that two integers are equal.
@param		state
			The test's state information tracking
			the result of the test.
@param		expected
			The expected @c int value.
@param		actual
			The actual @c int value.
*/
#define PLANCK_UNIT_ASSERT_INT_ARE_EQUAL(state, expected, actual)\
if (PLANCK_UNIT_FAILURE == planck_unit_assert_int_are_equal((state), (expected), (actual), __LINE__, __FILE__, __func__)) {return;}

/**
@brief		Assert that two integers are equal.
@param		state
			The test's state information tracking
			the result of the test.
@param		expected
			The @c int value we wish to ensure the
			@p actual is not equal to.
@param		actual
			The actual @c int value.
*/
#define PLANCK_UNIT_ASSERT_INT_ARE_NOT_EQUAL(state, expected, actual)\
if (PLANCK_UNIT_FAILURE == planck_unit_assert_int_are_not_equal((state), (expected), (actual), __LINE__, __FILE__, __func__)) {return;}

/**
@brief		Assert that two null-terninated character strings are
		equivalent.
@param		state
			The test's state information tracking
			the result of the test.
@param		expected
			A pointer to the expected string data.
@param		actual
			A pointer to the actual string data.
*/
#define PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(state, expected, actual)\
if (PLANCK_UNIT_FAILURE == planck_unit_assert_str_are_equal((state), (expected), (actual), __LINE__, __FILE__, __func__)) {return;}

/**
@brief		Assert that two null-terninated character strings are not
		equivalent.
@details	The string comparison will be performed using @c strcmp.
@param		state
			The test's state information tracking
			the result of the test.
@param		expected
			A pointer to the string data we wish to assert
			@p actual is not equivalent to.
@param		actual
			A pointer to the actual string data.
*/
#define PLANCK_UNIT_ASSERT_STR_ARE_NOT_EQUAL(state, expected, actual)\
if (PLANCK_UNIT_FAILURE == planck_unit_assert_str_are_not_equal((state), (expected), (actual), __LINE__, __FILE__, __func__)) {return;}

#ifdef  __cplusplus
}
#endif

#endif
