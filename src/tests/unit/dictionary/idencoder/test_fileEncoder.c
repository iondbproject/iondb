/*
 * linearhash.c
 *
 *  Created on: Apr 7, 2015
 *      Author: workstation
 */

#include <stdio.h>
#include <string.h>
#include <limits.h>
#include "./../../../CuTest.h"
#include "./../../../planckunit.h"
#include "./../../../../util/idencoder/fileEncoder.h"



/**
@brief 		Tests the encoding of dictionary id to encoded file name.

@param 		tc
				planck_unit_test_t
 */
void
test_encode_parent(
	planck_unit_test_t	*tc
)
{

	fe_filename_t parent_file;

	parent_file.instance_id 	= 0;

	parent_file.parent.type		= mastertable;

	fe_encode_parent_id(&parent_file);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0								== strcmp(parent_file.parent.parent_filename, "00000AAA.000"));

	free(parent_file.parent.parent_filename);

	parent_file.instance_id 	= 0;
	parent_file.parent.type		= flatfile;
	fe_encode_parent_id(&parent_file);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0								== strcmp(parent_file.parent.parent_filename, "00000AAA.100"));

	free(parent_file.parent.parent_filename);

	parent_file.instance_id 	= 0;
	parent_file.parent.type		= file_based_open_address_hash;
	fe_encode_parent_id(&parent_file);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0								== strcmp(parent_file.parent.parent_filename, "00000AAA.400"));

	free(parent_file.parent.parent_filename);

	parent_file.instance_id 	= 4095;
	parent_file.parent.type		= file_based_open_address_hash;
	fe_encode_parent_id(&parent_file);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0								== strcmp(parent_file.parent.parent_filename, "00000PPP.400"));

	free(parent_file.parent.parent_filename);

}


/**
@brief		Tests the encoding of child file ID's for an associated parent file.

@param 		tc
				planck_unit_test_t
 */
void
test_encode_child(
	planck_unit_test_t	*tc
)
{

	fe_filename_t parent_file;

	parent_file.instance_id 	= 0;
	parent_file.child.child_id	= 0;
	fe_encode_child_id(&parent_file);

	PLANCK_UNIT_ASSERT_TRUE(tc, 0								== strcmp(parent_file.child.child_filename, "AAAAAAAA.AAA"));

	free(parent_file.child.child_filename);

	parent_file.instance_id 	= 15;
	fe_encode_child_id(&parent_file);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0								== strcmp(parent_file.child.child_filename, "AAAAAAAA.PAA"));

	free(parent_file.child.child_filename);

	parent_file.instance_id 	= 16;
	fe_encode_child_id(&parent_file);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0								== strcmp(parent_file.child.child_filename, "AAAAAAAA.ABA"));

	free(parent_file.child.child_filename);

	parent_file.instance_id 	= 17;
	fe_encode_child_id(&parent_file);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0								== strcmp(parent_file.child.child_filename, "AAAAAAAA.BBA"));

	free(parent_file.child.child_filename);

	parent_file.instance_id 	= 4095;

	fe_encode_child_id(&parent_file);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0								== strcmp(parent_file.child.child_filename, "AAAAAAAA.PPP"));

	free(parent_file.child.child_filename);

	parent_file.instance_id 	= 4095;
	parent_file.child.child_id	= 1;
	fe_encode_child_id(&parent_file);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0								== strcmp(parent_file.child.child_filename, "BAAAAAAA.PPP"));

	free(parent_file.child.child_filename);

	parent_file.instance_id 	= 4095;
	parent_file.child.child_id	= 15;
	fe_encode_child_id(&parent_file);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0								== strcmp(parent_file.child.child_filename, "PAAAAAAA.PPP"));

	free(parent_file.child.child_filename);

	parent_file.instance_id 	= 4095;
	parent_file.child.child_id	= 16;
	fe_encode_child_id(&parent_file);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0								== strcmp(parent_file.child.child_filename, "ABAAAAAA.PPP"));

	free(parent_file.child.child_filename);

	parent_file.instance_id 	= 4095;
	parent_file.child.child_id	= 4294967295LL;

	fe_encode_child_id(&parent_file);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0								== strcmp(parent_file.child.child_filename, "PPPPPPPP.PPP"));

	free(parent_file.child.child_filename);

}

