
require "RUDL"
include RUDL
include Constant

if ARGV.size > 0
    picname = ARGV[0]
else
    picname = "mslug2-1.png"
end

pic = Surface.load_new(picname)
puts "#{pic.bitsize}-bit surface: #{picname} #{pic.w}x#{pic.h}"

# open window
win = DisplaySurface.new [40 + 2*pic.w, 40 + 2*pic.h]
win.fill [160,160,160]

# convert the picture to the same depth as the window
pic2 = Surface.new([pic.w,pic.h], win)
pic2.blit(pic, [0,0])

# scale2x it straight to the window, at position [20,20]
pic2.scale2x(win, [20,20])

win.update

loop do
    ev = EventQueue.wait
    break if ev.is_a? QuitEvent
end


