require 'RUDL'; include RUDL; include Constant

def sawtooth(frequency)
	s=''
	0.step(127, frequency) do |i|
		s+=i.chr
	end
	s
end

raw=File.open('c:/test.snd', 'rb').read

Mixer.init(44100, AUDIO_S16, 2)
(2..3).each do |i|
	s=Sound.import(Sound.convert(raw, [11025*i, AUDIO_S16, 2]))
	s.play(1000)
end

Timer.delay(10000)