/******************************************************************************/
/**
@file
@author		Graeme Douglas
@brief		A simple unit testing framework that emits JSON
		as well as human readable results, optionally.
@details	For more information, please see @ref planck_unit.h.
@copyright      Copyright 2015 Graeme Douglas
@license        Licensed under the Apache License, Version 2.0 (the "License");
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

#include "planck_unit.h"

#if defined(ARDUINO)
#include "../benchmark/ram/ram_util.h"
#endif

/**
@brief		If possible, flush all output so far.
*/
#define PLANCK_UNIT_FLUSH		fflush(stdout);

/**
@brief		Print a string.
@param		s
			A null-terminated character string (a pointer
			to the character array describing the string).
*/
#define PLANCK_UNIT_PRINT_STR(s)	printf(s);PLANCK_UNIT_FLUSH;

/**
@brief		Print a comma.
*/
#define PLANCK_UNIT_PRINT_COMMA		printf(",");PLANCK_UNIT_FLUSH;

/**
@brief		Print a newline.
*/
#define PLANCK_UNIT_PRINT_NEWLINE	printf("\n");PLANCK_UNIT_FLUSH;

/* Output style depends on build-time arguments. See @ref CMakeLists.txt for information. */
#if defined(PLANCK_UNIT_OUTPUT_STYLE_JSON)

void
planck_unit_print_result_json(
	planck_unit_test_t		*state
)
{
	printf(
		"{\"error_at_line\":%d,\"file\":\"%s\",\"function\":\"%s\",\"message\":\"%s\"}",
		state->line,
		state->file,
		state->func_name,
		state->message
	);
	PLANCK_UNIT_FLUSH;
	if (NULL != state->next)
	{
		PLANCK_UNIT_PRINT_COMMA;
	}
}

void
planck_unit_print_preamble_json(
	void
)
{
	PLANCK_UNIT_PRINT_STR("{\"results\":[");
}

void
planck_unit_print_postamble_json(
	planck_unit_suite_t	*suite
)
{
	printf(
		"],\"total_tests\":%d,\"total_passed\":%d}\n",
		suite->total_tests,
		suite->total_passed
	);
	PLANCK_UNIT_FLUSH;
}

planck_unit_print_funcs_t planck_unit_print_funcs_json =
        {
                planck_unit_print_result_json,
                planck_unit_print_preamble_json,
                planck_unit_print_postamble_json
        };

#endif
#if defined(PLANCK_UNIT_OUTPUT_STYLE_HUMAN)

void
planck_unit_print_result_human(
	planck_unit_test_t		*state
)
{
	if (PLANCK_UNIT_SUCCESS == state->result)
	{
		return;
	}
	printf(
		"in function '%s', at %s:%d: %s\n",
		state->func_name,
		state->file,
		state->line,
		state->message
	);
}

void
planck_unit_print_preamble_none(
	void
)
{
}

void
planck_unit_print_postamble_summary(
	planck_unit_suite_t	*suite
)
{
	planck_unit_test_t	*state;
	
	PLANCK_UNIT_PRINT_STR("\n");
	state			= suite->head;
	while (NULL != state)
	{
		if (PLANCK_UNIT_SUCCESS == state->result)
		{
			PLANCK_UNIT_PRINT_STR("*");
		}
		else
		{
			PLANCK_UNIT_PRINT_STR("F");
		}
		
		state		= state->next;
	}

	printf("\n\nTotal Tests:\t%d\n", suite->total_tests);PLANCK_UNIT_FLUSH;
	printf("Total Passed:\t%d\n", suite->total_passed);PLANCK_UNIT_FLUSH;
}

planck_unit_print_funcs_t planck_unit_print_funcs_human =
{
	planck_unit_print_result_human,
	planck_unit_print_preamble_none,
	planck_unit_print_postamble_summary
};

#endif
#if defined(PLANCK_UNIT_OUTPUT_STYLE_XML)

void
planck_unit_print_result_xml(
	planck_unit_test_t *state
)
{
	printf(
		"<test>line:\"%d\",file:\"%s\",function:\"%s\",message:\"%s\"</test>\n",
		state->line,
		state->file,
		state->func_name,
		state->message
	);
	PLANCK_UNIT_FLUSH;
}

void
planck_unit_print_preamble_xml(
	void
)
{
	PLANCK_UNIT_PRINT_STR("<suite>\n");
}

void
planck_unit_print_postamble_xml(
	planck_unit_suite_t *suite
)
{
	printf(
		"<summary>total_tests:\"%d\",total_passed:\"%d\"</summary>\n",
		suite->total_tests, 
		suite->total_passed
	);
	PLANCK_UNIT_FLUSH;
	PLANCK_UNIT_PRINT_STR("</suite>\n");
}

