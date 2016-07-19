/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Unit test for Skiplist data store
*/
/******************************************************************************/

#include "test_skip_list.h"

/**
@brief	  Helper function that creates a skiplist based on the given
			parameters.

@param	  skiplist
				Skiplist to initialize
@param	  key_type
				Type of key used
@param	  compare
				Function pointer to a comparison method used by the skiplist
@param	  maxheight
				Maximum height of the skiplist
@param	  key_size
				Size of key in bytes allowed
@param	  value_size
				Size of value in bytes allowed
@param	  pnum
				Probability numerator
@param	  pden
				Probability denominator
 */
void
initialize_skiplist(
	skiplist_t					*skiplist,
	key_type_t					key_type,
	ion_dictionary_compare_t	compare,
	int							maxheight,
	int							key_size,
	int							value_size,
	int							pnum,
	int							pden
) {
	sl_initialize(skiplist, key_type, key_size, value_size, maxheight, pnum, pden);
	skiplist->super.compare = compare;
}

/**
@brief	  Secondary helper function that creates a skiplist using the
			standard conditions variable. Simplifies the creation of a skiplist.

@param	  skiplist
				Skiplist to initialize
 */
void
initialize_skiplist_std_conditions(
	skiplist_t *skiplist
) {
	int							key_size, value_size, pden, pnum, maxheight;
	key_type_t					key_type;
	ion_dictionary_compare_t	compare;

	key_type	= key_type_numeric_signed;
	compare		= dictionary_compare_signed_value;
	key_size	= sizeof(int);
	value_size	= 10;
	pnum		= 1;
	pden		= 4;
	maxheight	= 7;

	initialize_skiplist(skiplist, key_type, compare, maxheight, key_size, value_size, pnum, pden);
}

/**
@brief	  Tests creation of the skiplist.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_initialize(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	int							key_size, value_size, pden, pnum, maxheight;
	key_type_t					key_type;
	ion_dictionary_compare_t	compare;
	skiplist_t					skiplist;

	key_type	= key_type_numeric_signed;
	compare		= dictionary_compare_signed_value;
	key_size	= sizeof(int);
	value_size	= 10;
	pnum		= 1;
	pden		= 4;
	maxheight	= 7;

	initialize_skiplist(&skiplist, key_type, compare, maxheight, key_size, value_size, pnum, pden);

#if DEBUG
	print_skiplist(&skip_list);
#endif

	ion_key_size_t		t_key_size		= skiplist.super.record.key_size;
	ion_value_size_t	t_value_size	= skiplist.super.record.value_size;

	PLANCK_UNIT_ASSERT_TRUE(tc, skiplist.super.key_type == key_type_numeric_signed);
	PLANCK_UNIT_ASSERT_TRUE(tc, skiplist.super.compare == dictionary_compare_signed_value);
	PLANCK_UNIT_ASSERT_TRUE(tc, t_key_size == key_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, t_value_size == value_size);
	PLANCK_UNIT_ASSERT_TRUE(tc, skiplist.maxheight == maxheight);
	PLANCK_UNIT_ASSERT_TRUE(tc, skiplist.pnum == pnum);
	PLANCK_UNIT_ASSERT_TRUE(tc, skiplist.pden == pden);

	sl_destroy(&skiplist);
}

/**
@brief	  Tests if the Skiplist is properly destroyed.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_free_all(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	sl_destroy(&skiplist);

	PLANCK_UNIT_ASSERT_TRUE(tc, skiplist.head == NULL);
}

/**
@brief	  Tests a single insert into the skiplist.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_single_insert(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	int			key = 6;
	ion_byte_t	value[10];

	strcpy((char *) value, "single.");

	ion_status_t status = sl_insert(&skiplist, (ion_key_t) &key, value);

#if DEBUG
	print_skiplist(&skip_list);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	PLANCK_UNIT_ASSERT_TRUE(tc, *((int *) skiplist.head->next[0]->key) == 6);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) skiplist.head->next[0]->value, "single.");

	sl_destroy(&skiplist);
}

/**
@brief	  Tests several insertions into the skiplist. The skiplist structure
			is accessed directly in order to verify the insertion has properly
			occurred.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_insert_multiple(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	ion_byte_t strs[5][6] = {
		"one", "two", "three", "four", "five"
	};

	int i;

	for (i = 1; i <= 5; i++) {
		ion_status_t status = sl_insert(&skiplist, (ion_key_t) &i, strs[i - 1]);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	print_skiplist(&skip_list);
#endif

	sl_node_t *cursor;

	cursor = skiplist.head->next[0];
	PLANCK_UNIT_ASSERT_TRUE(tc, *((int *) cursor->key) == 1);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) cursor->value, "one");

	cursor = skiplist.head->next[0]->next[0];
	PLANCK_UNIT_ASSERT_TRUE(tc, *((int *) cursor->key) == 2);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) cursor->value, "two");

	cursor = skiplist.head->next[0]->next[0]->next[0];
	PLANCK_UNIT_ASSERT_TRUE(tc, *((int *) cursor->key) == 3);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) cursor->value, "three");

	cursor = skiplist.head->next[0]->next[0]->next[0]->next[0];
	PLANCK_UNIT_ASSERT_TRUE(tc, *((int *) cursor->key) == 4);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) cursor->value, "four");

	cursor = skiplist.head->next[0]->next[0]->next[0]->next[0]->next[0];
	PLANCK_UNIT_ASSERT_TRUE(tc, *((int *) cursor->key) == 5);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) cursor->value, "five");

	sl_destroy(&skiplist);
}

/**
@brief	  Tests a mixed order insert. The structure is traversed and tested
			to confirm that (p + 1) >= ( p ) in all cases, where p is the key
			of the current node.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_randomized_insert(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	ion_byte_t str[10];

	strcpy((char *) str, "random");

	int i;

	for (i = 0; i < 100; i++) {
		int key				= rand() % 101;

		ion_status_t status = sl_insert(&skiplist, (ion_key_t) &key, str);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	print_skiplist(&skip_list);
#endif

	sl_node_t *cursor = skiplist.head;

	while (cursor->next[0]->next[0] != NULL) {
		int now		= *((int *) cursor->next[0]->key);
		int next	= *((int *) cursor->next[0]->next[0]->key);

		PLANCK_UNIT_ASSERT_TRUE(tc, next >= now);

		cursor = cursor->next[0];
	}

	sl_destroy(&skiplist);
}

/**
@brief	  Tests node search on a single node in a skiplist with only one node.

@details	The key searched for is exact. The test compares the given node key
			and value information with the information inserted into the
			skiplist. The test passes if they are the same.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_get_node_single(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	ion_byte_t str[10];

	strcpy((char *) str, "find this");

	int key				= 3;

	ion_status_t status = sl_insert(&skiplist, (ion_key_t) &key, str);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

	int			search	= 3;
	sl_node_t	*node	= sl_find_node(&skiplist, (ion_key_t) &search);

	PLANCK_UNIT_ASSERT_TRUE(tc, node != NULL);
	PLANCK_UNIT_ASSERT_TRUE(tc, *((int *) node->key) == key);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) str, (char *) node->value);

	sl_destroy(&skiplist);
}

/**
@brief	  Tests node search on a single node in a skiplist with only one node.

@details	The key searched for is higher than the inserted key. The test
			compares the given node key and value information with the
			information inserted into the skiplist. The test passes if they are
			the same.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_get_node_single_high(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	ion_byte_t str[10];

	strcpy((char *) str, "find this");

	int key				= 3;

	ion_status_t status = sl_insert(&skiplist, (ion_key_t) &key, str);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

#if DEBUG
	print_skiplist(&skip_list);
#endif

	int			search	= 10;
	sl_node_t	*node	= sl_find_node(&skiplist, (ion_key_t) &search);

	PLANCK_UNIT_ASSERT_TRUE(tc, *((int *) node->key) == key);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) str, (char *) node->value);

	sl_destroy(&skiplist);
}

/**
@brief	  Tests node search on a single node in a skiplist with only one node.

@details	The key searched for is lower than the inserted key. The test
			compares the given node key and value information with the
			information inserted into the skiplist. The test passes if the node
			returned is the same as the head node. (Since there's only one node,
			the only node that can be smaller than the one insert is the head
			node.)
@param	  tc
				CuTest dependency
 */
