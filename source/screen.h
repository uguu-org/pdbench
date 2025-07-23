// Benchmark screen refresh function.

#ifndef SCREEN_H_
#define SCREEN_H_

#include"pd_api.h"

void ScreenBenchmark(PlaydateAPI *pd, PDButtons buttons, int full_refresh);
void ResetScreenBenchmark(void);

#endif  // SCREEN_H_
