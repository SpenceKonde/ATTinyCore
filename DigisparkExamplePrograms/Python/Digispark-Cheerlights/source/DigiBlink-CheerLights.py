import wx
import usb # 1.0 not 0.4
import time
import urllib2
import sys
import ConfigParser, os
from arduino.usbdevice import ArduinoUsbDevice

TRAY_TOOLTIP = 'Digispark CheerLights'
TRAY_ICON = 'icons/white.png'
TIMER_ID = 100

config = ConfigParser.RawConfigParser()
config.read('colors.cfg')
global red,green,blue,cyan,white,warmwhite,purple,magenta,yellow,orange,black
red = config.get('Colors', 'red').split(",")
green = config.get('Colors', 'green').split(",")
blue = config.get('Colors', 'blue').split(",")
cyan = config.get('Colors', 'cyan').split(",")
white = config.get('Colors', 'white').split(",")
warmwhite = config.get('Colors', 'warmwhite').split(",")
purple = config.get('Colors', 'purple').split(",")
magenta = config.get('Colors', 'magenta').split(",")
yellow = config.get('Colors', 'yellow').split(",")
orange = config.get('Colors', 'orange').split(",")
black = config.get('Colors', 'black').split(",")

def create_menu_item(menu, label, func):
    item = wx.MenuItem(menu, -1, label)
    menu.Bind(wx.EVT_MENU, func, id=item.GetId())
    menu.AppendItem(item)
    return item

class TaskBarIcon(wx.TaskBarIcon):
    def __init__(self):
        super(TaskBarIcon, self).__init__()
        self.set_icon(TRAY_ICON)
        self.Bind(wx.EVT_TASKBAR_LEFT_DOWN, self.on_left_down)
    def CreatePopupMenu(self):
        menu = wx.Menu()
        create_menu_item(menu, 'Update Now', self.on_update)
        menu.AppendSeparator()
        create_menu_item(menu, 'Exit', self.on_exit)
        return menu
    def set_icon(self, path):
        icon = wx.IconFromBitmap(wx.Bitmap(path))
        self.SetIcon(icon, TRAY_TOOLTIP)
    def on_left_down(self, event):
        pass
    def on_update(self, event):
        getNewColor()
    def on_exit(self, event):
        updateColor("black")
        global timer
        timer.Stop()
        wx.CallAfter(self.Destroy)


def on_timer(event):
    getNewColor()

def getNewColor():
    newColor = urllib2.urlopen("http://api.thingspeak.com/channels/1417/field/1/last.txt").read()
    updateColor(newColor)

def updateColor(setColor):
    global color
    if (color != setColor):
        color = setColor
        global taskbar_icon
        try:
            theDevice = ArduinoUsbDevice(idVendor=0x16c0, idProduct=0x05df)
            icon = color
            if (icon == "black"):
                icon = "white"
            taskbar_icon.set_icon("icons/"+icon+".png")
            #
            print "Color:"+color

            global red,green,blue,cyan,white,warmwhite,purple,magenta,yellow,orange,black
            if color == 'red':
                color_list = red
            elif color == 'green':
                color_list = green
            elif color == 'blue':
                color_list = blue
            elif color == 'cyan':
                color_list = cyan
            elif color == 'white':
                color_list = white
            elif color == 'warmwhite':
                color_list = warmwhite
            elif color == 'purple':
                color_list = purple
            elif color == 'magenta':
                color_list = magenta
            elif color == 'yellow':
                color_list = yellow
            elif color == 'orange':
                color_list = orange
            elif color == 'black':
                color_list = black





           # while 1:
                #pin = int(pin)



                #if output == "\r":
                #    print line
                #    line =""
                #else:
                #    line += output
                #else:
            
                #    print "Pin response didn't match."

            #byte val = sys.argv[1]
            print color_list
            theDevice.write(ord("s"))

            if color_list[0] == 0:
                theDevice.write(0)
            else:
                theDevice.write(int(color_list[0]))

            if color_list[1] == 0:
                theDevice.write(0)
            else:
                theDevice.write(int(color_list[1]))

            if color_list[2] == 0:
                theDevice.write(0)
            else:
                theDevice.write(int(color_list[2]))
        except:
            print "No device found"


def main():
    global color
    color = "black"
    app = wx.PySimpleApp()
    global taskbar_icon
    taskbar_icon = TaskBarIcon()
    getNewColor()
    global timer
    timer = wx.Timer(app, TIMER_ID)
    timer.Start(16000)
    wx.EVT_TIMER(app, TIMER_ID, on_timer)
    app.MainLoop()

if __name__ == '__main__':
    main()