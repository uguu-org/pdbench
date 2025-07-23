#include"ruler.h"

// Screen dimensions, minus the bottom part that's reserved for status display.
#define GRID_WIDTH      LCD_COLUMNS
#define GRID_HEIGHT     (LCD_ROWS - 20)

// Dot pitch in millimeters per pixel from LS027B7DH01A datasheet.
#define DOT_PITCH_MM    0.147f

// Dot pitch in inches per pixel.
#define DOT_PITCH_IN    (DOT_PITCH_MM / 25.4f)

// Rectangle edges in pixel coordinates.  These are initialized to
// align with ruler grid on first use.
static int g_box_left = -1;
static int g_box_right = -1;
static int g_box_top = -1;
static int g_box_bottom = -1;

// Draw grid with evenly spaced lines.
//
// - Pixel distance is converted to physical unit distance by multiplying
//   against "pitch".
//
// - Every "steps_per_major_line" units gets a solid grid line.
//
// - Every "steps_per_minor_line" units gets a dotted grid line.
//
// - For the dotted lines, the dots will be spaced one physical unit apart.
static void DrawGenericGrid(float pitch,
                            int steps_per_minor_line,
                            int steps_per_major_line,
                            PlaydateAPI *pd)
{
   // Draw vertical grid lines.
   int last_length = -1;
   for(int x = 0; x < GRID_WIDTH; x++)
   {
      const int length = (int)(x * pitch);
      if( last_length == length )
         continue;
      if( (length % steps_per_major_line) == 0 )
      {
         pd->graphics->fillRect(x, 0, 1, GRID_HEIGHT, kColorBlack);
      }
      else if( (length % steps_per_minor_line) == 0 )
      {
         int last_dot_length = -1;
         for(int i = 0; i < GRID_HEIGHT; i++)
         {
            const int dot_length = (int)(i * pitch);
            if( last_dot_length == dot_length )
               continue;
            pd->graphics->setPixel(x, i, kColorBlack);
            last_dot_length = dot_length;
         }
      }
      last_length = length;
   }

   // Draw horizontal grid lines.
   for(int y = 0; y < GRID_HEIGHT; y++)
   {
      const int length = (int)(y * pitch);
      if( last_length == length )
         continue;
      if( (length % steps_per_major_line) == 0 )
      {
         pd->graphics->fillRect(0, y, GRID_WIDTH, 1, kColorBlack);
      }
      else if( (length % steps_per_minor_line) == 0 )
      {
         int last_dot_length = -1;
         for(int i = 0; i < GRID_WIDTH; i++)
         {
            const int dot_length = (int)(i * pitch);
            if( last_dot_length == dot_length )
               continue;
            pd->graphics->setPixel(i, y, kColorBlack);
            last_dot_length = dot_length;
         }
      }
      last_length = length;
   }
}

// Draw grid and labels for metric ruler.
static void DrawMetricGrid(PlaydateAPI *pd)
{
   // One dot every millimeter.
   // One dotted line every 5 millimeters.
   // One solid line every centimeter.
   DrawGenericGrid(DOT_PITCH_MM, 5, 10, pd);

   const int width = g_box_right - g_box_left;
   const int height = g_box_bottom - g_box_top;
   pd->graphics->fillRect(g_box_left, g_box_top, width, height, kColorXOR);

   const float width_mm = width * DOT_PITCH_MM;
   const float height_mm = height * DOT_PITCH_MM;

   char *text = NULL;
   const int length = pd->system->formatString(
      &text,
      "%d x %d = %.2f mm x %.2f mm = %.2f mm^2",
      width, height,
      (double)width_mm, (double)height_mm, (double)(width_mm * height_mm));
   pd->graphics->drawText(text, length, kASCIIEncoding, 1, GRID_HEIGHT + 1);
   pd->system->realloc(text, 0);
}

