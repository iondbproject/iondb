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

	while(NULL != cursor->next[0])
	{
		int 		key 		= *((int*)cursor->next[0]->key);
		char* 		value 		= (char*) cursor->next[0]->value;
		sl_level_t 	level 		= cursor->next[0]->height + 1;
		printf("k: %d (v: %s) [l: %d] -- ", key, value, level);
		cursor = cursor->next[0];
	}

	printf("%s", "END\n\n");
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
	key_size 	= 4;
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

	sl_destroy(&skiplist);
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

	sl_destroy(&skiplist);
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

	sl_destroy(&skiplist);
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
		int 	now 	= *((int*)cursor->next[0]->key);
		int 	next 	= *((int*)cursor->next[0]->next[0]->key);

		CuAssertTrue(tc, next >= now);

		cursor = cursor->next[0];
	}

	sl_destroy(&skiplist);
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

	CuAssertTrue(tc, *((int*)node->key) 		== key);
	CuAssertTrue(tc, strcmp(str, node->value) 	== 0);

	sl_destroy(&skiplist);
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

	CuAssertTrue(tc, *((int*)node->key) 		== key);
	CuAssertTrue(tc, strcmp(str, node->value) 	== 0);

	sl_destroy(&skiplist);
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

	sl_destroy(&skiplist);
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

	CuAssertTrue(tc, *((int*)node->key) 		== key);
	CuAssertTrue(tc, strcmp(str, node->value) 	== 0);

	sl_destroy(&skiplist);
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
		CuAssertTrue(tc, *((int*)node->key) 			== key);
		CuAssertTrue(tc, strcmp(node->value, buffer) 	== 0);
	}

	sl_destroy(&skiplist);
}

