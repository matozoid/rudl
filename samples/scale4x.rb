
# The Scale4x algorithm is the same as Scale2x applied twice.
# This example demonstrates it, as well as scale2x'ing directly in
# to a window (or other surface). The destination surface must
# have the same depth, though.

require "RUDL"
include RUDL
include Constant

if ARGV.size > 0
    picname = ARGV[0]
else
    picname = "media/mslug2-1.png"
end

pic = Surface.load_new(picname)
puts "#{pic.bitsize}-bit surface: #{picname} #{pic.w}x#{pic.h}"

# open window
win = DisplaySurface.new [20 + 4*pic.w, 20 + 4*pic.h]
win.set_caption "Scale4x"
win.fill [160,160,160]

# convert the picture to the same depth as the window
pic2 = Surface.new([pic.w,pic.h], win)
pic2.blit(pic, [0,0])

# scale2x it
pic3 = pic2.scale2x

# scale2x that straight to the window, at position [10,10]
pic3.scale2x(win, [10,10])

win.update

loop do
    ev = EventQueue.wait
    break if ev.is_a? QuitEvent
end