// Draw grid and labels for imperial ruler.
static void DrawImperialGrid(PlaydateAPI *pd)
{
   // One dot every 1/32 inch.
   // One dotted line every 1/8 inch.
   // One solid line every inch.
   DrawGenericGrid(DOT_PITCH_IN * 32, 4, 32, pd);

   const int width = g_box_right - g_box_left;
   const int height = g_box_bottom - g_box_top;
   pd->graphics->fillRect(g_box_left, g_box_top, width, height, kColorXOR);

   const float width_in = width * DOT_PITCH_IN;
   const float height_in = height * DOT_PITCH_IN;

   char *text = NULL;
   const int length = pd->system->formatString(
      &text,
      "%d x %d = %.2f in x %.2f in = %.2f in^2",
      width, height,
      (double)width_in, (double)height_in, (double)(width_in * height_in));
   pd->graphics->drawText(text, length, kASCIIEncoding, 1, GRID_HEIGHT + 1);
   pd->system->realloc(text, 0);
}

// Update parameter, and make sure it stays within bounds.
static void AdjustParam(int *param, int delta, int max)
{
   *param += delta;
   if( *param < 0 ) { *param = 0; }
   if( *param > max ) { *param = max; }
}

// Handle user input.
static void HandleInput(PlaydateAPI *pd, PDButtons buttons)
{
   static const LCDPattern kHorizontalStripes =
   {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x00, 0xff, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff
   };
   static const LCDPattern kVerticalStripes =
   {
      0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
      0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55
   };

   if( (buttons & (kButtonA | kButtonB)) != 0 )
      buttons |= kButtonLeft | kButtonRight | kButtonUp | kButtonDown;
   const int delta = pd->system->getCrankChange();

   // For each edge that's being adjusted, also draw a dotted line tangent
   // to that edge.
   //
   // Note that the lines are drawing using the position values before the
   // adjustment are done.  This is because we already drew a rectangular
   // area earlier using the pre-adjusted values, so the tangent lines will
   // seem to be misaligned if we use the post-adjusted values here.
   if( (buttons & kButtonLeft) != 0 )
   {
      pd->graphics->fillRect(g_box_left, 0, 1, GRID_HEIGHT,
                             (LCDColor)kHorizontalStripes);
      AdjustParam(&g_box_left, delta, GRID_WIDTH);
      if( g_box_left > g_box_right ) { g_box_right = g_box_left; }
   }
   if( (buttons & kButtonRight) != 0 )
   {
      pd->graphics->fillRect(g_box_right, 0, 1, GRID_HEIGHT,
                             (LCDColor)kHorizontalStripes);
      AdjustParam(&g_box_right, delta, GRID_WIDTH);
      if( g_box_right < g_box_left ) { g_box_left = g_box_right; }
   }
   if( (buttons & kButtonUp) != 0 )
   {
      pd->graphics->fillRect(0, g_box_top, GRID_WIDTH, 1,
                             (LCDColor)kVerticalStripes);
      AdjustParam(&g_box_top, delta, GRID_HEIGHT);
      if( g_box_top > g_box_bottom ) { g_box_bottom = g_box_top; }
   }
   if( (buttons & kButtonDown) != 0 )
   {
      pd->graphics->fillRect(0, g_box_bottom, GRID_WIDTH, 1,
                             (LCDColor)kVerticalStripes);
      AdjustParam(&g_box_bottom, delta, GRID_HEIGHT);
      if( g_box_bottom < g_box_top ) { g_box_top = g_box_bottom; }
   }
}

// Exported functions.
void MetricRuler(PlaydateAPI *pd, PDButtons buttons, int full_refresh)
{
   if( g_box_left < 0 )
   {
      g_box_left = g_box_top = 0;
      g_box_right = g_box_bottom = (int)(10 / DOT_PITCH_MM + 0.5f);
   }
   if( buttons != 0 || full_refresh != 0 )
   {
      pd->graphics->clear(kColorWhite);
      pd->graphics->markUpdatedRows(0, LCD_ROWS - 1);
      DrawMetricGrid(pd);
   }
   HandleInput(pd, buttons);
}

void ImperialRuler(PlaydateAPI *pd, PDButtons buttons, int full_refresh)
{
   if( g_box_left < 0 )
   {
      g_box_left = g_box_top = 0;
      g_box_right = g_box_bottom = (int)(1 / DOT_PITCH_IN + 0.5f);
   }
   if( buttons != 0 || full_refresh != 0 )
   {
      pd->graphics->clear(kColorWhite);
      pd->graphics->markUpdatedRows(0, LCD_ROWS - 1);
      DrawImperialGrid(pd);
   }
   HandleInput(pd, buttons);
}

void ResetRuler(void)
{
   g_box_left = g_box_right = g_box_top = g_box_bottom = -1;
}
