
# This example demonstrates the Scale2x algorithm, which was
# added in RUDL 0.8. It displays the original image, a 2x scaled,
# 3x scaled and 4x scaled version side by side.

# Note that the Scale3x algorithm is missing from RUDL, and it
# doesn't even look very good. Therefore this example scales
# down the 4x image by 75%, which looks pretty good :)
# (Especially if the image isn't 8-bit, then it'll be antialiased)

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
win = DisplaySurface.new [50 + 10*pic.w, 20 + 4*pic.h]
win.set_caption "Scale2x, 3x and 4x"
win.fill [160,160,160]

# blit the original
win.blit pic, [10,10]

# blit the 2x scaled
doublepic = pic.scale2x
win.blit doublepic, [20+pic.w,10]

# blit the 4x scaled
quadpic = doublepic.scale2x
win.blit quadpic, [40+6*pic.w,10]

# blit the 3x scaled
# since there's no scale3x, reduce the 4x one to 75% size :)
triplepic = quadpic.zoom(0.75, 0.75, true)
triplepic.unset_colorkey    # naughty zoom! sets a colorkey...
win.blit triplepic, [30+3*pic.w,10]
win.print([30+3*pic.w, 15+3*pic.h], "(pseudo 3x)", [0,0,0]) if pic.w*3 >= 78

win.update

loop do
    ev = EventQueue.wait
    break if ev.is_a? QuitEvent
end


