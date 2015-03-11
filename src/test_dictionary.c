/*
 * dictionary.c
 *
 *  Created on: Jul 15, 2014
 *      Author: workstation
 */

#include <limits.h>
#include <stdio.h>
#include <string.h>
#include "CuTest.h"
#include "dicttypes.h"
#include "dictionary.h"


void
test_dictionary_compare_numerics(
	CuTest		*tc
)
{

	ion_key_t 		key_one;
	ion_key_t 		key_two;

	key_one 		= (ion_key_t)&(int){1};
	key_two 		= (ion_key_t)&(int){1};

	CuAssertTrue(tc, IS_EQUAL ==
			dictionary_compare_signed_value(key_one, key_two, sizeof(int)));

	key_one 		= (ion_key_t)&(int){1};
	key_two 		= (ion_key_t)&(int){2};

	CuAssertTrue(tc, ZERO >
				dictionary_compare_signed_value(key_one, key_two,sizeof(int)));

	key_one 		= (ion_key_t)&(int){2};
	key_two 		= (ion_key_t)&(int){0};

	CuAssertTrue(tc, ZERO <
					dictionary_compare_signed_value(key_one, key_two,sizeof(int)));
int i;
	for (i = 1; i< 10; i++)
	{
		CuAssertTrue(tc, ZERO <
						dictionary_compare_signed_value((ion_key_t)&i, key_two ,sizeof(int)));
	}


	//case for unsigned signed char
	{
		unsigned char * key_one;
		unsigned char * key_two;

		key_one 		= &(unsigned char){UCHAR_MAX};
		key_two 		= &(unsigned char){0};

		CuAssertTrue(tc, ZERO <
							dictionary_compare_unsigned_value(key_one, key_two, sizeof(unsigned char)));

	}

	{
		unsigned short * key_one;
		unsigned short * key_two;

		key_one 		= &(unsigned short){USHRT_MAX};
		key_two 		= &(unsigned short){0};

		CuAssertTrue(tc, ZERO <
							dictionary_compare_unsigned_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(unsigned short)));

	}

	{
			unsigned int * key_one;
			unsigned int * key_two;

			key_one 		= &(unsigned int){UINT_MAX};
			key_two 		= &(unsigned int){0};

			CuAssertTrue(tc, ZERO <
								dictionary_compare_unsigned_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(unsigned int)));

		}

	{
			unsigned long * key_one;
			unsigned long * key_two;

			key_one 		= &(unsigned long){ULONG_MAX};
			key_two 		= &(unsigned long){0};

			CuAssertTrue(tc, ZERO <
								dictionary_compare_unsigned_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(unsigned long)));

		}

	{
		long * key_one;
		long * key_two;

		key_one 		= &(long){LONG_MAX};
		key_two 		= &(long){0};

		CuAssertTrue(tc, ZERO <
							dictionary_compare_signed_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(long)));

	}
	{
		char * key_one;
		char * key_two;

		key_one 		= &(char){CHAR_MAX};
		key_two 		= &(char){0};

		CuAssertTrue(tc, ZERO <
							dictionary_compare_signed_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(char)));

		}

		{
			char * key_one;
			char * key_two;

			key_one 		= &(char){0};
			key_two 		= &(char){-1};

			CuAssertTrue(tc, ZERO <
								dictionary_compare_signed_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(char)));

			}


		{
			short * key_one;
			short * key_two;

			short i,j;

			key_one 		= &i;
			key_two 		= &j;

			for (i = SHRT_MIN; i < SHRT_MAX; i++)
			{
				//printf("i %i\n",i);
				for (j = SHRT_MIN; j < SHRT_MAX; j++)
				{
					if (i < j)
					{
						CuAssertTrue(tc, ZERO >
								dictionary_compare_signed_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(short)));
					}else if (i == j)
					{
						CuAssertTrue(tc, ZERO ==
								dictionary_compare_signed_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(short)));
					}else
					{
						CuAssertTrue(tc, ZERO <
											dictionary_compare_signed_value((ion_key_t)key_one, (ion_key_t)key_two,sizeof(short)));
					}
				}
			}
			}
}


CuSuite*
dictionary_getsuite()
{
	CuSuite *suite = CuSuiteNew();

	SUITE_ADD_TEST(suite, test_dictionary_compare_numerics);
	return suite;
}

void
runalltests_dictionary()
{
	CuString	*output	= CuStringNew();
	CuSuite		*suite	= dictionary_getsuite();

	CuSuiteRun(suite);
	//CuSuiteSummary(suite, output);
	CuSuiteDetails(suite, output);
	printf("%s\n", output->buffer);

	CuSuiteDelete(suite);
	CuStringDelete(output);
}