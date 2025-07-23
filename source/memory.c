#include"memory.h"

// Default memory access counts.
//
// This is nonzero, since running the benchmark with zero operations causes
// the refresh rate to be too high to be readable.  We allow the user to
// adjust parameters do zero, we just don't want to start off with
// unreadable numbers.
//
// One alternative is to use a better font to render the frame rate, but
// that requires shipping extra fonts.
#define DEFAULT_ACCESS_COUNT  0x2000

// Maximum number of words to allocate.
#define MAX_WORD_COUNT        0x200000

// Operation counts.
static int g_seq_write = DEFAULT_ACCESS_COUNT;
static int g_seq_read = DEFAULT_ACCESS_COUNT;
static int g_rand_write = DEFAULT_ACCESS_COUNT;
static int g_rand_read = DEFAULT_ACCESS_COUNT;

// Preallocated memory buffer, declared volatile to disable compiler operations.
static volatile int g_memory[MAX_WORD_COUNT];

// Random seed.
static int g_seed = 1;

// Return a random non-negative integer.
//
// We define our own random number generator here so that it can be inlined.
// If we used the one from stdlib.h, most of the cost will be on the function
// call to rand().
static inline int Rand(int *seed)
{
   // Linear congruential generator using glibc params:
   // https://en.wikipedia.org/wiki/Linear_congruential_generator
   //
   // By the way, ARM toolchain uses Newlib, and the random number generator
   // there is another LCG, except it uses 64bit operations instead of 32bits.
   *seed = (*seed * 1103515245 + 12345) & 0x7fffffff;

   // glibc will actually return bits 30..0, so we should return
   // "*seed & 0x3fffffff".  We are not doing that here because we are
   // only going to use the lower bits, because the returned value will
   // be fed to modulus.
   //
   // We would get better randomness if we use the higher bits, but
   // here we care a bit more about speed.
   return *seed;
}

// Run access test.
// https://gcc.godbolt.org/z/qdn493886
static void RunBenchmark(void)
{
   // Read result.  We don't need to make this volatile to disable
   // optimizations, since g_memory is already volatile.
   int read_result = 0;

   for(int i = 0; i < g_seq_write; i++)
      g_memory[i] = i;
   for(int i = 0; i < g_seq_read; i++)
      read_result = g_memory[i];

   // Make random seed local to reduce memory access.
   int seed = g_seed;
   if( g_rand_write > 0 )
   {
      for(int i = 0; i < g_rand_write; i++)
         g_memory[Rand(&seed) % g_rand_write] = i;
   }
   if( g_rand_read > 0 )
   {
      for(int i = 0; i < g_rand_read; i++)
         read_result = g_memory[Rand(&seed) % g_rand_read];
   }

   // Read from read_result, otherwise we get a compiler warning for
   // having a local variable that's set but not used.
   seed ^= read_result & 1;

   // Save random seed.
   g_seed = seed;
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
         "sequential: write = %d, read = %d\n"
         "random: write = %d, read = %d\n\n"
         /* Left */  "\u2b05 + crank: adjust sequential writes\n"
         /* Up */    "\u2b06 + crank: adjust sequential reads\n"
         /* Right */ "\u27a1 + crank: adjust random writes\n"
         /* Down */  "\u2b07 + crank: adjust random reads\n"
         /* A */     "\u24b6 + crank: adjust everything at once",
         (double)fps,
         g_seq_write * sizeof(int), g_seq_read * sizeof(int),
         g_rand_write * sizeof(int), g_rand_read * sizeof(int));
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
   if( *op > MAX_WORD_COUNT ) { *op = MAX_WORD_COUNT; }
}

// Handle user input.
static void HandleInput(PlaydateAPI *pd, PDButtons buttons)
{
   if( (buttons & (kButtonA | kButtonB)) != 0 )
   {
      pd->graphics->fillRect(0, 165, LCD_COLUMNS, 20, kColorXOR);
      buttons |= kButtonLeft | kButtonRight | kButtonUp | kButtonDown;
   }
   const int delta = 256 * pd->system->getCrankChange();

   if( (buttons & kButtonLeft) != 0 )
   {
      pd->graphics->fillRect(0, 85, LCD_COLUMNS, 20, kColorXOR);
      AdjustOp(&g_seq_write, delta);
   }
   if( (buttons & kButtonUp) != 0 )
   {
      pd->graphics->fillRect(0, 105, LCD_COLUMNS, 20, kColorXOR);
      AdjustOp(&g_seq_read, delta);
   }
   if( (buttons & kButtonRight) != 0 )
   {
      pd->graphics->fillRect(0, 125, LCD_COLUMNS, 20, kColorXOR);
      AdjustOp(&g_rand_write, delta);
   }
   if( (buttons & kButtonDown) != 0 )
   {
      pd->graphics->fillRect(0, 145, LCD_COLUMNS, 20, kColorXOR);
      AdjustOp(&g_rand_read, delta);
   }
}

// Exported functions.
void MemoryBenchmark(PlaydateAPI *pd, PDButtons buttons, int full_refresh)
{
   if( buttons != 0 )
      full_refresh = 1;

   RunBenchmark();
   DrawStatus(pd, full_refresh);
   HandleInput(pd, buttons);

   pd->graphics->markUpdatedRows(5, full_refresh != 0 ? 184 : 24);
}

void ResetMemoryBenchmark(void)
{
   g_seq_write = g_seq_read = g_rand_write = g_rand_read = DEFAULT_ACCESS_COUNT;
}