void
test_skiplist_get_node_single_low(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	ion_byte_t str[10];

	strcpy((char *) str, "find this");

	int key				= 3;

	ion_status_t status = sl_insert(&skiplist, (ion_key_t) &key, str);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

#if DEBUG
	print_skiplist(&skip_list);
#endif

	int			search	= 2;
	sl_node_t	*node	= sl_find_node(&skiplist, (ion_key_t) &search);

	PLANCK_UNIT_ASSERT_TRUE(tc, node == skiplist.head);

	sl_destroy(&skiplist);
}

/**
@brief	  Tests node search on a single node in a skiplist with several nodes.

@details	Tests node search on a single node in a skiplist with several nodes.
			The key searched for is exact. The test compares the given node key
			and value information with the information inserted into the
			skiplist. The test passes if they are the same.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_get_node_single_many(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	ion_byte_t str[10];

	strcpy((char *) str, "find this");

	int key				= 25;

	ion_status_t status = sl_insert(&skiplist, (ion_key_t) &key, str);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

	ion_byte_t junk[10];

	strcpy((char *) junk, "big junk");

	int i;

	for (i = 0; i < 100; i++) {
		if ((i >= 10) && (i <= 35)) {
			/* Create a gap �10 from the target key */
			continue;
		}

		status = sl_insert(&skiplist, (ion_key_t) &i, junk);
		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	print_skiplist(&skip_list);
#endif

	int			search	= 25;
	sl_node_t	*node	= sl_find_node(&skiplist, (ion_key_t) &search);

	PLANCK_UNIT_ASSERT_TRUE(tc, *((int *) node->key) == key);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) str, (char *) node->value);

	sl_destroy(&skiplist);
}

/**
@brief	  Randomly generates 50 random key value pairs and inserts them into
			the skiplist. The nodes are then recalled and the key/value pairs
			are compared to the original inserted ones for accuracy.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_get_node_several(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

#if DEBUG
	/* If debugging, use a static seed */
	srand(0xDEADBEEF);
