# The beginnings of a state engine
#
# Copyright (c) 2004 Brian Palmer, Pocket Martian Software
# Released under the ruby license, do whatever you want with it
# Just give credit where it's due
#
# Brainstorm: changeState doesn't immediately end the calling function --
# we have to make sure it's the last thing done inside the old state, or
# Very Bad Things might happen.  This has already bitten me once.
#
# Brainstorm: I'm very interested in the idea of inheritance in states.
# For instance, there might be 10 different states that all need to draw
# the main playfield.  What if there's a base state that handles drawing
# the playfield, doing the playfield physics, etc., but then other states
# inherit from that state?
# I need to mess with this idea, it would improve my code a lot.


def changeState(newState)
  $state::shutdown if $state
  $state = newState
  $state::init
end

def curState
    $state
end

# Allows calling any function on modules (or classes) that have safeCall in their definitions without worrying if it exists
# Basically a singleton version of method_missing, I didn't want it to affect all classes so I did it this way
class Module
  def safeCall
    module_eval "def #{self.name}.method_missing(n, *y) nil; end"
  end
end