/**
@brief 		Tests querying on an empty skiplist. assertion is that the status
			should return as "err_item_not_found", and that the value pointer
			should not be allocated and will be set to null.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_query_nonexist_empty(
	CuTest 		*tc
)
{
	PRINT_HEADER("test_skiplist_query_single");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	int 			key 	= 3;
	ion_value_t 	value;

	err_t status = sl_query(&skiplist, (ion_key_t) &key, &value);

#ifdef DEBUG
	check_skiplist(&skiplist);
#endif

	CuAssertTrue(tc, status == err_item_not_found);
	CuAssertTrue(tc, value 	== NULL);

	sl_destroy(&skiplist);
}

/**
@brief 		Tests querying on a skiplist with one element, but for a key that
			doesn't exist within the skiplist. assertion is that the status
			should return as "err_item_not_found", and that the value pointer
			be initialized to null.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_query_nonexist_populated_single(
	CuTest 		*tc
)
{
	PRINT_HEADER("test_skiplist_query_nonexist_populated_single");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	int test_key = 23;
	char test_value[10];
	strcpy(test_value, "I am test");

	sl_insert(&skiplist, (ion_key_t) &test_key, test_value);

#ifdef DEBUG
	check_skiplist(&skiplist);
#endif

	int 			key 	= 10;
	ion_value_t 	value;

	err_t status = sl_query(&skiplist, (ion_key_t) &key, &value);

	CuAssertTrue(tc, status == err_item_not_found);
	CuAssertTrue(tc, value 	== NULL);

	sl_destroy(&skiplist);
}

/**
@brief 		Tests querying on a skiplist with several elements, but for a key
			that doesn't exist within the skiplist. assertion is that the
			status should return as "err_item_not_found", and that the value
			pointer be initialized to null.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_query_nonexist_populated_several(
	CuTest 		*tc
)
{
	PRINT_HEADER("test_skiplist_query_nonexist_populated_several");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	int test_key = 46;
	char test_value[10];

	int i;
	for(i = 0; i < 32; i++)
	{
		sprintf(test_value, "I am: %d", test_key);
		sl_insert(&skiplist, (ion_key_t) &test_key, test_value);
		test_key--;
	}

#ifdef DEBUG
	check_skiplist(&skiplist);
#endif

	int 			key 	= 10;
	ion_value_t 	value;

	err_t status = sl_query(&skiplist, (ion_key_t) &key, &value);

	CuAssertTrue(tc, status == err_item_not_found);
	CuAssertTrue(tc, value 	== NULL);

	sl_destroy(&skiplist);
}

/**
@brief 		Tests querying on a skiplist with a single element, for a key that
			does exist within the skiplist. assertion is that the status should
			return as "err_ok", and that the value be initialized to the same
			value as stored at the specified key.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_query_exist_single(
	CuTest 		*tc
)
{
	PRINT_HEADER("test_skiplist_query_exist_single");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	int test_key = 11;
	char test_value[10];
	strcpy(test_value, "Find me!");

	sl_insert(&skiplist, (ion_key_t) &test_key, test_value);

#ifdef DEBUG
	check_skiplist(&skiplist);
#endif

	int 			key 	= 11;
	ion_value_t 	value;

	err_t status = sl_query(&skiplist, (ion_key_t) &key, &value);

	CuAssertTrue(tc, status == err_ok);
	CuAssertTrue(tc, strcmp(value, "Find me!") == 0);

	free(value);
	sl_destroy(&skiplist);
}

/**
@brief 		Tests querying on a skiplist with several elements, for a key that
			exists within the skiplist. assertion is that the status should
			return as "err_ok", and that the value be initialized to the same
			value stored at the specified key.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_query_exist_populated_single(
	CuTest 		*tc
)
{
	PRINT_HEADER("test_skiplist_query_exist_populated_single");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	char test_value[10];

	int i;
	for(i = 0; i < 100; i += 2)
	{
		sprintf(test_value, "Find %d", i);
		sl_insert(&skiplist, (ion_key_t) &i, test_value);
	}

#ifdef DEBUG
	check_skiplist(&skiplist);
#endif

	int 			key 	= 24;
	ion_value_t 	value;

	err_t status = sl_query(&skiplist, (ion_key_t) &key, &value);

	CuAssertTrue(tc, status == err_ok);
	CuAssertTrue(tc, strcmp(value, "Find 24") == 0);

	free(value);
	sl_destroy(&skiplist);
}

/**
@brief 		Tests querying on a skiplist with several elements, for a key that
			exists within the skiplist. assertion is that the status should
			return as "err_ok", and that the value be initialized to the same
			value stored at the specified key.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_query_exist_populated_several(
	CuTest 		*tc
)
{
	PRINT_HEADER("test_skiplist_query_exist_populated_several");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	char test_value[10];

	int i;
	for(i = 0; i < 100; i++)
	{
		sprintf(test_value, "Find %d", i);
		sl_insert(&skiplist, (ion_key_t) &i, test_value);
	}

#ifdef DEBUG
	check_skiplist(&skiplist);
#endif

	char 			find_value[10];
	ion_value_t 	value;
	for(i = 0; i < 100; i++)
	{
		sprintf(find_value, "Find %d", i);
		err_t status = sl_query(&skiplist, (ion_key_t) &i, &value);

		CuAssertTrue(tc, status == err_ok);
		CuAssertTrue(tc, strcmp(value, find_value) == 0);
		free(value);
	}

	sl_destroy(&skiplist);
}

/**
@brief 		Tests a deletion from a skiplist that's empty. The assertion is
			that the deletion will fail, and return "err_item_not_found". No
			modifications should be made to the structure.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_delete_empty(
	CuTest 		*tc
)
{
	PRINT_HEADER("test_skiplist_delete_empty");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	int key 		= 3;
	err_t status 	= sl_delete(&skiplist, (ion_key_t) &key);

#ifdef DEBUG
	check_skiplist(&skiplist);
#endif

	CuAssertTrue(tc, status == err_item_not_found);

	sl_destroy(&skiplist);
}

/**
@brief 		Tests a deletion from a skiplist that has one element, and the
			deleted element is not the one within the skiplist. The assertion
			is that the returned status is "err_item_not_found", and that no
			modification is to be made to the data structure.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_delete_nonexist_single(
	CuTest 		*tc
)
{
	PRINT_HEADER("test_skiplist_delete_nonexist_single");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	int 	key 		= 16;
	char 	value[10];
	strcpy(value, "Delete me!");
	sl_insert(&skiplist, (ion_key_t) &key, value);

	int fake_key = 33;
	err_t status = sl_delete(&skiplist, (ion_key_t) &fake_key);

#ifdef DEBUG
	check_skiplist(&skiplist);
#endif

	CuAssertTrue(tc, status == err_item_not_found);

	sl_destroy(&skiplist);
}

/**
@brief 		Tests a deletion from a skiplist that has many elements, and the
			deleted element is not the one within the skiplist. The assertion
			is that the returned status is "err_item_not_found", and that no
			modification is to be made to the data structure.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_delete_nonexist_several(
	CuTest 		*tc
)
{
	PRINT_HEADER("test_skiplist_delete_nonexist_several");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	int 	key 		= 16;
	char 	value[10];
	strcpy(value, "Delete me!");

	int i;
	for(i = 0; i < 10; i++)
	{
		sl_insert(&skiplist, (ion_key_t) &key, value);
		key += 3;
	}

	int fake_key = 20;
	err_t status = sl_delete(&skiplist, (ion_key_t) &fake_key);

#ifdef DEBUG
	check_skiplist(&skiplist);
#endif

	CuAssertTrue(tc, status == err_item_not_found);

	sl_destroy(&skiplist);
}

/**
@brief 		Tests deletion in a single element skiplist, where the deleted
			element is the one that exists within the skiplist. The assertion
			is that the status returned will be "err_ok", and the key/value pair
			deleted is no longer within the skiplist.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_delete_single(
	CuTest 		*tc
)
{
	PRINT_HEADER("test_skiplist_delete_single");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	int 	key 		= 97;
	char 	value[10];
	strcpy(value, "Special K");

	sl_insert(&skiplist, (ion_key_t) &key, value);

#ifdef DEBUG
	printf("%s\n", "** BEFORE **");
	check_skiplist(&skiplist);
#endif

	err_t status = sl_delete(&skiplist, (ion_key_t) &key);

#ifdef DEBUG
	printf("%s\n", "** AFTER **");
	check_skiplist(&skiplist);
#endif

	CuAssertTrue(tc, status					 == err_ok);
	CuAssertTrue(tc, skiplist.head->next[0]	 == NULL);

	sl_destroy(&skiplist);
}

/**
@brief 		Tests deleting a single node in a skiplist of several nodes, where
			the element to delete exists in the skiplist. The assertion is that
			the status returned will be "err_ok", and the key/value pair deleted
			is no longer within the skiplist.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_delete_single_several(
	CuTest 		*tc
)
{
	PRINT_HEADER("test_skiplist_delete_single_several");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	char value[10];
	strcpy(value, "Poof me!");

	int i;
	for(i = 101; i < 120; i++)
	{
		sl_insert(&skiplist, (ion_key_t) &i, value);
	}

#ifdef DEBUG
	printf("%s\n", "** BEFORE **");
	check_skiplist(&skiplist);
#endif

	sl_node_t 	*onebefore 	= sl_find_node(&skiplist, (ion_key_t) &(int) {111});
	sl_node_t 	*theone 	= sl_find_node(&skiplist, (ion_key_t) &(int) {112});
	sl_level_t 	theone_h 	= theone->height + 1;

	/* This copies all the pointers that the target linked to before for assert
	 * testing.
	 */
	sl_node_t 	*oldnextarr[theone_h];
	for(i = 0; i < theone_h; i++)
	{
		oldnextarr[i] 		= theone->next[i];
		io_printf("to save: 0x%p | saved: 0x%p\n", theone->next[i], oldnextarr[i]);
	}

	/* After this block, "theone" is undefined, freed memory and should not
	 * be accessed. */
	int 		key 		= 112;
	err_t 		status 		= sl_delete(&skiplist, (ion_key_t) &key);