#endif

	int			targets[50];
	ion_byte_t	buffer[10];
	int			i;

	for (i = 0; i < 50; i++) {
		int key = rand() % 1000;

		targets[i] = key;
		sprintf((char *) buffer, "TEST %d", key);

		ion_status_t status = sl_insert(&skiplist, (ion_key_t) &key, buffer);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	print_skiplist(&skip_list);
#endif

	for (i = 0; i < 50; i++) {
		int			key		= targets[i];
		sl_node_t	*node	= sl_find_node(&skiplist, (ion_key_t) &key);

		sprintf((char *) buffer, "TEST %d", key);
		PLANCK_UNIT_ASSERT_TRUE(tc, *((int *) node->key) == key);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) node->value, (char *) buffer);
	}

	sl_destroy(&skiplist);
}

/**
@brief	  Tests querying on an empty skiplist. assertion is that the status
			should return as "err_item_not_found", and that the value pointer
			should not be allocated and will be set to null.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_query_nonexist_empty(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	int			key			= 3;
	ion_byte_t	value[10]	= "NULL";

	ion_status_t status		= sl_query(&skiplist, (ion_key_t) &key, value);

#if DEBUG
	print_skiplist(&skip_list);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, "NULL");

	sl_destroy(&skiplist);
}

/**
@brief	  Tests querying on a skiplist with one element, but for a key that
			doesn't exist within the skiplist. assertion is that the status
			should return as "err_item_not_found", and that the value pointer
			be initialized to null.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_query_nonexist_populated_single(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	int			test_key = 23;
	ion_byte_t	test_value[10];

	strcpy((char *) test_value, "I am test");

	ion_status_t status = sl_insert(&skiplist, (ion_key_t) &test_key, test_value);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

#if DEBUG
	print_skiplist(&skip_list);
#endif

	int			key			= 10;
	ion_byte_t	value[10]	= "NULL";

	status = sl_query(&skiplist, (ion_key_t) &key, value);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, "NULL");

	sl_destroy(&skiplist);
}

/**
@brief	  Tests querying on a skiplist with several elements, but for a key
			that doesn't exist within the skiplist. assertion is that the
			status should return as "err_item_not_found", and that the value
			pointer be initialized to null.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_query_nonexist_populated_several(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	int			test_key = 46;
	ion_byte_t	test_value[10];

	int i;

	for (i = 0; i < 32; i++) {
		sprintf((char *) test_value, "I am: %d", test_key);

		ion_status_t status = sl_insert(&skiplist, (ion_key_t) &test_key, test_value);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
		test_key--;
	}

#if DEBUG
	print_skiplist(&skip_list);
#endif

	int			key			= 10;
	ion_byte_t	value[10]	= "NULL";

	ion_status_t status		= sl_query(&skiplist, (ion_key_t) &key, value);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, "NULL");

	sl_destroy(&skiplist);
}

/**
@brief	  Tests querying on a skiplist with a single element, for a key that
			does exist within the skiplist. assertion is that the status should
			return as "err_ok", and that the value be initialized to the same
			value as stored at the specified key.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_query_exist_single(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	int			test_key = 11;
	ion_byte_t	test_value[10];

	strcpy((char *) test_value, "Find me!");

	ion_status_t status = sl_insert(&skiplist, (ion_key_t) &test_key, test_value);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

#if DEBUG
	print_skiplist(&skip_list);
#endif

	int			key = 11;
	ion_byte_t	value[10];

	status = sl_query(&skiplist, (ion_key_t) &key, value);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, "Find me!");

	sl_destroy(&skiplist);
}

/**
@brief	  Tests querying on a skiplist with several elements, for a key that
			exists within the skiplist. assertion is that the status should
			return as "err_ok", and that the value be initialized to the same
			value stored at the specified key.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_query_exist_populated_single(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	ion_byte_t test_value[10];

	int i;

	for (i = 0; i < 100; i += 2) {
		sprintf((char *) test_value, "Find %d", i);

		ion_status_t status = sl_insert(&skiplist, (ion_key_t) &i, test_value);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	print_skiplist(&skip_list);
#endif

	int			key		= 24;
	ion_byte_t	value[10];

	ion_status_t status = sl_query(&skiplist, (ion_key_t) &key, value);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, "Find 24");

	sl_destroy(&skiplist);
}

/**
@brief	  Tests querying on a skiplist with several elements, for a key that
			exists within the skiplist. assertion is that the status should
			return as "err_ok", and that the value be initialized to the same
			value stored at the specified key.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_query_exist_populated_several(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	ion_byte_t test_value[10];

	int i;

	for (i = 0; i < 100; i++) {
		sprintf((char *) test_value, "Find %d", i);

		ion_status_t status = sl_insert(&skiplist, (ion_key_t) &i, test_value);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	print_skiplist(&skip_list);
#endif

	char		find_value[10];
	ion_byte_t	value[10];

	for (i = 0; i < 100; i++) {
		sprintf(find_value, "Find %d", i);

		ion_status_t status = sl_query(&skiplist, (ion_key_t) &i, value);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, find_value);
	}

	sl_destroy(&skiplist);
}

/**
@brief	  Tests a deletion from a skiplist that's empty. The assertion is
			that the deletion will fail, and return "err_item_not_found". No
			modifications should be made to the structure.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_delete_empty(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	int				key		= 3;
	ion_status_t	status	= sl_delete(&skiplist, (ion_key_t) &key);

#if DEBUG
	print_skiplist(&skip_list);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);

	sl_destroy(&skiplist);
}

/**
@brief	  Tests a deletion from a skiplist that has one element, and the
			deleted element is not the one within the skiplist. The assertion
			is that the returned status is "err_item_not_found", and that no
			modification is to be made to the data structure.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_delete_nonexist_single(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	int			key = 16;
	ion_byte_t	value[10];

	strcpy((char *) value, "Delete me");

	ion_status_t status = sl_insert(&skiplist, (ion_key_t) &key, value);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

	int fake_key = 33;

	status = sl_delete(&skiplist, (ion_key_t) &fake_key);

#if DEBUG
	print_skiplist(&skip_list);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);

	sl_destroy(&skiplist);
}

/**
@brief	  Tests a deletion from a skiplist that has many elements, and the
			deleted element is not the one within the skiplist. The assertion
			is that the returned status is "err_item_not_found", and that no
			modification is to be made to the data structure.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_delete_nonexist_several(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	int			key = 16;
	ion_byte_t	value[10];

	strcpy((char *) value, "Delete me");

	int i;

	for (i = 0; i < 10; i++) {
		ion_status_t status = sl_insert(&skiplist, (ion_key_t) &key, value);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

		key += 3;
	}

	int				fake_key	= 20;
	ion_status_t	status		= sl_delete(&skiplist, (ion_key_t) &fake_key);

#if DEBUG
	print_skiplist(&skip_list);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, err_item_not_found == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0 == status.count);

	sl_destroy(&skiplist);
}

/**
@brief	  Tests deletion in a single element skiplist, where the deleted
			element is the one that exists within the skiplist. The assertion
			is that the status returned will be "err_ok", and the key/value pair
			deleted is no longer within the skiplist.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_delete_single(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	int			key = 97;
	ion_byte_t	value[10];

	strcpy((char *) value, "Special K");

	ion_status_t status = sl_insert(&skiplist, (ion_key_t) &key, value);

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

#if DEBUG
	printf("%s\n", "** BEFORE **");
	print_skiplist(&skip_list);
#endif

	status = sl_delete(&skiplist, (ion_key_t) &key);

#if DEBUG
	printf("%s\n", "** AFTER **");
	print_skiplist(&skip_list);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	PLANCK_UNIT_ASSERT_TRUE(tc, skiplist.head->next[0] == NULL);

	sl_destroy(&skiplist);
}

/**
@brief	  Tests deleting a single node in a skiplist of several nodes, where
			the element to delete exists in the skiplist. The assertion is that
			the status returned will be "err_ok", and the key/value pair deleted
			is no longer within the skiplist.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_delete_single_several(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	ion_byte_t value[10];

	strcpy((char *) value, "Poof me!");

	int i;

	for (i = 101; i < 120; i++) {
		ion_status_t status = sl_insert(&skiplist, (ion_key_t) &i, value);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	printf("%s\n", "** BEFORE **");
	print_skiplist(&skip_list);
#endif

	sl_node_t	*onebefore	= sl_find_node(&skiplist, IONIZE(111, int));
	sl_node_t	*theone		= sl_find_node(&skiplist, IONIZE(112, int));
	sl_level_t	theone_h	= theone->height + 1;
	sl_level_t	onebefore_h = onebefore->height + 1;

	/* This copies all the pointers that the target linked to before for assert
	 * testing.
	 */
	sl_node_t *oldnextarr[theone_h];

	for (i = 0; i < theone_h; i++) {
		oldnextarr[i] = theone->next[i];
	}

	/* After this block, "theone" is undefined, freed memory and should not
	 * be accessed. */
	int				key		= 112;
	ion_status_t	status	= sl_delete(&skiplist, (ion_key_t) &key);

