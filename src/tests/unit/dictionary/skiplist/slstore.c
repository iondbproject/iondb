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

#define PRINT_HEADER(fcn) io_printf("=== [%s] ===\n", fcn)

void
check_skiplist(
	skiplist_t 	*skiplist
)
{
	sl_node_t 	*cursor = skiplist->head;
	sl_level_t 	h;

	for(h = skiplist->head->height; h >= 0; --h)
	{
		sl_node_t 	*oldcursor = cursor;
		while(NULL != cursor->next[h])
		{
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
	skiplist_t 	*skiplist,
	int 		maxheight,
	int 		key_size,
	int 		value_size,
	int 	 	pnum,
	int 		pden
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
	key_size 	= 8;
	value_size 	= 10;
	pnum 		= 1;
	pden 		= 4;
	maxheight 	= 7;

	initialize_skiplist(skiplist, maxheight, key_size, value_size, pnum, pden);
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
	PRINT_HEADER("test_skiplist_initialize");
	int key_size, value_size, pden, pnum, maxheight;
	key_size 	= 4;
	value_size 	= 10;
	pnum 		= 1;
	pden 		= 4;
	maxheight 	= 7;
	skiplist_t skiplist;

	initialize_skiplist(&skiplist, maxheight, key_size, value_size, pnum, pden);

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
	PRINT_HEADER("test_skiplist_free_all");
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
	PRINT_HEADER("test_skiplist_generate_levels_std_conditions");
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
	for(i = 0; i < 99; i++)
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
	PRINT_HEADER("test_skiplist_single_insert");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	int 	key 		= 6;
	char 	value[10];

	strcpy(value, "single.");

	sl_insert(&skiplist, (ion_key_t) &key, value);


#ifdef DEBUG
	check_skiplist(&skiplist);
#endif

	CuAssertTrue(tc, *((int*) skiplist.head->next[0]->key) 				== 6);
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
	PRINT_HEADER("test_skiplist_insert_multiple");
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

/**
@brief 		Tests a mixed order insert. The structure is traversed and tested
			to confirm that (p + 1) >= ( p ) in all cases, where p is the key
			of the current node.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_randomized_insert(
	CuTest 		*tc
)
{
	PRINT_HEADER("test_skiplist_randomized_insert");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	char str[10];
	strcpy(str, "random");

	int i;
	for(i = 0; i < 100; i++)
	{
		int key = rand() % 101;
		sl_insert(&skiplist, (ion_key_t) &key, str);
	}

#ifdef DEBUG
	check_skiplist(&skiplist);
#endif

	sl_node_t 	*cursor = skiplist.head;
	while(cursor->next[0]->next[0] != NULL)
	{
		int 	now 	= (int) *cursor->next[0]->key;
		int 	next 	= (int) *cursor->next[0]->next[0]->key;

		CuAssertTrue(tc, next >= now);

		cursor = cursor->next[0];
	}
}

/**
@brief 		Tests node search on a single node in a skiplist with only one node.

@details 	Tests node search on a single node in a skiplist with only one node.
			The key searched for is exact. The test compares the given node key
			and value information with the information inserted into the
			skiplist. The test passes if they are the same.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_get_node_single(
	CuTest 			*tc
)
{
	PRINT_HEADER("test_skiplist_get_node_single");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	char str[10];
	strcpy(str, "find this");

	int key = 3;

	sl_insert(&skiplist, (ion_key_t) &key, str);

	int search = 3;
	sl_node_t *node = sl_find_node(&skiplist, (ion_key_t) &search);

	CuAssertTrue(tc, (int) *node->key 			== key);
	CuAssertTrue(tc, strcmp(str, node->value) 	== 0);
}

/**
@brief 		Tests node search on a single node in a skiplist with only one node.

@details 	Tests node search on a single node in a skiplist with only one node.
			The key searched for is higher than the inserted key. The test
			compares the given node key and value information with the
			information inserted into the skiplist. The test passes if they are
			the same.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_get_node_single_high(
	CuTest 			*tc
)
{
	PRINT_HEADER("test_skiplist_get_node_single_high");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	char str[10];
	strcpy(str, "find this");

	int key = 3;

	sl_insert(&skiplist, (ion_key_t) &key, str);

#ifdef DEBUG
	check_skiplist(&skiplist);
#endif

	int search = 10;
	sl_node_t *node = sl_find_node(&skiplist, (ion_key_t) &search);

	CuAssertTrue(tc, (int) *node->key 			== key);
	CuAssertTrue(tc, strcmp(str, node->value) 	== 0);
}

/**
@brief 		Tests node search on a single node in a skiplist with only one node.

@details 	Tests node search on a single node in a skiplist with only one node.
			The key searched for is lower than the inserted key. The test
			compares the given node key and value information with the
			information inserted into the skiplist. The test passes if the node
			returned is the same as the head node. (Since there's only one node,
			the only node that can be smaller than the one insert is the head
			node.)
@param 		tc
				CuTest dependency
 */
void
test_skiplist_get_node_single_low(
	CuTest 			*tc
)
{
	PRINT_HEADER("test_skiplist_get_node_single_low");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	char str[10];
	strcpy(str, "find this");

	int key = 3;

	sl_insert(&skiplist, (ion_key_t) &key, str);

#ifdef DEBUG
	check_skiplist(&skiplist);
#endif

	int search = 2;
	sl_node_t *node = sl_find_node(&skiplist, (ion_key_t) &search);

	CuAssertTrue(tc, node == skiplist.head);
}

/**
@brief 		Tests node search on a single node in a skiplist with several nodes.

@details 	Tests node search on a single node in a skiplist with several nodes.
			The key searched for is exact. The test compares the given node key
			and value information with the information inserted into the
			skiplist. The test passes if they are the same.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_get_node_single_many(
	CuTest 			*tc
)
{
	PRINT_HEADER("test_skiplist_get_node_single_many");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	char str[10];
	strcpy(str, "find this");

	int key = 25;

	sl_insert(&skiplist, (ion_key_t) &key, str);

	char junk[10];
	strcpy(junk, "big junk");
	int i;
	for(i = 0; i < 100; i++)
	{
		if(i >= 10 && i <= 35)
		{
			//Create a gap ±10 from the target key
			continue;
		}
		sl_insert(&skiplist, (ion_key_t) &i, junk);
	}

#ifdef DEBUG
	check_skiplist(&skiplist);
#endif

	int search = 25;
	sl_node_t *node = sl_find_node(&skiplist, (ion_key_t) &search);

	CuAssertTrue(tc, (int) *node->key 			== key);
	CuAssertTrue(tc, strcmp(str, node->value) 	== 0);
}

/**
@brief 		Randomly generates 50 random key value pairs and inserts them into
			the skiplist. The nodes are then recalled and the key/value pairs
			are compared to the original inserted ones for accuracy.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_get_node_several(
	CuTest 			*tc
)
{
	PRINT_HEADER("test_skiplist_get_node_several");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

#ifdef DEBUG
	// If debugging, use a static seed
	srand(0xDEADBEEF);
#endif

	int targets[50];
	char buffer[10];
	int i;
	for(i = 0; i < 50; i++)
	{
		/* TODO For some reason all keys are being treated like chars (limit of 128?) Also If the keysize is set to 8 something segfaults.... */
		int key 	= rand() % 1000;
		targets[i] 	= key;
		sprintf(buffer, "TEST %d", key);
		sl_insert(&skiplist, (ion_key_t) &key, buffer);
	}

#ifdef DEBUG
	check_skiplist(&skiplist);
#endif

	for(i = 0; i < 50; i++)
	{
		int 		key		= targets[i];
		sl_node_t 	*node 	= sl_find_node(&skiplist, (ion_key_t) &key);
		sprintf(buffer, "TEST %d", key);
		CuAssertTrue(tc, (int) *node->key 				== key);
		CuAssertTrue(tc, strcmp(node->value, buffer) 	== 0);
	}
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
	SUITE_ADD_TEST(suite, test_skiplist_randomized_insert);
	SUITE_ADD_TEST(suite, test_skiplist_get_node_single);
	SUITE_ADD_TEST(suite, test_skiplist_get_node_single_high);
	SUITE_ADD_TEST(suite, test_skiplist_get_node_single_low);
	SUITE_ADD_TEST(suite, test_skiplist_get_node_single_many);
	SUITE_ADD_TEST(suite, test_skiplist_get_node_several);

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