/**
@brief		Tests the encoding of a config file name for an associated parent file.

@param 		tc
				planck_unit_test_t
 */
void
test_encode_config(
	planck_unit_test_t	*tc
)
{

	fe_filename_t parent_file;

	parent_file.parent.type		= mastertable;

	parent_file.instance_id 	= 0;
	fe_encode_config_id(&parent_file);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0								== strcmp(parent_file.config.config_filename, "00000000.AAA"));
	free(parent_file.parent.parent_filename);

	parent_file.instance_id 	= 4095;
	fe_encode_config_id(&parent_file);
	PLANCK_UNIT_ASSERT_TRUE(tc, 0								== strcmp(parent_file.config.config_filename, "00000000.PPP"));
	free(parent_file.parent.parent_filename);

	parent_file.instance_id 	= 4096;
	PLANCK_UNIT_ASSERT_TRUE(tc, err_illegal_state				== fe_encode_config_id(&parent_file)		);

}

/**
@brief		Tests error checking on ranges for IDs for parent files.

@param 		tc
				planck_unit_test_t
 */
void
test_encode_parent_illegal_state(
	planck_unit_test_t	*tc
)
{

	fe_filename_t parent_file;

	parent_file.instance_id 	= 4096;

	parent_file.parent.type		= mastertable;

	PLANCK_UNIT_ASSERT_TRUE(tc, err_illegal_state							== fe_encode_parent_id(&parent_file));

	free(parent_file.parent.parent_filename);


	parent_file.instance_id 	= 1;

	parent_file.parent.type		= 4096;

	PLANCK_UNIT_ASSERT_TRUE(tc, err_illegal_state							== fe_encode_parent_id(&parent_file));

	free(parent_file.parent.parent_filename);
}



/**
@brief		Tests error checking on ranges for IDs for child files.

@param 		tc
				planck_unit_test_t
 */
void
test_encode_child_illegal_state(
	planck_unit_test_t	*tc
)
{

	fe_filename_t parent_file;

	parent_file.instance_id 	= 4096;

	PLANCK_UNIT_ASSERT_TRUE(tc, err_illegal_state							== fe_encode_child_id(&parent_file));

	free(parent_file.parent.parent_filename);

	parent_file.instance_id 	= 1;

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok										== fe_encode_child_id(&parent_file));

	free(parent_file.parent.parent_filename);
}



/**
@brief		Tests the operation of the destroy function.

@param 		tc
				planck_unit_test_t
 */
void
test_destroy(
	planck_unit_test_t	*tc
)
{

	fe_filename_t parent_file;

	parent_file.instance_id 	= 4096;

	PLANCK_UNIT_ASSERT_TRUE(tc, err_illegal_state							== fe_encode_child_id(&parent_file));

	parent_file.destroy(&parent_file);

	parent_file.instance_id 	= 1;

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok										== fe_encode_child_id(&parent_file));

	parent_file.destroy(&parent_file);

	parent_file.instance_id 	= 1;
	parent_file.parent.type = flatfile;

	PLANCK_UNIT_ASSERT_TRUE(tc, err_ok										== fe_encode_parent_id(&parent_file));

	parent_file.destroy(&parent_file);


}

planck_unit_suite_t*
encode_file_id_getsuite()
{
	planck_unit_suite_t *suite = planck_unit_new_suite();

	planck_unit_add_to_suite(suite, test_encode_parent);
	planck_unit_add_to_suite(suite, test_encode_child);
	planck_unit_add_to_suite(suite, test_encode_config);
	planck_unit_add_to_suite(suite, test_encode_parent_illegal_state);
	planck_unit_add_to_suite(suite, test_encode_child_illegal_state);
	planck_unit_add_to_suite(suite, test_destroy);
	return suite;
}


void
runalltests_file_encode_id()
{
	//CuString	*output	= CuStringNew();
	planck_unit_suite_t		*suite	= encode_file_id_getsuite();

	planck_unit_run_suite(suite);
	//CuSuiteSummary(suite, output);
	//CuSuiteDetails(suite, output);
	//printf("%s\n", output->buffer);

	//CuSuiteDelete(suite);
	//CuStringDelete(output);
}
