require "RUDL"
include RUDL
include Constant

win = DisplaySurface.new [300,200]

win.fill [0,0,0]

win.fill [0,255,0], [10,10,50,1]
win.fill [0,255,0], [10,20,1,50]

win.fill [0,255,0], [100,10,50,2]
win.fill [0,255,0], [100,20,2,50]

win.fill [0,255,0], [190,10,50,3]
win.fill [0,255,0], [190,20,3,50]



win.filled_rectangle [10,100,50,1], [255,255,0]
win.filled_rectangle [10,120,1,50], [255,255,0]

win.filled_rectangle [100,100,50,2], [255,255,0]
win.filled_rectangle [100,120,2,50], [255,255,0]

win.filled_rectangle [190,100,50,3], [255,255,0]
win.filled_rectangle [190,120,3,50], [255,255,0]


win.update

loop do
    ev = EventQueue.wait
    break if ev.is_a? QuitEvent or ev.is_a? MouseButtonDownEvent
end
