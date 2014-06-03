/*
 * oahash.c
 *
 *  Created on: May 28, 2014
 *      Author: workstation
 */


#include <stdio.h>
#include <string.h>
#include "./../../../CuTest.h"
#include "./../../../../dictionary/openaddresshash/oahash.h"

void
test_open_address_hashmap_create(
	CuTest		*tc
)
{/*
	byte			*segment;
	memchunk_handler_t	handler;
	status_t		status;

	status = memchunk_create_segment(&segment, numitems, itemsize);

	CuAssertTrue(tc, IS_STATUS_OK(status));

	status = memchunk_init_handler(&handler, segment, numitems, itemsize);
	CuAssertTrue(tc, IS_STATUS_OK(status));

	CuAssertTrue(tc, numitems		== handler.super.numitems);
	CuAssertTrue(tc, itemsize		== handler.super.itemsize);
	CuAssertTrue(tc, CHUNKTYPE_INMEMORY	== handler.super.type);
	CuAssertTrue(tc, NULL			!= handler.segment);

	status = memchunk_destroy_segment(&(handler.segment));
	CuAssertTrue(tc, IS_STATUS_OK(status));
	CuAssertTrue(tc, NULL			== handler.segment);*/
}


CuSuite*
open_address_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_open_address_hashmap_create);
/*	SUITE_ADD_TEST(suite, test_memorystorage_memchunk_create_2);
	SUITE_ADD_TEST(suite, test_memorystorage_memchunk_create_3);
	SUITE_ADD_TEST(suite, test_memorystorage_memchunk_writeread_1);
	SUITE_ADD_TEST(suite, test_memorystorage_memchunk_writeread_2);
	SUITE_ADD_TEST(suite, test_memorystorage_memchunk_writeread_3);
	SUITE_ADD_TEST(suite, test_memorystorage_memchunk_outsideofchunk_1);
	SUITE_ADD_TEST(suite, test_memorystorage_memchunk_outsideofchunk_2);*/

	return suite;
}

void
runalltests_open_address_hash()
{
	CuString	*output	= CuStringNew();
	CuSuite		*suite	= open_address_getsuite();

	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);

	CuSuiteDelete(suite);
	CuStringDelete(output);
}
