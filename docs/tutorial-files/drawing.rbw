
require "RUDL"
include RUDL
include Constant

win = DisplaySurface.new([260,200])
win.set_caption "RUDL gfx example"

win.fill [128,160,128]
win.fill [128,128,160], [20,20,220,160]

white = [255,255,255]
black = [0,0,0]

win.plot [30,30], white
win[40,40] = win[30,30]

win.line [70,60], [90,90], black
win.line [90,90], [120,100], black

win.rectangle [80,30,30,30], white
win.rectangle [90,50,10,3], white

surf = Surface.new([51,71], win)
surf.blit(win, [0,0], [70,30,51,71])
win.blit(surf.mirror_x, [121,30])

win.update

loop do
    event = EventQueue.wait
    break if event.is_a? QuitEvent
end

