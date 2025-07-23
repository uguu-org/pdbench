#include"arith.h"

// Default operation counts.
//
// This is nonzero, since running the benchmark with zero operations causes
// the frame rate to be too high to be readable.  We allow the user to
// adjust parameters to zero, we just don't want to start off with
// unreadable numbers.
//
// One alternative is to use a better font to render the frame rate, but
// that requires shipping extra fonts.
#define DEFAULT_OPERATION_COUNT  0x8000

// Operation counts.
static int g_int_add = DEFAULT_OPERATION_COUNT;
static int g_int_mul = DEFAULT_OPERATION_COUNT;
static int g_float_add = DEFAULT_OPERATION_COUNT;
static int g_float_mul = DEFAULT_OPERATION_COUNT;

// Run computations.
// https://gcc.godbolt.org/z/sb8sjhde3
static void RunBenchmark(void)
{
   // Intermediate results.  These are declared volatile to disable
   // compiler optimizations around them.
   volatile int int_result = 0;
   volatile float float_result = 0;

   for(int i = 0; i < g_int_add; i++)
      int_result += i;
   for(int i = 0; i < g_int_mul; i++)
      int_result *= i;

   for(int i = 0; i < g_float_add; i++)
      float_result += i;
   for(int i = 0; i < g_float_mul; i++)
      float_result *= i;
}

// Draw frame rate and help text.
static void DrawStatus(PlaydateAPI *pd, int full_refresh)
{
   const float fps = pd->display->getFPS();

   char *text = NULL;
   int length;
   if( full_refresh != 0 )
   {
      pd->graphics->fillRect(0, 0, LCD_COLUMNS, 185, kColorWhite);
      length = pd->system->formatString(
         &text,
         "FPS = %.1f\n"
         "int: add = %d, mul = %d\n"
         "float: add = %d, mul = %d\n\n"
         /* Left */  "\u2b05 + crank: adjust integer additions\n"
         /* Up */    "\u2b06 + crank: adjust integer multiplications\n"
         /* Right */ "\u27a1 + crank: adjust floating point additions\n"
         /* Down */  "\u2b07 + crank: adjust floating point multiplications\n"
         /* A */     "\u24b6 + crank: adjust everything at once",
         (double)fps,
         g_int_add, g_int_mul,
         g_float_add, g_float_mul);
   }
   else
   {
      pd->graphics->fillRect(0, 0, 128, 25, kColorWhite);
      length = pd->system->formatString(&text, "FPS = %.1f", (double)fps);
   }
   pd->graphics->drawText(text, length, kUTF8Encoding, 5, 5);
   pd->system->realloc(text, 0);
}

// Apply adjustment to a single count, clamping results to predefined bounds.
static void AdjustOp(int *op, int delta)
{
   *op += delta;
   if( *op < 0 ) { *op = 0; }
   if( *op > 0xffffff ) { *op = 0xffffff; }
}

// Handle user input.
static void HandleInput(PlaydateAPI *pd, PDButtons buttons)
{
   if( (buttons & (kButtonA | kButtonB)) != 0 )
   {
      pd->graphics->fillRect(0, 165, LCD_COLUMNS, 20, kColorXOR);
      buttons |= kButtonLeft | kButtonRight | kButtonUp | kButtonDown;
   }
   const int delta = 100 * pd->system->getCrankChange();

   if( (buttons & kButtonLeft) != 0 )
   {
      pd->graphics->fillRect(0, 85, LCD_COLUMNS, 20, kColorXOR);
      AdjustOp(&g_int_add, delta);
   }
   if( (buttons & kButtonUp) != 0 )
   {
      pd->graphics->fillRect(0, 105, LCD_COLUMNS, 20, kColorXOR);
      AdjustOp(&g_int_mul, delta);
   }
   if( (buttons & kButtonRight) != 0 )
   {
      pd->graphics->fillRect(0, 125, LCD_COLUMNS, 20, kColorXOR);
      AdjustOp(&g_float_add, delta);
   }
   if( (buttons & kButtonDown) != 0 )
   {
      pd->graphics->fillRect(0, 145, LCD_COLUMNS, 20, kColorXOR);
      AdjustOp(&g_float_mul, delta);
   }
}

// Exported functions.
void ArithmeticBenchmark(PlaydateAPI *pd, PDButtons buttons, int full_refresh)
{
   if( buttons != 0 )
      full_refresh = 1;

   RunBenchmark();
   DrawStatus(pd, full_refresh);
   HandleInput(pd, buttons);

   pd->graphics->markUpdatedRows(5, full_refresh != 0 ? 184 : 24);
}

void ResetArithmeticBenchmark(void)
{
   g_int_add = g_int_mul = g_float_add = g_float_mul = DEFAULT_OPERATION_COUNT;
}
