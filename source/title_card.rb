#!/usr/bin/ruby -w
# Draw ruler image.

require 'png'

if ARGV.length != 1 then
   print "#{$0} {output.png}\n"
   exit 1
end

# Screen size in pixels.
WIDTH = 350
HEIGHT = 155

# Distance between pixels in millimeters.
#
# These numbers are from LS027B7DH01A datasheet:
# Viewing area = 58.8mm x 35.28mm
# Dot pitch = 0.147mm x 0.147mm
DOT_PITCH = 0.147

# Conversion factor from millimeters to 1/8 of one inch.
MM_TO_8TH_IN = (1 / 25.4) * 8

# Digit bitmaps.
DIGITS =
[
   [" ### ",
    "#   #",
    "#   #",
    "#   #",
    " ### "],

   [" ##  ",
    "  #  ",
    "  #  ",
    "  #  ",
    " ### "],

   [" ### ",
    "    #",
    " ### ",
    "#    ",
    "#####"],

   ["#### ",
    "    #",
    " ### ",
    "    #",
    "#### "],

   ["  ## ",
    " # # ",
    "#  # ",
    "#####",
    "   # "],

   ["#####",
    "#    ",
    "#### ",
    "    #",
    "#### "],
]

# Draw a single digit at some offset.
def draw_digit(pixels, x, y, d)
   image = DIGITS[d]
   image.size.times{|i|
      image[i].size.times{|j|
         if image[i][j] == '#' then
            pixels[(y + i) * WIDTH + x + j] = 0
         end
      }
   }
end


# Create blank canvas filled with white pixels.
pixels = Array.new(WIDTH * HEIGHT, 0xff)

# Draw metric ruler on the top edge.
last_length = -1
WIDTH.times{|x|
   float_length = x * DOT_PITCH
   length = float_length.floor

   # Add new tick mark when we move on to a new millimeter.
   if last_length != length then
      tick_height = (1 / DOT_PITCH).round
      if length % 10 == 0 then
         tick_height = (2 / DOT_PITCH).round
         if length > 0 then
            draw_digit(pixels, x - 7, tick_height - 5, length / 10)
         end
      elsif length % 5 == 0 then
         tick_height = (1.6 / DOT_PITCH).round
      end

      tick_height.times{|y|
         pixels[y * WIDTH + x] = 0
      }
   end

   last_length = length
}

# Draw imperial ruler on the bottom edge.
last_length = -1
WIDTH.times{|x|
   float_length = x * DOT_PITCH * MM_TO_8TH_IN
   length = float_length.floor

   # Add new tick mark when we move on to a new 1/8 of one inch.
   if last_length != length then
      tick_height = (1 / DOT_PITCH).round
      if length % 8 == 0 then
         tick_height = (2 / DOT_PITCH).round
         if length > 0 then
            draw_digit(pixels, x - 7, HEIGHT - tick_height, length / 8)
         end
      elsif length % 4 == 0 then
         tick_height = (1.6 / DOT_PITCH).round
      end

      tick_height.times{|y|
         pixels[(HEIGHT - 1 - y) * WIDTH + x] = 0
      }
   end

   last_length = length
}

# Write output image.
output = PNG::Encoder.new(WIDTH, HEIGHT, :pixel_format => :GRAY)
IO.binwrite(ARGV[0], output << pixels.pack("C*"))
