/******************************************************************************/
/**
@file		run_slstore.c
@author		Kris Wallperington
@brief		Entry point for Skiplist unit tests
*/
/******************************************************************************/

void
runalltests_skiplist();

void
runalltests_skiplist_handler();

int
main()
{
	runalltests_skiplist();
	runalltests_skiplist_handler();
	return 0;
}
