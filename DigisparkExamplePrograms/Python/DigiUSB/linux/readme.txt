# Make executable
chmod a+x "DigiUSB Monitor"

# Run
sudo "./DigiUSB Monitor"

# In case you're interested what's inside this bundle, you can mount it: 
mkdir DigiUSBMonitor.AppDir
sudo mount -oloop "./DigiUSB Monitor" DigiUSBMonitor.AppDir/
ls DigiUSBMonitor.AppDir/


From probono on the forums: http://digistump.com/board/index.php?p=/discussion/141/digiusb-monitor-standalone-executable-for-linux#Item_1