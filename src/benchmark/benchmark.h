/**
* benchmark.h
* Author: Eric
* Date: Feb. 4th 2015
*/

#ifndef BENCHMARK_H_
#define BENCHMARK_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "./../kv_system.h"
#include "./../kv_io.h"
#include "./../ram/ramutil.h"
#include "./../serial_io/serial_c_iface.h"

static unsigned volatile int ram_counter;
static unsigned volatile int time_counter;

void
benchmark_start(
    void
);

void
benchmark_stop(
    void
);

#ifdef __cplusplus
}
#endif

#endif /* BENCHMARK_H_ */