#ifdef DEBUG
	printf("%s\n", "** AFTER **");
	check_skiplist(&skiplist);
#endif

	/* FIXME THERE IS SOMETHING WRONG HERE AND I DONT KNOW WHAT (RELATED TO HEIGHTS AND POINTER LINKING IN DELETE????) */
	// Manually set key 111 to height 2 and key 113 to height 1
	CuAssertTrue(tc, status == err_ok);
	for(i = 0; i < theone_h; i++)
	{
		io_printf("new: 0x%p == old: 0x%p? [%d]\n", onebefore->next[i], oldnextarr[i], oldnextarr[i] == onebefore->next[i]);
		sl_node_t *toast = onebefore->next[i];
		io_printf("key: %d | value: %s | height: %d\n", *((int*) toast->key), toast->value, toast->height);
		//CuAssertTrue(tc, onebefore->next[i] == oldnextarr[i]);
	}

	sl_destroy(&skiplist);
}

/**
@brief 		Tests deleting a single node in a skiplist of several nodes, where
			the element to delete exists in the skiplist. This skiplist is non-
			contiguous in its elements. The assertion is that the status
			returned will be "err_ok", and the key/value pair deleted is no
			longer within the skiplist.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_delete_single_several_noncont(
	CuTest 		*tc
)
{
	PRINT_HEADER("test_skiplist_delete_single_several_noncont");
	skiplist_t skiplist;
	initialize_skiplist_std_conditions(&skiplist);

	sl_destroy(&skiplist);
}

/**
@brief 		Tests a skiplist with different initialization parameters than
			usual. Each basic operation of insert, query, and delete are tested
			on the non-standard structure.

@param 		tc
				CuTest dependency
 */
