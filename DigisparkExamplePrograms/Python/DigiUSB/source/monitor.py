#!/usr/bin/python
# -*- coding: <<encoding>> -*-
#-------------------------------------------------------------------------------
#   <<project>>
# 
#-------------------------------------------------------------------------------


import wx
import sys, time
import usb # 1.0 not 0.4

sys.path.append("..")

from arduino.usbdevice import ArduinoUsbDevice



class Frame(wx.Frame):
    def __init__(self, title):
        wx.Frame.__init__(self, None, title=title, pos=(150,150), size=(450,450))
        self.Bind(wx.EVT_CLOSE, self.OnClose)

        global status
        self.statusbar = self.CreateStatusBar()
        status = self.statusbar

        self.timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER, self.checkForData, self.timer)
        self.timer.Start(10)

        panel = wx.Panel(self)
        box = wx.BoxSizer(wx.VERTICAL)

        m_text = wx.StaticText(panel, -1, "Send to device:")
        m_text.SetFont(wx.Font(12, wx.SWISS, wx.NORMAL, wx.BOLD))
        m_text.SetSize(m_text.GetBestSize())
        box.Add(m_text, 0, wx.ALL, 10)
        global printArea, inputArea
        inputArea = wx.TextCtrl(panel,-1, size = (400, 20),style = wx.TE_PROCESS_ENTER)
        box.Add(inputArea, 0, wx.ALL, 10)
 
        inputArea.Bind(wx.EVT_TEXT_ENTER, self.OnSend)


        m_text = wx.StaticText(panel, -1, "Received from device:")
        m_text.SetFont(wx.Font(12, wx.SWISS, wx.NORMAL, wx.BOLD))
        m_text.SetSize(m_text.GetBestSize())
        box.Add(m_text, 0, wx.ALL, 10)

        
        printArea = wx.TextCtrl(panel,-1, size = (400, 250),style = wx.TE_WORDWRAP | wx.TE_MULTILINE | wx.TE_READONLY)
        box.Add(printArea, 0, wx.ALL, 10)



        m_close = wx.Button(panel, wx.ID_CLOSE, "Close")
        m_close.Bind(wx.EVT_BUTTON, self.OnClose)
        box.Add(m_close, 0, wx.ALL, 10)

        panel.SetSizer(box)
        panel.Layout()


        

    def checkForData(self, event):
        global printArea, theDevice
        try:
            charRead = chr(theDevice.read())
            printArea.AppendText(charRead)
            self.timer.Start(1)
        except:
            # TODO: Check for exception properly
            self.timer.Start(500)

        
    
    def OnSend(self, event):
        global printArea, inputArea, theDevice
        userInput = inputArea.GetValue()+"\n";

        for c in userInput:
            theDevice.write(ord(c))

        printArea.AppendText("Sent: "+userInput)
        inputArea.Clear()

    def OnClose(self, event):
        self.Destroy()


    #chr(theDevice.read())

global status,inputArea, theDevice


app = wx.App(redirect=False)   # Error messages go to popup window
top = Frame("DigiUSB Monitor")

try:
    theDevice = ArduinoUsbDevice(idVendor=0x16c0, idProduct=0x05df)       
except:
    # TODO: Check for exception properly
    dlg = wx.MessageDialog(top, "No DigiUSB Device Detected", "No Device", wx.OK | wx.ICON_WARNING)
    dlg.ShowModal()
    app.Destroy();
    exit();


status.SetStatusText("Found: 0x%04x 0x%04x %s %s" % (theDevice.idVendor, 
                                      theDevice.idProduct,
                                      theDevice.productName,
                                      theDevice.manufacturer))
top.Show()
app.MainLoop()
