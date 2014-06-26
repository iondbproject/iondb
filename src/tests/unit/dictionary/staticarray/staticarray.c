/******************************************************************************/
/**
@file	 staticarray.c
@author	 Raffi Kudlac
@brief	 ###BRIEF HERE###
*/
/******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "./../../../CuTest.h"
#include "./../../../../dictionary/staticarray/staticarray.h"
#include "./../../../../dictionary/dicttypes.h"
#include "./../../../../dictionary/dictionary.h"

#define MAX_HASH_TEST 100
#define STD_MAP_SIZE 10


void
test_create(
	CuTest *tc
)
{

	int key_size = 3;
	int value_size = 10;
	long array_size = 100;

	static_array_t st;

	//st.maxelements = ipow(256, key_size);
	//st.key_size = key_size;
	//st.value_size = value_size;
	//st.array_size = array_size;

	sa_dictionary_create(&st,key_size,value_size,array_size);

	int x;

	for(x=0;x<array_size;x++){
		CuAssertTrue(tc,((bucket_t *)&(st.array[x]))->status == EMPTY);//think this line is wrong
	}

	free(st.array);
	st.array = NULL;
	printf("\n");

}

void
test_create_invalid_params(CuTest *tc
)
{
	int key_size = 7;
	int value_size = 10000;
	long array_size = -1;

	static_array_t st;

	//st.maxelements = ipow(256, key_size);
	//st.key_size = key_size;
	//st.value_size = value_size;
	//st.array_size = array_size;

	sa_dictionary_create(&st,key_size,value_size,array_size);

	int x;

	for(x=0;x<array_size;x++){
		CuAssertTrue(tc,((bucket_t *)&(st.array[x]))->status == EMPTY);//think this line is wrong
	}

	CuAssertTrue(tc,st.array_size==st.maxelements);
	CuAssertTrue(tc,st.key_size==4);
	CuAssertTrue(tc,st.value_size==1000);

	free(st.array);
	st.array = NULL;


}

void
test_insert(CuTest *tc
)
{

}

CuSuite*
open_address_staticarray_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_insert);
	SUITE_ADD_TEST(suite, test_create_invalid_params);

	SUITE_ADD_TEST(suite, test_create);

	return suite;
}

void
runalltests_open_address_staticarray()
{
	CuString	*output	= CuStringNew();
	CuSuite		*suite	= open_address_staticarray_getsuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);

	CuSuiteDelete(suite);
	CuStringDelete(output);
}
