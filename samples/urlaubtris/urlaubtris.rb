# TODO: 
# - fix "too many players. too little defined keys" crash
# - high score list (also save number of players/game size of score)
# - cleanup
# - multiple config files (?)
# - sound effects ?
# - icon!

include Math
require 'RUDL'
include RUDL
require 'YAML'
include Constant

require "utils.rb"
require "interface.rb"
include UI

BW,BH=16,16
MH=68
MUSIC=false
DEBUG=false


def debug(*args)
	puts args if DEBUG
end
$VERBOSE=DEBUG


class Generatorcollection < Array
	attr_accessor :keys

	def initialize(numplayers,gw,lvl,keys)
		@keys=keys
		numplayers.times { |i|
			self[i]=Blockgenerator.new(((gw/(numplayers+1)).floor*(i+1)-2)*BW,0)
		}
		@activeblocks=Array.new(numplayers)
		@lvl=lvl
	end
	def draw_to(img)
		each_with_index { |b,i| 
			b.activeblock.draw_to(img,"P#{i+1}")
			b.draw_to(img)
		}
	end
	def makeblock(gnum=false)
		if gnum then self[num].makeblock
		else self.each { |b| b.makeblock } end
	end
	def move(dir)
		each { |b| b.activeblock.move(dir) } 
	end
	def keypress(key)
		each_with_index { |bg,i|
			debug "IO iteration #{i}:"
			p @keys if DEBUG
			if key==@keys[i][0] then bg.activeblock.rotateleft end
			if key==@keys[i][1] then bg.activeblock.rotateright end
			if key==@keys[i][2] then bg.activeblock.move([-BW,0]) end
			if key==@keys[i][3] then bg.activeblock.move([BW,0]) end
			if key==@keys[i][4]
				g=0
				while !@lvl.testblock(bg.activeblock)
					bg.activeblock.move([0,BH])
					g+=1
				end
				$game.score_move(g)
			end
			if @lvl.testblock(bg.activeblock) then bg.activeblock.undo() end
		}
	end
	def testblocks()
		each { |b| 
			if @lvl.testblock(b.activeblock) 
				b.activeblock.undo()
				if b.activeblock.x == b.x && b.activeblock.y == b.y
					delete(b)
					$game.over() if length==0
				end
				@lvl.addblock(b.activeblock)
				b.activeblock.each_row { |y|
					@lvl.rowcad(b.activeblock.y/BH+y)
				}
				b.makeblock
			end
		}
		$game.score_lines()
	end
end

class Blockgenerator
	attr_accessor :activeblock
	attr_reader :x,:y

	def initialize(x,y)
		@x,@y=x,y
		@nextsurf=Surface.new([4*PW,4*PH],SWSURFACE).convert()
		randnext()
	end
	PW,PH=6,6
	NUMTYPES= 7
	BLOCKTYPES=	[[[[ 0,0,0,0 ],
							[ 0,1,1,0 ],
							[ 0,1,1,0 ],
							[ 0,0,0,0 ]] , [255,0,0] ],
						 [[[ 0,1,0,0 ],
							[ 0,1,0,0 ],
							[ 0,1,0,0 ],
							[ 0,1,0,0 ]] , [0,255,0] ],
						 [[[ 0,0,0,0 ],
							[ 0,1,0,0 ],
							[ 1,1,1,0 ],
							[ 0,0,0,0 ]] , [0,0,255] ],
						 [[[ 0,0,0,0 ],
							[ 0,1,1,0 ],
							[ 1,1,0,0 ],
							[ 0,0,0,0 ]] , [150,150,0] ],							
						 [[[ 0,0,0,0 ],
							[ 0,1,1,0 ],
							[ 0,0,1,1 ],
							[ 0,0,0,0 ]] , [0,150,150] ],
						 [[[ 0,0,0,0 ],
							[ 0,0,1,0 ],
							[ 1,1,1,0 ],
							[ 0,0,0,0 ]] , [150,0,150] ],
						 [[[ 0,0,0,0 ],
							[ 1,0,0,0 ],
							[ 1,1,1,0 ],
							[ 0,0,0,0 ]] , [150,150,150] ]]
	
	def randnext()
		r=rand(NUMTYPES)
		@nextblock=Tetrisblock.new(@x,@y,BLOCKTYPES[r][1],
											  BLOCKTYPES[r][0] )
		@nextsurf.fill([0,0,0])
		@nextblock.each_coordinate { |x,y|
			@nextsurf.fill(@nextblock.color,[x*PW,y*PH,PW,PH])
		}
	end
	
	def draw_to(bmp)
		bmp.print([@x,bmp.h-PH*4-12],"Next :",[200,200,200])
		bmp.blit(@nextsurf,[@x+BW,bmp.h-PH*4])		
	end
	
	def makeblock()
		@activeblock = @nextblock
		randnext()
	end
end

