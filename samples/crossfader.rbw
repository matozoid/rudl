=begin
@file Samples
@class Crossfader
Crossfading example for RUDL (5 Jan 2004)

Code and photos copyright © 2004 Renne Nissinen <rennex@iki.fi>

Simple way to crossfade:
<ul>
<li>blit pic2 to the window
<li>pic1.set_alpha the current opacity (1%, 2%, 3%, 4%, 5% ...)
<li>blit pic1 to the window
<li>repeat
</ul>

That way we have to do two blits every time.

This example shows a faster way to crossfade:
<ul>
<li>pic1.set_alpha a "delta opacity" (1/100, 1/99, 1/98, 1/97, 1/96 ...)
<li>blit pic1 to the window
<li>repeat
</ul>

This results in the same linear fade, with half the amount of blitting!
(though not quite half the cpu usage...)

Note that due to only 24-bit color accuracy, you might see color gradients
"crawl" during the fade, but only in some cases and if you look closely.

<code>
<pre>
require "RUDL"
include RUDL
include Constant

open a window
win = DisplaySurface.new [400,300]
win.set_caption "Crossfader - click in the window!"

load the pictures
pic1 = Surface.load_new("media/lake.jpg")
pic2 = Surface.load_new("media/sky.jpg")

time to wait between blits (in milliseconds, for Timer.delay)
delay = 20

steps = number of steps, 1000 = time for the fade (1 second)
steps = 1000/delay

n = current step (countdown to 0)
n = steps
</pre>
</code>
=end

loop do
    handle pending events
    EventQueue.get.each do |ev|
        case ev
            window closed
            when QuitEvent
                exit

            mouse click
            when MouseButtonDownEvent
                # we're not in the middle of a fade, are we?
                if n == 0
                    ok, start a new fade
                    n = steps
                    swap the pictures
                    pic1, pic2 = pic2, pic1
                end
        end
    end

    # are we fading currently?
    if n > 0
        pic1.set_alpha(255/n)
        win.blit(pic1, [0,0])
        win.update
        n -= 1
    end

    Timer.delay(delay)
end