#if DEBUG
	printf("%s\n", "** AFTER **");
	print_skiplist(&skip_list);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

	for (i = 0; i < (theone_h >= onebefore_h ? onebefore_h : theone_h); i++) {
		PLANCK_UNIT_ASSERT_TRUE(tc, onebefore->next[i] == oldnextarr[i]);
	}

	sl_destroy(&skiplist);
}

/**
@brief	  Tests deleting a single node in a skiplist of several nodes, where
			the element to delete exists in the skiplist. This skiplist is non-
			contiguous in its elements. The assertion is that the status
			returned will be "err_ok", and the key/value pair deleted is no
			longer within the skiplist.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_delete_single_several_noncont(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	ion_byte_t value[10];

	strcpy((char *) value, "Bop me!");

	int i;

	for (i = 230; i < 300; i += 5) {
		ion_status_t status = sl_insert(&skiplist, (ion_key_t) &i, value);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	printf("%s\n", "** BEFORE **");
	print_skiplist(&skip_list);
#endif

	sl_node_t	*onebefore	= sl_find_node(&skiplist, IONIZE(235, int));
	sl_node_t	*theone		= sl_find_node(&skiplist, IONIZE(240, int));
	sl_level_t	theone_h	= theone->height + 1;
	sl_level_t	onebefore_h = onebefore->height + 1;

	/* This copies all the pointers that the target linked to before for assert
	 * testing.
	 */
	sl_node_t *oldnextarr[theone_h];

	for (i = 0; i < theone_h; i++) {
		oldnextarr[i] = theone->next[i];
	}

	/* After this block, "theone" is undefined, freed memory and should not
	 * be accessed. */
	int				key		= 240;
	ion_status_t	status	= sl_delete(&skiplist, (ion_key_t) &key);

