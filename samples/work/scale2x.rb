
require "RUDL"
include RUDL
include Constant

if ARGV.size > 0
    picname = ARGV[0]
else
    picname = "mslug2-1.png"
end

origpic = Surface.load_new(picname)
puts "#{origpic.bitsize}-bit surface: #{picname} #{origpic.w}x#{origpic.h}"

# if it's 24-bit, which isn't supported yet, then turn it to 32-bit.
if origpic.bytesize == 3
    pic = Surface.new [origpic.w,origpic.h], SWSURFACE, 32
    pic.blit origpic, [0,0]
else
    pic = origpic
end

# open window
win = DisplaySurface.new [50 + 10*pic.w, 20 + 4*pic.h]
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
# since there's no scale3x, reduce the 4x one to 75% size B)
triplepic = quadpic.zoom(0.75, 0.75, true)
win.blit triplepic, [30+3*pic.w,10]
win.print [30+3*pic.w, 15+3*pic.h], "(pseudo 3x)", [0,0,0]

win.update

loop do
    ev = EventQueue.wait
    break if ev.is_a? QuitEvent
end


