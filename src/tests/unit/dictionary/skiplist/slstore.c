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
	sl_level_t h;

	for(h = skiplist->head->height; h >= 0; --h)
	{
		while(NULL != cursor->next[h])
		{
			int key = 0, value = 0;
			key 	= *((int*) cursor->key);
			value 	= *((int*) cursor->value);
			printf("k: %i (v: %i) -- ", key, value);
			cursor = cursor->next[h];
		}

		if(NULL == cursor->next[h])
		{
			printf("%s\n", " -- NULL");
		}
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
	for(i = 0; i < TEST_SIZE; i++) {
		CuAssertTrue(tc, sl_gen_level(&skiplist) == prediction[i]);
	}
}

/**
@brief 		Tests a single, direct insert into the skiplist. The result is
			verified by visualizing the list structure against a prediction.

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

	char key[4] 	= {0x11, 0x22, 0x33, 0x44}; // 1,144,201,745 in Dec
	char value[10] 	= {0x44, 0x33, 0x22, 0x11, 0x00,
						0x00, 0x00, 0x00, 0x00, 0x00};



	//sl_insert(&skiplist, key, value);
}

CuSuite*
skiplist_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_skiplist_initialize);
	SUITE_ADD_TEST(suite, test_skiplist_free_all);
	SUITE_ADD_TEST(suite, test_skiplist_generate_levels_std_conditions);

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
