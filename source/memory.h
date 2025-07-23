// Benchmark for memory access.

#ifndef MEMORY_H_
#define MEMORY_H_

#include"pd_api.h"

void MemoryBenchmark(PlaydateAPI *pd, PDButtons buttons, int full_refresh);
void ResetMemoryBenchmark(void);

#endif  // MEMORY_H_
