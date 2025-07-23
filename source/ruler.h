// Measure size of area on screen.

#ifndef RULER_H_
#define RULER_H_

#include"pd_api.h"

void MetricRuler(PlaydateAPI *pd, PDButtons buttons, int full_refresh);
void ImperialRuler(PlaydateAPI *pd, PDButtons buttons, int full_refresh);
void ResetRuler(void);

#endif  // RULER_H_