module Blockstructure
	def [](x,y)
		@blocks[x][y]
	end

	def to_s()
		s="\n"
		@blocks.each { |r|
			s+="["
			r.each_with_index { |b,i|
				s+=b.to_s
				s+="," if i!=r.length-1
			}
			s+="]\n"
		}
		s
	end
	
	def each_coordinate()
		@blocks.each_index { |x|
			@blocks[x].each_index { |y|
				yield x,y if @blocks[x][y]!=0
			}
		}
	end
	
	def each_row()
		yielded=[]
		@blocks.each_index { |x|
			@blocks[x].each_with_index { |b,y| 
				if b!=0
					debug "\n->each_row - y:#{y} - x:#{x} - b:#{b}"
					yield y unless yielded.has?(y)
					yielded+=[y]
					debug yielded
				end 
			}
		}
	end
end

class Tetrisblock
	include Blockstructure
	attr_reader :x,:y,:blocks,:color
	
	def initialize(x,y,color,blockshape)
		@x,@y,@color=x,y,color
		@blocks=blockshape
		@undo = proc { move([0,0]) }
	end
		
	def modify()
		oldblocks=@blocks.duplicate()
		@blocks=oldblocks.duplicate()
		@blocks.each_index { |x|
			@blocks[x].each_index { |y|
				yield x,y,oldblocks
			}
		}
	end
	
	def draw_to(bmp,str)
		printed=false
		each_coordinate { |x,y|
			if !printed
					bmp.print([@x+x*BW,@y+y*BH-8],str,[180,180,180])
					printed=true
			end
			bmp.fill(@color,[@x+x*BW,@y+y*BH,BW,BH])
		}
	end
	
	def rotateleft()
		@undo=proc { rotateright() }
		modify { |x,y,o| @blocks[x][y]=o[3-y][x] }
	end
	
	def rotateright()
		@undo=proc { rotateleft() }
		modify { |x,y,o| @blocks[x][y]=o[y][3-x] }
	end
	
	def move(dir)
		@undo=proc { move([-dir[0],-dir[1]]) }
		@x+=dir[0]
		@y+=dir[1]
	end
	
	def undo()
		@undo.call()
	end
end
	
class Tetrislevel
	include Blockstructure

	def initialize(gw,gh)
		@gw,@gh=gw,gh
		@blocks= Array.new(gw)
		@blocks.each_index { |i|
			@blocks[i]=Array.new(gh,0)
		}
		debug self
	end
	
	def testblock(bl)
		bxb=bl.x/BW
		byb=bl.y/BH
		bl.each_coordinate { |x,y|
			if ( bxb+x<0 || !defined?(@blocks[bxb+x][byb+y]) || 
					@blocks[bxb+x][byb+y]!=0) 
				return true 
			end			
		}
		false
	end
	
	def addblock(bl)
		bxb=bl.x/BW
		byb=bl.y/BH
		bl.each_coordinate { |x,y|
			@blocks[bxb+x][byb+y] = bl.color
		} 
	end
	
	def draw_to(bmp)
		each_coordinate { |x,y|
			bmp.fill(@blocks[x][y] || 0,[x*BW,y*BH,BW,BH])
			bmp.rectangle([x*BW,y*BH,BW,BH], @blocks[x][y].collect{|v| v/1.5} || 0) 
		}
	end
	
	def rowcad(row)
		debug "\n\nRow: #{row}\nrow: #{@blocks[row]}\n     "
		@blocks.each_index { |i|
			debug "INDEX=#{i}\n"
			debug "@blocks[i][row]=#{@blocks[i][row]}\n"
			if @blocks[i][row]==0
				debug "Problemstelle: #{i}"
				return false 
			end 
		}
		debug "\n","REIHE VOLL!\n"*5,"\n"
		$game.scorelines+=1
		@blocks.each_index { |i| 
			@blocks[i].delete_at(row)
			@blocks[i][0..1]=[0,0,0]
		}
	end
end

