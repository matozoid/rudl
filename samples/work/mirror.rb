
require "RUDL"
include RUDL
include Constant

picname = "mslug2-1.png"
depth = nil

# process args (filename, depth)
ARGV.each do |arg|
    if arg =~ /^\d+$/
        depth = arg.to_i
    else
        picname = arg
    end
end

# methods to mirror an image manually, with get/set_row and get/set_column
def manualmirror_y(pic)
    pic2 = Surface.new pic.size, pic
    h = pic.h
    h.times do |i|
        pic2.set_row(h-i-1, pic.get_row(i))
    end
    pic2
end

def manualmirror_x(pic)
    pic2 = Surface.new pic.size, pic
    w = pic.w
    w.times do |i|
        pic2.set_column(w-i-1, pic.get_column(i))
    end
    pic2
end

# load the pic
pic = Surface.load_new(picname)
pic = pic.scale2x.scale2x

if depth
    pic2 = Surface.new pic.size, 0, depth
    pic2.set_palette(0, pic.palette) if pic.bitsize == 8 and depth == 8
    pic2.blit pic, [0,0]
    pic = pic2
    puts "Converted to #{pic.bytesize} bytes per pixel"
else
    puts "Image has #{pic.bytesize} bytes per pixel"
end

# decide whether we put them left to right or top to bottom
if pic.w > 1.33*pic.h
    winsize = [pic.w + 20, pic.h*2 + 30]
    horiz = false
else
    winsize = [pic.w*2 + 30, pic.h + 20]
    horiz = true
end

# open window
win = DisplaySurface.new winsize
win.fill [160,160,160]

# mirror on both axis manually and with RUDL function
xpics = [pic.mirror_x, manualmirror_x(pic)]
ypics = [pic.mirror_y, manualmirror_y(pic)]
if xpics[0].pixels == xpics[1].pixels and ypics[0].pixels == ypics[1].pixels
    puts "Both versions equal!"
else
    puts "Oh God please no!"
end

which = 0
time = 0

loop do
    # every 0.5 seconds:
    if time == 5
        time = 0
        which ^= 1  # switch which!

        # blit the x mirrored
        win.blit xpics[which], [10,10]

        # blit the y mirrored
        win.blit ypics[which], if horiz then [20+pic.w,10] else [10,20+pic.h] end

        # clear old number & print this number
        win.fill [160,160,160], [0,0,8,8]
        win.print [0,0], which.to_s, [0,0,0]

        win.update
    end

    Timer.delay 100
    time += 1

    EventQueue.get.each do |ev|
        exit if ev.is_a? QuitEvent or ev.is_a? MouseButtonDownEvent or ev.is_a? KeyDownEvent
    end
end





