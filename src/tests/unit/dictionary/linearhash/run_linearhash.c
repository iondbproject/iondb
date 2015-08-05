
#define NO_SPLIT 1

void
runalltests_file_linked_list();

void
runalltests_linear_hash();

void
runalltests_linear_hash_handler();

int
main()
{
	runalltests_file_linked_list();
	runalltests_linear_hash();
	runalltests_linear_hash_handler();
	return 0;
}
