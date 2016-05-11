/******************************************************************************/
/**
@file		run_skip_list_store.c
@author		Kris Wallperington
@brief		Entry point for Skiplist unit tests
*/
/******************************************************************************/

void
runalltests_skiplist();

void
runalltests_skiplist_handler();

int
main(void)
{
	runalltests_skiplist();
	runalltests_skiplist_handler();
	return 0;
}