planck_unit_print_funcs_t planck_unit_print_funcs_xml =
{
	planck_unit_print_result_xml,
	planck_unit_print_preamble_xml,
	planck_unit_print_postamble_xml
};

#endif

#if defined(PLANCK_UNIT_OUTPUT_STYLE_CONCISE)

void
planck_unit_print_result_concise(
    planck_unit_test_t		*state
)
{
    printf("[");
    printf("%s", state->func_name);
    printf(": ");
    if (PLANCK_UNIT_SUCCESS == state->result) {
        printf("pass");
    }
    else {
        printf("%d", state->line);
        printf(": ");
        printf("%s", state->message);
    }
    printf("]");
    PLANCK_UNIT_FLUSH;
    PLANCK_UNIT_PRINT_NEWLINE;
}

void
planck_unit_print_preamble_concise(
    void
)
{
    printf("[");
    printf("[");
    printf("Results");
    printf("]");
    printf("]");
    PLANCK_UNIT_PRINT_NEWLINE;
}

void
planck_unit_print_postamble_concise(
    planck_unit_suite_t	*suite
)
{
    printf("[");
    printf("[");
    printf("Passed");
    printf(": ");
    printf("%d", suite->total_passed);
    printf("/");
    printf("%d", suite->total_tests);
    printf("]");
    printf("]");
    PLANCK_UNIT_PRINT_NEWLINE;
    PLANCK_UNIT_FLUSH;
}

planck_unit_print_funcs_t planck_unit_print_funcs_concise =
{
    planck_unit_print_result_concise,
    planck_unit_print_preamble_concise,
    planck_unit_print_postamble_concise
};

/* End of build-time-enabled output styles. */
#endif

/**
@brief      Check whether enough memory space is available for a
            string based test's output to be defined and
            printed.
@param      message
            Output line surrounding the test's expected and
            actual values.
@param		expected
			A pointer to the expected string data.
@param		actual
			A pointer to the actual string data.
@returns    The size needed if available, otherwise 0 (false).
*/
int
planck_unit_check_string_space(
    const char*   message,
    void*   expected,
    void*   actual
)
{
    int			message_size;
    message_size		 = strlen(message);
    message_size		+= strlen((char*)expected);
    message_size		+= strlen((char*)actual);

//    return ((free_ram() > message_size)? message_size : 0);
	return 1;
}

/**
@brief      Check whether enough memory space is available for an
            @c int based test's output to be defined and
            printed.
@param      message
            Output line surrounding the test's expected and
            actual values.
@param		expected
			A pointer to the expected @c int value.
@param		actual
			A pointer to the actual @c int value.
@returns    The size needed if available, otherwise 0 (false).
*/
int
planck_unit_check_int_space(
    const char*   message,
    void*   expected,
    void*   actual
)
{
    int			message_size;
    message_size		 = strlen(message);
    /* Quick overestimation of characters needed to represent integer. */
    message_size		+= 2*(4*sizeof(int) + 2);

//    return ((free_ram() > message_size)? message_size : 0);
	return 1;
}

void
planck_unit_init_suite(
	planck_unit_suite_t		*suite,
	planck_unit_print_funcs_t	print_funcs
)
{
	suite->total_tests	= 0;
	suite->total_passed	= 0;
	suite->head		= NULL;
	suite->tail		= NULL;
	suite->print_functions	= print_funcs;
}

planck_unit_suite_t *
planck_unit_new_suite(
	void
)
{
	planck_unit_suite_t	*suite;
	suite			= malloc(sizeof(planck_unit_suite_t));
	if (NULL == suite)
	{
		return suite;
	}

    /* Messy preprocessor statements since output types are determined at build time.
        Left as such (rather than value-based) as multiple options may be
        independently defined at once in the make/build settings. */

#if defined(PLANCK_UNIT_OUTPUT_STYLE_JSON)
    planck_unit_init_suite(
            suite,
            planck_unit_print_funcs_json
    );
#elif defined(PLANCK_UNIT_OUTPUT_STYLE_HUMAN)
    planck_unit_init_suite(
            suite,
            planck_unit_print_funcs_human
    );
#elif defined(PLANCK_UNIT_OUTPUT_STYLE_XML)
    planck_unit_init_suite(
            suite,
            planck_unit_print_funcs_xml
    );
#else
    planck_unit_init_suite(
            suite,
            planck_unit_print_funcs_concise
    );
#endif
	return suite;
}