#if DEBUG
	printf("%s\n", "** AFTER **");
	print_skiplist(&skip_list);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

	for (i = 0; i < (theone_h >= onebefore_h ? onebefore_h : theone_h); i++) {
		PLANCK_UNIT_ASSERT_TRUE(tc, onebefore->next[i] == oldnextarr[i]);
	}

	sl_destroy(&skiplist);
}

/**
@brief	  Tests the deletion of all elements from a skiplist that has several.
			The assertion is that every deletion will return an "err_ok" status,
			and that at the end of the operations the skiplist must be empty.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_delete_several_all(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	ion_byte_t value[10];

	strcpy((char *) value, "Wipe");

	int i;

	for (i = 9; i < 99; i += 3) {
		ion_status_t status = sl_insert(&skiplist, (ion_key_t) &i, value);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	printf("%s\n", "** BEFORE **");
	print_skiplist(&skip_list);
#endif

	for (i = 9; i < 99; i += 3) {
		ion_status_t status = sl_delete(&skiplist, (ion_key_t) &i);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	printf("%s\n", "** AFTER **");
	print_skiplist(&skip_list);
#endif

	sl_node_t	*cursor = skiplist.head;
	sl_level_t	h;

	for (h = cursor->height; h >= 0; h--) {
		PLANCK_UNIT_ASSERT_TRUE(tc, cursor->next[h] == NULL);
	}

	sl_destroy(&skiplist);
}

/**
@brief	  Tests the updating of a node in an empty skiplist. The assertion is
			that the update will instead insert the node.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_update_single_nonexist(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	ion_status_t status = sl_update(&skiplist, IONIZE(72, int), (ion_value_t) (char *) { "test val" });

#if DEBUG
	print_skiplist(&skip_list);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	PLANCK_UNIT_ASSERT_TRUE(tc, *(int *) skiplist.head->next[0]->key == 72);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) skiplist.head->next[0]->value, "test val");

	sl_destroy(&skiplist);
}

/**
@brief	  Tests the updating of a node in a skiplist with one element, but
			not with the same key that is being inserted. The assertion is
			that the update will instead insert the node.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_update_single_nonexist_nonempty(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	ion_status_t status = sl_insert(&skiplist, IONIZE(99, int), (ion_value_t) (char *) { "not val" });

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

#if DEBUG
	printf("%s\n", "** BEFORE **");
	print_skiplist(&skip_list);
#endif

	status = sl_update(&skiplist, IONIZE(13, int), (ion_value_t) (char *) { "test val" });

#if DEBUG
	printf("%s\n", "** AFTER **");
	print_skiplist(&skip_list);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	PLANCK_UNIT_ASSERT_TRUE(tc, *(int *) skiplist.head->next[0]->key == 13);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) skiplist.head->next[0]->value, "test val");

	sl_destroy(&skiplist);
}

/**
@brief	  Tests the updating of a node in a skiplist with many elements, but
			not with the same key that is being inserted. The assertion is
			that the update will instead insert the node.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_update_many_nonexist_nonempty(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	int i;

	for (i = 30; i < 40; i += 2) {
		ion_status_t status = sl_insert(&skiplist, (ion_key_t) &i, (ion_value_t) (char *) { "not it!" });

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	printf("%s\n", "** BEFORE **");
	print_skiplist(&skip_list);
#endif

	ion_status_t status = sl_update(&skiplist, IONIZE(45, int), (ion_value_t) (char *) { "test val" });

#if DEBUG
	printf("%s\n", "** AFTER **");
	print_skiplist(&skip_list);
#endif

	sl_node_t *cursor = sl_find_node(&skiplist, IONIZE(38, int));

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	PLANCK_UNIT_ASSERT_TRUE(tc, *(int *) cursor->next[0]->key == 45);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) cursor->next[0]->value, "test val");

	sl_destroy(&skiplist);
}

/**
@brief	  Tests the updating of a node in a skiplist with one element, where
			the one element is the node we attempt to update. The assertion is
			that the update will return the status of "err_ok", and the changes
			will be reflected within the node.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_update_single_exist(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	ion_status_t status = sl_insert(&skiplist, IONIZE(45, int), (ion_value_t) (char *) { "old val" });

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

#if DEBUG
	printf("%s\n", "** BEFORE **");
	print_skiplist(&skip_list);
#endif

	status = sl_update(&skiplist, IONIZE(45, int), (ion_value_t) (char *) { "new val" });

#if DEBUG
	printf("%s\n", "** AFTER **");
	print_skiplist(&skip_list);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	PLANCK_UNIT_ASSERT_TRUE(tc, *(int *) skiplist.head->next[0]->key == 45);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) skiplist.head->next[0]->value, "new val");

	sl_destroy(&skiplist);
}

/**
@brief	  Tests the updating of a node in a skiplist with many elements, where
			the one to be updated exists in the skiplist. The assertion is
			that the update will return the status of "err_ok", and the changes
			will be reflected within the targeted node.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_update_single_many_exist(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	int i;

	for (i = 20; i < 46; i += 2) {
		ion_status_t status = sl_insert(&skiplist, (ion_key_t) &i, (ion_value_t) (char *) { "MATH" });

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	printf("%s\n", "** BEFORE **");
	print_skiplist(&skip_list);
#endif

	ion_status_t status = sl_update(&skiplist, IONIZE(30, int), (ion_value_t) (char *) { "COSC" });

#if DEBUG
	printf("%s\n", "** AFTER **");
	print_skiplist(&skip_list);
#endif

	sl_node_t *cursor = sl_find_node(&skiplist, IONIZE(30, int));

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	PLANCK_UNIT_ASSERT_TRUE(tc, *(int *) cursor->key == 30);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) cursor->value, "COSC");

	sl_destroy(&skiplist);
}

/**
@brief	  Tests the updating of nodes in a skiplist with many elements, where
			all the nodes are to be updated. The assertion is that the update
			will return the status of "err_ok", and the changes will be
			reflected within all nodes.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_update_several_many_exist(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	int i;

	for (i = 60; i < 99; i += 3) {
		ion_status_t status = sl_insert(&skiplist, (ion_key_t) &i, (ion_value_t) (char *) { "TEST" });

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	printf("%s\n", "** BEFORE **");
	print_skiplist(&skip_list);
#endif

	for (i = 60; i < 99; i += 3) {
		ion_status_t status = sl_update(&skiplist, (ion_key_t) &i, (ion_value_t) (char *) { "VALUE" });

		sl_node_t *cursor	= sl_find_node(&skiplist, (ion_key_t) &i);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
		PLANCK_UNIT_ASSERT_TRUE(tc, *(int *) cursor->key == i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) cursor->value, "VALUE");
	}

#if DEBUG
	printf("%s\n", "** AFTER **");
	print_skiplist(&skip_list);
#endif

	sl_destroy(&skiplist);
}

/**
@brief	  Tests an update in a skiplist containing several elements, all of
			the same key. The assertion is that all elements should be updated,
			with none of the original values remaining in the skiplist.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_update_several_same_key(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	int i;

	for (i = 0; i < 100; i++) {
		ion_status_t status = sl_insert(&skiplist, IONIZE(64, int), (ion_value_t) (char *) { "samez U" });

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	printf("%s\n", "** INSERT **");
	print_skiplist(&skip_list);
#endif

	ion_status_t status = sl_update(&skiplist, IONIZE(64, int), (ion_value_t) (char *) { "new same" });

#if DEBUG
	printf("%s\n", "** UPDATE **");
	print_skiplist(&skip_list);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 100 == status.count);

	sl_node_t *cursor = skiplist.head;

	for (i = 0; i < 100; i++) {
		cursor = cursor->next[0];
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, "new same", (char *) cursor->value);
	}

	sl_destroy(&skiplist);
}

/**
@brief	  Tests an update in a skiplist containing several elements, some of
			which with the same key. The assertion is that all elements of that
			specific key be updated, with none of the original values remaining.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_update_several_same_key_in_mix(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	sl_insert(&skiplist, IONIZE(32, int), (ion_value_t) (char *) { "samez U" });
	sl_insert(&skiplist, IONIZE(33, int), (ion_value_t) (char *) { "samez U" });
	sl_insert(&skiplist, IONIZE(35, int), (ion_value_t) (char *) { "samez U" });

	int i;

	for (i = 0; i < 100; i++) {
		sl_insert(&skiplist, IONIZE(55, int), (ion_value_t) (char *) { "samez  U" });
	}

	sl_insert(&skiplist, IONIZE(100, int), (ion_value_t) (char *) { "samez U" });
	sl_insert(&skiplist, IONIZE(101, int), (ion_value_t) (char *) { "samez U" });

#if DEBUG
	printf("%s\n", "** INSERT **");
	print_skiplist(&skip_list);
#endif

	ion_status_t status = sl_update(&skiplist, IONIZE(55, int), (ion_value_t) (char *) { "new same" });

#if DEBUG
	printf("%s\n", "** UPDATE **");
	print_skiplist(&skip_list);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 100 == status.count);

	sl_node_t *find = sl_find_node(&skiplist, IONIZE(55, int));

	for (i = 0; i < 100; i++) {
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) find->value, (char *) { "new same" });
		find = find->next[0];
	}

	sl_destroy(&skiplist);
}

/**
@brief	  Tests a deletion of a skiplist with one element, and then test a
			reinsertion of a different key/value pair into the same skiplist.
			The assertion is that the insertion should work OK with no errors.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_delete_then_insert_single(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	ion_status_t status = sl_insert(&skiplist, IONIZE(66, int), (ion_value_t) (char *) { "toaster" });

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

#if DEBUG
	printf("%s\n", "** INSERT **");
	print_skiplist(&skip_list);
#endif

	status = sl_delete(&skiplist, IONIZE(66, int));

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

#if DEBUG
	printf("%s\n", "** DELETE **");
	print_skiplist(&skip_list);
#endif

	status = sl_insert(&skiplist, IONIZE(365, int), (ion_value_t) (char *) { "potato" });

#if DEBUG
	printf("%s\n", "** REINSERT **");
	print_skiplist(&skip_list);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	PLANCK_UNIT_ASSERT_TRUE(tc, skiplist.head->next[0] != NULL);
	PLANCK_UNIT_ASSERT_TRUE(tc, *(int *) skiplist.head->next[0]->key == 365);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) skiplist.head->next[0]->value, "potato");

	sl_destroy(&skiplist);
}

/**
@brief	  Tests a deletion of a skiplist with several elements, and then test
			a reinsertion of several different key/value pairs into the same
			skiplist. The assertion is that the insertion should work OK with
			no errors.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_delete_then_insert_several(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	int i;

	for (i = 0; i < 50; i++) {
		ion_status_t status = sl_insert(&skiplist, (ion_key_t) &i, (ion_value_t) (char *) { "cake" });

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	printf("%s\n", "** INSERT **");
	print_skiplist(&skip_list);
#endif

	for (i = 0; i < 50; i++) {
		ion_status_t status = sl_delete(&skiplist, (ion_key_t) &i);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	printf("%s\n", "** DELETE **");
	print_skiplist(&skip_list);
#endif

	for (i = 50; i < 100; i++) {
		ion_status_t status = sl_insert(&skiplist, (ion_key_t) &i, (ion_value_t) (char *) { "pie" });

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	printf("%s\n", "** REINSERT **");
	print_skiplist(&skip_list);
#endif

	sl_node_t *cursor = skiplist.head;

	for (i = 50; i < 100; i++) {
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) cursor->next[0]->value, "pie");
		cursor = cursor->next[0];
	}

	sl_destroy(&skiplist);
}

/**
@brief	  Tests a deletion in a skiplist containing several elements, all of
			the same key. The assertion is that all elements should be deleted,
			with nothing remaining in the skiplist.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_delete_several_same_key(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	int i;

	for (i = 0; i < 100; i++) {
		ion_status_t status = sl_insert(&skiplist, IONIZE(64, int), (ion_value_t) (char *) { "samez" });

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	printf("%s\n", "** INSERT **");
	print_skiplist(&skip_list);
#endif

	ion_status_t status = sl_delete(&skiplist, IONIZE(64, int));

#if DEBUG
	printf("%s\n", "** DELETE **");
	print_skiplist(&skip_list);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 100 == status.count);

	int h;

	for (h = skiplist.head->height; h >= 0; h--) {
		PLANCK_UNIT_ASSERT_TRUE(tc, skiplist.head->next[h] == NULL);
	}

	sl_destroy(&skiplist);
}

/**
@brief	  Tests a deletion in a skiplist containing several elements, some of
			which with the same key. The assertion is that all elements of the
			specific key be deleted, with only the other elements remaining.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_delete_several_same_key_in_mix(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	sl_insert(&skiplist, IONIZE(32, int), (ion_value_t) (char *) { "samez" });
	sl_insert(&skiplist, IONIZE(33, int), (ion_value_t) (char *) { "samez" });
	sl_insert(&skiplist, IONIZE(35, int), (ion_value_t) (char *) { "samez" });

	int i;

	for (i = 0; i < 100; i++) {
		sl_insert(&skiplist, IONIZE(55, int), (ion_value_t) (char *) { "samez" });
	}

	sl_insert(&skiplist, IONIZE(100, int), (ion_value_t) (char *) { "samez" });
	sl_insert(&skiplist, IONIZE(101, int), (ion_value_t) (char *) { "samez" });

#if DEBUG
	printf("%s\n", "** INSERT **");
	print_skiplist(&skip_list);
#endif

	ion_status_t status = sl_delete(&skiplist, IONIZE(55, int));

#if DEBUG
	printf("%s\n", "** DELETE **");
	print_skiplist(&skip_list);
#endif

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 100 == status.count);

	sl_node_t *find = sl_find_node(&skiplist, IONIZE(55, int));

	PLANCK_UNIT_ASSERT_TRUE(tc, skiplist.super.compare(find->key, IONIZE(55, int), skiplist.super.record.key_size) != 0);

	sl_destroy(&skiplist);
}

/**
@brief	  Tests a skiplist with different initialization parameters than
			usual. Each basic operation of insert, query, and delete are tested
			on the non-standard structure.

@param	  tc
				CuTest dependency
 */
