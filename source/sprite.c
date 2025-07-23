#include"sprite.h"
#include<stdlib.h>

#define MAX_SPRITES     10000
#define MAX_SPRITE_SIZE 512

// Sprite benchmark parameters.
static int g_circle_count = 0;
static int g_circle_size = 8;
static int g_square_count = 0;
static int g_square_size = 8;

// Sprite animation parameters.
typedef struct
{
   int x, y, vx, vy;
} Sprite;
static Sprite g_circles[MAX_SPRITES];
static Sprite g_squares[MAX_SPRITES];
static int g_circles_initialized = 0;
static int g_squares_initialized = 0;

// Sprite bitmap settings, lazily updated on change.
static LCDBitmap *g_circle_bitmap = NULL;
static LCDBitmap *g_square_bitmap = NULL;
static int g_circle_bitmap_size = 0;
static int g_square_bitmap_size = 0;

// Initialize or update circle sprite.
static void UpdateCircleSprite(PlaydateAPI *pd)
{
   if( g_circle_bitmap_size == g_circle_size )
      return;
   g_circle_bitmap_size = g_circle_size;

   if( g_circle_bitmap != NULL )
      pd->graphics->freeBitmap(g_circle_bitmap);
   g_circle_bitmap = pd->graphics->newBitmap(
      g_circle_size, g_circle_size, kColorClear);
   pd->graphics->pushContext(g_circle_bitmap);
   const int r = g_circle_size / 2;
   const int limit = r * r;
   for(int y = 0; y < g_circle_size; y++)
   {
      const int y2 = (y - r) * (y - r);
      for(int x = 0; x < g_circle_size; x++)
      {
         // Using radius within the circle to shade the circle.
         //
         // The two inequalities here (r2<=limit, rand>r2/limit) are
         // chosen such that the outer edge gets assigned a black
         // pixel, even if g_circle_size is 1.
         const int r2 = (x - r) * (x - r) + y2;
         if( r2 <= limit )
         {
            const LCDColor color =
               rand() / (RAND_MAX + 1.0f) > (float)r2 / limit ? kColorWhite
                                                              : kColorBlack;
            pd->graphics->setPixel(x, y, color);
         }
      }
   }
   pd->graphics->popContext();
}

// Initialize or update square sprite.
static void UpdateSquareSprite(PlaydateAPI *pd)
{
   if( g_square_bitmap_size == g_square_size )
      return;
   g_square_bitmap_size = g_square_size;

   if( g_square_bitmap != NULL )
      pd->graphics->freeBitmap(g_square_bitmap);
   g_square_bitmap = pd->graphics->newBitmap(
      g_square_size, g_square_size, kColorClear);
   pd->graphics->pushContext(g_square_bitmap);
   pd->graphics->fillRect(0, 0, g_square_size, g_square_size, kColorBlack);
   pd->graphics->popContext();
}

// Return a random positive number.
static int RandomVelocity(void)
{
   return (int)((8.0f / RAND_MAX) * rand()) + 1;
}

// Return a random signed nonzero number.
static int RandomSignedVelocity(void)
{
   return rand() > RAND_MAX / 2 ? RandomVelocity() : -RandomVelocity();
}

// Initialize or update sprite positions.
static void AnimateSprite(Sprite *sprites, int *init_count, int count)
{
   // Set initial positions and directions.
   for(int i = *init_count; i < count; i++)
   {
      sprites[i].x = (int)((LCD_COLUMNS / (RAND_MAX + 1.0f)) * rand());
      sprites[i].y = (int)((LCD_ROWS / (RAND_MAX + 1.0f)) * rand());
      sprites[i].vx = RandomSignedVelocity();
      sprites[i].vy = RandomSignedVelocity();
   }
   *init_count = count;

   // Animate sprites.
   for(int i = 0; i < count; i++)
   {
      sprites[i].x += sprites[i].vx;
      sprites[i].y += sprites[i].vy;

      if( sprites[i].x < 0 )
      {
         sprites[i].vx = RandomVelocity();
      }
      else if( sprites[i].x >= LCD_COLUMNS )
      {
         sprites[i].vx = -RandomVelocity();
      }
      if( sprites[i].y < 0 )
      {
         sprites[i].vy = RandomVelocity();
      }
      else if( sprites[i].y >= LCD_ROWS )
      {
         sprites[i].vy = -RandomVelocity();
      }
   }
}

