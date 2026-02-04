# IRLib demo script
# version 1.0 by Chris Young http://tech.cyborg5.com/irlib/
# Displays a "Virtual remote" on your screen. Clicking on the
# buttons sends serial datato the Arduino which in turn
# since IR signals to a cable box in TV.

# Import all of the necessary pieces of code
import serial, sys, pygame, pygame.mixer
from pygame.locals import *

# You will have to edit this to the proper port and speed
ser = serial.Serial('COM4', 9600)

pygame.init()
# Established screen size, size of buttons and position
size = width, height = 400, 768
button_size=54; button_offset=71
button_x1=65;button_y1=39
max_rows=10; max_columns=4

# Specify a font. I'm using Arial narrow bold from my Windows
# font folder. However the default font shown below also works.
myfont =pygame.font.Font ("c:/windows/fonts/ARIALNB.TTF",30)
#myfont=pygame.font.Font(None,36)

# These are the text labels that will appear on each button
label_text=("TVp", "CBp", "P^", "Pv",\
           "<<", ">", ">>", "->",\
           "Rec", "=", "s", "<-",\
           "Gd", "^", "Fav", "Inf",\
            
          "<", "sel", ">", "Lis",\
          "ret", "v", "Prv", "Mnu",\
          "1", "2", "3", "Ch+",\
            
          "4", "5", "6", "Ch-",\
          "7", "8", "9", "Vol+",\
          "Pip", "0", "Mut", "Vol-",\
            )
# Each of these 40 strings of text correspond to the
# protocol in code which will be sent over the USB serial
# to the Arduino. The first number is the protocol number.
# See the defined protocols in "IRLib.h"for the
# enum IRTYPES at about line 50. This example uses
# protocol 3 which is "RC5" used by my Magnavox TV
# and protocol 5 "PANASONIC_OLD" used by my Scientific
# Atlantic SA 8300 DVR. The protocol number is followed by
# the hex code to be transmitted. That is followed by the
# number of bits. Note that the PANASONIC_OLD protocol
# does not need the number of bits specified so they are omitted.
IR_Codes= ("3,180c,13","5,37c107","5,36d924","5,37d904",\
           "5,37291a","5,37990c","5,36293a","5,36b129",\
           "5,375914","5,374117","5,365934","5,37c906",\
           "5,36c127","5,36812f","5,37f101","5,36213b",\
           
           "5,37810f","5,366133","5,364137","5,36c926",\
           "5,366932","5,37a10b","5,36e123","5,373918",\
           "5,36113d","5,37111d","5,36912d","5,377111",\
           
           "5,37910d","5,365135","5,375115","5,36f121",\
           "5,36d125","5,37d105","5,363139","3,1810,13",\
           "5,37b908","5,373119","3,180d,13","3,1811,13",\
           )
# This function gets called to shut everything down
def Finished():
    pygame.quit()
    sys.exit()

# Gets the button index based on mouse position. Returned
# value is from 0 to 39 (number of buttons-1)
# Returns -1 if you are not over a button.
def ComputeButton():
    mx,my=pygame.mouse.get_pos()
    mx=mx-button_x1
    my=my-button_y1
    bx=mx/button_offset;     by=my/button_offset
    if bx<0 or bx>=max_columns:return -1
    if by<0 or by> max_rows:return -1
    if (mx%button_offset)>button_size:return -1
    if (my%button_offset)>button_size:return -1
    return bx+by*max_columns

# Blits the button text from button number "i"
# onto the specified layer using the specified color.
def Show_Text(i,Layer,color=(0,0,0)):
    t=label_text[i]
    label = myfont.render (t,1,color)
    labelpos= label.get_rect()
    labelpos.centerx=button_x1+button_size/2+i%max_columns*button_offset
    labelpos.centery=button_y1+button_size/2+i/max_columns*button_offset
    Layer.blit(label,labelpos)

# Create the screen and load the background image.
screen = pygame.display.set_mode(size)
bg = pygame.image.load("remotebg.png")

# Blit black text labels onto the background image
for i in range (max_rows*max_columns):
    Show_Text(i, bg)
# Copy the background to the display
screen.blit(bg,(0,0))
pygame.display.flip()

# Load the clicking sound
Click=pygame.mixer.Sound("click.wav")

# Used to detect when the mouse hovers over a different button
previous=-1

while 1:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            Finished()
        elif event.type == KEYDOWN and event.key == K_ESCAPE:
            Finished ()
        elif event.type == MOUSEBUTTONDOWN:
            i=ComputeButton()  #which button did we click
            if i>=0:
                Click.play()  #play the sound
            ser.write(IR_Codes[i])  #send the codes
        elif event.type==MOUSEMOTION:
            i=ComputeButton() #which button are we over
            if i!=previous:   #difference in the last one?
                if i>=0:      #turn it red
                    Show_Text(i,screen,(255,0,0))
                else:         #or put it back the way it was
                    screen.blit(bg,(0,0))
                previous=i
                pygame.display.flip() #update the display
# That's all folks
            
        
                                    
