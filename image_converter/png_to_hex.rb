#!/usr/bin/env ruby
require "chunky_png"

input = ARGV.shift or fail "Specify a PNG filename as an argument"

canvas = ChunkyPNG::Image.from_file input

WIDTH = 64
HEIGHT = 48
fail "Image not #{WIDTH}x#{HEIGHT}" if canvas.width != WIDTH || canvas.height != HEIGHT

data = Array.new WIDTH * HEIGHT / 8, 0

data.each_with_index do |_, index|
  y, col = index.divmod WIDTH
  first_row = HEIGHT - y * 8 - 1
  0.upto(7).each do |bit|
    row = first_row - bit
    color = canvas[col, row]

    value = ChunkyPNG::Color.a(color).zero? ? 0 : 1
    data[index] |= value << bit
  end

  print ((col % WIDTH).zero? ? "\n" : "")
  print ((col % 16).zero? ? "\n" : " ")
  print "0x%02X," % data[index]
end
print "\n"

