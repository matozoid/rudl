require "RUDL"
include RUDL
include Constant

win = DisplaySurface.new([300,200])
win.fill [128,128,128]

ball = Surface.load_new("ball.gif")
win.blit(ball, [90,70])

ball.set_colorkey [255,255,255]
win.blit(ball, [160,70])

ball.set_alpha 128
win.blit(ball, [125,100])

win.update

loop do
    event = EventQueue.wait
    break if event.is_a? QuitEvent
end
