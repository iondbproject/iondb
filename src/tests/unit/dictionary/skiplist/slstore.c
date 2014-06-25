/******************************************************************************/
/**
@file		slstore.c
@author		Kris Wallperington
@brief		Unit tests for Skiplist data store
*/
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "./../../../CuTest.h"
#include "./../../../../dictionary/skiplist/slstore.h"
#include "./../../../../dictionary/skiplist/slhandler.h"
#include "./../../../../dictionary/dicttypes.h"
#include "./../../../../dictionary/dictionary.h"

#define TEST_SIZE 99

void
check_skiplist(
	skiplist_t 	*skiplist
)
{
	sl_node_t 	*cursor = skiplist->head;
	sl_level_t 	h;

	int 		width 		= 0;
	sl_node_t 	*counter 	= skiplist->head;
	while(counter->next[0] != NULL)
	{
		counter = counter->next[0];
		width++;
	}

	for(h = skiplist->head->height; h >= 0; --h)
	{
		sl_node_t 	*oldcursor = cursor;
		while(NULL != cursor->next[h])
		{
			/* TODO The print doesn't look pretty. How to fix? */
			int 	key 	= (int) *cursor->next[h]->key;
			char* 	value 	= (char*) cursor->next[h]->value;
			printf("k: %i (v: %s) -- ", key, value);
			cursor = cursor->next[h];
		}

		if(NULL == cursor->next[h])
		{
			printf("%s\n", "NULL");
		}

		cursor = oldcursor;
	}

	printf("%s", "\n");
}

void
initialize_skiplist(
	int 		maxheight,
	int 		key_size,
	int 		value_size,
	int 	 	pnum,
	int 		pden,
	skiplist_t 	*skiplist
)
{
	sl_initialize(skiplist, key_size, value_size, maxheight, pnum, pden);
}

void
initialize_skiplist_std_conditions(
		skiplist_t 	*skiplist
)
{
	int key_size, value_size, pden, pnum, maxheight;
	key_size 	= 4;
	value_size 	= 10;
	pnum 		= 1;
	pden 		= 4;
	maxheight 	= 7;

	initialize_skiplist(maxheight, key_size, value_size, pnum, pden, skiplist);
}

/**
@brief 		Tests creation of the skiplist.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_initialize(
	CuTest 		*tc
)
{
	int key_size, value_size, pden, pnum, maxheight;
	key_size 	= 4;
	value_size 	= 10;
	pnum 		= 1;
	pden 		= 4;
	maxheight 	= 7;
	skiplist_t skiplist;

	initialize_skiplist(maxheight, key_size, value_size, pnum, pden, &skiplist);

#ifdef DEBUG
	check_skiplist(&skiplist);
#endif

	CuAssertTrue(tc, skiplist.key_size 		== key_size);
	CuAssertTrue(tc, skiplist.value_size 	== value_size);
	CuAssertTrue(tc, skiplist.maxheight 	== maxheight);
	CuAssertTrue(tc, skiplist.pnum 			== pnum);
	CuAssertTrue(tc, skiplist.pden 			== pden);
}

/**
@brief 		Tests if the Skiplist is properly destroyed.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_free_all(
	CuTest 		*tc
)
{
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	sl_destroy(&skiplist);

	CuAssertTrue(tc, skiplist.head 	== NULL);
}

/**
@brief 		Tests random height generation with a known seed against prior
			known values. Sample size is 100.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_generate_levels_std_conditions(
	CuTest 		*tc
)
{
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	/*
	 * This is 100 level generations with maxheight = 7, pnum = 1, pden = 4.
	 * The seed used is 0xDEADBEEF.
	 */
	int prediction[] =
	{		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 0, 1, 0, 1, 0, 0,
			1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 3, 0, 0, 0,
			1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 2, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1,
			0, 3, 0, 0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 3, 0,
			0, 0, 0, 1, 0, 0, 1
	};

	srand(0xDEADBEEF);
	int i;
	for(i = 0; i < TEST_SIZE; i++)
	{
		CuAssertTrue(tc, sl_gen_level(&skiplist) == prediction[i]);
	}

	/* TODO add another test with a different seed */
}

/**
@brief 		Tests a single insert into the skiplist.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_single_insert(
	CuTest 		*tc
)
{
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	int 	key 		= 6;
	char 	value[10];

	strcpy(value, "single.");

	sl_insert(&skiplist, (ion_key_t) &key, value);


#ifdef DEBUG
	check_skiplist(&skiplist);
#endif

	CuAssertTrue(tc, *((int*) skiplist.head->next[0]->key) 		== 6);
	CuAssertTrue(tc, strcmp(skiplist.head->next[0]->value, "single.") 	== 0);

}

/**
@brief 		Tests several insertions into the skiplist. The skiplist structure
			is accessed directly in order to verify the insertion has properly
			occurred.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_insert_multiple(
	CuTest 		*tc
)
{
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	char strs[5][6] =
	{		"one",
			"two",
			"three",
			"four",
			"five"
	};

	int i;
	for(i = 1; i <= 5; i++)
	{
		sl_insert(&skiplist, (ion_key_t) &i, strs[i - 1]);
	}

#ifdef DEBUG
		check_skiplist(&skiplist);
#endif

	CuAssertTrue(tc, *((int*) skiplist.head->next[0]->key) 													== 1);
	CuAssertTrue(tc, strcmp(skiplist.head->next[0]->value, "one") 											== 0);

	CuAssertTrue(tc, *((int*) skiplist.head->next[0]->next[0]->key) 										== 2);
	CuAssertTrue(tc, strcmp(skiplist.head->next[0]->next[0]->value, "two") 									== 0);

	CuAssertTrue(tc, *((int*) skiplist.head->next[0]->next[0]->next[0]->key) 								== 3);
	CuAssertTrue(tc, strcmp(skiplist.head->next[0]->next[0]->next[0]->value, "three") 						== 0);

	CuAssertTrue(tc, *((int*) skiplist.head->next[0]->next[0]->next[0]->next[0]->key) 						== 4);
	CuAssertTrue(tc, strcmp(skiplist.head->next[0]->next[0]->next[0]->next[0]->value, "four") 				== 0);

	CuAssertTrue(tc, *((int*) skiplist.head->next[0]->next[0]->next[0]->next[0]->next[0]->key) 				== 5);
	CuAssertTrue(tc, strcmp(skiplist.head->next[0]->next[0]->next[0]->next[0]->next[0]->value, "five") 		== 0);
}

CuSuite*
skiplist_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_skiplist_initialize);
	SUITE_ADD_TEST(suite, test_skiplist_free_all);
	SUITE_ADD_TEST(suite, test_skiplist_generate_levels_std_conditions);
	SUITE_ADD_TEST(suite, test_skiplist_single_insert);
	SUITE_ADD_TEST(suite, test_skiplist_insert_multiple);

	return suite;
}

void
runalltests_skiplist()
{
	CuString	*output	= CuStringNew();
	CuSuite		*suite	= skiplist_getsuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);

	CuSuiteDelete(suite);
	CuStringDelete(output);
}

void
runalltests_skiplist_handler()
{
	dictionary_handler_t handler;

	sldict_init(&handler);
}
