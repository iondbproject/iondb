/**
* benchmark.h
* Author: Eric
* Date: Feb. 4th 2015
*/

#if !defined(BENCHMARK_H_)
#define BENCHMARK_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include "./../serial/serial_c_iface.h"
#include "../key_value/kv_system.h"
#include "../key_value/kv_io.h"
#include "ram/ram_util.h"

static unsigned volatile int	ram_counter;
static unsigned volatile int	time_counter;

void
benchmark_start(
	void
);

void
benchmark_stop(
	void
);

#if defined(__cplusplus)
}
#endif

#endif /* BENCHMARK_H_ */