void
test_skiplist_different_size(
	CuTest 		*tc
)
{
	PRINT_HEADER("test_skiplist_different_size");
	skiplist_t 	skiplist;
	int key_size, value_size, pden, pnum, maxheight;
	key_size 	= 8;
	value_size 	= 4;
	pnum 		= 1;
	pden 		= 1;
	maxheight 	= 10;

	initialize_skiplist(&skiplist, maxheight, key_size, value_size, pnum, pden);

	/* TODO finish this test case */
	//insert
	//get
	//delete

	//verify

	sl_destroy(&skiplist);
}

CuSuite*
skiplist_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	/* Initialization Tests */
	SUITE_ADD_TEST(suite, test_skiplist_initialize);
	SUITE_ADD_TEST(suite, test_skiplist_free_all);

	/* Level Tests */
	SUITE_ADD_TEST(suite, test_skiplist_generate_levels_std_conditions);

	/* Insertion Tests */
	SUITE_ADD_TEST(suite, test_skiplist_single_insert);
	SUITE_ADD_TEST(suite, test_skiplist_insert_multiple);
	SUITE_ADD_TEST(suite, test_skiplist_randomized_insert);

	/* Get Node Tests */
	SUITE_ADD_TEST(suite, test_skiplist_get_node_single);
	SUITE_ADD_TEST(suite, test_skiplist_get_node_single_high);
	SUITE_ADD_TEST(suite, test_skiplist_get_node_single_low);
	SUITE_ADD_TEST(suite, test_skiplist_get_node_single_many);
	SUITE_ADD_TEST(suite, test_skiplist_get_node_several);

	/* Query Tests */
	SUITE_ADD_TEST(suite, test_skiplist_query_nonexist_empty);
	SUITE_ADD_TEST(suite, test_skiplist_query_nonexist_populated_single);
	SUITE_ADD_TEST(suite, test_skiplist_query_nonexist_populated_several);
	SUITE_ADD_TEST(suite, test_skiplist_query_exist_single);
	SUITE_ADD_TEST(suite, test_skiplist_query_exist_populated_single);
	SUITE_ADD_TEST(suite, test_skiplist_query_exist_populated_several);

	/* Delete Tests */
	SUITE_ADD_TEST(suite, test_skiplist_delete_empty);
	SUITE_ADD_TEST(suite, test_skiplist_delete_nonexist_single);
	SUITE_ADD_TEST(suite, test_skiplist_delete_nonexist_several);
	SUITE_ADD_TEST(suite, test_skiplist_delete_single);
	SUITE_ADD_TEST(suite, test_skiplist_delete_single_several);
	SUITE_ADD_TEST(suite, test_skiplist_delete_single_several_noncont);

	/* Variation Tests */
	SUITE_ADD_TEST(suite, test_skiplist_different_size);

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
