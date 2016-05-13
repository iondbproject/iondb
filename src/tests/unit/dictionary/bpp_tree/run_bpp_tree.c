#include "../../../../serial/serial_c_iface.h"

void
run_all_tests_bpptreehandler(
);

int
main(void)
{
#if defined(ARDUINO)
	serial_init(9600);
#endif

	run_all_tests_bpptreehandler();

#if defined(ARDUINO)
	serial_close();
#endif

	return 0;
}
