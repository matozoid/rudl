
# test 4.8 million pixel writes or reads
# remove "win[x,y] = c" to benchmark empty "times" loop
# remove " = c" to benchmark pixel reads (which creates [r,g,b,a] arrays)

require "RUDL"
include RUDL
include Constant

win = DisplaySurface.new([300,200])

# create variables in advance
c = x = y = nil
start = now = totaltime = 0

10.times do
    for c in [[128,128,128],[128,128,160],[128,160,128],[128,160,160],[160,128,128],[160,128,160],[160,160,128],[160,160,160]] do
        start = Timer.ticks
        win.h.times do |y|
            win.w.times do |x|
                win[x,y] = c
            end
        end
        now = Timer.ticks
        totaltime += now - start
        win.update
        EventQueue.get.each do |ev|
            exit if ev.is_a? QuitEvent
        end
    end
end

win.print [5,5], "4.8 million pixels, #{totaltime} ms", [255,255,255]
win.update

loop do
    ev = EventQueue.wait
    break if ev.is_a? QuitEvent
end
