#include <stdio.h>
#include <string.h>
#include "../../../CuTest.h"
#include "../../../../commontypes.h"
#include "../../../../storage/memory/memorystorage.h"
#include "../../../../storage/chunktypes.h"

void
test_memorystorage_memchunk_create(
	CuTest		*tc,
	my_size_t	itemsize,
	my_size_t	numitems
)
{
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
	CuAssertTrue(tc, NULL			== handler.segment);
}

void
test_memorystorage_memchunk_writeread(
	CuTest			*tc,
	memchunk_handler_t	*handler,
	byte			*data,
	offset_t		idx,
	bool_t			outofbounds
)
{
	status_t	status;
	
	status	= memchunk_write(handler, data, idx);
	
	if (outofbounds)
	{
		CuAssertTrue(tc, ERROR_OUTSIDEOFCHUNK	== status.error);
		return;
	}
	
	CuAssertTrue(tc, IS_STATUS_OK(status));
	
	/* Create zeroed-out data holder. */
	byte 		readinto[handler->super.itemsize];
	my_size_t i;
	for (i = 0; i < handler->super.itemsize; ++i)
	{
		readinto[i] = 0;
	}
	
	status	= memchunk_read(handler, readinto, idx);
	CuAssertTrue(tc, IS_STATUS_OK(status));
	
	CuAssertTrue(tc, 0 == strcmp((char*)data, (char*)readinto));
}

void
test_memorystorage_memchunk_create_1(CuTest *tc)
{
	test_memorystorage_memchunk_create(tc, 10, 200);
}

void
test_memorystorage_memchunk_create_2(CuTest *tc)
{
	test_memorystorage_memchunk_create(tc, 1, 1);
}

void
test_memorystorage_memchunk_create_3(CuTest *tc)
{
	test_memorystorage_memchunk_create(tc, 100, 37);
}

void
test_memorystorage_memchunk_writeread_1(CuTest *tc)
{
	byte			*segment;
	memchunk_handler_t	handler;
	my_size_t		itemsize	= 10;
	my_size_t		numitems	= 20;
	offset_t		idx		= 4;
	
	memchunk_create_segment(&segment, numitems, itemsize);
	memchunk_init_handler(&handler, segment, numitems, itemsize);
	
	byte			data[10]	= "abcdefghi";
	
	test_memorystorage_memchunk_writeread(tc, &handler, data, idx, false);
	
	memchunk_destroy_segment(&(handler.segment));
}

void
test_memorystorage_memchunk_writeread_2(CuTest *tc)
{
	byte			*segment;
	memchunk_handler_t	handler;
	my_size_t		itemsize	= 2;
	my_size_t		numitems	= 200;
	offset_t		idx		= 199;
	
	memchunk_create_segment(&segment, numitems, itemsize);
	memchunk_init_handler(&handler, segment, numitems, itemsize);
	
	byte			data[2]	= "7";
	
	test_memorystorage_memchunk_writeread(tc, &handler, data, idx, false);
	
	memchunk_destroy_segment(&(handler.segment));
}

void
test_memorystorage_memchunk_writeread_3(CuTest *tc)
{
	byte			*segment;
	memchunk_handler_t	handler;
	my_size_t		itemsize		= sizeof(int);
	my_size_t		numitems		= 1;
	offset_t		idx			= 0;
	int			value			= 9;
	
	memchunk_create_segment(&segment, numitems, itemsize);
	memchunk_init_handler(&handler, segment, numitems, itemsize);
	
	byte			*data			= (byte*)&value;
	
	test_memorystorage_memchunk_writeread(tc, &handler, data, idx, false);
	
	memchunk_destroy_segment(&(handler.segment));
}

void
test_memorystorage_memchunk_outsideofchunk_1(CuTest *tc)
{
	byte			*segment;
	memchunk_handler_t	handler;
	my_size_t		itemsize	= 2;
	my_size_t		numitems	= 200;
	offset_t		idx		= 200;
	
	memchunk_create_segment(&segment, numitems, itemsize);
	memchunk_init_handler(&handler, segment, numitems, itemsize);
	
	byte			data[2]	= "7";
	
	test_memorystorage_memchunk_writeread(tc, &handler, data, idx, true);
	
	memchunk_destroy_segment(&(handler.segment));
}

void
test_memorystorage_memchunk_outsideofchunk_2(CuTest *tc)
{
	byte			*segment;
	memchunk_handler_t	handler;
	my_size_t		itemsize	= 2;
	my_size_t		numitems	= 200;
	offset_t		idx		= -1;
	
	memchunk_create_segment(&segment, numitems, itemsize);
	memchunk_init_handler(&handler, segment, numitems, itemsize);
	
	byte			data[2]	= "7";
	
	test_memorystorage_memchunk_writeread(tc, &handler, data, idx, true);
	
	memchunk_destroy_segment(&(handler.segment));
}

CuSuite*
memorystorage_getsuite()
{
	CuSuite *suite = CuSuiteNew();
	
	SUITE_ADD_TEST(suite, test_memorystorage_memchunk_create_1);
	SUITE_ADD_TEST(suite, test_memorystorage_memchunk_create_2);
	SUITE_ADD_TEST(suite, test_memorystorage_memchunk_create_3);
	SUITE_ADD_TEST(suite, test_memorystorage_memchunk_writeread_1);
	SUITE_ADD_TEST(suite, test_memorystorage_memchunk_writeread_2);
	SUITE_ADD_TEST(suite, test_memorystorage_memchunk_writeread_3);
	SUITE_ADD_TEST(suite, test_memorystorage_memchunk_outsideofchunk_1);
	SUITE_ADD_TEST(suite, test_memorystorage_memchunk_outsideofchunk_2);
	
	return suite;
}

void
runalltests_memorystorage()
{
	CuString	*output	= CuStringNew();
	CuSuite		*suite	= memorystorage_getsuite();
	
	CuSuiteRun(suite);
	CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);
	
	CuSuiteDelete(suite);
	CuStringDelete(output);
}