void
test_skiplist_different_size(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t					skiplist;
	int							key_size, value_size, pden, pnum, maxheight;
	key_type_t					key_type;
	ion_dictionary_compare_t	compare;

	key_type	= key_type_numeric_unsigned;
	compare		= dictionary_compare_unsigned_value;
	key_size	= sizeof(long long);
	value_size	= 4;
	pnum		= 1;
	pden		= 1;
	maxheight	= 10;

	initialize_skiplist(&skiplist, key_type, compare, maxheight, key_size, value_size, pnum, pden);

	sl_insert(&skiplist, (ion_key_t) &(long long) { 64 }, (ion_value_t) (char *) { "pop" });
	sl_insert(&skiplist, (ion_key_t) &(long long) { 32 }, (ion_value_t) (char *) { "bep" });
	sl_insert(&skiplist, (ion_key_t) &(long long) { 16 }, (ion_value_t) (char *) { "tot" });

#if DEBUG
	printf("%s\n", "** INSERT **");
	print_skiplist(&skip_list);
#endif

	sl_node_t *cursor;

	cursor = skiplist.head->next[0];
	PLANCK_UNIT_ASSERT_TRUE(tc, *(long long *) cursor->key == 16);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) cursor->value, "tot");

	cursor = skiplist.head->next[0]->next[0];
	PLANCK_UNIT_ASSERT_TRUE(tc, *(long long *) cursor->key == 32);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) cursor->value, "bep");

	cursor = skiplist.head->next[0]->next[0]->next[0];
	PLANCK_UNIT_ASSERT_TRUE(tc, *(long long *) cursor->key == 64);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) cursor->value, "pop");

	ion_byte_t		value[10];
	ion_status_t	status;

	status = sl_query(&skiplist, (ion_key_t) &(long long) { 64 }, value);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, "pop");

	status = sl_query(&skiplist, (ion_key_t) &(long long) { 32 }, value);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, "bep");

	status = sl_query(&skiplist, (ion_key_t) &(long long) { 16 }, value);
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) value, "tot");

	status = sl_delete(&skiplist, (ion_key_t) &(long long) { 64 });
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

	status = sl_delete(&skiplist, (ion_key_t) &(long long) { 32 });
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

	status = sl_delete(&skiplist, (ion_key_t) &(long long) { 16 });
	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
	PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);

