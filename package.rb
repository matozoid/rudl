puts 'This will create a RUDL package.'
puts 'It is only used by developers on the RUDL project.'

DESTINATION='rudl_packages'
VERSION='0.6'

filelist=[]
filelist+=Dir['**/*.{c,h,rb,rbw}']
filelist-=Dir[DESTINATION+'/**/*']

filelist.each