class Tetrisgame
	ID_MOVE = 50
	SCOREINT=700
	MAXPLAYERS=8
	SIZESTEP=4
	MINSIZE=6
	MAXSIZE=60
	
	def adjust_val(val,k,lh,rh,step=1)
		val-=step if k==K_LEFT
		val+=step if k==K_RIGHT
		if val<lh then val=rh
		else if val>rh then val=lh end end
		val
	end
	
	def initialize(gw=12,gh=20,numplayers=1,interval=600)
		@gw,@gh,@startinterval,@numplayers=gw,gh,interval,numplayers
		@interval=interval
		@score,@scorelines=0,0
		
		@keys=[ [K_a,K_s,K_d,K_g,K_f],
				[K_k,K_l,K_LEFT,K_RIGHT,K_DOWN],
				[K_a,K_u,K_LEFT,K_END,K_4],
				[K_s,K_i,K_RIGHT,K_PAGEDOWN,K_5],
				[K_z,K_j,K_DOWN,K_PAGEUP,K_6] ]
		
		@Display=DisplaySurface.new([300,300], DOUBLEBUF|NOFRAME)
		@Display.set_caption("Urlaubstris by PT - v0.1")
		@Display.fill([0, 0, 0])	
		
		@interface=Interface.new()
		@inmenu=true
		@ingame=false
			
		playerproc = proc { |t,m,k|
			@numplayers=adjust_val(@numplayers,k,1,MAXPLAYERS)
			t.text="Players: #{@numplayers}"
		}
		gwproc =  proc { |t,m,k|
			@gw=adjust_val(@gw,k,MINSIZE,MAXSIZE,SIZESTEP)
			t.text="Game width: #{@gw}"
		}
		ghproc =  proc { |t,m,k|
			@gh=adjust_val(@gh,k,MINSIZE,MAXSIZE,SIZESTEP)
			t.text="Game height: #{@gh}"
		}
		keyconfproc = proc { |t,m,k|
			return unless k==K_RETURN #lol
			desc=["ROTATE LEFT","ROTATE RIGHT",
				"MOVE LEFT","MOVE RIGHT","MOVE DOWN"]
			@keys = Array.new(@numplayers) { [] }
			@keys.each_with_index { |kk,p_ind|
				desc.each { |des|
					@interface.add(Keyprompt.new(@Display,@Display.rect,
									proc { |k,b| kk.push(k); @interface.remove(b) },
									"Enter key for","Player #{p_ind+1}",des))
					while @interface.modal?
						runonce()
					end
					debug "Key set: #{kk[-1]}  -  Player #{p_ind+1}/#{des}"
				}
			}
		}
		savesetproc = proc { |t,m,k|
			return unless k==K_RETURN
			curset = {"Players" => @numplayers, 
						  "GameWidth" => @gw,
						  "GameHeight" => @gh, 
						  "Keys" => @keys}
			File.open("settings.cfg","w+b") { |f|
				f.print curset.to_yaml
			}
		}
		loadsetproc = proc { |t,m,k|
			return unless k==K_RETURN 
			if(File.exists?("settings.cfg"))
				newset = YAML::load(File.open("settings.cfg"))
				@numplayers=newset["Players"] 
				@gw=newset["GameWidth"]; @gh=newset["GameHeight"]
				@keys = newset["Keys"]
			else
				@interface.add(Keyprompt.new(@Display,@Display.rect,
									proc { |k,b| @interface.remove(b) },
									"File \"settings.cfg\" not found!",
									"Save some settings first."))
			end
		}
		
		@interface.add(Menu.new(@Display,@Display.rect,
				["New Game", proc { |t,m,k| @interface.remove(m); start; @inmenu=false }],
				[proc {"Players: #{@numplayers}"}, playerproc],
				["Configure Keys", keyconfproc],
				[proc {"Game width: #{@gw}"}, gwproc],
				[proc {"Game height: #{@gh}"}, ghproc],
				["Save Settings", savesetproc],
				["Load Settings", loadsetproc],
				["Quit", proc { exit }] ) )
		 
	end
	
	attr_reader :score
	attr_accessor :scorelines
	def set_score(val)
		debug "SCHANGE     "*10
		@score=val
		set_interval(@startinterval-@score/100)
	end
	def score_lines()
		set_score(@score+(SCOREINT-@interval)*(@scorelines*@scorelines)) if @scorelines>0
		@scorelines=0
	end
	def score_move(dist)
		set_score(@score+((SCOREINT-@interval)/100)*dist+2*dist)
	end
	
	def over()
		@interface.add(Keyprompt.new(@Display,@Display.rect,
			proc { |k,b| @interface.remove(b) }, " G A M E "," O V E R "))
		$game=Tetrisgame.new()
		$game.run()
	end
	
	attr_reader :interval
	def set_interval(i)
		debug "ICHANGE     "*10
		@interval=i
		@movetimer=EventTimer.new(@interval,ID_MOVE)		
	end
	
	def start()
		@Display.destroy
		@Display=DisplaySurface.new([@gw*BW,@gh*BH+MH], DOUBLEBUF)
		@lvl=Tetrislevel.new(@gw,@gh)
		@generators=Generatorcollection.new(@numplayers,@gw,@lvl,@keys)
		
		@interface.add(Hline.new(@Display,[0,@gh*BH,@Display.w,4]))
		@interface.add(Text.new(@Display,[2,@gh*BH+7],proc { "Score: #{@score}" } ))

		if MUSIC
			Mixer.init(11025,16,1)
			@music=Music.new("audio/music.wav")
		end

		@movetimer=EventTimer.new(@interval,ID_MOVE)
		@generators.makeblock()
		@music.play(256) if MUSIC
		
		@ingame=true
		@inmenu=false
	end
	
	def run()
		while(true)
			runonce()
		end
	end
	def runonce()
		@Display.fill([0, 0, 0])
		if @ingame
			@generators.draw_to(@Display)
			@lvl.draw_to(@Display)
		end
		@interface.draw()
		@Display.flip()
		
		event=EventQueue.poll
		if event.class==KeyDownEvent
			if event.key==K_ESCAPE then exit
			else 
				@generators.keypress(event.key) if @ingame
				@interface.keypress(event.key) if @inmenu
			end
		end
		if event.class==TimerEvent && event.id==ID_MOVE && !@inmenu
			@generators.move([0,BH])
			@generators.testblocks()
		end	
	end
end

$game=Tetrisgame.new()
$game.run()