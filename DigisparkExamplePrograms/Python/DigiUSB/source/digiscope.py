"""

Modified to create Digiscope by Erik Kettenburg, Digistump LLC - modifications udner public domain 

--Original text:
This demo demonstrates how to draw a dynamic mpl (matplotlib) 
plot in a wxPython application.

It allows "live" plotting as well as manual zooming to specific
regions.

Both X and Y axes allow "auto" or "manual" settings. For Y, auto
mode sets the scaling of the graph to see all the data points.
For X, auto mode makes the graph "follow" the data. Set it X min
to manual 0 to always see the whole data from the beginning.

Note: press Enter in the 'manual' text box to make a new value 
affect the plot.

Eli Bendersky (eliben@gmail.com)
License: this code is in the public domain
Last modified: 31.07.2008
"""
import os
import pprint
import random
import sys
import wx


from arduino.usbdevice import ArduinoUsbDevice





# The recommended way to use wx with mpl is with the WXAgg
# backend. 
#
import matplotlib
matplotlib.use('WXAgg')
from matplotlib.figure import Figure
from matplotlib.backends.backend_wxagg import \
    FigureCanvasWxAgg as FigCanvas, \
    NavigationToolbar2WxAgg as NavigationToolbar
import numpy as np
import pylab


class GraphFrame(wx.Frame):
    """ The main frame of the application
    """
    title = 'DigiScope'
    
    def __init__(self):
        wx.Frame.__init__(self, None, -1, self.title)
        

        self.data = [0]
        self.paused = False
        self.redraw = False
        
        self.create_menu()
        self.create_status_bar()
        self.create_main_panel()
        
        self.redraw_timer = wx.Timer(self)
        self.data_timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER, self.on_redraw_timer, self.redraw_timer)        
        self.Bind(wx.EVT_TIMER, self.on_data_timer, self.data_timer)        
        self.redraw_timer.Start(40)
        self.data_timer.Start(20)

    def create_menu(self):
        self.menubar = wx.MenuBar()
        
        menu_file = wx.Menu()
        m_expt = menu_file.Append(-1, "&Save plot\tCtrl-S", "Save plot to file")
        self.Bind(wx.EVT_MENU, self.on_save_plot, m_expt)
        menu_file.AppendSeparator()
        m_exit = menu_file.Append(-1, "E&xit\tCtrl-X", "Exit")
        self.Bind(wx.EVT_MENU, self.on_exit, m_exit)
                
        self.menubar.Append(menu_file, "&File")
        self.SetMenuBar(self.menubar)

    def create_main_panel(self):
        self.panel = wx.Panel(self)

        self.init_plot()
        self.canvas = FigCanvas(self.panel, -1, self.fig)

        self.pause_button = wx.Button(self.panel, -1, "Pause")
        self.Bind(wx.EVT_BUTTON, self.on_pause_button, self.pause_button)
        self.Bind(wx.EVT_UPDATE_UI, self.on_update_pause_button, self.pause_button)
        

        
        self.hbox1 = wx.BoxSizer(wx.HORIZONTAL)
       

        self.volts = wx.StaticText(self.panel, -1, "0v", style=wx.ALIGN_CENTER)
        self.volts.SetForegroundColour((255,0,0))
        font = wx.Font(24, wx.SWISS, wx.NORMAL, wx.BOLD)
        self.volts.SetFont(font)

        self.hbox1.Add(self.volts, border=5, flag=wx.ALL | wx.ALIGN_CENTER_VERTICAL)
        self.hbox1.AddSpacer([100,1])
        self.hbox1.Add(self.pause_button, border=5, flag=wx.ALL | wx.ALIGN_CENTER_VERTICAL)

        self.vbox = wx.BoxSizer(wx.VERTICAL)
        self.vbox.Add(self.canvas, 0, border = 0, flag=wx.EXPAND)        
        self.vbox.Add(self.hbox1, 0)
        
        self.panel.SetSizer(self.vbox)
        self.vbox.Fit(self)
    
    def create_status_bar(self):
        self.statusbar = self.CreateStatusBar()

    def init_plot(self):
        self.dpi = 100
        self.fig = Figure((16.0, 6.0), dpi=self.dpi)

        self.axes = self.fig.add_subplot(111)
        self.axes.set_axis_bgcolor('black')
        self.axes.set_title('Digispark Scope', size=8)
     
        
        #pylab.setp(self.axes.get_xticklabels(), fontsize=8)
        pylab.setp(self.axes.get_yticklabels(), fontsize=8)

        # plot the data as a line series, and save the reference 
        # to the plotted line series
        #
        self.plot_data = self.axes.plot(
            self.data, 
            linewidth=1,
            color=(1, 1, 0),
            )[0]

    def draw_plot(self):
        """ Redraws the plot
        """
        # when xmin is on auto, it "follows" xmax to produce a 
        # sliding window effect. therefore, xmin is assigned after
        # xmax.
        #

        xmax = len(self.data) if len(self.data) > 250 else 250

            
          
        xmin = xmax - 250


        # for ymin and ymax, find the minimal and maximal values
        # in the data set and add a mininal margin.
        # 
        # note that it's easy to change this scheme to the 
        # minimal/maximal value in the current display, and not
        # the whole data set.
        # 

        ymin = -0.5
        

        ymax = 5.5

        self.axes.set_xbound(lower=xmin, upper=xmax)
        self.axes.set_ybound(lower=ymin, upper=ymax)
        
        # anecdote: axes.grid assumes b=True if any other flag is
        # given even if b is set to False.
        # so just passing the flag into the first statement won't
        # work.
        #
        self.axes.grid(True, axis='y', color='gray')


        # Using setp here is convenient, because get_xticklabels
        # returns a list over which one needs to explicitly 
        # iterate, and setp already handles this.
        #  
        pylab.setp(self.axes.get_xticklabels(), visible=0)
        
        self.plot_data.set_xdata(np.arange(len(self.data)))
        self.plot_data.set_ydata(np.array(self.data))
        
        self.canvas.draw()
    
    def on_pause_button(self, event):
        self.paused = not self.paused
    
    def on_update_pause_button(self, event):
        label = "Resume" if self.paused else "Pause"
        self.pause_button.SetLabel(label)
    
    def on_cb_grid(self, event):
        self.draw_plot()
    
    def on_cb_xlab(self, event):
        self.draw_plot()
    
    def on_save_plot(self, event):
        file_choices = "PNG (*.png)|*.png"
        
        dlg = wx.FileDialog(
            self, 
            message="Save plot as...",
            defaultDir=os.getcwd(),
            defaultFile="plot.png",
            wildcard=file_choices,
            style=wx.SAVE)
        
        if dlg.ShowModal() == wx.ID_OK:
            path = dlg.GetPath()
            self.canvas.print_figure(path, dpi=self.dpi)
            self.flash_status_message("Saved to %s" % path)
    
    def on_redraw_timer(self, event):
        # if paused do not add data, but still redraw the plot
        # (to respond to scale modifications, grid change, etc.)
        #
        #GET DATA FROM DIGISPARK


        self.draw_plot()

    def on_data_timer(self, event):
        global theDevice
        # if paused do not add data, but still redraw the plot
        # (to respond to scale modifications, grid change, etc.)
        #
        #GET DATA FROM DIGISPARK

        if not self.paused:
            try:
                floatvolts = float(theDevice.read())/53.6842
                self.data.append(floatvolts) # for 4.75 volts max
                self.volts.SetLabel( '%04.2F' % round(floatvolts,2)+"v")
                theDevice.write(ord("1"))
            except:
                pass
            

    
    def on_exit(self, event):
        global theDevice
        self.data_timer.Stop()
        self.redraw_timer.Stop()
        theDevice.write(ord("1"))
        self.Destroy()
    
    def flash_status_message(self, msg, flash_len_ms=1500):
        self.statusbar.SetStatusText(msg)
        self.timeroff = wx.Timer(self)
        self.Bind(
            wx.EVT_TIMER, 
            self.on_flash_status_off, 
            self.timeroff)
        self.timeroff.Start(flash_len_ms, oneShot=True)
    
    def on_flash_status_off(self, event):
        self.statusbar.SetStatusText('')


if __name__ == '__main__':
    global theDevice
    app = wx.PySimpleApp()
    app.frame = GraphFrame()
    
    try:
        theDevice = ArduinoUsbDevice(idVendor=0x16c0, idProduct=0x05df)
    except:
        dlg = wx.MessageDialog(app.frame, "No DigiUSB Device Detected", "No Device", wx.OK | wx.ICON_WARNING)
        dlg.ShowModal()
        app.Destroy();
        exit();

    app.frame.Show()
    
    app.MainLoop()

