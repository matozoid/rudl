require "RUDL"
include RUDL
include Constant

win = DisplaySurface.new([300,200])
win.fill [100,100,100]                      # dark
win.fill [225,225,225], [150,0,150,200]     # bright
win.fill [255,100,100], [100,110,100,22]    # red
win.fill [255,255,100], [100,132,100,22]    # yellow

ball = Surface.load_new("ball.png")
win.blit(ball, [83,40])
win.blit(ball, [153,40])
win.blit(ball, [118,100])
win.update

loop do
    event = EventQueue.wait
    break if event.is_a? QuitEvent
end
