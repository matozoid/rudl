# Converts X11 color list (copied from somewhere, appended to the
# data section of this source file) into colors.rb

# 25.12.2003 - Fixed & beautified by rennex. Merry Christmas. I got a watch :)


f = File.open('colors.rb', 'w')

# print header
f.print <<ENNND
# This is the X11 color list converted to r,g,b arrays
# Access it with RUDL::Constant::colors["color_name"]

module RUDL
    module Constant
        @@Colors = {
ENNND

# scan and process name,value pairs
DATA.read.scan(/(\w+?)\r?\n(\#[0-9a-f]{6})/i) do |name,value|
    r = value[1..2].hex
    g = value[3..4].hex
    b = value[5..6].hex

    name[0,1] = name[0,1].downcase
    name.gsub!(/[A-Z]/) {|c| "_" + c.downcase }
    f.puts "            \"#{name}\" => [#{r}, #{g}, #{b}],"
end

# print footer
f.print <<ENNND
        }
        def self.colors
            @@Colors
        end
    end
end
ENNND

__END__
LightPink
#FFB6C1

Pink
#FFC0CB

Crimson
#DC143C

LavenderBlush
#FFF0F5

PaleVioletRed
#DB7093

HotPink
#FF69B4

DeepPink
#FF1493

MediumVioletRed
#C71585

Orchid
#DA70D6

Thistle
#D8BFD8

Plum
#DDA0DD

Violet
#EE82EE

Magenta
#FF00FF

Fuchsia
#FF00FF

DarkMagenta
#8B008B

Purple
#800080

MediumOrchid
#BA55D3

DarkViolet
#9400D3

DarkOrchid
#9932CC

Indigo
#4B0082

BlueViolet
#8A2BE2

MediumPurple
#9370DB

MediumSlateBlue
#7B68EE

SlateBlue
#6A5ACD

DarkSlateBlue
#483D8B

Lavender
#E6E6FA

GhostWhite
#F8F8FF

Blue
#0000FF

MediumBlue
#0000CD

MidnightBlue
#191970

DarkBlue
#00008B

Navy
#000080

RoyalBlue
#4169E1

CornflowerBlue
#6495ED

LightSteelBlue
#B0C4DE

LightSlateGray
#778899

SlateGray
#708090

DodgerBlue
#1E90FF

AliceBlue
#F0F8FF

SteelBlue
#4682B4

LightSkyBlue
#87CEFA

SkyBlue
#87CEEB

DeepSkyBlue
#00BFFF

LightBlue
#ADD8E6

PowderBlue
#B0E0E6

CadetBlue
#5F9EA0

Azure
#F0FFFF

LightCyan
#E0FFFF

PaleTurquoise
#AFEEEE

Cyan
#00FFFF

Aqua
#00FFFF

DarkTurquoise
#00CED1

DarkSlateGray
#2F4F4F

DarkCyan
#008B8B

Teal
#008080

MediumTurquoise
#48D1CC

LightSeaGreen
#20B2AA

Turquoise
#40E0D0

Aquamarine
#7FFFD4

MediumAquamarine
#66CDAA

MediumSpringGreen
#00FA9A

MintCream
#F5FFFA

SpringGreen
#00FF7F

MediumSeaGreen
#3CB371

SeaGreen
#2E8B57

Honeydew
#F0FFF0

LightGreen
#90EE90

PaleGreen
#98FB98

DarkSeaGreen
#8FBC8F

LimeGreen
#32CD32

Lime
#00FF00

ForestGreen
#228B22

Green
#008000

DarkGreen
#006400

Chartreuse
#7FFF00

LawnGreen
#7CFC00

GreenYellow
#ADFF2F

DarkOliveGreen
#556B2F

YellowGreen
#9ACD32

OliveDrab
#6B8E23

Beige
#F5F5DC

LightGoldenrodYellow
#FAFAD2

Ivory
#FFFFF0

LightYellow
#FFFFE0

Yellow
#FFFF00

Olive
#808000

DarkKhaki
#BDB76B

LemonChiffon
#FFFACD

PaleGoldenrod
#EEE8AA

Khaki
#F0E68C

Gold
#FFD700

Cornsilk
#FFF8DC

Goldenrod
#DAA520

DarkGoldenrod
#B8860B

FloralWhite
#FFFAF0

OldLace
#FDF5E6

Wheat
#F5DEB3

Moccasin
#FFE4B5

Orange
#FFA500

PapayaWhip
#FFEFD5

BlanchedAlmond
#FFEBCD

NavajoWhite
#FFDEAD

AntiqueWhite
#FAEBD7

Tan
#D2B48C

BurlyWood
#DEB887

Bisque
#FFE4C4

DarkOrange
#FF8C00

Linen
#FAF0E6

Peru
#CD853F

PeachPuff
#FFDAB9

SandyBrown
#F4A460

Chocolate
#D2691E

SaddleBrown
#8B4513

Seashell
#FFF5EE

Sienna
#A0522D

LightSalmon
#FFA07A

Coral
#FF7F50

OrangeRed
#FF4500

DarkSalmon
#E9967A

Tomato
#FF6347

MistyRose
#FFE4E1

Salmon
#FA8072

Snow
#FFFAFA

LightCoral
#F08080

RosyBrown
#BC8F8F

IndianRed
#CD5C5C

Red
#FF0000

Brown
#A52A2A

FireBrick
#B22222

DarkRed
#8B0000

Maroon
#800000

White
#FFFFFF

WhiteSmoke
#F5F5F5

Gainsboro
#DCDCDC

LightGrey
#D3D3D3

Silver
#C0C0C0

DarkGray
#A9A9A9

Gray
#808080

DimGray
#696969

Black
#000000