// Draw sprites.
static void DrawSprites(PlaydateAPI *pd)
{
   if( g_circle_count > 0 )
   {
      UpdateCircleSprite(pd);
      AnimateSprite(g_circles, &g_circles_initialized, g_circle_count);
      pd->graphics->setDrawMode(kDrawModeCopy);
      const int center = g_circle_size / 2;
      for(int i = 0; i < g_circle_count; i++)
      {
         pd->graphics->drawBitmap(
            g_circle_bitmap,
            g_circles[i].x - center,
            g_circles[i].y - center,
            kBitmapUnflipped);
      }
   }

   if( g_square_count > 0 )
   {
      UpdateSquareSprite(pd);
      AnimateSprite(g_squares, &g_squares_initialized, g_square_count);
      pd->graphics->setDrawMode(kDrawModeNXOR);
      const int center = g_square_size / 2;
      for(int i = 0; i < g_square_count; i++)
      {
         pd->graphics->drawBitmap(
            g_square_bitmap,
            g_squares[i].x - center,
            g_squares[i].y - center,
            kBitmapUnflipped);
      }
   }
}

// Draw frame rate and help text.
static void DrawStatus(PlaydateAPI *pd)
{
   const float fps = pd->display->getFPS();

   char *text = NULL;
   const int length = pd->system->formatString(
      &text,
      "FPS = %.1f\n"
      "circle: count = %d, size = %d\n"
      "square: count = %d, size = %d\n\n"
      /* Left */  "\u2b05 + crank: adjust circle count\n"
      /* Up */    "\u2b06 + crank: adjust circle size\n"
      /* Right */ "\u27a1 + crank: adjust square count\n"
      /* Down */  "\u2b07 + crank: adjust square size\n"
      /* A */     "\u24b6 + crank: adjust everything at once",
      (double)fps,
      g_circle_count, g_circle_size,
      g_square_count, g_square_size);

   pd->graphics->fillRect(0, 0, 256, 64, kColorWhite);
   pd->graphics->setDrawMode(kDrawModeNXOR);
   pd->graphics->drawText(text, length, kUTF8Encoding, 5, 5);
   pd->system->realloc(text, 0);
}

// Apply adjustment to a single parameter.
static void AdjustParam(int *param, int delta, int min, int max)
{
   *param += delta;
   if( *param < min ) { *param = min; }
   if( *param > max ) { *param = max; }
}

// Handle user input.
static void HandleInput(PlaydateAPI *pd, PDButtons buttons)
{
   if( (buttons & (kButtonA | kButtonB)) != 0 )
   {
      pd->graphics->fillRect(0, 165, LCD_COLUMNS, 20, kColorXOR);
      buttons |= kButtonLeft | kButtonRight | kButtonUp | kButtonDown;
   }
   const int delta = pd->system->getCrankChange();

   if( (buttons & kButtonLeft) != 0 )
   {
      pd->graphics->fillRect(0, 85, LCD_COLUMNS, 20, kColorXOR);
      AdjustParam(&g_circle_count, delta, 0, MAX_SPRITES);
   }
   if( (buttons & kButtonUp) != 0 )
   {
      pd->graphics->fillRect(0, 105, LCD_COLUMNS, 20, kColorXOR);
      AdjustParam(&g_circle_size, delta, 1, MAX_SPRITE_SIZE);
   }
   if( (buttons & kButtonRight) != 0 )
   {
      pd->graphics->fillRect(0, 125, LCD_COLUMNS, 20, kColorXOR);
      AdjustParam(&g_square_count, delta, 0, MAX_SPRITES);
   }
   if( (buttons & kButtonDown) != 0 )
   {
      pd->graphics->fillRect(0, 145, LCD_COLUMNS, 20, kColorXOR);
      AdjustParam(&g_square_size, delta, 1, MAX_SPRITE_SIZE);
   }
}

// Exported functions.
void SpriteBenchmark(PlaydateAPI *pd, PDButtons buttons)
{
   pd->graphics->clear(kColorWhite);
   DrawSprites(pd);
   DrawStatus(pd);
   HandleInput(pd, buttons);
   pd->graphics->markUpdatedRows(0, LCD_ROWS - 1);
}

void ResetSpriteBenchmark(void)
{
   g_circle_count = 0;
   g_circle_size = 8;
   g_square_count = 0;
   g_square_size = 8;
}