/**
@brief		Assert that a condition is true.
@details	One should not call this method directly, but only through
		@ref PLANCK_UNIT_ASSERT_TRUE.
@param		state
			A pointer to the test structure describing the
			result of this test's execution.
@param		condition
			The condition's evalutation result. If nonzero,
			the condition is assumed to be true.
@param		line
			The line at which the assertion is made.
@param		file
			The name of the file where the assertion is being made.
@param		func
			The name of the function in which the assertion
			is being made.
@param		message
			A message describing why the assertion may have failed.
			This will only be used if the assertion actually fails.
@returns	The result of the assertion, either @ref PLANCK_UNIT_SUCCESS
		on success or @ref PLANCK_UNIT_FAILURE otherwise.
*/
planck_unit_result_t
planck_unit_assert_true(
	planck_unit_test_t	*state,
	int			condition,
	int			line,
	const char		*file,
	const char		*func,
	char			*message
)
{
	if (condition)
	{
		/* Do this now, since message pointer gets replaced */
		if (1 == state->allocated_message) { free(message); }

		line		= -1;
		file		= "";
		func		= "";
		message		= "";
		/* Message has been freed or replaced, and is no longer allocated */
		state->allocated_message = 0;
		state->result	= PLANCK_UNIT_SUCCESS;
	}
	else
	{
		state->result	= PLANCK_UNIT_FAILURE;
	}
	state->line		= line;
	state->file		= file;
	state->func_name	= func;
	state->message		= message;
	
	return state->result;
}

/**
@brief		Assert that two integers are equal.
@details	One should not call this method directly, but only through
		@ref PLANCK_UNIT_INT_ARE_EQUAL.
@param		state
			A pointer to the test structure describing the
			result of this test's execution.
@param		expected
			The expected integer value.
@param		actual
			The actual integer value.
@param		line
			The line at which the assertion is made.
@param		file
			The name of the file where the assertion is being made.
@param		func
			The name of the function in which the assertion
			is being made.
@returns	The result of the assertion, either @ref PLANCK_UNIT_SUCCESS
		on success or @ref PLANCK_UNIT_FAILURE otherwise.
*/
planck_unit_result_t
planck_unit_assert_int_are_equal(
	planck_unit_test_t	*state,
	int			expected,
	int			actual,
	int			line,
	const char		*file,
	const char		*func
)
{
	const char		*message;
	int			message_size;
	char			*buffer;
	message			 =
		"expected int %d, got %d";

    message_size     = planck_unit_check_int_space(message, &expected, &actual);
	state->allocated_message = 1;
	buffer			 = malloc(message_size);
	if (NULL == buffer || !message_size)
	{
        free(buffer);
		PLANCK_UNIT_PRINT_STR("out of memory");
		exit(-1);
	}
	sprintf(buffer, message, expected, actual);
	
	return
	planck_unit_assert_true(
		state,
		expected == actual,
		line,
		file,
		func,
		buffer
	);
}

/**
@brief		Assert that two integers are not equal.
@details	One should not call this method directly, but only through
		@ref PLANCK_UNIT_INT_ARE_NOT_EQUAL.
@param		state
			A pointer to the test structure describing the
			result of this test's execution.
@param		expected
			The integer value which @p actual should not be equal
			to.
@param		actual
			The actual integer value.
@param		line
			The line at which the assertion is made.
@param		file
			The name of the file where the assertion is being made.
@param		func
			The name of the function in which the assertion
			is being made.
@returns	The result of the assertion, either @ref PLANCK_UNIT_SUCCESS
		on success or @ref PLANCK_UNIT_FAILURE otherwise.
*/
planck_unit_result_t
planck_unit_assert_int_are_not_equal(
	planck_unit_test_t	*state,
	int			expected,
	int			actual,
	int			line,
	const char		*file,
	const char		*func
)
{
	const char		*message;
	int			message_size;
	char			*buffer;
	
	message			 =
		"expected not int %d, got %d";

    message_size     = planck_unit_check_int_space(message, &expected, &actual);
	state->allocated_message = 1;
	buffer			 = malloc(message_size);
	if (NULL == buffer || !message_size)
	{
        free(buffer);
		PLANCK_UNIT_PRINT_STR("out of memory");
		exit(-1);
	}
	sprintf(buffer, message, expected, actual);
	
	return
	planck_unit_assert_true(
		state,
		expected != actual,
		line,
		file,
		func,
		buffer
	);
}

