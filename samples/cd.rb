require 'RUDL'

puts "This example will demonstrate the CDROM class.\n"
puts "CDROM is only meant to play audio tracks."
puts "We will ask CDROM for the amount of CD players and their names.\n"

cd_count=RUDL::CDROM.count
puts "There are #{cd_count} CD players available"

puts "Creating objects for all players..."
cd_players=[]
(0...cd_count).each do |cd_number|
	cd_player=RUDL::CDROM.new(cd_number)
	cd_players.push(cd_player)
end

puts "Information:"

cd_players.each do |cd_player|
	puts "CD player #{cd_player.name}"
	puts "	track#	start	size	type"
	(0...cd_player.num_tracks).each do |track_nr|
		info_line="	#{track_nr}"
		info_line+="	#{cd_player.track_start(track_nr).round}"
		info_line+="	#{cd_player.track_length(track_nr).round}"
		if(cd_player.audiotrack?(track_nr))
			info_line+="	audio (playing 2 secs)"
		else
			info_line+="	data"
		end
			
		puts info_line
		
		if(cd_player.audiotrack?(track_nr))
			cd_player.play(track_nr)
			RUDL::Timer.delay(2000)
		end
	end
end

puts "Done!"