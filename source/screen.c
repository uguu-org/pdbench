#include"screen.h"
#include<string.h>

// Minimum distance between rows.  This should be the same as the
// height of one line of text.  It can't be smaller than one row of
// text because we always need to refresh the frame rate display.
#define MIN_REFRESH_HEIGHT    20

// Refresh rows.
static int g_min_row = 0;
static int g_max_row = LCD_ROWS - 1;

// Text positions.
static int g_fps_y = 0;
static int g_help_y = 0;

// Sweeping rectangle.
static int g_sweep_x = 0;
static int g_sweep_w = 0;

// Draw frame rate and help text.
static void DrawStatus(PlaydateAPI *pd, int full_refresh)
{
   static const char kHelp[] =
      /* Up */    "\u2b06 + crank: adjust minimum row\n"
      /* Left */  "\u2b05 + crank: adjust maximum row\n"
      /* A */     "\u24b6 + crank: adjust both rows once\n";

   const float fps = pd->display->getFPS();

   if( full_refresh != 0 )
   {
      pd->graphics->clear(kColorWhite);

      char *text = NULL;
      const int length = pd->system->formatString(
         &text,
         "FPS = %.1f\n"
         "refresh rows: min = %d, max = %d, size = %d",
         (double)fps, g_min_row, g_max_row, g_max_row - g_min_row + 1);
      pd->graphics->drawText(text, length, kASCIIEncoding, 5, g_fps_y);
      pd->system->realloc(text, 0);
      pd->graphics->drawText(kHelp, strlen(kHelp), kUTF8Encoding, 5, g_help_y);

      // Reset vertical strip.
      g_sweep_x = (g_sweep_x + g_sweep_w) % LCD_COLUMNS;
      g_sweep_w = 0;
   }
   else
   {
      // Draw vertical strip across all rows.  This is so that we can see
      // which rows aren't getting refreshed.
      if( g_sweep_w < LCD_COLUMNS / 2 )
      {
         g_sweep_w++;
      }
      else
      {
         pd->graphics->fillRect(
            g_sweep_x, g_min_row, 1, g_max_row - g_min_row + 1, kColorXOR);
         g_sweep_x = (g_sweep_x + 1) % LCD_COLUMNS;
      }
      pd->graphics->fillRect(
         (g_sweep_x + g_sweep_w - 1) % LCD_COLUMNS, g_min_row,
         1, g_max_row - g_min_row + 1, kColorXOR);

      // Draw a white rectangle strip to erase the old frame rate text.
      pd->graphics->fillRect(0, g_fps_y, LCD_COLUMNS, 20, kColorWhite);

      // Draw frame rate text.
      char *text = NULL;
      const int length =
         pd->system->formatString(&text, "FPS = %.1f", (double)fps);
      pd->graphics->drawText(text, length, kASCIIEncoding, 5, g_fps_y);
      pd->system->realloc(text, 0);

      // Draw rectangle over the frame rate text to patch up what was erased.
      pd->graphics->fillRect(g_sweep_x, g_fps_y, g_sweep_w, 20, kColorXOR);
      const int residue = g_sweep_x + g_sweep_w - LCD_COLUMNS;
      if( residue > 0 )
         pd->graphics->fillRect(0, g_fps_y, residue, 20, kColorXOR);
   }
}

// Handle user input.
static void HandleInput(PlaydateAPI *pd, PDButtons buttons)
{
   if( (buttons & (kButtonA | kButtonB)) != 0 )
   {
      pd->graphics->fillRect(0, g_help_y + 40, LCD_COLUMNS, 20, kColorXOR);
      buttons |= kButtonLeft | kButtonRight | kButtonUp | kButtonDown;
   }

   if( (buttons & (kButtonUp | kButtonDown | kButtonLeft | kButtonRight)) != 0 )
   {
      static const LCDPattern kDottedLine =
      {
         0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
         0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
      };
      pd->graphics->fillRect(0, g_min_row, LCD_COLUMNS, 1,
                             (LCDColor)kDottedLine);
      pd->graphics->fillRect(0, g_max_row, LCD_COLUMNS, 1,
                             (LCDColor)kDottedLine);
   }

   const int delta = pd->system->getCrankChange();
   if( (buttons & (kButtonUp | kButtonRight)) != 0 )
   {
      pd->graphics->fillRect(0, g_help_y, LCD_COLUMNS, 20, kColorXOR);

      g_min_row += delta;
      if( g_min_row < 0 )
         g_min_row = 0;
      if( g_min_row + MIN_REFRESH_HEIGHT >= LCD_ROWS )
         g_min_row = LCD_ROWS - MIN_REFRESH_HEIGHT;
      if( g_min_row + MIN_REFRESH_HEIGHT - 1 > g_max_row )
         g_max_row = g_min_row + MIN_REFRESH_HEIGHT - 1;
   }
   if( (buttons & (kButtonDown | kButtonLeft)) != 0 )
   {
      pd->graphics->fillRect(0, g_help_y + 20, LCD_COLUMNS, 20, kColorXOR);

      g_max_row += delta;
      if( g_max_row < MIN_REFRESH_HEIGHT - 1 )
         g_max_row = MIN_REFRESH_HEIGHT - 1;
      if( g_max_row >= LCD_ROWS )
         g_max_row = LCD_ROWS - 1;
      if( g_max_row - MIN_REFRESH_HEIGHT + 1 < g_min_row )
         g_min_row = g_max_row - MIN_REFRESH_HEIGHT + 1;
   }
}

// Exported functions.
void ScreenBenchmark(PlaydateAPI *pd, PDButtons buttons, int full_refresh)
{
   if( buttons != 0 )
      full_refresh = 1;

   g_fps_y = (g_min_row + g_max_row - MIN_REFRESH_HEIGHT) / 2;
   g_help_y = g_fps_y > LCD_ROWS / 2 ? 5 : LCD_ROWS - 65;

   DrawStatus(pd, full_refresh);
   HandleInput(pd, buttons);

   if( full_refresh != 0 )
   {
      pd->graphics->markUpdatedRows(0, LCD_ROWS - 1);
   }
   else
   {
      pd->graphics->markUpdatedRows(g_min_row, g_max_row);
   }
}

void ResetScreenBenchmark(void)
{
   g_min_row = 0;
   g_max_row = LCD_ROWS - 1;
}