/**
@brief		Assert that two null-terminated character array
		strings are content-equivalent.
@details	One should not call this method directly, but only through
		@ref PLANCK_UNIT_STR_ARE_EQUAL.
@param		state
			A pointer to the test structure describing the
			result of this test's execution.
@param		expected
			The expected string data.
@param		actual
			The actual string data.
@param		line
			The line at which the assertion is made.
@param		file
			The name of the file where the assertion is being made.
@param		func
			The name of the function in which the assertion
			is being made.
@returns	The result of the assertion, either @ref PLANCK_UNIT_SUCCESS
		on success or @ref PLANCK_UNIT_FAILURE otherwise.
*/
planck_unit_result_t
planck_unit_assert_str_are_equal(
	planck_unit_test_t	*state,
	char			*expected,
	char			*actual,
	int			line,
	const char		*file,
	const char		*func
)
{
	const char		*message;
    int             message_size;
	char			*buffer;
	message			 =
		"expected \\\"%s\\\", got \\\"%s\\\"";
	state->allocated_message = 1;
    message_size     = planck_unit_check_string_space(message, expected, actual);
	buffer			 = malloc(message_size);
	if (NULL == buffer || !message_size)
	{
        free(buffer);
		PLANCK_UNIT_PRINT_STR("out of memory");
		exit(-1);
	}
	sprintf(buffer, message, expected, actual);
	
	return
	planck_unit_assert_true(
		state,
		0==strcmp(expected, actual),
		line,
		file,
		func,
		buffer
	);
}

/**
@brief		Assert that two null-terminated character array
		strings are not content-equivalent.
@details	One should not call this method directly, but only through
		@ref PLANCK_UNIT_STR_ARE_NOT_EQUAL.
@param		state
			A pointer to the test structure describing the
			result of this test's execution.
@param		expected
			The string data which @p actual should not be
			content-equivalent to.
@param		actual
			The actual string data.
@param		line
			The line at which the assertion is made.
@param		file
			The name of the file where the assertion is being made.
@param		func
			The name of the function in which the assertion
			is being made.
@returns	The result of the assertion, either @ref PLANCK_UNIT_SUCCESS
		on success or @ref PLANCK_UNIT_FAILURE otherwise.
*/
planck_unit_result_t
planck_unit_assert_str_are_not_equal(
	planck_unit_test_t	*state,
	char			*expected,
	char			*actual,
	int			line,
	const char		*file,
	const char		*func
)
{
	const char		*message;
	int			message_size;
	char			*buffer;
	message			 =
		"expected not \\\"%s\\\", got \\\"%s\\\"";
    message_size     = planck_unit_check_string_space(message, expected, actual);
	state->allocated_message = 1;
	buffer			 = malloc(message_size);
	if (NULL == buffer || !message_size)
	{
        free(buffer);
		PLANCK_UNIT_PRINT_STR("out of memory");
		exit(-1);
	}
	sprintf(buffer, message, expected, actual);
	
	return
	planck_unit_assert_true(
		state,
		0!=strcmp(expected, actual),
		line,
		file,
		func,
		buffer
	);
}

void
planck_unit_add_to_suite(
	planck_unit_suite_t	*suite,
	planck_unit_test_func_t	test_func
)
{
	planck_unit_test_t			*next;
	
	next				= malloc(sizeof(planck_unit_test_t));
	if (NULL == next)
	{
		PLANCK_UNIT_PRINT_STR("out of memory\n");
		exit(-1);
	}
	
	next->test_func			= test_func;
	next->next			= NULL;
	next->suite			= suite;
	next->allocated_message		= 0;
	
	if (NULL == suite->head)
	{
		suite->tail		= next;
		suite->head		= next;
	}
	else
	{
		suite->tail->next	= next;
		suite->tail		= next;
	}
}

void
planck_unit_run_suite(
	planck_unit_suite_t	*suite
)
{
	planck_unit_test_t		*state;
	
	state			= suite->head;
	
	suite->print_functions.print_preamble();
	while (NULL != state)
	{
		state->test_func(state);
		suite->total_tests++;
		if (PLANCK_UNIT_SUCCESS == state->result)
		{
			suite->total_passed++;
		}
		suite->print_functions.print_result(state);
		if (PLANCK_UNIT_FAILURE == state->result && 1==state->allocated_message)
		{
			free(state->message);
		}
		state		= state->next;
	}
	suite->print_functions.print_postamble(suite);
	PLANCK_UNIT_PRINT_NEWLINE;
}

void
planck_unit_destroy_suite(
	planck_unit_suite_t	*suite
)
{
	/* We will use the tail pointer to track the next test state to free. */
	while (NULL != suite->head)
	{
		suite->tail		= suite->head->next;
		free(suite->head);
		suite->head		= suite->tail;
	}
	free(suite);
}
