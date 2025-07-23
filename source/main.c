#include<math.h>
#include<stdint.h>
#include<stdlib.h>
#include<string.h>

#include"pd_api.h"
#include"arith.h"
#include"memory.h"
#include"sprite.h"
#include"screen.h"
#include"ruler.h"

#include"build/version.h"

enum
{
   kArithmeticBenchmarkMode,
   kMemoryBenchmarkMode,
   kSpriteBenchmarkMode,
   kScreenBenchmarkMode,
   kMetricRulerMode,
   kImperialRulerMode,

   kModeCount
};
static const char *kModeNames[kModeCount] =
{
   "math", "memory", "sprites", "screen", "metric ruler", "imperial ruler"
};

// Selected benchmark.
static int g_mode = kArithmeticBenchmarkMode;
static int g_previous_mode = -1;
static PDMenuItem *g_mode_option = NULL;

// Button state, used for tracking when to refresh.
static PDButtons g_button_state = 0;
static PDButtons g_previous_button_state = kButtonA | kButtonB;

// Info card for pause menu.
static LCDBitmap *g_info = NULL;

// Update callback.
static int Update(void *userdata)
{
   PlaydateAPI *pd = userdata;

   int full_refresh = 0;
   if( g_previous_mode != g_mode )
   {
      pd->graphics->clear(kColorWhite);
      full_refresh = 1;
   }

   PDButtons pushed, released;
   pd->system->getButtonState(&g_button_state, &pushed, &released);
   if( g_previous_button_state != g_button_state )
   {
      g_previous_button_state = g_button_state;
      full_refresh = 1;
   }

   switch( g_mode )
   {
      case kArithmeticBenchmarkMode:
         ArithmeticBenchmark(pd, g_button_state, full_refresh);
         break;
      case kMemoryBenchmarkMode:
         MemoryBenchmark(pd, g_button_state, full_refresh);
         break;
      case kSpriteBenchmarkMode:
         SpriteBenchmark(pd, g_button_state);
         break;
      case kScreenBenchmarkMode:
         ScreenBenchmark(pd, g_button_state, full_refresh);
         break;
      case kMetricRulerMode:
         MetricRuler(pd, g_button_state, full_refresh);
         break;
      case kImperialRulerMode:
         ImperialRuler(pd, g_button_state, full_refresh);
         break;
   }

   if( g_previous_mode != g_mode )
   {
      g_previous_mode = g_mode;
      pd->graphics->markUpdatedRows(0, LCD_ROWS - 1);
   }
   return 1;
}

// Menu callbacks.
static void ChangeBenchmarkMode(void *userdata)
{
   PlaydateAPI *pd = userdata;
   g_mode = pd->system->getMenuItemValue(g_mode_option);
}

static void Reset(void *unused_userdata)
{
   switch( g_mode )
   {
      case kArithmeticBenchmarkMode:
         ResetArithmeticBenchmark();
         break;
      case kMemoryBenchmarkMode:
         ResetMemoryBenchmark();
         break;
      case kSpriteBenchmarkMode:
         ResetSpriteBenchmark();
         break;
      case kScreenBenchmarkMode:
         ResetScreenBenchmark();
         break;
      case kMetricRulerMode:
      case kImperialRulerMode:
         ResetRuler();
         break;
   }

   // Force full refresh.
   g_previous_mode = -1;
}

// Initialize info card.
static void SetMenuImage(PlaydateAPI *pd)
{
   if( g_info != NULL )
      return;
   g_info = pd->graphics->newBitmap(LCD_COLUMNS, LCD_ROWS, kColorClear);
   pd->graphics->pushContext(g_info);

   static const LCDPattern kShade =
   {
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
      0xff, 0xaa, 0xff, 0x55, 0xff, 0xaa, 0xff, 0x55
   };
   pd->graphics->fillRect(0, 0, LCD_COLUMNS, 196, (LCDColor)kShade);
   pd->graphics->fillRect(0, 196, LCD_COLUMNS, 44, kColorWhite);

   pd->graphics->drawText(
      kPDBenchVersion, strlen(kPDBenchVersion), kASCIIEncoding, 4, 198);

   static const char kContact[] = "omoikane@uguu.org";
   pd->graphics->drawText(kContact, strlen(kContact), kASCIIEncoding, 4, 220);

   pd->graphics->popContext();
   pd->system->setMenuImage(g_info, 0);
}

#ifdef _WINDLL
__declspec(dllexport)
#endif
int eventHandler(PlaydateAPI *pd, PDSystemEvent event, uint32_t unused_arg)
{
   switch( event )
   {
      case kEventInit:
         pd->system->setUpdateCallback(Update, pd);
         pd->system->addMenuItem("reset", Reset, NULL);
         g_mode_option = pd->system->addOptionsMenuItem(
            "test", kModeNames, kModeCount, ChangeBenchmarkMode, pd);

         // Update at maximum frame rate.
         pd->display->setRefreshRate(0);

         srand(pd->system->getCurrentTimeMilliseconds());
         break;

      case kEventPause:
         SetMenuImage(pd);
         break;

      default:
         break;
   }
   return 0;
}
