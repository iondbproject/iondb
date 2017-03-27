/******************************************************************************/
/**
@file
@author		Kris Wallperington
@brief		Entry point for Skiplist unit tests
*/
/******************************************************************************/

#include "test_skip_list.h"
#include "test_skip_list_handler.h"

int
main(
	void
) {
	fdeleteall();
	runalltests_skiplist();
	runalltests_skiplist_handler();
	return 0;
}