#if DEBUG
	printf("%s\n", "** DELETE **");
	print_skiplist(&skip_list);
#endif

	sl_destroy(&skiplist);
}

/**
@brief	  Tests a skiplist under standard conditions with large keys. This
			is intended to test the comparison function, which used to break
			when keys were greater than 256.

@param	  tc
				Cutest dependency
 */
void
test_skiplist_big_keys(
	planck_unit_test_t *tc
) {
	PRINT_HEADER();

	skiplist_t skiplist;

	initialize_skiplist_std_conditions(&skiplist);

	int i;

	for (i = 900; i < 999; i++) {
		ion_status_t status = sl_insert(&skiplist, (ion_key_t) &i, (ion_value_t) (char *) { "BIG!" });

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

	for (i = 900; i < 999; i++) {
		sl_node_t *cursor = sl_find_node(&skiplist, (ion_key_t) &i);

		PLANCK_UNIT_ASSERT_TRUE(tc, *(int *) cursor->key == i);
		PLANCK_UNIT_ASSERT_STR_ARE_EQUAL(tc, (char *) cursor->value, "BIG!");
	}

#if DEBUG
	printf("%s\n", "** INSERT **");
	print_skiplist(&skip_list);
#endif

	for (i = 900; i < 999; i++) {
		ion_status_t status = sl_delete(&skiplist, (ion_key_t) &i);

		PLANCK_UNIT_ASSERT_TRUE(tc, err_ok == status.error);
		PLANCK_UNIT_ASSERT_TRUE(tc, 1 == status.count);
	}

#if DEBUG
	printf("%s\n", "** DELETE **");
	print_skiplist(&skip_list);
#endif

	int h;

	for (h = skiplist.head->height; h >= 0; h--) {
		PLANCK_UNIT_ASSERT_TRUE(tc, skiplist.head->next[h] == NULL);
	}

	sl_destroy(&skiplist);
}

/**
@brief	  Creates the suite to test using CuTest.
@return	 Pointer to a CuTest suite.
*/
planck_unit_suite_t *
skiplist_getsuite(
) {
	planck_unit_suite_t *suite = planck_unit_new_suite();

	/* Initialization Tests */
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_initialize);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_free_all);

	/* Insertion Tests */
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_single_insert);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_insert_multiple);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_randomized_insert);

	/* Get Node Tests */
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_get_node_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_get_node_single_high);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_get_node_single_low);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_get_node_single_many);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_get_node_several);

	/* Query Tests */
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_query_nonexist_empty);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_query_nonexist_populated_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_query_nonexist_populated_several);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_query_exist_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_query_exist_populated_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_query_exist_populated_several);

	/* Delete Tests */
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_delete_empty);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_delete_nonexist_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_delete_nonexist_several);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_delete_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_delete_single_several);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_delete_single_several_noncont);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_delete_several_all);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_delete_several_same_key);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_delete_several_same_key_in_mix);

	/* Update Tests */
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_update_single_nonexist);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_update_single_nonexist_nonempty);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_update_many_nonexist_nonempty);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_update_single_exist);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_update_single_many_exist);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_update_several_many_exist);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_update_several_same_key);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_update_several_same_key_in_mix);

	/* Hybrid Tests */
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_delete_then_insert_single);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_delete_then_insert_several);

	/* Variation Tests */
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_different_size);
	PLANCK_UNIT_ADD_TO_SUITE(suite, test_skiplist_big_keys);

	return suite;
}

/**
@brief	  Runs all skiplist related test and outputs the result.
 */
void
runalltests_skiplist(
) {
	/* CuString	*output	= CuStringNew(); */
	planck_unit_suite_t *suite = skiplist_getsuite();

	planck_unit_run_suite(suite);
	/* CuSuiteSummary(suite, output); */
	/* CuSuiteDetails(suite, output); */
	/* printf("----\nSkiplist Tests:\n%s\n", output->buffer); */

	planck_unit_destroy_suite(suite);
	/* CuSuiteDelete(suite); */
	/* CuStringDelete(output); */
}
