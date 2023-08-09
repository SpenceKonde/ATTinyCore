#!/usr/bin/python3
import re

clocktobaud = {
  "20": {
  "f_cpu":"20000000UL",
  "name":"20 MHz",
  "xl":False,
  "hard":"115200",
  "lin":"115200",
  "soft":"57600",
  },
  "16_5": {
  "f_cpu":"16500000UL",
  "name":"16.5 MHz",
  "xl":False,
  "hard":"115200",
  "lin":"115200",
  "soft":"57600",
  },
  "16": {
  "f_cpu":"16000000UL",
  "name":"16 MHz",
  "xl":False,
  "hard":"76800",
  "lin":"115200",
  "soft":"57600",
  },
  "12_8": {
  "f_cpu":"12800000UL",
  "name":"12.8 MHz",
  "xl":False,
  "hard":"115200",
  "lin":"115200",
  "soft":"57600",
  },
  "12": {
  "f_cpu":"12000000UL",
  "name":"12 MHz",
  "xl"  :False,
  "hard":"115200",
  "lin":"115200",
  "soft":"57600",
  },
  "10": {
  "f_cpu":"10000000UL",
  "name":"10 MHz",
  "xl"  :False,
  "hard":"57600",
  "lin":"115200",
  "soft":"38400",
  },
  "8": {
  "f_cpu":"8000000UL",
  "name":"8 MHz",
  "xl":False,
  "hard":"76800",
  "lin":"115200",
  "soft":"28800",
  },
  "6": {
  "f_cpu":"6000000UL",
  "name":"6 MHz",
  "xl":True,
  "hard":"57600",
  "lin":"76800",
  "soft":"28800",
  },
  "4": {
  "f_cpu":"4000000UL",
  "intat1":True,
  "name":"4 MHz",
  "xl":True,
  "hard":"38400",
  "lin":"57600",
  "soft":"14400",
  "hardint":"9600",
  "softint":"4800",
  },
  "2": {
  "f_cpu":"2000000UL",
  "intat1":True,
  "name":"2 MHz",
  "xl":True,
  "hard":"19200",
  "lin":"19200",
  "soft":"9600",
  "hardint":"9600",
  "softint":"4800",
  },
  "1": {
  "f_cpu":"1000000UL",
  "name":"1 MHz",
  "xl"  :True,
  "hard":"9600",
  "lin":"9600",
  "soft":"4800",
  },
  "500k": {
  "f_cpu":"500000UL",
  "name":"0.5 MHz",
  "xl"  :True,
  "hard":"4800",
  "lin":"4800",
  "soft":"2400",
  },
  "7372k": {
  "f_cpu":"7372000UL",
  "name":"7.372 MHz",
  "xl"  :True,
  "hard":"76800",
  "lin":"76800",
  "soft":"28800",
  },
  "9216k": {
  "f_cpu":"9216000UL",
  "name":"9.216 MHz",
  "xl"  :False,
  "hard":"115200",
  "lin":"115200",
  "soft":"38400",
  },
  "3686k": {
  "f_cpu":"3686000UL",
  "name":"3.686 MHz",
  "xl"  :False,
  "hard":"57600",
  "lin":"57600",
  "soft":"14400",
  },
  "11059k": {
  "f_cpu":"11059000UL",
  "name":"11.059 MHz",
  "xl":False,
  "hard":"115200",
  "lin":"115200",
  "soft":"38400",
  },
  "14746k": {
  "f_cpu":"14746000UL",
  "name":"14.746 MHz",
  "xl":False,
  "hard":"115200",
  "lin":"115200",
  "soft":"57600",
  },
  "18432k": {
  "f_cpu":"18432000UL",
  "name":"18.432 MHz",
  "xl":False,
  "hard":"115200",
  "lin":"115200",
  "soft":"57600",
  }
}
#    F_CPU    |  Hard Baud  |  Soft Baud  | LIN (x7) Baud | Notes
#  0.5 MHz    |      4800   |     -       |        -      | x313 only
#    1 MHz    |      9600   |   4800      |      9600     |
#    2 MHz    |      9600   |   4800      |      9600     |
#    4 MHz    |      9600   |   4800      |      9600     |
#    4 MHz *  |     38400   |     -       |        -      | x313 only
#    6 MHz    |     57600   |  28800      |     57600     |
#    8 MHz    |     76800   |  28800      |     76800     |
#   12 MHz    |    115200   |  57600      |    115200     |
# 12.8 MHz    | Unsupported | Unsupported |   Unsupported | Micronucleus 84/841  Only
#   16 MHz    |     76800   |  38400      |     76800     | The way the baud rates divide up is not favorable here.
# 16.5 MHz    | Unsupported | Unsupported |   Unsupported | Micronucleus 85/861  Only
# 3686 kHz    |     57600   |  14400      |     57600     | For minimum voltage operation with perfect USART clock
# USART <=  8 |     76800   |  28800      |     76800     | Any USART crystal above 8 and below 12
# USART >   8 |    115200   |  as below   |    115200     | All get 115.2k uploads via hardware serial.
# USART <  12 |  as above*  |  38400      |    115200     | But not all get the same soft serial speeds.
# USART >= 12 |  as above*  |  57600      |    115200     | Why are you using soft serial anyway with a USART crystal anyway?

def printProp(board,string):
  print(board+string)
  print(board+string,file = f1)
def printLit(string):
  print(string)
  print(string,file = f1)
def getspeed(string,speed):
  t = {}
  if "intat1" in clocktobaud[speed]:
    t["intat1"] = True
    t["hardint"] = string+clocktobaud["1"]["hard"]
    t["softint"] = string+clocktobaud["1"]["soft"]
    t["bootspeed"] = string[:-13]+"bootloader.speed=1m"
  else:
    t["intat1"] = False
  t["hard"] = string+clocktobaud[speed]["hard"]
  t["soft"] = string+clocktobaud[speed]["soft"]
  t["lin"] = string+clocktobaud[speed]["lin"]
  return t

boardnames = [
  "attinyx4.name=ATtiny84/44/24 (No Bootloader)",
  "attinyx41.name=ATtiny841/441 (No Bootloader)",
  "attinyx5.name=ATtiny85/45/25 (No Bootloader)",
  "attinyx61.name=ATtiny861/461/261 (No Bootloader)",
  "attinyx7.name=ATtiny167/87 (No Bootloader)",
  "attinyx8.name=ATtiny88/48 (No Bootloader)",
  "attiny1634.name=ATtiny1634 (No Bootloader)",
  "attinyx313.name=ATtiny4313/2313 (No Bootloader)",
  "attiny828.name=ATtiny828 (No Bootloader)",
  "attiny43u.name=ATtiny43 (No Bootloader)",
  "attiny26.name=ATtiny26 (No Bootloader)",
  "attinyx4opti.name=ATtiny84/44 w/Optiboot serial bootloader",
  "attinyx41opti.name=ATtiny841/441 w/Optiboot serial bootloader",
  "attinyx5opti.name=ATtiny85/45 w/Optiboot serial bootloader",
  "attinyx61opti.name=ATtiny861/461 w/Optiboot serial bootloader",
  "attinyx7opti.name=ATtiny167/87 w/Optiboot serial bootloader",
  "attinyx8opti.name=ATtiny88/48 w/Optiboot serial bootloader",
  "attiny1634opti.name=ATtiny1634 w/Optiboot serial bootloader",
  "attiny828opti.name=ATtiny828 w/Optiboot serial bootloader",
  "attiny85micr.name=ATtiny85 w/Micronucleus (Digispark)",
  "attiny167micr.name=ATtiny167 w/Micronucleus (Digispark Pro)",
  "attiny88micr.name=ATtiny88 w/Micronucleus (MH-Tiny/MHET)",
  "attiny841micr.name=ATtiny841 w/Micronucleus (Nanite, Bitboss, etc)",
  "attiny87micr.name=ATtiny87 w/Micronucleus",
  "attiny861micr.name=ATtiny861 w/Micronucleus  (Azduino USB 861)",
  "attiny1634micr.name=ATtiny1634 w/Micronucleus (Azduino USB 1634)",
  "attiny84mi12.name=ATtiny84 w/Micronucleus (12 MHz - pushing bounds on USB functioning)",
  "attiny84micr.name=ATtiny84 w/Micronucleus (12.8 MHz, bootloader takes more flash)"]

chipmenu = {
  "attinyx4":"attinyx4.menu.chip.84=ATtiny84\nattinyx4.menu.chip.84.build.mcu=attiny84\nattinyx4.menu.chip.84.upload.maximum_size=8192\nattinyx4.menu.chip.84.upload.maximum_data_size=512\nattinyx4.menu.chip.44=ATtiny44\nattinyx4.menu.chip.44.build.mcu=attiny44\nattinyx4.menu.chip.44.upload.maximum_size=4096\nattinyx4.menu.chip.44.upload.maximum_data_size=256\nattinyx4.menu.chip.24=ATtiny24\nattinyx4.menu.chip.24.build.mcu=attiny24\nattinyx4.menu.chip.24.upload.maximum_size=2048\nattinyx4.menu.chip.24.upload.maximum_data_size=128",
  "attinyx4opti":"attinyx4opti.menu.chip.84=ATtiny84\nattinyx4opti.menu.chip.84.build.mcu=attiny84\nattinyx4opti.menu.chip.84.upload.maximum_size=7552\nattinyx4opti.menu.chip.84.upload.maximum_data_size=512\nattinyx4opti.menu.chip.44=ATtiny44\nattinyx4opti.menu.chip.44.build.mcu=attiny44\nattinyx4opti.menu.chip.44.upload.maximum_size=3456\nattinyx4opti.menu.chip.44.upload.maximum_data_size=256",
  "attinyx4micr":"",
  "attinyx5":"attinyx5.menu.chip.85=ATtiny85\nattinyx5.menu.chip.85.build.mcu=attiny85\nattinyx5.menu.chip.85.upload.maximum_size=8192\nattinyx5.menu.chip.85.upload.maximum_data_size=512\nattinyx5.menu.chip.45=ATtiny45\nattinyx5.menu.chip.45.build.mcu=attiny45\nattinyx5.menu.chip.45.upload.maximum_size=4096\nattinyx5.menu.chip.45.upload.maximum_data_size=256\nattinyx5.menu.chip.25=ATtiny25\nattinyx5.menu.chip.25.build.mcu=attiny25\nattinyx5.menu.chip.25.upload.maximum_size=2048\nattinyx5.menu.chip.25.upload.maximum_data_size=128",
  "attinyx5opti":"attinyx5opti.menu.chip.85=ATtiny85\nattinyx5opti.menu.chip.85.build.mcu=attiny85\nattinyx5opti.menu.chip.85.upload.maximum_size=7616\nattinyx5opti.menu.chip.85.upload.maximum_data_size=512\nattinyx5opti.menu.chip.45=ATtiny45\nattinyx5opti.menu.chip.45.build.mcu=attiny45\nattinyx5opti.menu.chip.45.upload.maximum_size=3520\nattinyx5opti.menu.chip.45.upload.maximum_data_size=256",
  "attinyx4micr": "",
  "attinyx8": "attinyx8.menu.chip.88=ATtiny88\nattinyx8.menu.chip.88.build.mcu=attiny88\nattinyx8.menu.chip.88.upload.maximum_size=8192\nattinyx8.menu.chip.88.upload.maximum_data_size=512\nattinyx8.menu.chip.48=ATtiny48\nattinyx8.menu.chip.48.build.mcu=attiny48\nattinyx8.menu.chip.48.upload.maximum_size=4096\nattinyx8.menu.chip.48.upload.maximum_data_size=256",
  "attinyx8opti": "attinyx8opti.menu.chip.88=ATtiny88\nattinyx8opti.menu.chip.88.build.mcu=attiny88\nattinyx8opti.menu.chip.88.upload.maximum_size=7552\nattinyx8opti.menu.chip.88.upload.maximum_data_size=512\nattinyx8opti.menu.chip.48=ATtiny48\nattinyx8opti.menu.chip.48.build.mcu=attiny48\nattinyx8opti.menu.chip.48.upload.maximum_size=3456\nattinyx8opti.menu.chip.48.upload.maximum_data_size=256",
  "attinyx8micr": "",
  "attinyx7": "attinyx7.menu.chip.167=ATtiny167\nattinyx7.menu.chip.167.build.mcu=attiny167\nattinyx7.menu.chip.167.upload.maximum_size=16384\nattinyx7.menu.chip.87=ATtiny87\nattinyx7.menu.chip.87.build.mcu=attiny87\nattinyx7.menu.chip.87.upload.maximum_size=8196",
  "attinyx7opti": "attinyx7opti.menu.chip.167=ATtiny167\nattinyx7opti.menu.chip.167.build.mcu=attiny167\nattinyx7opti.menu.chip.167.upload.maximum_size=15744\nattinyx7opti.menu.chip.87=ATtiny87\nattinyx7opti.menu.chip.87.build.mcu=attiny87\nattinyx7opti.menu.chip.87.upload.maximum_size=7552",
  "attinyx7micr": "",
  "attinyx61": "attinyx61.menu.chip.861=ATtiny861\nattinyx61.menu.chip.861.build.mcu=attiny861\nattinyx61.menu.chip.861.upload.maximum_size=8192\nattinyx61.menu.chip.861.upload.maximum_data_size=512\nattinyx61.menu.chip.461=ATtiny461\nattinyx61.menu.chip.461.build.mcu=attiny461\nattinyx61.menu.chip.461.upload.maximum_size=4096\nattinyx61.menu.chip.461.upload.maximum_data_size=256\nattinyx61.menu.chip.261=ATtiny261\nattinyx61.menu.chip.261.build.mcu=attiny261\nattinyx61.menu.chip.261.upload.maximum_size=2048\nattinyx61.menu.chip.261.upload.maximum_data_size=128",
  "attinyx61opti": "attinyx61opti.menu.chip.861=ATtiny861\nattinyx61opti.menu.chip.861.build.mcu=attiny861\nattinyx61opti.menu.chip.861.upload.maximum_size=7552\nattinyx61opti.menu.chip.861.upload.maximum_data_size=512\nattinyx61opti.menu.chip.461=ATtiny461\nattinyx61opti.menu.chip.461.build.mcu=attiny461\nattinyx61opti.menu.chip.461.upload.maximum_size=3456\nattinyx61opti.menu.chip.461.upload.maximum_data_size=256",
  "attinyx61micr": "",
  "attinyx41": "attinyx41.menu.chip.841=ATtiny841\nattinyx41.menu.chip.841.build.mcu=attiny841\nattinyx41.menu.chip.841.upload.maximum_size=8192\nattinyx41.menu.chip.841.upload.maximum_data_size=512\nattinyx41.menu.chip.441=ATtiny441\nattinyx41.menu.chip.441.build.mcu=attiny441\nattinyx41.menu.chip.441.upload.maximum_size=4096\nattinyx41.menu.chip.441.upload.maximum_data_size=256",
  "attinyx41opti": "attinyx41opti.menu.chip.841=ATtiny841\nattinyx41opti.menu.chip.841.build.mcu=attiny841\nattinyx41opti.menu.chip.841.upload.maximum_size=7552\nattinyx41opti.menu.chip.841.upload.maximum_data_size=512\nattinyx41opti.menu.chip.441=ATtiny441\nattinyx41opti.menu.chip.441.build.mcu=attiny441\nattinyx41opti.menu.chip.441.upload.maximum_size=3456\nattinyx41opti.menu.chip.441.upload.maximum_data_size=256",
  "attinyx41micr": "",
  "attinyx313": "attinyx313.menu.chip.4313=ATtiny4313\nattinyx313.menu.chip.4313.build.mcu=attiny4313\nattinyx313.menu.chip.4313.upload.maximum_size=4096\nattinyx313.menu.chip.4313.upload.maximum_data_size=256\nattinyx313.menu.chip.2313=ATtiny2313/ATtiny2313A\nattinyx313.menu.chip.2313.build.mcu=attiny2313a\nattinyx313.menu.chip.2313.upload.maximum_size=2048\nattinyx313.menu.chip.2313.upload.maximum_data_size=128",
  "attiny1634": "",
  "attiny1634opti": "",
  "attiny1634micr": "",
  "attiny43u": "",
  "attiny828": "",
  "attiny828opti": ""
}

chiplist = {
  "attinyx4":   ["attiny84","attiny44"],
  "attinyx4":   ["attiny84","attiny44"],
  "attinyx5":   ["attiny85","attiny45"],
  "attinyx8":   ["attiny88","attiny48"],
  "attinyx7":   ["attiny167","attiny87"],
  "attinyx61":  ["attiny861","attiny461"],
  "attinyx41":  ["attiny841","attiny441"],
  "attiny1634": ["attiny1634"],
  "attiny828":  ["attiny828"]
}


# Headers for sections:
boards_no_loader="################################################################################\n#                                                                              #\n###                 ####    ###    ###   ####   ####    ###                  ###\n#####               #   #  #   #  #   #  #   #  #   #  #                   #####\n#######             ####   #   #  #####  ####   #   #   ###              #######\n#####               #   #  #   #  #   #  #  #   #   #      #               #####\n###                 ####    ###   #   #  #   #  ####    ###                  ###\n#______________________________________________________________________________#\n#                            Without Bootloaders                               #\n#           All supported parts can be used with an ISP programmer             #\n#______________________________________________________________________________#"
boards_optiboot="################################################################################\n#     ##                                                                ##     #\n#     ##            ####    ###    ###   ####   ####    ###             ##     #\n#     ##            #   #  #   #  #   #  #   #  #   #  #                ##     #\n#     ##            ####   #   #  #####  ####   #   #   ###             ##     #\n#     ##            #   #  #   #  #   #  #  #   #   #      #            ##     #\n#     ##            ####    ###   #   #  #   #  ####    ###             ##     #\n#_____##________________________________________________________________##_____#\n#                     With the Optiboot Serial bootloader                      #\n#     Available for all parts with 4k+ flash which come in a version that      #\n#     has at least 8k of flash and which support self-programming.             #\n#    43, 44, 84, 441, 841, 45, 85, 461, 861, 87, 167, 48, 88, 828, and 1634    #\n#______________________________________________________________________________#"
boards_micronucleus="################################################################################\n#           #                                                      #           #\n############        ####    ###    ###   ####   ####    ###         ############\n#           #       #   #  #   #  #   #  #   #  #   #  #           #           #\n############        ####   #   #  #####  ####   #   #   ###         ############\n#           #       #   #  #   #  #   #  #  #   #   #      #       #           #\n############        ####    ###   #   #  #   #  ####    ###         ############\n#___________#______________________________________________________#___________#\n#            With Micronucleus, the VUSB bootloader like Digispark             #\n#     Available for parts with at least 8k of flash that are decent.           #\n#     85 (Digispark and clones), 87, 167 (Digispark Pro and clones), 84,       #\n#     88 (MH-TINY), 841 (Wattuino Nanite and others),                          #\n#     861 and 1634 (Azduino USB 861, 1634, coming soon)                        #\n#______________________________________________________________________________#"


fancybodmenu = [
  ".menu.bod.1v8=B.O.D. Enabled (1.8v)", ".menu.bod.1v8.bootloader.bod_bits=110",
  ".menu.bod.2v7=B.O.D. Enabled (2.7v)", ".menu.bod.2v7.bootloader.bod_bits=101",
  ".menu.bod.4v3=B.O.D. Enabled (4.3v)", ".menu.bod.4v3.bootloader.bod_bits=100",
  ".menu.bodact.disabled=B.O.D. Disabled (saves power)", ".menu.bodact.disabled.bootloader.bodact_bits=11",
  ".menu.bodact.enabled=B.O.D. Enabled", ".menu.bodact.enabled.bootloader.bodact_bits=10",
  ".menu.bodact.sampled=B.O.D. Sampled Mode", ".menu.bodact.sampled.bootloader.bodact_bits=01",
  ".menu.bodpd.disabled=B.O.D. Disabled (saves power)", ".menu.bodpd.disabled.bootloader.bodpd_bits=11",
  ".menu.bodpd.enabled=B.O.D. Enabled", ".menu.bodpd.enabled.bootloader.bodpd_bits=10",
  ".menu.bodpd.sampled=B.O.D. Sampled Mode", ".menu.bodpd.sampled.bootloader.bodpd_bits=01"]

plainbodmenu = [
  ".menu.bod.disable=B.O.D. Disabled (saves power)",".menu.bod.disable.bootloader.bod_bits=111",
  ".menu.bod.1v8=B.O.D. Enabled (1.8v)",".menu.bod.1v8.bootloader.bod_bits=110",
  ".menu.bod.2v7=B.O.D. Enabled (2.7v)",".menu.bod.2v7.bootloader.bod_bits=101",
  ".menu.bod.4v3=B.O.D. Enabled (4.3v)",".menu.bod.4v3.bootloader.bod_bits=100"]
eesavemenu = [
  ".menu.eesave.aenable=EEPROM retained",".menu.eesave.aenable.bootloader.eesave_bit=0",
  ".menu.eesave.disable=EEPROM not retained",".menu.eesave.disable.bootloader.eesave_bit=1"]
softsermenu = [
  ".menu.softserial.txonly=No receiving, transmit only.",".menu.softserial.txonly.build.softser=-DSOFT_TX_ONLY",".menu.softserial.txonly.build.softserabr=ssTX"]
softsermenu_861= ["enable_AIN1=RX on PA7, TX on PA (default PA6)",
  "enable_AIN0=RX on PA6, TX on PA (default PA7)", "enable_AIN0.build.softser=-DSOFTSERIAL_RXAIN0", "enable_AIN0.build.softserabr=ssAIN0",
  "enable_AIN2=RX on PA5, TX on PA (default PA6)", "enable_AIN2.build.softser=-DSOFTSERIAL_RXAIN2", "enable_AIN2.build.softserabr=ssAIN2",
  "txonly=No receiving, transmit only. TX on PA, default PA6.", "txonly.build.softserabr=ssTX", "txonly.build.softser=-DSOFT_TX_ONLY",
  "enable_AIN1.bootloader.uart=_rx7tx6", "enable_AIN0.bootloader.uart=_rx6tx7", "enable_AIN2.bootloader.uart=_rx5tx6", "txonly.bootloader.uart=_rx7tx6"]
millismenu = [
  ".menu.millis.enabled=Enabled",".menu.millis.enabled.build.millis=",".menu.millis.enabled.build.millisabr=",
  ".menu.millis.disabled=Disabled (saves flash)",".menu.millis.disabled.build.millis=-DDISABLEMILLIS",".menu.millis.disabled.build.millisabr=mNONE"]

# all of the menus should have been done like this, instead of repeating the menu name a million times.
# this is the last menu I added...
wiremodesmenu = [
  "master=Master Only","master.build.wiremode=-DWIRE_MASTER_ONLY","master.build.wiremodeabr=",
  "slave=Slave Only","slave.build.wiremode=-DWIRE_SLAVE_ONLY","slave.build.wiremodeabr=wS",
  "both=Either (uses more flash)","both.build.wiremode=-DWIRE_BOTH","slave.build.wiremodeabr=wB"]

resetpinmenu = [
  "reset=Reset pin. Further ISP programming possible","reset.bootloader.rstbit=1",
  "gpio=I/O pin - DANGER: If anything goes wrong, only HV programming can unbrick!","gpio.bootloader.rstbit=0"]

bootmodesopti = [
  ".menu.bootentry.1s=Standard (1s wait, for use w/autoreset)",".menu.bootentry.1s.bootloader.file={runtime.platform.path}/bootloaders/optiboot/hex/optiboot_{build.mcu}_{build.f_cpu}{bootloader.uart}.hex",
  ".menu.bootentry.8s=8-second (8s wait, for use w. out/autoreset)",".menu.bootentry.8s_8sec.bootloader.file={runtime.platform.path}/bootloaders/optiboot/hex/optiboot_{build.mcu}_{build.f_cpu}{bootloader.uart}_8sec.hex"]

bootmodesmicr = [
  ".menu.bootentry.extrf_porf=External Reset and Power On Reset",".menu.bootentry.extrf_porf.bootloader.entrymode=extrf_porf",
  ".menu.bootentry.extrf_safe=External Reset, or if reset pin disabled and HIGH",".menu.bootentry.extrf_safe.bootloader.entrymode=extrf_safe",
  ".menu.bootentry.extrfonly_safe=External Reset only, or if reset pin disabled and HIGH",".menu.bootentry.extrfonly_safe.bootloader.entrymode=extrfonly_safe",
  ".menu.bootentry.wdrf=Watchdog Reset only",".menu.bootentry.wdrf.bootloader.entrymode=wdrf",
  ".menu.bootentry.porf=Power On Reset only",".menu.bootentry.porf.bootloader.entrymode=porf",
  ".menu.bootentry.anyreset=Any reset (but not jump from app)",".menu.bootentry.anyreset.bootloader.entrymode=anyreset",
  ".menu.bootentry.always=Always (permits jump from app)",".menu.bootentry.always.bootloader.entrymode=always",
  ".menu.bootentry.jumper=Jumper (see documentation) (permits jump from app)",".menu.bootentry.jumper.bootloader.entrymode=jumper"]

intclocks = [[[".menu.clock.internal_8m=8 MHz (internal)", ".menu.clock.internal_8m.bootloader.low_fuses=0xE2",
       ".menu.clock.internal_8m.build.f_cpu=8000000UL", ".menu.clock.internal_8m.build.speed=8m",
       ".menu.clock.internal_8m.build.clocksource=0"],getspeed(".menu.clock.internal_8m.upload.speed=","8")],
      [[".menu.clock.internal_1m=1 MHz (internal)", ".menu.clock.internal_1m.bootloader.low_fuses=0x62",
       ".menu.clock.internal_1m.build.f_cpu=1000000UL", ".menu.clock.internal_1m.build.speed=1m",
       ".menu.clock.internal_1m.build.clocksource=0x10"], getspeed(".menu.clock.internal_1m.upload.speed=","1")],
      [[".menu.clock.internal_4m=4 MHz (internal)", ".menu.clock.internal_4m.bootloader.low_fuses=0x62",
       ".menu.clock.internal_4m.build.f_cpu=4000000UL", ".menu.clock.internal_4m.build.speed=4m",
       ".menu.clock.internal_4m.build.clocksource=0x10",".menu.clock.internal_4m.bootloader.f_cpu=1000000UL"], getspeed(".menu.clock.internal_4m.upload.speed=","4")],
      [[".menu.clock.internal_2m=2 MHz (internal)", ".menu.clock.internal_2m.bootloader.low_fuses=0x62",
       ".menu.clock.internal_2m.build.f_cpu=2000000UL", ".menu.clock.internal_2m.build.speed=2m",
       ".menu.clock.internal_2m.build.clocksource=0x10",".menu.clock.internal_2m.bootloader.f_cpu=1000000UL"], getspeed(".menu.clock.internal_2m.upload.speed=","2")]]
pllclocks = [[[".menu.clock.pll_16m=16 MHz (PLL)", ".menu.clock.pll_16m.bootloader.low_fuses=0xF1",
       ".menu.clock.pll_16m.build.f_cpu=16000000UL", ".menu.clock.pll_16m.build.speed=16m",
       ".menu.clock.pll_16m.build.clocksource=6"], getspeed(".menu.clock.pll_16m.upload.speed=","16")],
       [[".menu.clock.pll_16m5=16.5 MHz (PLL, tweaked)", ".menu.clock.pll_16m5.bootloader.low_fuses=0xF1",
       ".menu.clock.pll_16m5.build.f_cpu=16500000UL", ".menu.clock.pll_16m5.build.speed=16m5",
       ".menu.clock.pll_16m5.build.clocksource=6",".menu.clock.pll_16m5.bootloader.f_cpu=16000000UL"], getspeed(".menu.clock.pll_16m5.upload.speed=","16")]]
tunedclocks = [[[".menu.clock.internal_12m=12 MHz (internal, tuned)", ".menu.clock.internal_12m.bootloader.low_fuses=0xE2",
       ".menu.clock.internal_12m.build.f_cpu=12000000UL", ".menu.clock.internal_12m.build.speed=12m",
       ".menu.clock.internal_12m.build.clocksource=0",".menu.clock.internal_12m.bootloader.f_cpu=8000000"], getspeed(".menu.clock.internal_12m.upload.speed=","8")],
       [[".menu.clock.internal_12m8=12.8 MHz (internal, tuned)", ".menu.clock.internal_12m8.bootloader.low_fuses=0xE2",
       ".menu.clock.internal_12m8.build.f_cpu=12800000L", ".menu.clock.internal_12m8.build.speed=12m",
       ".menu.clock.internal_12m8.build.clocksource=0", ".menu.clock.internal_12m8.bootloader.f_cpu=8000000"], getspeed(".menu.clock.internal_12m8.upload.speed=","8")],
       ]
tune8stdclk = [[[".menu.clock.internal_8m_tuned=8 MHz (internal, tuned)", ".menu.clock.internal_8m_tuned.bootloader.low_fuses=0xE2",
       ".menu.clock.internal_8m_tuned.build.f_cpu=8000000UL", ".menu.clock.internal_8m_tuned.build.speed=8m",
       ".menu.clock.internal_8m_tuned.build.clocksource=0"], getspeed(".menu.clock.internal_8m_tuned.upload.speed=","8")],]
tune8vdepclk = [[[".menu.clock.internal_8m_tuned=8 MHz (internal, tuned, < 4.0V)", ".menu.clock.internal_8m_tuned_low.bootloader.low_fuses=0xE2",
       ".menu.clock.internal_8m_tuned_low.build.f_cpu=8000000UL", ".menu.clock.internal_8m_tuned_low.build.speed=8m",
       ".menu.clock.internal_8m_tuned_low.build.clocksource=0"], getspeed(".menu.clock.internal_8m_tuned_low.upload.speed=","8")],
       [[".menu.clock.internal_8m_tuned_high=8 MHz (internal, tuned, > 4.0V)", ".menu.clock.internal_8m_tuned_high.bootloader.low_fuses=0xE2",
       ".menu.clock.internal_8m_tuned_high.build.f_cpu=8000000UL", ".menu.clock.internal_8m_tuned_high.build.speed=8m",
       ".menu.clock.internal_8m_tuned_high.build.clocksource=0"], getspeed(".menu.clock.internal_8m_tuned_high.upload.speed=","8")]]
xtalclocks = ["16m", "8m", "20m", "12m", "6m", "18432k", "14746k", "11059k", "9216k", "7372k", "3686k"]

wdtclocks = [".menu.clock.internal_128k=128 kHz (internal WDT)",".menu.clock.internal_128k.build.f_cpu=128000L",".menu.clock.internal_128k.build.speed=128k",
       ".menu.clock.internal_128k.build.clocksource=3"]

ulpclocks = [".menu.clock.internal_512k=512 kHz (internal ULP)",".menu.clock.internal_512k.bootloader.low_fuses=0xE4",
       ".menu.clock.internal_512k.bootloader.ulp_bits=011",".menu.clock.internal_512k.build.f_cpu=512000L",
       ".menu.clock.internal_512k.build.speed=512k",".menu.clock.internal_512k.build.clocksource=4",
       ".menu.clock.internal_256k=256 kHz (internal ULP)",".menu.clock.internal_256k.bootloader.low_fuses=0xE4",
       ".menu.clock.internal_256k.bootloader.ulp_bits=100",".menu.clock.internal_256k.build.f_cpu=256000L",
       ".menu.clock.internal_256k.build.speed=256k",".menu.clock.internal_256k.build.clocksource=4",
       ".menu.clock.internal_128k=128 kHz (internal ULP)",".menu.clock.internal_128k.bootloader.low_fuses=0xE4",
       ".menu.clock.internal_128k.bootloader.ulp_bits=101",".menu.clock.internal_128k.build.f_cpu=128000L",
       ".menu.clock.internal_128k.build.speed=128k",".menu.clock.internal_128k.build.clocksource=4",
       ".menu.clock.internal_64k=64 kHz (internal ULP)",".menu.clock.internal_64k.bootloader.low_fuses=0xE4",
       ".menu.clock.internal_64k.bootloader.ulp_bits=110",".menu.clock.internal_64k.build.f_cpu=64000L",
       ".menu.clock.internal_64k.build.speed=64k",".menu.clock.internal_64k.build.clocksource=4",
       ".menu.clock.internal_32k=32 kHz (internal ULP)",".menu.clock.internal_32k.bootloader.low_fuses=0xE4",
       ".menu.clock.internal_32k.bootloader.ulp_bits=111",".menu.clock.internal_32k.build.f_cpu=32000L",
       ".menu.clock.internal_32k.build.speed=32k",".menu.clock.internal_32k.build.clocksource=4"]

titles={
  "attinyx4":"\n#*******************************************************************************\n#   ###  #####                          #  #\n#  #   #   #    #  #                    #  #\n#  #####   #   ###    ###  #  #     # # ####\n#  #   #   #    #  #  #  # #  #      #     #\n#  #   #   #    #  #  #  #  ###     # #    #\n#_____________________________#_________________________________________________\n                           ###\n",
  "attinyx5":"\n#*******************************************************************************\n#   ###  #####                          ####\n#  #   #   #    #  #                    #\n#  #####   #   ###    ###  #  #     # # ###\n#  #   #   #    #  #  #  # #  #      #     #\n#  #   #   #    #  #  #  #  ###     # # ###\n#_____________________________#_________________________________________________\n                           ###\n",
  "attinyx8":"\n#*******************************************************************************\n#   ###  #####                           ###\n#  #   #   #    #  #                    #   #\n#  #####   #   ###    ###  #  #     # #  ###\n#  #   #   #    #  #  #  # #  #      #  #   #\n#  #   #   #    #  #  #  #  ###     # #  ###\n#_____________________________#_________________________________________________\n                           ###\n",
  "attinyx7":"\n#*******************************************************************************\n#   ###  #####                          #####\n#  #   #   #    #  #                       #\n#  #####   #   ###    ###  #  #     # #   #\n#  #   #   #    #  #  #  # #  #      #    #\n#  #   #   #    #  #  #  #  ###     # #   #\n#_____________________________#_________________________________________________\n                           ###\n",
  "attiny26":"\n#*******************************************************************************\n#   ###  #####                       ##    #\n#  #   #   #    #  #                #  #  #\n#  #####   #   ###    ###  #  #       #  ####\n#  #   #   #    #  #  #  # #  #      #   #   #\n#  #   #   #    #  #  #  #  ###     ####  ###\n#_____________________________#_________________________________________________\n                           ###\n",
  "attinyx61":"\n#*******************************************************************************\n#   ###  #####                            #     #\n#  #   #   #    #  #                     #     ##\n#  #####   #   ###    ###  #  #     # # ####    #\n#  #   #   #    #  #  #  # #  #      #  #   #   #\n#  #   #   #    #  #  #  #  ###     # #  ###   ###\n#_____________________________#_________________________________________________\n                           ###\n",
  "attinyx41":"\n#*******************************************************************************\n#   ###  #####                          #  #   #\n#  #   #   #    #  #                    #  #  ##\n#  #####   #   ###    ###  #  #     # # ####   #\n#  #   #   #    #  #  #  # #  #      #     #   #\n#  #   #   #    #  #  #  #  ###     # #    #  ###\n#_____________________________#_________________________________________________\n                           ###\n",
  "attiny828":"\n#*******************************************************************************\n#   ###  #####                       ###   ##   ###\n#  #   #   #    #  #                #   # #  # #   #\n#  #####   #   ###    ###  #  #      ###    #   ###\n#  #   #   #    #  #  #  # #  #     #   #  #   #   #\n#  #   #   #    #  #  #  #  ###      ###  ####  ###\n#_____________________________#_________________________________________________\n                           ###\n",
  "attiny1634":"\n#*******************************************************************************\n#   ###  #####                       #    #   ###  #  #\n#  #   #   #    #  #                ##   #       # #  #\n#  #####   #   ###    ###  #  #      #  ####   ##  ####\n#  #   #   #    #  #  #  # #  #      #  #   #    #    #\n#  #   #   #    #  #  #  #  ###     ###  ###  ###     #\n#_____________________________#_________________________________________________\n                           ###\n",
  "attinyx313":"\n#*******************************************************************************\n#   ###  #####                          ###    #  ###\n#  #   #   #    #  #                       #  ##     #\n#  #####   #   ###    ###  #  #     # #  ##    #   ##\n#  #   #   #    #  #  #  # #  #      #     #   #     #\n#  #   #   #    #  #  #  #  ###     # # ###   ### ###\n#_____________________________#_________________________________________________\n                           ###\n",
  "attiny43u":"\n#*******************************************************************************\n#   ###  #####                      #  # ###\n#  #   #   #    #  #                #  #    #\n#  #####   #   ###    ###  #  #     ####  ##\n#  #   #   #    #  #  #  # #  #        #    #\n#  #   #   #    #  #  #  #  ###        # ###\n#_____________________________#_________________________________________________\n                           ###\n",
  "attinyx4opti":"\n#*******************************************************************************\n#   ###  #####                          #  #      ###\n#  #   #   #    #  #                    #  #     #   #       #  #\n#  #####   #   ###    ###  #  #     # # ####     #   # ###  ###\n#  #   #   #    #  #  #  # #  #      #     #     #   # #  #  #  #\n#  #   #   #    #  #  #  #  ###     # #    #      ###  ###   #  #\n#_____________________________#________________________#________________________\n                           ###                         #\n",
  "attinyx5opti":"\n#*******************************************************************************\n#   ###  #####                          ####      ###\n#  #   #   #    #  #                    #        #   #       #  #\n#  #####   #   ###    ###  #  #     # # ###      #   # ###  ###\n#  #   #   #    #  #  #  # #  #      #     #     #   # #  #  #  #\n#  #   #   #    #  #  #  #  ###     # # ###       ###  ###   #  #\n#_____________________________#________________________#________________________\n                           ###                         #\n",
  "attinyx8opti":"\n#*******************************************************************************\n#   ###  #####                           ###       ###\n#  #   #   #    #  #                    #   #     #   #       #  #\n#  #####   #   ###    ###  #  #     # #  ###      #   # ###  ###\n#  #   #   #    #  #  #  # #  #      #  #   #     #   # #  #  #  #\n#  #   #   #    #  #  #  #  ###     # #  ###       ###  ###   #  #\n#_____________________________#_________________________#_______________________\n                           ###                          #\n",
  "attinyx7opti":"\n#*******************************************************************************\n#   ###  #####                          #####      ###\n#  #   #   #    #  #                       #      #   #       #  #\n#  #####   #   ###    ###  #  #     # #   #       #   # ###  ###\n#  #   #   #    #  #  #  # #  #      #    #       #   # #  #  #  #\n#  #   #   #    #  #  #  #  ###     # #   #        ###  ###   #  #\n#_____________________________#_________________________#_______________________\n                           ###                          #\n",
  "attinyx61opti":"\n#*******************************************************************************\n#   ###  #####                            #     #       ###\n#  #   #   #    #  #                     #     ##      #   #       #  #\n#  #####   #   ###    ###  #  #     # # ####    #      #   # ###  ###\n#  #   #   #    #  #  #  # #  #      #  #   #   #      #   # #  #  #  #\n#  #   #   #    #  #  #  #  ###     # #  ###   ###      ###  ###   #  #\n#_____________________________#______________________________#__________________\n                           ###                               #\n",
  "attinyx41opti":"\n#*******************************************************************************\n#   ###  #####                          #  #   #       ###\n#  #   #   #    #  #                    #  #  ##      #   #       #  #\n#  #####   #   ###    ###  #  #     # # ####   #      #   # ###  ###\n#  #   #   #    #  #  #  # #  #      #     #   #      #   # #  #  #  #\n#  #   #   #    #  #  #  #  ###     # #    #  ###      ###  ###   #  #\n#_____________________________#_____________________________#___________________\n                           ###                              #\n",
  "attiny828opti":"\n#*******************************************************************************\n#   ###  #####                       ###   ##   ###       ###\n#  #   #   #    #  #                #   # #  # #   #     #   #       #  #\n#  #####   #   ###    ###  #  #      ###    #   ###      #   # ###  ###\n#  #   #   #    #  #  #  # #  #     #   #  #   #   #     #   # #  #  #  #\n#  #   #   #    #  #  #  #  ###      ###  ####  ###       ###  ###   #  #\n#_____________________________#________________________________#________________\n                           ###                                 #\n",
  "attiny1634opti":"\n#*******************************************************************************\n#   ###  #####                       #    #   ###  #  #      ###\n#  #   #   #    #  #                ##   #       # #  #     #   #       #  #\n#  #####   #   ###    ###  #  #      #  ####   ##  ####     #   # ###  ###\n#  #   #   #    #  #  #  # #  #      #  #   #    #    #     #   # #  #  #  #\n#  #   #   #    #  #  #  #  ###     ###  ###  ###     #      ###  ###   #  #\n#_____________________________#____________________________________#____________\n                           ###                                     #\n",
  "attiny84mi12":"\n#*******************************************************************************\n#   ###  #####                          #  #         #   #  ###  ####\n#  #   #   #    #  #                    #  #         #   # #     #   #\n#  #####   #   ###    ###  #  #     # # ####     # # #   #  ###  ####\n#  #   #   #    #  #  #  # #  #      #     #     # # #   #     # #   #\n#  #   #   #    #  #  #  #  ###     # #    #      #   ###  ####  ####\n#_____________________________#_________________________________________________\n                           ###\n",
  "attiny84micr":"\n#*******************************************************************************\n#   ###  #####                          #  #         #   #  ###  ####\n#  #   #   #    #  #                    #  #         #   # #     #   #\n#  #####   #   ###    ###  #  #     # # ####     # # #   #  ###  ####\n#  #   #   #    #  #  #  # #  #      #     #     # # #   #     # #   #\n#  #   #   #    #  #  #  #  ###     # #    #      #   ###  ####  ####\n#_____________________________#_________________________________________________\n                           ###\n",
  "attiny85micr":"\n#*******************************************************************************\n#   ###  #####                          ####         #   #  ###  ####\n#  #   #   #    #  #                    #            #   # #     #   #\n#  #####   #   ###    ###  #  #     # # ###      # # #   #  ###  ####\n#  #   #   #    #  #  #  # #  #      #     #     # # #   #     # #   #\n#  #   #   #    #  #  #  #  ###     # # ###       #   ###  ####  ####\n#_____________________________#_________________________________________________\n                           ###\n",
  "attiny88micr":"\n#*******************************************************************************\n#   ###  #####                           ###          #   #  ###  ####\n#  #   #   #    #  #                    #   #         #   # #     #   #\n#  #####   #   ###    ###  #  #     # #  ###      # # #   #  ###  ####\n#  #   #   #    #  #  #  # #  #      #  #   #     # # #   #     # #   #\n#  #   #   #    #  #  #  #  ###     # #  ###       #   ###  ####  ####\n#_____________________________#_________________________________________________\n                           ###\n",
  "attiny167micr":"\n#*******************************************************************************\n#   ###  #####                       #    #  #####         #   #  ###  ####\n#  #   #   #    #  #                ##   #       #         #   # #     #   #\n#  #####   #   ###    ###  #  #      #  ####    #      # # #   #  ###  ####\n#  #   #   #    #  #  #  # #  #      #  #   #  #       # # #   #     # #   #\n#  #   #   #    #  #  #  #  ###     ###  ###  #         #   ###  ####  ####\n#_____________________________#_________________________________________________\n                           ###\n",
  "attiny87micr":"\n#*******************************************************************************\n#   ###  #####                       ### #####         #   #  ###  ####\n#  #   #   #    #  #                #   #    #         #   # #     #   #\n#  #####   #   ###    ###  #  #      ###    #      # # #   #  ###  ####\n#  #   #   #    #  #  #  # #  #     #   #  #       # # #   #     # #   #\n#  #   #   #    #  #  #  #  ###      ###  #         #   ###  ####  ####\n#_____________________________#_________________________________________________\n                           ###\n",
  "attiny861micr":"\n#*******************************************************************************\n#   ###  #####                            #    ##          #   #  ###  ####\n#  #   #   #    #  #                     #    ###          #   # #     #   #\n#  #####   #   ###    ###  #  #     # # ####   ##      # # #   #  ###  ####\n#  #   #   #    #  #  #  # #  #      #  #   #  ##      # # #   #     # #   #\n#  #   #   #    #  #  #  #  ###     # #  ###  ####      #   ###  ####  ####\n#_____________________________#_________________________________________________\n                           ###\n",
  "attiny841micr":"\n#*******************************************************************************\n#   ###  #####                          #  #   #          #   #  ###  ####\n#  #   #   #    #  #                    #  #  ##          #   # #     #   #\n#  #####   #   ###    ###  #  #     # # ####   #      # # #   #  ###  ####\n#  #   #   #    #  #  #  # #  #      #     #   #      # # #   #     # #   #\n#  #   #   #    #  #  #  #  ###     # #    #  ###      #   ###  ####  ####\n#_____________________________#_________________________________________________\n                           ###\n",
  "attiny1634micr":"\n#*******************************************************************************\n#   ###  #####                       #    #   ###  #  #        #   #  ###  ####\n#  #   #   #    #  #                ##   #       # #  #        #   # #     #   #\n#  #####   #   ###    ###  #  #      #  ####   ##  ####    # # #   #  ###  ####\n#  #   #   #    #  #  #  # #  #      #  #   #    #    #    # # #   #     # #   #\n#  #   #   #    #  #  #  #  ###     ###  ###  ###     #     #   ###  ####  ####\n#_____________________________#_________________________________________________\n                           ###\n"
}


boards = {
  "header1":{
    "header":boards_no_loader
  },
  "attinyx4":{
    "title":True,
    "chipmenu":True,
    "bootloader":"",
    "haspll":False,
    "hasvoltdependance":False,
    "defaultvariant":"tinyx4_cw",
    "hasxtal":True,
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "haswdt":True,
    "wdtcode":"4",
    "fancybod":False,
    "pinmap":[
      ".menu.pinmap.default=Standard (clockwise)",
      ".menu.pinmap.ccw=Legacy (counterclockwise)",
      ".menu.pinmap.ccw.build.variant=tinyx4_legacy",
      ".menu.pinmap.default.build.pinmapabr=.cw",
      ".menu.pinmap.ccw.build.pinmapabr=.ccw"],
    "softser":True,
    "fancysoftser":False,
    "ssport":"A",
    "ssrx":"2",
    "sstx":"1",
    "hfuse":"0b{bootloader.rstbit}101{bootloader.eesave_bit}{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "attinyx5":{
    "title":True,
    "chipmenu":True,
    "bootloader":"",
    "haspll":True,
    "hasvoltdependance":False,
    "defaultvariant":"tinyx5",
    "hasxtal":True,
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "haswdt":True,
    "wdtcode":"4",
    "fancybod":False,
    "softser":True,
    "fancysoftser":False,
    "ssport":"B",
    "ssrx":"1",
    "sstx":"0",
    "hfuse":"0b{bootloader.rstbit}101{bootloader.eesave_bit}{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "attinyx8":{
    "title":True,
    "chipmenu":True,
    "bootloader":"",
    "haspll":False,
    "hasvoltdependance":False,
    "defaultvariant":"tinyx8",
    "lfuse_ext":"0xE0",
    "hasxtal":False,
    "haswdt":True,
    "wdtcode":"3",
    "fancybod":False,
    "pinmap":[
      ".menu.pinmap.default=Standard",
      ".menu.pinmap.mhet=MH-ET Tiny",
      ".menu.pinmap.mhet.build.variant=tinyx8_mh",
      ".menu.pinmap.default.build.pinmapabr=",
      ".menu.pinmap.mhet.build.pinmapabr=.mhet"],
    "softser":True,
    "fancysoftser":False,
    "ssport":"D",
    "ssrx":"7",
    "sstx":"6",
    "hfuse":"0b{bootloader.rstbit}101{bootloader.eesave_bit}{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "attinyx7":{
    "title":True,
    "chipmenu":True,
    "sram":"512",
    "bootloader":"",
    "haspll":False,
    "hasvoltdependance":False,
    "defaultvariant":"tinyx7",
    "hasxtal":True,
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "haswdt":True,
    "wdtcode":"3",
    "pinmap":[
      ".menu.pinmap.default=Standard",
      ".menu.pinmap.digi=Digispark Pro",
      ".menu.pinmap.digi.build.variant=tinyx7_digi",
      ".menu.pinmap.legacy=Legacy",
      ".menu.pinmap.legacy.build.variant=tinyx7_legacy",
      ".menu.pinmap.default.build.pinmapabr=",
      ".menu.pinmap.digi.build.pinmapabr=.digi",
      ".menu.pinmap.legacy.build.pinmapabr=.old"],
    "fancybod":False,
    "softser":False,
    "fancysoftser":False,
    "hfuse":"0b{bootloader.rstbit}101{bootloader.eesave_bit}{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "attiny26":{
    "title":True,
    "chipmenu":False,
    "flash":"2048",
    "sram":"128",
    "bootloader":"",
    "haspll":False,
    "hasvoltdependance":False,
    "defaultvariant":"tiny26",
    "internalclock":[
      [[".menu.clock.internal_1m=1 MHz (internal) Start-up time: 6 CK + 64ms",
        ".menu.clock.internal_1m.bootloader.low_fuses=0xE1",
        ".menu.clock.internal_1m.build.f_cpu=1000000UL",
        ".menu.clock.internal_1m.build.speed=1m",
        ".menu.clock.internal_1m.build.clocksource=0x10"],
        getspeed(".menu.clock.internal_1m.upload.speed=","1")],
      [[".menu.clock.internal_2m=2 MHz (internal) Start-up time: 6 CK + 64ms",
        ".menu.clock.internal_2m.bootloader.low_fuses=0xD2",
        ".menu.clock.internal_2m.build.f_cpu=2000000UL",
        ".menu.clock.internal_2m.build.speed=2m",
        ".menu.clock.internal_2m.build.clocksource=0x10"],
        getspeed(".menu.clock.internal_2m.upload.speed=","2")],
      [[".menu.clock.internal_4m=4 MHz (internal) Start-up time: 6 CK + 64ms",
        ".menu.clock.internal_4m.bootloader.low_fuses=0xE3",
        ".menu.clock.internal_4m.build.f_cpu=4000000UL",
        ".menu.clock.internal_4m.build.speed=4m",
        ".menu.clock.internal_4m.build.clocksource=0x10"],
        getspeed(".menu.clock.internal_4m.upload.speed=","4")],
      [[".menu.clock.internal_8m=8 MHz (internal) Start-up time: 6 CK + 64ms",
        ".menu.clock.internal_8m.bootloader.low_fuses=0xE4",
        ".menu.clock.internal_8m.build.f_cpu=8000000UL",
        ".menu.clock.internal_8m.build.speed=8m",
        ".menu.clock.internal_8m.build.clocksource=0x10"],
        getspeed(".menu.clock.internal_8m.upload.speed=","8")],
      [[".menu.clock.pll_16m=16 MHz (PLL) Start-up time: 1k CK + 64ms",
        ".menu.clock.pll_16m.bootloader.low_fuses=0x61",
        ".menu.clock.pll_16m.build.f_cpu=16000000UL",
        ".menu.clock.pll_16m.build.speed=16m",
        ".menu.clock.pll_16m.build.clocksource=6"],
        getspeed(".menu.clock.pll_16m.upload.speed=","16")],
    ],
    "bodmenu": [
      ".menu.bod.disable=B.O.D. Disabled (saves power)", ".menu.bod.disable.bootloader.bod_bits=11",
      ".menu.bod.2v7=B.O.D. Enabled (2.7v)", ".menu.bod.2v7.bootloader.bod_bits=10",
      ".menu.bod.4v3=B.O.D. Enabled (4.3v)", ".menu.bod.4v3.bootloader.bod_bits=00"
    ],
    "notunedclocks":True,
    "hasxtal":True,
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "haswdt":False,
    "fancybod":False,
    "softser":True,
    "fancysoftser":False,
    "ssport":"A",
    "ssrx":"7",
    "sstx":"6",
    "hfuse":"0b111{bootloader.rstbit}0{bootloader.eesave_bit}{bootloader.bod_bits}",
    "efuse":"0xFF",
  },
    "attinyx61":{
    "title":True,
    "chipmenu":True,
    "bootloader":"",
    "haspll":True,
    "hasvoltdependance":False,
    "defaultvariant":"tinyx61",
    "hasxtal":True,
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "haswdt":True,
    "wdtcode":"3",
    "fancybod":False,
    "pinmap":[
      ".menu.pinmap.default=Standard",
      ".menu.pinmap.legacy=Legacy",
      ".menu.pinmap.default.build.variant=tinyx61",
      ".menu.pinmap.legacy.build.variant=tinyx61_legacy",
      ".menu.pinmap.default.build.pinmapabr=",
      ".menu.pinmap.legacy.build.pinmapabr=.old"],
    "remap":[
      ".menu.remap.default=USI on PB0-PB2",
      ".menu.remap.alternate=USI on PA0-PA2",
      ".menu.remap.default.build.remap=",
      ".menu.remap.alternate.build.remap-=DSET_REMAPUSI",
      ".menu.remap.default.build.remapabr=",
      ".menu.remap.alternate.build.remapabr=rU"],
    "softser":True,
    "fancysoftser":True,
    "hfuse":"0b{bootloader.rstbit}101{bootloader.eesave_bit}{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "attinyx41":{
    "title":True,
    "chipmenu":True,
    "bootloader":"",
    "haspll":False,
    "hasvoltdependance":True,
    "defaultvariant":"tinyx41_cw",
    "hasxtal":True,
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xEE",
    "lfuse_xl":"0xED",
    "hasulp":True,
    "haswdt":False,
    "fancybod":True,
    "wiremodes":True,
    "pinmap":[
      ".menu.pinmap.default=Standard (clockwise)",
      ".menu.pinmap.ccw=Legacy (counterclockwise)",
      ".menu.pinmap.ccw.build.variant=tinyx41_legacy",
      ".menu.pinmap.default.build.pinmapabr=.cw",
      ".menu.pinmap.ccw.build.pinmapabr=.ccw"],
    "remap":[
      ".menu.remap.defaultboth=UART0: TX PA1, RX PA2. SPI: SCK PA4, MISO PA5, MOSI PA6, SS PA7",
      ".menu.remap.defaultuart=UART0: TX PA1, RX PA2. SPI: SCK PA3, MISO PA0, MOSI PA1, SS PA2",
      ".menu.remap.defaultspi=UART0: TX PA7, RX PB2. SPI: SCK PA4, MISO PA5, MOSI PA6, SS PA7",
      ".menu.remap.altboth=UART0: TX PA7, RX PB2. SPI: SCK PA3, MISO PA0, MOSI PA1, SS PA2",
      ".menu.remap.defaultboth.build.remap=",
      ".menu.remap.defaultuart.build.remap=-DSET_REMAP=2",
      ".menu.remap.defaultspi.build.remap=-DSET_REMAP=1",
      ".menu.remap.altboth.build.remap=-DSET_REMAP=3",
      ".menu.remap.defaultboth.build.remapabr=",
      ".menu.remap.defaultuart.build.remapabr=rS",
      ".menu.remap.defaultspi.build.remapabr=rU",
      ".menu.remap.altboth.build.remapabr=rB"],
    "softser":False,
    "fancysoftser":False,
    "hfuse":"0b{bootloader.rstbit}101{bootloader.eesave_bit}{bootloader.bod_bits}",
    "efuse":"0b{bootloader.ulp_bits}{bootloader.bodpd_bits}{bootloader.bodact_bits}0",
  },
    "attiny828":{
    "title":True,
    "chipmenu":False,
    "flash":"8192",
    "sram":"512",
    "bootloader":"",
    "haspll":False,
    "hasvoltdependance":True,
    "defaultvariant":"tiny828",
    "lfuse_ext":"0xEC",
    "hasxtal":False,
    "haswdt":False,
    "fancybod":True,
    "wiremodes":True,
    "softser":False,
    "fancysoftser":False,
    "hfuse":"0b{bootloader.rstbit}101{bootloader.eesave_bit}{bootloader.bod_bits}",
    "efuse":"0b{bootloader.bodpd_bits}{bootloader.bodact_bits}1111",
  },
    "attiny1634":{
    "title":True,
    "chipmenu":False,
    "flash":"16384",
    "sram":"1024",
    "bootloader":"",
    "haspll":False,
    "hasvoltdependance":True,
    "defaultvariant":"tiny1634_cw",
    "hasxtal":True,
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "haswdt":False,
    "fancybod":True,
    "pinmap":[
      ".menu.pinmap.default=Standard (clockwise)",
      ".menu.pinmap.ccw=Legacy (counterclockwise)",
      ".menu.pinmap.ccw.build.variant=tiny1634_legacy",
      ".menu.pinmap.default.build.pinmapabr=.cw",
      ".menu.pinmap.ccw.build.pinmapabr=.ccw"],
    "softser":False,
    "fancysoftser":False,
    "hfuse":"0b{bootloader.rstbit}101{bootloader.eesave_bit}{bootloader.bod_bits}",
    "efuse":"0b111{bootloader.bodpd_bits}{bootloader.bodact_bits}0",
  },
    "attinyx313":{
    "title":True,
    "chipmenu":True,
    "bootloader":"",
    "haspll":False,
    "hasvoltdependance":False,
    "defaultvariant":"tinyx313",
    "internalclock":[[[".menu.clock.internal_8m=8 MHz (internal)", ".menu.clock.internal_8m.bootloader.low_fuses=0xE4",
       ".menu.clock.internal_8m.build.f_cpu=8000000UL", ".menu.clock.internal_8m.build.speed=8m",
       ".menu.clock.internal_8m.build.clocksource=0"],getspeed(".menu.clock.internal_8m.upload.speed=","8")],
      [[".menu.clock.internal_1m=1 MHz (internal)", ".menu.clock.internal_1m.bootloader.low_fuses=0x64",
       ".menu.clock.internal_1m.build.f_cpu=1000000UL", ".menu.clock.internal_1m.build.speed=1m",
       ".menu.clock.internal_1m.build.clocksource=0x10"], getspeed(".menu.clock.internal_1m.upload.speed=","1")],
      [[".menu.clock.internal_4m=4 MHz (internal)", ".menu.clock.internal_4m.bootloader.low_fuses=0xE2",
        ".menu.clock.internal_4m.build.f_cpu=4000000UL", ".menu.clock.internal_4m.build.speed=4m",
        ".menu.clock.internal_4m.build.clocksource=0x10"], getspeed(".menu.clock.internal_4m.upload.speed=","4")],
      [[".menu.clock.internal_2m=2 MHz (internal)", ".menu.clock.internal_2m.bootloader.low_fuses=0x64",
       ".menu.clock.internal_2m.build.f_cpu=2000000UL", ".menu.clock.internal_2m.build.speed=2m",
       ".menu.clock.internal_2m.build.clocksource=0x10",".menu.clock.internal_2m.bootloader.f_cpu=1000000UL"], getspeed(".menu.clock.internal_2m.upload.speed=","2")],
      [[".menu.clock.internal_500k=0.5 MHz (internal)", ".menu.clock.internal_500k.bootloader.low_fuses=0x62",
       ".menu.clock.internal_500k.build.f_cpu=500000UL", ".menu.clock.internal_500k.build.speed=500k",
       ".menu.clock.internal_500k.build.clocksource=0x10",".menu.clock.internal_500k.bootloader.f_cpu=500000UL"], getspeed(".menu.clock.internal_2m.upload.speed=","2")]],
    "hasxtal":True,
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "haswdt":True,
    "wdtcode":"6",
    "fancybod":False,
    "softser":False,
    "fancysoftser":False,
    "hfuse":"0b1{bootloader.eesave_bit}01{bootloader.bod_bits}1",
    "efuse":"0xFE",
  },
    "attiny43u":{
    "title":True,
    "chipmenu":False,
    "flash":"4096",
    "sram":"256",
    "bootloader":"",
    "haspll":False,
    "hasvoltdependance":False,
    "defaultvariant":"tiny43",
    "hasxtal":False,
    "onlytunesto8":True,
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "haswdt":True,
    "wdtcode":"3",
    "fancybod":False,
    "softser":True,
    "fancysoftser":False,
    "ssport":"A",
    "ssrx":"5",
    "sstx":"4",
    "hfuse":"0b{bootloader.rstbit}101{bootloader.eesave_bit}{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "header2":{
  "header":boards_optiboot
  },
    "attinyx4opti":{
    "title":True,
    "chipmenu":True,
    "bootloader":"Optiboot",
    "haspll":False,
    "hasvoltdependance":False,
    "defaultvariant":"tinyx4_cw",
    "hasxtal":True,
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "haswdt":False,
    "fancybod":False,
    "pinmap":[
     ".menu.pinmap.default=Standard (clockwise)",
      ".menu.pinmap.ccw=Legacy (counterclockwise)",
      ".menu.pinmap.ccw.build.variant=tinyx4_legacy",
      ".menu.pinmap.default.build.pinmapabr=.cw",
      ".menu.pinmap.ccw.build.pinmapabr=.ccw"],
    "softser":True,
    "fancysoftser":False,
    "ssport":"A",
    "ssrx":"2",
    "sstx":"1",
    "hfuse":"0b{bootloader.rstbit}1011{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "attinyx5opti":{
    "title":True,
    "chipmenu":True,
    "bootloader":"Optiboot",
    "haspll":True,
    "hasvoltdependance":False,
    "defaultvariant":"tinyx5",
    "hasxtal":True,
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "haswdt":False,
    "fancybod":False,
    "softser":True,
    "fancysoftser":False,
    "ssport":"B",
    "ssrx":"1",
    "sstx":"0",
    "hfuse":"0b{bootloader.rstbit}1011{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "attinyx8opti":{
    "title":True,
    "chipmenu":True,
    "bootloader":"Optiboot",
    "haspll":False,
    "hasvoltdependance":False,
    "defaultvariant":"tinyx8",
    "lfuse_ext":"0xE0",
    "hasxtal":False,
    "haswdt":False,
    "fancybod":False,
    "pinmap":[
      ".menu.pinmap.default=Standard",
      ".menu.pinmap.mhet=MH-ET Tiny",
      ".menu.pinmap.,mhet.build.variant=tinyx8_mh",
      ".menu.pinmap.default.build.pinmapabr=",
      ".menu.pinmap.mhet.build.pinmapabr=.mhet"],
    "softser":True,
    "fancysoftser":False,
    "ssport":"D",
    "ssrx":"7",
    "sstx":"6",
    "hfuse":"0b{bootloader.rstbit}1011{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "attinyx7opti":{
    "title":True,
    "chipmenu":True,
    "sram":"512",
    "bootloader":"Optiboot",
    "haspll":False,
    "hasvoltdependance":False,
    "defaultvariant":"tinyx7",
    "hasxtal":True,
    "haslin":True,
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "haswdt":False,
    "fancybod":False,
    "pinmap":[
      ".menu.pinmap.default=Standard",
      ".menu.pinmap.digi=Digispark Pro",
      ".menu.pinmap.digi.build.variant=tinyx7_digi",
      ".menu.pinmap.legacy=Legacy",
      ".menu.pinmap.legacy.build.variant=tinyx7_legacy",
      ".menu.pinmap.default.build.pinmapabr=",
      ".menu.pinmap.digi.build.pinmapabr=.digi",
      ".menu.pinmap.legacy.build.pinmapabr=.old"],
    "softser":False,
    "fancysoftser":False,
    "hfuse":"0b{bootloader.rstbit}1011{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "attinyx61opti":{
    "title":True,
    "chipmenu":True,
    "bootloader":"Optiboot",
    "haspll":True,
    "hasvoltdependance":False,
    "defaultvariant":"tinyx61",
    "hasxtal":True,
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "haswdt":False,
    "fancybod":False,
    "pinmap":[
      ".menu.pinmap.default=Standard",
      ".menu.pinmap.legacy=Legacy",
      ".menu.pinmap.default.build.variant=tinyx61",
      ".menu.pinmap.legacy.build.variant=tinyx61_legacy",
      ".menu.pinmap.default.build.pinmapabr=",
      ".menu.pinmap.legacy.build.pinmapabr=.old"],
    "remap":[
      ".menu.remap.default=USI on PB0-PB2",
      ".menu.remap.alternate=USI on PA0-PA2",
      ".menu.remap.default.build.remap=",
      ".menu.remap.alternate.build.remap-=DSET_REMAPUSI",
      ".menu.remap.default.build.remapabr=",
      ".menu.remap.alternate.build.remapabr=rU"],
    "softser":True,
    "fancysoftser":True,
    "hfuse":"0b{bootloader.rstbit}1011{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "attinyx41opti":{
    "title":True,
    "chipmenu":True,
    "bootloader":"Optiboot",
    "haspll":False,
    "hasvoltdependance":True,
    "defaultvariant":"tinyx41_cw",
    "hasxtal":True,
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xEE",
    "lfuse_xl":"0xED",
    "haswdt":False,
    "fancybod":True,
    "wiremodes":True,
    "twouart":True,
    "uartremap":True,
    "pinmap":[
      ".menu.pinmap.default=Standard (clockwise)",
      ".menu.pinmap.ccw=Legacy (counterclockwise)",
      ".menu.pinmap.ccw.build.variant=tinyx41_legacy",
      ".menu.pinmap.default.build.pinmapabr=.cw",
      ".menu.pinmap.ccw.build.pinmapabr=.ccw"],
    "remap":[
      ".menu.remap.defaultboth=UART0: TX PA1, RX PA2. SPI: SCK PA4, MISO PA5, MOSI PA6, SS PA7",
      ".menu.remap.defaultuart=UART0: TX PA1, RX PA2. SPI: SCK PA3, MISO PA0, MOSI PA1, SS PA2",
      ".menu.remap.defaultspi=UART0: TX PA7, RX PB2 SPI: SCK PA4, MISO PA5, MOSI PA6, SS PA7",
      ".menu.remap.altboth=UART0: TX PA7, RX PB2. SPI: SCK PA3, MISO PA0, MOSI PA1, SS PA2",
      ".menu.remap.defaultboth.build.remap=",
      ".menu.remap.defaultuart.build.remap=-DSET_REMAP=2",
      ".menu.remap.defaultspi.build.remap=-DSET_REMAP=1",
      ".menu.remap.altboth.build.remap=-DSET_REMAP=3",
      ".menu.remap.defaultboth.build.remapabr=",
      ".menu.remap.defaultuart.build.remapabr=rS",
      ".menu.remap.defaultspi.build.remapabr=rU",
      ".menu.remap.altboth.build.remapabr=rB"],
    "optipins":[
      ".menu.bootloaderuart.uart0=Serial 0 (TX: PA1, RX: PA2, LED: PB2)",
      ".menu.bootloaderuart.uartr=Serial 0 alt (TX: PA7, RX: PB2, LED: PA2)",
      ".menu.bootloaderuart.uart1=Serial 1 (TX: PA4, RX: PA5, LED: PB2)",
      ".menu.bootloaderuart.uart0.bootloader.uart=",
      ".menu.bootloaderuart.uartr.bootloader.uart=_serR",
      ".menu.bootloaderuart.uart1.bootloader.uart=_ser1"],
    "softser":False,
    "fancysoftser":False,
    "hfuse":"0b{bootloader.rstbit}1011{bootloader.bod_bits}",
    "efuse":"0b111{bootloader.bodpd_bits}{bootloader.bodact_bits}0",
  },
    "attiny828opti":{
    "title":True,
    "chipmenu":False,
    "flash":"7680",
    "sram":"512",
    "bootloader":"Optiboot",
    "haspll":False,
    "hasvoltdependance":True,
    "defaultvariant":"tiny828",
    "lfuse_ext":"0xEC",
    "hasxtal":False,
    "haswdt":False,
    "fancybod":True,
    "wiremodes":True,
    "softser":False,
    "fancysoftser":False,
    "hfuse":"0b{bootloader.rstbit}1011{bootloader.bod_bits}",
    "efuse":"0b{bootloader.bodpd_bits}{bootloader.bodact_bits}1100",
  },
    "attiny1634opti":{
    "title":True,
    "chipmenu":False,
    "flash":"15744",
    "sram":"1024",
    "bootloader":"Optiboot",
    "haspll":False,
    "hasvoltdependance":True,
    "defaultvariant":"tiny1634_cw",
    "hasxtal":True,
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "haswdt":False,
    "fancybod":True,
    "twouart":True,
    "pinmap":[
      ".menu.pinmap.default=Standard (clockwise)",
      ".menu.pinmap.ccw=Legacy (counterclockwise)",
      ".menu.pinmap.ccw.build.variant=tiny1634_legacy",
      ".menu.pinmap.default.build.pinmapabr=.cw",
      ".menu.pinmap.ccw.build.pinmapabr=.ccw"],
    "optipins":[
      ".menu.bootloaderuart.uart0=Serial 0 (TX: PB0, RX: PA7, LED: PC0)",
      ".menu.bootloaderuart.uart1=Serial 1 (TX: PB2, RX: PB1, LED: PC0)",
      ".menu.bootloaderuart.uart0.bootloader.uart=",
      ".menu.bootloaderuart.uart1.bootloader.uart=_ser1"],
    "softser":False,
    "fancysoftser":False,
    "hfuse":"0b{bootloader.rstbit}1011{bootloader.bod_bits}",
    "efuse":"0b111{bootloader.bodpd_bits}{bootloader.bodact_bits}0",
  },
    "header3":{
  "header":boards_micronucleus
  },
    "attiny84mi12":{
    "title":True,
    "chipmenu":False,
    "bootname":"attiny84",
    "flash":"6588",
    "entrymodesizes":[
      ".menu.bootentry.jumper.upload.maximum_size=6524"],
    "sram":"512",
    "bootloader":"Micronucleus",
    "clock":[
      ".menu.clock.internal_8m=8 MHz",
      ".menu.clock.internal_8m.bootloader.low_fuses=0xE2",
      ".menu.clock.internal_8m.build.f_cpu=8000000UL",
      ".menu.clock.internal_8m.build.speed=8m",
      ".menu.clock.internal_8m.build.clocksource=0",
      ".menu.clock.internal_8m.build.extra_flags=-DBOOT_TUNED120",
      ".menu.clock.internal_12m=12 MHz (tuned by bootloader)",
      ".menu.clock.internal_12m.bootloader.low_fuses=0xE2",
      ".menu.clock.internal_12m.build.f_cpu=12000000UL",
      ".menu.clock.internal_12m.build.speed=12m",
      ".menu.clock.internal_12m.build.clocksource=0",
      ".menu.clock.internal_12m.build.extra_flags=-DBOOT_TUNED120",
      ".menu.clock.internal_1m=1 MHz internal",
      ".menu.clock.internal_1m.bootloader.low_fuses=0x62",
      ".menu.clock.internal_1m.build.f_cpu=1000000UL",
      ".menu.clock.internal_1m.build.speed=1m",
      ".menu.clock.internal_1m.build.clocksource=0",
      ".menu.clock.internal_1m.build.extra_flags=-DBOOT_TUNED120"],
    "hasvoltdependance":False,
    "defaultvariant":"tinyx4_cw",
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "fancybod":False,
    "rstdisbl":True,
    "pinmap":[
      ".menu.pinmap.default=Standard (clockwise)",
      ".menu.pinmap.ccw=Legacy (counterclockwise)",
      ".menu.pinmap.ccw.build.variant=tinyx4_legacy",
      ".menu.pinmap.default.build.pinmapabr=.cw",
      ".menu.pinmap.ccw.build.pinmapabr=.ccw"],
    "USBPins":[
      ".menu.usbpins.pb2plus=PB0: LED, PB1: D-, PB2: D+",
      ".menu.usbpins.pb1plus=PB0: D-, PB1: D+, PB2: LED",
      ".menu.usbpins.pb0plus=PB0: D+, PB1: D-, PB2: LED",
      ".menu.usbpins.pb2plus.build.usbpinset=-DUSB_TWOPLUS -DLED_BUILTIN=PIN_PB0",
      ".menu.usbpins.pb1plus.build.usbpinset=-DUSB_ONEPLUS",
      ".menu.usbpins.pb0plus.build.usbpinset=-DUSB_ZEROPLUS",
      ".menu.usbpins.pb2plus.bootloader.usbpinsetname=_pb2plus",
      ".menu.usbpins.pb1plus.bootloader.usbpinsetname=_pb1plus",
      ".menu.usbpins.pb0plus.bootloader.usbpinsetname=_pb0plus"],
    "softser":True,
    "fancysoftser":False,
    "ssport":"A",
    "ssrx":"2",
    "sstx":"1",
    "hfuse":"0b{bootloader.rstbit}1011{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "attiny84micr":{
    "title":True,
    "chipmenu":False,
    "bootname":"attiny84_128",
    "flash":"6268",
    "sram":"512",
    "bootloader":"Micronucleus",
    "clock":[
      ".menu.clock.internal_8m=8 MHz",
      ".menu.clock.internal_8m.bootloader.low_fuses=0xE2",
      ".menu.clock.internal_8m.build.f_cpu=8000000UL",
      ".menu.clock.internal_8m.build.speed=8m",
      ".menu.clock.internal_8m.build.clocksource=0",
      ".menu.clock.internal_8m.build.extra_flags=-DBOOT_TUNED120",
      ".menu.clock.internal_12m8=12.8 MHz (tuned by bootloader)",
      ".menu.clock.internal_12m8.bootloader.low_fuses=0xE2",
      ".menu.clock.internal_12m8.build.f_cpu=12800000L",
      ".menu.clock.internal_12m8.build.speed=12m8",
      ".menu.clock.internal_12m8.build.clocksource=0",
      ".menu.clock.internal_12m8.build.extra_flags=-DBOOT_TUNED128",
      ".menu.clock.internal_1m=1 MHz internal",
      ".menu.clock.internal_1m.bootloader.low_fuses=0x62",
      ".menu.clock.internal_1m.build.f_cpu=1000000UL",
      ".menu.clock.internal_1m.build.speed=1m",
      ".menu.clock.internal_1m.build.clocksource=0",
      ".menu.clock.internal_1m.build.extra_flags=-DBOOT_TUNED128"],
    "hasvoltdependance":False,
    "defaultvariant":"tinyx4_cw",
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "fancybod":False,
    "rstdisbl":True,
    "pinmap":[
      ".menu.pinmap.default=Standard (clockwise)",
      ".menu.pinmap.ccw=Legacy (counterclockwise)",
      ".menu.pinmap.ccw.build.variant=tinyx4_legacy",
      ".menu.pinmap.default.build.pinmapabr=.cw",
      ".menu.pinmap.ccw.build.pinmapabr=.ccw"],
    "USBPins":[
      ".menu.usbpins.pb2plus=PB0: LED, PB1: D-, PB2: D+",
      ".menu.usbpins.pb1plus=PB0: D-, PB1: D+, PB2: LED",
      ".menu.usbpins.pb0plus=PB0: D+, PB1: D-, PB2: LED",
      ".menu.usbpins.pb2plus.build.usbpinset=-DUSB_TWOPLUS -DLED_BUILTIN=PIN_PB0",
      ".menu.usbpins.pb1plus.build.usbpinset=-DUSB_ONEPLUS",
      ".menu.usbpins.pb0plus.build.usbpinset=-DUSB_ZEROPLUS",
      ".menu.usbpins.pb2plus.bootloader.usbpinsetname=_pb2plus",
      ".menu.usbpins.pb1plus.bootloader.usbpinsetname=_pb1plus",
      ".menu.usbpins.pb0plus.bootloader.usbpinsetname=_pb0plus"],
    "softser":True,
    "fancysoftser":False,
    "ssport":"A",
    "ssrx":"2",
    "sstx":"1",
    "hfuse":"0b{bootloader.rstbit}1011{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "attiny85micr":{
    "title":True,
    "chipmenu":False,
    "bootname":"attiny85",
    "flash":"6524",
    "entrymodesizes":[
      ".menu.bootentry.always.upload.maximum_size=6588"],
    "sram":"512",
    "bootloader":"Micronucleus",
    "clock":[
      ".build.extra_flags=-DBOOT_TUNED165",
      ".menu.clock.pll_16m5=16.5 MHz (internal tuned PLL)",
      ".menu.clock.pll_16m5.bootloader.low_fuses=0xF1",
      ".menu.clock.pll_16m5.build.f_cpu=16500000UL",
      ".menu.clock.pll_16m5.build.speed=16m5",
      ".menu.clock.pll_16m5.build.clocksource=6",
      ".menu.clock.pll_16m=16 MHz (internal PLL)",
      ".menu.clock.pll_16m.bootloader.low_fuses=0xF1",
      ".menu.clock.pll_16m.build.f_cpu=16000000UL",
      ".menu.clock.pll_16m.build.speed=16",
      ".menu.clock.pll_16m.build.clocksource=6",
      ".menu.clock.pll_8m=8 MHz (prescaled 16 MHz internal PLL)",
      ".menu.clock.pll_8m.bootloader.low_fuses=0xF1",
      ".menu.clock.pll_8m.build.f_cpu=8000000UL",
      ".menu.clock.pll_8m.build.speed=8",
      ".menu.clock.pll_8m.build.clocksource=0x16",
      ".menu.clock.pll_4m=4 MHz (prescaled 16 MHz internal PLL)",
      ".menu.clock.pll_4m.bootloader.low_fuses=0xF1",
      ".menu.clock.pll_4m.build.f_cpu=4000000UL",
      ".menu.clock.pll_4m.build.speed=4",
      ".menu.clock.pll_4m.build.clocksource=0x16",
      ".menu.clock.pll_2m=2 MHz (prescaled 16 MHz internal PLL)",
      ".menu.clock.pll_2m.bootloader.low_fuses=0xF1",
      ".menu.clock.pll_2m.build.f_cpu=2000000UL",
      ".menu.clock.pll_2m.build.speed=2",
      ".menu.clock.pll_2m.build.clocksource=0x16",
      ".menu.clock.pll_1m=1 MHz (prescaled 16 MHz internal PLL)",
      ".menu.clock.pll_1m.bootloader.low_fuses=0xF1",
      ".menu.clock.pll_1m.build.f_cpu=1000000UL",
      ".menu.clock.pll_1m.build.speed=1",
      ".menu.clock.pll_1m.build.clocksource=0x16"],
    "defaultvariant":"tinyx5",
    "lfuse_int8":"0xE2",
    "lfuse_int4":"0x62",
    "lfuse_int2":"0x62",
    "lfuse_int1":"0x62",
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "fancybod":False,
    "rstdisbl":True,
    "USBPins":[
      ".menu.usbpins.standard=Standard, PB3: D- PB4: D+ PB1: LED",
      ".menu.usbpins.standard.bootloader.usbpinset=",
      ".menu.usbpins.standard.bootloader.usbpinsetname="],
    "softser":True,
    "fancysoftser":False,
    "ssport":"B",
    "ssrx":"1",
    "sstx":"0",
    "hfuse":"0b{bootloader.rstbit}1011{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "attiny88micr":{
    "title":True,
    "chipmenu":False,
    "bootname":"attiny88",
    "flash":"6782",
    "entrymodesizes":[
      ".menu.bootentry.jumper.upload.maximum_size=6718"],
    "sram":"512",
    "bootloader":"Micronucleus",
    "clock":[
      ".menu.clock.extclk_16m=16 MHz (external clock)",
      ".menu.clock.extclk_16m.bootloader.low_fuses=0xE0",
      ".menu.clock.extclk_16m.build.f_cpu=16000000UL",
      ".menu.clock.extclk_16m.build.speed=16",
      ".menu.clock.extclk_16m.build.clocksource=2",
      ".menu.clock.extclk_8m=8 MHz (prescaled 16 MHz external clock)",
      ".menu.clock.extclk_8m.bootloader.low_fuses=0xE0",
      ".menu.clock.extclk_8m.build.f_cpu=8000000UL",
      ".menu.clock.extclk_8m.build.speed=8",
      ".menu.clock.extclk_8m.build.clocksource=0x12",
      ".menu.clock.extclk_4m=4 MHz (prescaled 16 MHz external clock)",
      ".menu.clock.extclk_4m.bootloader.low_fuses=0xE0",
      ".menu.clock.extclk_4m.build.f_cpu=4000000UL",
      ".menu.clock.extclk_4m.build.speed=4",
      ".menu.clock.extclk_4m.build.clocksource=0x12",
      ".menu.clock.extclk_2m=2 MHz (prescaled 16 MHz external clock)",
      ".menu.clock.extclk_2m.bootloader.low_fuses=0xE0",
      ".menu.clock.extclk_2m.build.f_cpu=2000000UL",
      ".menu.clock.extclk_2m.build.speed=2",
      ".menu.clock.extclk_2m.build.clocksource=0x12",
      ".menu.clock.extclk_1m=1 MHz (prescaled 16 MHz external clock)",
      ".menu.clock.extclk_1m.bootloader.low_fuses=0xE0",
      ".menu.clock.extclk_1m.build.f_cpu=1000000UL",
      ".menu.clock.extclk_1m.build.speed=1",
      ".menu.clock.extclk_1m.build.clocksource=0x12"],
    "defaultvariant":"tinyx8_mh",
    "fancybod":False,
    "rstdisbl":True,
    "pinmap":[ ".menu.pinmap.mhet=MH-Tiny",
      ".menu.pinmap.default=Standard",
      ".menu.pinmap.mhet.build.variant=tinyx8_mh",
      ".menu.pinmap.default.build.pinmapabr=",
      ".menu.pinmap.mhet.build.pinmapabr=.mhet"],
    "USBPins":[
      ".menu.usbpins.standard=MH Tiny - PD1: D-, PD2: D+, PD0: LED",
      ".menu.usbpins.standard.bootloader.usbpinset=",
      ".menu.usbpins.standard.bootloader.usbpinsetname="],
    "softser":True,
    "fancysoftser":False,
    "lfuse_ext":"0xE0",
    "ssport":"D",
    "ssrx":"7",
    "sstx":"6",
    "hfuse":"0b{bootloader.rstbit}1011{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "attiny167micr":{
    "title":True,
    "chipmenu":False,
    "bootname":"attiny167",
    "flash":"14844",
    "sram":"512",
    "bootloader":"Micronucleus",
    "entrymodesizes":[
      ".menu.bootentry.jumper.upload.upload.maximum_size=14716"],
    "clock":[
      ".menu.clock.crystal_16m=16 MHz (external crystal)",
      ".menu.clock.crystal_16m.bootloader.low_fuses=0xF0",
      ".menu.clock.crystal_16m.build.f_cpu=16000000UL",
      ".menu.clock.crystal_16m.build.speed=16",
      ".menu.clock.crystal_16m.build.clocksource=1",
      ".menu.clock.crystal_8m=8 MHz (prescaled 16 MHz external crystal)",
      ".menu.clock.crystal_8m.bootloader.low_fuses=0xF0",
      ".menu.clock.crystal_8m.build.f_cpu=8000000UL",
      ".menu.clock.crystal_8m.build.speed=8",
      ".menu.clock.crystal_8m.build.clocksource=0x11",
      ".menu.clock.crystal_4m=4 MHz (prescaled 16 MHz external crystal)",
      ".menu.clock.crystal_4m.bootloader.low_fuses=0xF0",
      ".menu.clock.crystal_4m.build.f_cpu=4000000UL",
      ".menu.clock.crystal_4m.build.speed=4",
      ".menu.clock.crystal_4m.build.clocksource=0x11",
      ".menu.clock.crystal_2m=2 MHz (prescaled 16 MHz external crystal)",
      ".menu.clock.crystal_2m.bootloader.low_fuses=0xF0",
      ".menu.clock.crystal_2m.build.f_cpu=2000000UL",
      ".menu.clock.crystal_2m.build.speed=2",
      ".menu.clock.crystal_2m.build.clocksource=0x11",
      ".menu.clock.crystal_1m=1 MHz (prescaled 16 MHz external crystal)",
      ".menu.clock.crystal_1m.bootloader.low_fuses=0xF0",
      ".menu.clock.crystal_1m.build.f_cpu=1000000UL",
      ".menu.clock.crystal_1m.build.speed=1",
      ".menu.clock.crystal_1m.build.clocksource=0x11"],
    "defaultvariant":"tinyx7_digi",
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "fancybod":False,
    "rstdisbl":True,
    "pinmap":[
      ".menu.pinmap.digi=Digispark Pro",
      ".menu.pinmap.standard=Standard/Azduino",
      ".menu.pinmap.standard.build.variant=tinyx7",
      ".menu.pinmap.digi.build.variant=tinyx7_digi",
      ".menu.pinmap.legacy=Legacy",
      ".menu.pinmap.legacy.build.variant=tinyx7_legacy",
      ".menu.pinmap.default.build.pinmapabr=",
      ".menu.pinmap.digi.build.pinmapabr=.digi",
      ".menu.pinmap.legacy.build.pinmapabr=.old"],
    "USBPins":[
      ".menu.usbpins.digispark=Digispark Pro - PB3: D-, PB6: D+, PB1: LED",
      ".menu.usbpins.azduino=Azduino 167 USB - PB3: D-, PB6: D+, PA6: LED",
      ".menu.usbpins.digispark.build.usbpinset=-DLED_BUILTIN=PIN_PB1",
      ".menu.usbpins.azduino.build.usbpinset=-DLED_BUILTIN=PIN_PA6",
      ".menu.usbpins.digispark.bootloader.usbpinsetname=pb1led",
      ".menu.usbpins.azduino.bootloader.usbpinsetname=pa6led"],
    "softser":False,
    "fancysoftser":False,
    "hfuse":"0b{bootloader.rstbit}1011{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "attiny87micr":{
    "title":True,
    "chipmenu":False,
    "bootname":"attiny87",
    "flash":"6780",
    "sram":"512",
    "bootloader":"Micronucleus",
    "entrymodesizes":[
      ".menu.bootentry.jumper.upload.maximum_size=6716"],
    "clock":[
      ".menu.clock.crystal_16m=16 MHz (external crystal)",
      ".menu.clock.crystal_16m.bootloader.low_fuses=0xF0",
      ".menu.clock.crystal_16m.build.f_cpu=16000000UL",
      ".menu.clock.crystal_16m.build.speed=16m",
      ".menu.clock.crystal_16m.build.clocksource=1",
      ".menu.clock.crystal_8m=8 MHz (prescaled 16 MHz external crystal)",
      ".menu.clock.crystal_8m.bootloader.low_fuses=0xF0",
      ".menu.clock.crystal_8m.build.f_cpu=8000000UL",
      ".menu.clock.crystal_8m.build.speed=8m",
      ".menu.clock.crystal_8m.build.clocksource=0x11",
      ".menu.clock.crystal_4m=4 MHz (prescaled 16 MHz external crystal)",
      ".menu.clock.crystal_4m.bootloader.low_fuses=0xF0",
      ".menu.clock.crystal_4m.build.f_cpu=4000000UL",
      ".menu.clock.crystal_4m.build.speed=4m",
      ".menu.clock.crystal_4m.build.clocksource=0x11",
      ".menu.clock.crystal_2m=2 MHz (prescaled 16 MHz external crystal)",
      ".menu.clock.crystal_2m.bootloader.low_fuses=0xF0",
      ".menu.clock.crystal_2m.build.f_cpu=2000000UL",
      ".menu.clock.crystal_2m.build.speed=2m",
      ".menu.clock.crystal_2m.build.clocksource=0x11",
      ".menu.clock.crystal_1m=1 MHz (prescaled 16 MHz external crystal)",
      ".menu.clock.crystal_1m.bootloader.low_fuses=0xF0",
      ".menu.clock.crystal_1m.build.f_cpu=1000000UL",
      ".menu.clock.crystal_1m.build.speed=1m",
      ".menu.clock.crystal_1m.build.clocksource=0x11"],
    "defaultvariant":"tinyx7_digi",
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "fancybod":False,
    "rstdisbl":True,
    "pinmap":[
      ".menu.pinmap.digi=Digispark Pro",
      ".menu.pinmap.standard=Standard/Azduino",
      ".menu.pinmap.standard.build.variant=tinyx7",
      ".menu.pinmap.digi.build.variant=tinyx7_digi",
      ".menu.pinmap.legacy=Legacy",
      ".menu.pinmap.legacy.build.variant=tinyx7_legacy",
      ".menu.pinmap.default.build.pinmapabr=",
      ".menu.pinmap.digi.build.pinmapabr=.digi",
      ".menu.pinmap.legacy.build.pinmapabr=.old"],
    "USBPins":[
      ".menu.usbpins.digispark=Digispark Pro - PB3: D-, PB6: D+, PB1: LED",
      ".menu.usbpins.azduino=Azduino 167 USB - PB3: D-, PB6: D+, PA6: LED",
      ".menu.usbpins.digispark.build.usbpinset=-DLED_BUILTIN=PIN_PB1",
      ".menu.usbpins.azduino.build.usbpinset=-DLED_BUILTIN=PIN_PA6",
      ".menu.usbpins.digispark.bootloader.usbpinsetname=pb1led",
      ".menu.usbpins.azduino.bootloader.usbpinsetname=pa6led",
      ".menu.usbpins.azduino.upload.maximum_size=6652"],
    "softser":False,
    "fancysoftser":False,
    "hfuse":"0b{bootloader.rstbit}1011{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "attiny861micr":{
    "title":True,
    "chipmenu":False,
    "bootname":"attiny861",
    "flash":"6524",
    "sram":"512",
    "bootloader":"Micronucleus",
    "clock":[
      ".build.extra_flags=-DBOOT_TUNED165",
      ".menu.clock.pll_16m5=16.5 MHz (internal tuned PLL)",
      ".menu.clock.pll_16m5.bootloader.low_fuses=0xF1",
      ".menu.clock.pll_16m5.build.f_cpu=16500000UL",
      ".menu.clock.pll_16m5.build.speed=16m5",
      ".menu.clock.pll_16m5.build.clocksource=6",
      ".menu.clock.pll_16m=16 MHz (internal PLL)",
      ".menu.clock.pll_16m.bootloader.low_fuses=0xF1",
      ".menu.clock.pll_16m.build.f_cpu=16000000UL",
      ".menu.clock.pll_16m.build.speed=16m",
      ".menu.clock.pll_16m.build.clocksource=6",
      ".menu.clock.pll_8m=8 MHz (prescaled 16 MHz internal PLL)",
      ".menu.clock.pll_8m.bootloader.low_fuses=0xF1",
      ".menu.clock.pll_8m.build.f_cpu=8000000UL",
      ".menu.clock.pll_8m.build.speed=8m",
      ".menu.clock.pll_8m.build.clocksource=0x16",
      ".menu.clock.pll_4m=4 MHz (prescaled 16 MHz internal PLL)",
      ".menu.clock.pll_4m.bootloader.low_fuses=0xF1",
      ".menu.clock.pll_4m.build.f_cpu=4000000UL",
      ".menu.clock.pll_4m.build.speed=4m",
      ".menu.clock.pll_4m.build.clocksource=0x16",
      ".menu.clock.pll_2m=2 MHz (prescaled 16 MHz internal PLL)",
      ".menu.clock.pll_2m.bootloader.low_fuses=0xF1",
      ".menu.clock.pll_2m.build.f_cpu=2000000UL",
      ".menu.clock.pll_2m.build.speed=2m",
      ".menu.clock.pll_2m.build.clocksource=0x16",
      ".menu.clock.pll_1m=1 MHz (prescaled 16 MHz internal PLL)",
      ".menu.clock.pll_1m.bootloader.low_fuses=0xF1",
      ".menu.clock.pll_1m.build.f_cpu=1000000UL",
      ".menu.clock.pll_1m.build.speed=1m",
      ".menu.clock.pll_1m.build.clocksource=0x16"],
    "defaultvariant":"tinyx61",
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "fancybod":False,
    "rstdisbl":True,
    "pinmap":[
      ".menu.pinmap.default=Standard",
      ".menu.pinmap.legacy=Legacy",
      ".menu.pinmap.default.build.variant=tinyx61",
      ".menu.pinmap.legacy.build.variant=tinyx61_legacy",
      ".menu.pinmap.default.build.pinmapabr=",
      ".menu.pinmap.legacy.build.pinmapabr=.old"],
    "remap":[
      ".menu.remap.default=USI on PB0-PB2",
      ".menu.remap.alternate=USI on PA0-PA2",
      ".menu.remap.default.build.remap=",
      ".menu.remap.alternate.build.remap=-DSET_REMAPUSI",
      ".menu.remap.default.build.remapabr=",
      ".menu.remap.alternate.build.remapabr=rU"],
    "USBPins":[
      ".menu.usbpins.standard=Standard - PB4: D-, PB6: D+, PA5: LED",
      ".menu.usbpins.standard.bootloader.usbpinset=-DLED_BUILTIN=PIN_PB4 -DLED_BUILTIN2=PIN_PA4",
      ".menu.usbpins.standard.bootloader.usbpinsetname=",
      ".menu.usbpins.prototype=Prototype - PA4: D+, PA5: D-, PB6: LED",
      ".menu.usbpins.prototype.bootloader.usbpinset=-DLED_BUILTIN=PIN_PB6",
      ".menu.usbpins.prototype.bootloader.usbpinsetname=_proto",
      ".menu.usbpins.prototype.upload.maximum_size=6460"],
    "softser":True,
    "fancysoftser":True,
    "hfuse":"0b{bootloader.rstbit}1011{bootloader.bod_bits}",
    "efuse":"0xFE",
  },
    "attiny841micr":{
    "title":True,
    "chipmenu":False,
    "flash":"6524",
    "bootname":"attiny841",
    "entrymodesizes":[
      ".menu.bootentry.always.upload.maximum_size=6588"],
    "sram":"512",
    "bootloader":"Micronucleus",
    "clock":[
      ".menu.clock.internal_8m=8 MHz internal",
      ".menu.clock.internal_8m.bootloader.low_fuses=0xE2",
      ".menu.clock.internal_8m.build.f_cpu=8000000UL",
      ".menu.clock.internal_8m.build.speed=8m",
      ".menu.clock.internal_8m.build.clocksource=0",
      ".menu.clock.internal_8m.build.extra_flags=-DBOOT_TUNED120",
      ".menu.clock.internal_16m=16 MHz (internal via tuning/guess)",
      ".menu.clock.internal_16m.bootloader.low_fuses=0xE2",
      ".menu.clock.internal_16m.build.f_cpu=16000000UL",
      ".menu.clock.internal_16m.build.speed=16m",
      ".menu.clock.internal_16m.build.clocksource=0",
      ".menu.clock.internal_16m.build.extra_flags=-DBOOT_TUNED120",
      ".menu.clock.internal_12m=12 MHz (tuned by bootloader)",
      ".menu.clock.internal_12m.bootloader.low_fuses=0xE2",
      ".menu.clock.internal_12m.build.f_cpu=12000000UL",
      ".menu.clock.internal_12m.build.speed=12m",
      ".menu.clock.internal_12m.build.clocksource=0",
      ".menu.clock.internal_12m.build.extra_flags=-DBOOT_TUNED120",
      ".menu.clock.internal_1m=1 MHz internal",
      ".menu.clock.internal_1m.bootloader.low_fuses=0x62",
      ".menu.clock.internal_1m.build.f_cpu=1000000UL",
      ".menu.clock.internal_1m.build.speed=1m",
      ".menu.clock.internal_1m.build.clocksource=0",
      ".menu.clock.internal_1m.build.extra_flags=-DBOOT_TUNED120"],
    "hasvoltdependance":True,
    "defaultvariant":"tinyx41_cw",
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFE",
    "lfuse_xl":"0xED",
    "fancybod":True,
    "wiremodes":True,
    "rstdisbl":True,
    "pinmap":[
      ".menu.pinmap.default=Standard (clockwise)",
      ".menu.pinmap.ccw=Legacy (counterclockwise)",
      ".menu.pinmap.ccw.build.variant=tinyx41_legacy",
      ".menu.pinmap.default.build.pinmapabr=.cw",
      ".menu.pinmap.ccw.build.pinmapabr=.ccw"],
    "remap":[
      ".menu.remap.defaultboth=UART0: TX PA1, RX PA2. SPI: SCK PA4, MISO PA5, MOSI PA6, SS PA7",
      ".menu.remap.defaultuart=UART0: TX PA1, RX PA2. SPI: SCK PA3, MISO PA0, MOSI PA1, SS PA2",
      ".menu.remap.defaultspi=UART0: TX PA7, RX PB2. SPI: SCK PA4, MISO PA5, MOSI PA6, SS PA7",
      ".menu.remap.altboth=UART0: TX PA7, RX PB2. SPI: SCK PA3, MISO PA0, MOSI PA1, SS PA2",
      ".menu.remap.defaultboth.build.remap=",
      ".menu.remap.defaultuart.build.remap=-DSET_REMAP=2",
      ".menu.remap.defaultspi.build.remap=-DSET_REMAP=1",
      ".menu.remap.altboth.build.remap=-DSET_REMAP=3",
      ".menu.remap.defaultboth.build.remapabr=",
      ".menu.remap.defaultuart.build.remapabr=rS",
      ".menu.remap.defaultspi.build.remapabr=rU",
      ".menu.remap.altboth.build.remapabr=rB"],
    "USBPins":[
      ".menu.usbpins.pb1plus=PB0: D-, PB1: D+, PB2: LED (Bitboss)",
      ".menu.usbpins.pb0plus=PB0: D+, PB1: D-, PB2: LED (Wattuino)",
      ".menu.usbpins.pb2plus=PB0: LED, PB1: D-, PB2: D+",
      ".menu.usbpins.pb2plus.build.usbpinset=-DUSB_TWOPLUS -DLED_BUILTIN=PIN_PB0",
      ".menu.usbpins.pb1plus.build.usbpinset=-DUSB_ONEPLUS",
      ".menu.usbpins.pb0plus.build.usbpinset=-DUSB_ZEROPLUS",
      ".menu.usbpins.pb2plus.bootloader.usbpinsetname=_pb2plus",
      ".menu.usbpins.pb1plus.bootloader.usbpinsetname=_pb1plus",
      ".menu.usbpins.pb0plus.bootloader.usbpinsetname=_pb0plus"],
    "softser":False,
    "fancysoftser":False,
    "hfuse":"0b{bootloader.rstbit}1011{bootloader.bod_bits}",
    "efuse":"0b111{bootloader.bodpd_bits}{bootloader.bodact_bits}0",
  },
    "attiny1634micr":{
    "title":True,
    "chipmenu":False,
    "flash":"14720",
    "bootname":"attiny1634",
    "sram":"1024",
    "bootloader":"Micronucleus",
    "clock":[
      ".menu.clock.internal_8m=8 MHz",
      ".menu.clock.internal_8m.bootloader.low_fuses=0xE2",
      ".menu.clock.internal_8m.build.f_cpu=8000000UL",
      ".menu.clock.internal_8m.build.speed=8m",
      ".menu.clock.internal_8m.build.clocksource=0",
      ".menu.clock.internal_8m.build.extra_flags=-DBOOT_TUNED120",
      ".menu.clock.internal_12m=12 MHz (tuned by bootloader)",
      ".menu.clock.internal_12m.bootloader.low_fuses=0xE2",
      ".menu.clock.internal_12m.build.f_cpu=12000000UL",
      ".menu.clock.internal_12m.build.speed=12m",
      ".menu.clock.internal_12m.build.clocksource=0",
      ".menu.clock.internal_12m.build.extra_flags=-DBOOT_TUNED120",
      ".menu.clock.internal_1m=1 MHz internal",
      ".menu.clock.internal_1m.bootloader.low_fuses=0x62",
      ".menu.clock.internal_1m.build.f_cpu=1000000UL",
      ".menu.clock.internal_1m.build.speed=1m",
      ".menu.clock.internal_1m.build.clocksource=0",
      ".menu.clock.internal_1m.build.extra_flags=-DBOOT_TUNED120"],
    "hasvoltdependance":True,
    "defaultvariant":"tiny1634_cw",
    "lfuse_ext":"0xE0",
    "lfuse_xh":"0xFF",
    "lfuse_xl":"0xFD",
    "fancybod":True,
    "rstdisbl":True,
    "pinmap":[
      ".menu.pinmap.default=Standard (clockwise)",
      ".menu.pinmap.ccw=Legacy (counterclockwise)",
      ".menu.pinmap.ccw.build.variant=tiny1634_legacy",
      ".menu.pinmap.default.build.pinmapabr=.cw",
      ".menu.pinmap.ccw.build.pinmapabr=.ccw"],
    "USBPins":[
      ".menu.usbpins.standard=Standard: PC4: D-, PC5: D+, PC2: LED",
      ".menu.usbpins.standard.bootloader.usbpinset=-DLED_BUILTIN=PIN_PC2",
      ".menu.usbpins.standard.bootloader.usbpinsetname="],
    "softser":False,
    "fancysoftser":False,
    "hfuse":"0b{bootloader.rstbit}1011{bootloader.bod_bits}",
    "efuse":"0b111{bootloader.bodpd_bits}{bootloader.bodact_bits}0",
    }
}
def printSubMenuHeader(menuname):
  printLit("\n################################")
  printLit("# "+menuname.ljust(28)+" #")
  printLit("################################")

def printMenuItem(board,menuname,menuoption,optionprop):
  printProp(x,".menu."+menuname+"."+menuoption+"."+optionprop)
def printMenuItems(board,menuname,menuoption,optionproplist):
  for z in optionproplist:
    printProp(x,".menu."+menuname+"."+menuoption+"."+z)

def printmake(string):
  print(string,file = makefile)

clocklist={}
opticlocklist={}
baudlist={}
def printProp(board,string):
  print(board+string)
  print(board+string,file = f1)
  mboot=re.search("build\\.f_cpu=(\\d+UL)",string)
  if mboot:
    clocklist[board].append(mboot.expand("\\1"))
  mboot=re.search("bootloader\\.f_cpu=(\\d+UL)",string)
  if mboot:
      opticlocklist[board].append(mboot.expand("\\1"))
  n=re.search("upload\\.speed=(\\d+)",string)
  if n:
    if len(clocklist[board]) > len(opticlocklist[board]):
      opticlocklist[board].append(clocklist[board][-1])
    baudlist[board].append(n.expand("\\1"))
def printLit(string):
  print(string)
  print(string,file = f1)

fhead = open("boards_header.txt")
fheadtext=fhead.read()
f1 = open("boards.txt","w")
printLit(fheadtext)
for x in boardnames:
  printLit(x)
for x in boards:
  printLit("")
  if "header" in boards[x]:
    printLit("")
    printLit(boards[x]["header"])
    continue
  if boards[x]["title"]:
    printLit(titles[x])
    printLit("")
  clocklist[x]=[]
  opticlocklist[x]=[]
  baudlist[x]=[]
  if boards[x]["bootloader"]:
    printProp(x,".build.board=AVR_"+x[:-4].upper())
  else:
    printProp(x,".build.board=AVR_"+x.upper())
  printProp(x,".build.core=tiny")
  printProp(x,".build.variant="+boards[x]["defaultvariant"])
  printProp(x,".build.export_merged_output=false")
  printProp(x,".bootloader.extended_fuses="+boards[x]["efuse"])
  printProp(x,".bootloader.high_fuses="+boards[x]["hfuse"])
  if "hasulp" in boards[x]:
    printProp(x,".bootloader.ulp_bits=111")
  printProp(x,".bootloader.rstbit=1")
  printProp(x,".build.usbpinset=")
  printProp(x,".build.tuneorcal=")
  printProp(x,".build.softserabr=")
  printProp(x,".build.softser=")
  if not "wiremodes" in boards[x]:
    printProp(x,".build.wiremodeabr=")
    printProp(x,".build.wiremode=")
  if not "pinmap" in boards[x]:
    printProp(x,".build.pinmapabr=")
  if not "remap" in boards[x]:
    printProp(x,".build.remap=")
    printProp(x,".build.remapabr=")
  if boards[x]["bootloader"] == "Micronucleus":
    printProp(x,".build.bootloader=-DUSING_BOOTLOADER=0x08")
    printProp(x,".build.bootloaderabr=micr")
    printProp(x,".upload.tool=micronucleus")
    printProp(x,".upload.protocol=usb")
    printProp(x,".bootloader.devpostfix=")
  else:
    printProp(x,".upload.tool=avrdude")
    printProp(x,".bootloader.tool=avrdude")
    if boards[x]["bootloader"] == "Optiboot":
      printProp(x,".bootloader.flashstring=-Uflash:w:{bootloader.file}:i")
      printProp(x,".build.bootloader=-DUSING_BOOTLOADER=0x01")
      printProp(x,".build.bootloaderabr=opti")
      printProp(x,".bootloader.speed={build.speed}")
      printProp(x,".upload.protocol=arduino")
      printProp(x,".bootloader.uart=")
    else:
      printProp(x,".build.bootloader=")
      printProp(x,".build.bootloaderabr=")
      printProp(x,".bootloader.flashstring=")
  printSubMenuHeader("Optimization menu")
  printProp(x,".menu.optimization.size=-Os (size, recommended)")
  printProp(x,".menu.optimization.othree=-O3 (speed)")
  printProp(x,".menu.optimization.debug=-Og (for advanced debugging)")
  printProp(x,".menu.optimization.size.build.optimize=-Os")
  printProp(x,".menu.optimization.othree.build.optimize=-O3")
  printProp(x,".menu.optimization.debug.build.optimize=-Og")
  if boards[x]["chipmenu"]:
    printSubMenuHeader("Microcontroller menu")
    printLit(chipmenu[x])
  else:
    if boards[x]["bootloader"]:
      printProp(x,".build.mcu="+x[:-4])
    else:
      printProp(x,".build.mcu="+x)
  if "flash" in boards[x]:
    printProp(x,".upload.maximum_size="+boards[x]["flash"])
  if "sram" in boards[x]:
    printProp(x,".upload.maximum_data_size="+boards[x]["sram"])
  printSubMenuHeader("Clocking menu options")
  if boards[x]["bootloader"] == "Micronucleus":
    # Micronucleus boards have their own clock option menu sets. Everything else shares them.
    for y in boards[x]["clock"]:
      printProp(x,y)
  else:
    if boards[x]["hasvoltdependance"]:
      printProp(x,".menu.clock.internal_8m_4v5=8 MHz (internal > 4.5V)")
      printProp(x,".menu.clock.internal_8m_4v5.bootloader.low_fuses=0xE2")
      printProp(x,".menu.clock.internal_8m_4v5.build.f_cpu=8000000UL")
      printProp(x,".menu.clock.internal_8m_4v5.build.speed=8m")
      printProp(x,".menu.clock.internal_8m_4v5.build.clocksource=0")
      printProp(x,".menu.clock.internal_8m_4v5.build.tuneorcal=-DCAL_NUDGE_DOWN")
      if boards[x]["bootloader"]=="Optiboot":
        printProp(x,".menu.clock.internal_8m_4v5.upload.speed=76800")
    internalclocks=intclocks
    if "internalclock" in boards[x]:
      internalclocks = boards[x]["internalclock"]
    for y in internalclocks:
      for z in y[0]:
        printProp(x,z)
      if boards[x]["bootloader"]=="Optiboot":
        if y[1]["intat1"]:
          if boards[x]["softser"]:
            printProp(x,y[1]["softint"])
          else:
            printProp(x,y[1]["hardint"])
        else:
          if boards[x]["softser"]:
            printProp(x,y[1]["soft"])
          elif "haslin" in boards[x]:
            printProp(x,y[1]["lin"])
          else:
            printProp(x,y[1]["hard"])
    if boards[x]["haspll"]:
      for y in pllclocks:
        for z in y[0]:
          printProp(x,z)
        if boards[x]["bootloader"]=="Optiboot":
          if boards[x]["softser"]:
            printProp(x,y[1]["soft"])
          else:
            printProp(x,y[1]["hard"])
    if x[:9] == "attinyx41":
      printProp(x,".menu.clock.internal_16m=16 MHz (internal via tuning/guess)")
      printProp(x,".menu.clock.internal_16m.bootloader.low_fuses=0xE2")
      printProp(x,".menu.clock.internal_16m.build.f_cpu=16000000UL")
      printProp(x,".menu.clock.internal_16m.build.speed=16m")
      printProp(x,".menu.clock.internal_16m.build.clocksource=0")
      if boards[x]["bootloader"]=="Optiboot":
        printProp(x,".menu.clock.internal_16m.upload.speed=76800")
    if boards[x]["hasxtal"]:
      for y in xtalclocks:
        printProp(x,".menu.clock.crystal_"+y+"="+clocktobaud[y.strip("m")]["name"]+" (crystal)")
        if clocktobaud[y.strip("m")]["xl"]:
          printProp(x,".menu.clock.crystal_"+y+".bootloader.low_fuses="+boards[x]["lfuse_xl"])
        else:
          printProp(x,".menu.clock.crystal_"+y+".bootloader.low_fuses="+boards[x]["lfuse_xh"])
        printProp(x,".menu.clock.crystal_"+y+".build.f_cpu="+clocktobaud[y.strip("m")]["f_cpu"])
        printProp(x,".menu.clock.crystal_"+y+".build.speed="+y)
        printProp(x,".menu.clock.crystal_"+y+".build.clocksource=1")
        if boards[x]["bootloader"]=="Optiboot":
          temp=getspeed(".menu.clock.crystal_"+y+".upload.speed=",y.strip("m"))
          if boards[x]["softser"]:
            printProp(x,temp["soft"])
          else:
            printProp(x,temp["hard"])
    if not "internalclock" in boards[x]:
      tunedclklist=tunedclocks
    if not "notunedclocks" in boards[x] or ("notunedclocks" in boards[x] and not boards[x]["notunedclocks"]):
      if not("onlytunesto8" in boards[x] and boards[x]["onlytunesto8"]):
        #for a few parts we can't tune to anything higher than 8 and expect joy.
        for y in tunedclocks:
          for z in y[0]:
            printProp(x,z)
          if boards[x]["bootloader"]=="Optiboot":
            if boards[x]["softser"]:
              printProp(x,y[1]["soft"])
            else:
              printProp(x,y[1]["hard"])
    if boards[x]["hasvoltdependance"]:
      for y in tune8vdepclk:
        for z in y[0]:
          printProp(x,z)
        if boards[x]["bootloader"]=="Optiboot":
          if boards[x]["softser"]:
            printProp(x,y[1]["soft"])
          else:
            printProp(x,y[1]["hard"])
    else:
      if not "notunedclocks" in boards[x] or ("notunedclocks" in boards[x] and not boards[x]["notunedclocks"]):
        for y in tune8stdclk:
          for z in y[0]:
            printProp(x,z)
          if boards[x]["bootloader"]=="Optiboot":
            if boards[x]["softser"]:
              printProp(x,y[1]["soft"])
            else:
              printProp(x,y[1]["hard"])
    if boards[x]["haswdt"]:
      for y in wdtclocks:
        printProp(x,y)
      printProp(x,".menu.clock.internal_128k.bootloader.low_fuses=0xC"+boards[x]["wdtcode"])
    if "hasulp" in boards[x]:
      for y in ulpclocks:
        printProp(x,y)
    #External Clocks
    if x !="attiny43u":
      for y in xtalclocks:
        printProp(x,".menu.clock.extclk_"+y+"="+clocktobaud[y.strip("m")]["name"]+" (external clock)")
        printProp(x,".menu.clock.extclk_"+y+".bootloader.low_fuses="+boards[x]["lfuse_ext"])
        printProp(x,".menu.clock.extclk_"+y+".build.f_cpu="+clocktobaud[y.strip("m")]["f_cpu"])
        printProp(x,".menu.clock.extclk_"+y+".build.speed="+y)
        printProp(x,".menu.clock.extclk_"+y+".build.clocksource=2")
        if boards[x]["bootloader"]=="Optiboot":
          temp=getspeed(".menu.clock.extclk_"+y+".upload.speed=",y.strip("m"))
          if boards[x]["softser"]:
            printProp(x,temp["soft"])
          else:
            printProp(x,temp["hard"])
    else:
      printProp(x,".menu.clock.extclk_8m=8 MHz (external clock)")
      printProp(x,".menu.clock.extclk_8m.bootloader.low_fuses="+boards[x]["lfuse_ext"])
      printProp(x,".menu.clock.extclk_8m.build.f_cpu=8000000UL")
      printProp(x,".menu.clock.extclk_8m.build.speed=8m")
      printProp(x,".menu.clock.extclk_8m.build.clocksource=2")
  if "pinmap" in boards[x]:
    printSubMenuHeader("Pin Mapping menu   ")
    for y in boards[x]["pinmap"]:
      printProp(x,y)
  if not boards[x]["bootloader"]:
    printSubMenuHeader("Retain EEPROM menu ")
    for y in eesavemenu:
      printProp(x,y)
  if "wiremodes" in boards[x]:
    printSubMenuHeader("Wire Modes menu    ")
    for y in wiremodesmenu:
      printProp(x,".menu.wiremode."+y)
  if "remap" in boards[x]:
      for y in boards[x]["remap"]:
        printProp(x,y)
  if boards[x]["softser"]:
    printSubMenuHeader("Software Serial menu ")
    if boards[x]["fancysoftser"]:
      if boards[x]["bootloader"] == "Optiboot":
        for y in softsermenu_861:
          printProp(x, ".menu.softserial861."+y)
      else:
        for y in softsermenu_861:
          printProp(x, ".menu.softserial861."+y)
    else:
      printProp(x,".menu.softserial.enable=RX on P"+boards[x]["ssport"]+boards[x]["ssrx"]+", TX on P"+boards[x]["ssport"]+" (default P"+boards[x]["ssport"]+boards[x]["sstx"]+")")
      for y in softsermenu:
        printProp(x,y)
  printSubMenuHeader("BrownOut Detect menu ")
  if "bodmenu" in boards[x]:
    for y in boards[x]["bodmenu"]:
      printProp(x,y)
  else:
    if boards[x]["fancybod"]:
      for y in fancybodmenu:
        printProp(x,y)
    else:
      for y in plainbodmenu:
        printProp(x,y)
  printSubMenuHeader("millis/micros menu ")
  for y in millismenu:
    printProp(x,y)
  if boards[x]["bootloader"]=="Micronucleus":
    printSubMenuHeader("Boot Entrymode menu  ")
    for y in bootmodesmicr:
      printProp(x,y)
    if "entrymodesizes" in boards[x]:
      for y in boards[x]["entrymodesizes"]:
        printProp(x,y)
    printSubMenuHeader("Pins used for USB     ")
    for y in boards[x]["USBPins"]:
      printProp(x,y)
    if "rstdisbl" in boards[x]:
      if boards[x]["rstdisbl"]:
        for y in resetpinmenu:
          printProp(x,".menu.resetpin."+y)
    printSubMenuHeader("Upgrade vs ISP install menu ")
    printProp(x,".menu.burnmode.upgrade=Upgrade (via USB)")
    printProp(x,".menu.burnmode.upgrade.bootloader.file={runtime.platform.path}/bootloaders/micronucleus/upgrade-"+boards[x]["bootname"]+"{bootloader.devpostfix}{bootloader.usbpinsetname}"+"_"+"{bootloader.entrymode}.hex")
    printProp(x,".menu.burnmode.upgrade.bootloader.tool=micronucleus")
    printProp(x,".menu.burnmode.install=Fresh Install (via ISP)")
    printProp(x,".menu.burnmode.install.bootloader.file={runtime.platform.path}/bootloaders/micronucleus/"+boards[x]["bootname"]+"{bootloader.devpostfix}{bootloader.usbpinsetname}"+"_"+"{bootloader.entrymode}.hex")
    printProp(x,".menu.burnmode.install.bootloader.tool=avrdude")
    printProp(x,".menu.burnmode.install.bootloader.flashstring=-Uflash:w:{bootloader.file}:i")
  if boards[x]["bootloader"]=="Optiboot":
    printSubMenuHeader("Boot Entrymode menu  ")
    for y in bootmodesopti:
      printProp(x,y)
    if "optipins" in boards[x]:
      printSubMenuHeader("Serial Port:        ")
      for y in boards[x]["optipins"]:
        printProp(x,y)

makehead=open("MakefileHead.txt")
makeheadtext=makehead.read()
makehead.close()
makefile=open("Makefile.tiny","w")
optibootbuilds={};
optibootbuilds_proto={};
for x in boards:
  if "header" in boards[x]:
    continue
  if boards[x]["bootloader"]=="Optiboot":
    for y in chiplist[x[:-4]]:
      optibootbuilds[y]={}
      optibootbuilds_proto[y]={"clocks":clocklist[x]}
      optibootbuilds_proto[y]["opticlocks"]=opticlocklist[x]
      optibootbuilds_proto[y]["baud"]=baudlist[x]
      if boards[x]["hasvoltdependance"]:
        optibootbuilds[y]["hasvoltdependance"]=True
      else:
        optibootbuilds[y]["hasvoltdependance"]=False
      if "twouart" in boards[x]:
        optibootbuilds[y]["twouart"]=True
      else:
        optibootbuilds[y]["twouart"]=False
      if "uartremap" in boards[x]:
        optibootbuilds[y]["remap"]=True
      else:
        optibootbuilds[y]["remap"]=False
      if boards[x]["fancysoftser"]:
        optibootbuilds[y]["fancysoftser"]=True
      else:
        optibootbuilds[y]["fancysoftser"]=False
      optibootbuilds[y]["clocks"]=[];
      optibootbuilds[y]["baud"]=[];
      for z in range(len(optibootbuilds_proto[y]["clocks"])):
        if optibootbuilds_proto[y]["opticlocks"][z] in optibootbuilds[y]["clocks"]:
          continue
        optibootbuilds[y]["clocks"].append(optibootbuilds_proto[y]["opticlocks"][z])
        optibootbuilds[y]["baud"].append(optibootbuilds_proto[y]["baud"][z])

listofbuilds=[]
fhead = open("MakefileHead.txt")
fheadtext=fhead.read()
f2 = open("Makefile.tiny","w")
print(fheadtext,file=f2)
for x in optibootbuilds:
  if optibootbuilds[x]["twouart"]:
    if optibootbuilds[x]["remap"]:
      optlist={"":"","_serR":"REMAP=1","_ser1":"UART=1"}
    else:
      optlist={"":"","_ser1":"UART=1"}
  elif optibootbuilds[x]["fancysoftser"]:
    optlist={"_rx7tx6":"TXPIN=6 RXPIN=7","_rx6tx7":"TXPIN=7 RXPIN=6","_rx5tx6":"TXPIN=6 RXPIN=5"}
  else:
    optlist={"":""}
  for y in range(len(optibootbuilds[x]["baud"])):
    z = optibootbuilds[x]["clocks"][y]
    b = optibootbuilds[x]["baud"][y]
    for t in optlist:
      listofbuilds.append(x+"at"+z+t)
      listofbuilds.append(x+"at"+z+t+"_8sec")
      print(x+"at"+z+t+":\n\t$(MAKE) "+x+"\tAVR_FREQ="+z+" "+optlist[t]+" BAUD_RATE="+b+"\n\tmv $(PROGRAM)_"+x+".hex $(PROGRAM)_"+x+"_"+z+t+".hex\nifndef PRODUCTION\n\tmv $(PROGRAM)_"+x+".lst $(PROGRAM)_"+x+"_"+z+t+".lst\nendif\n\n",file=f2)
      print(x+"at"+z+t+"_8sec:\n\t$(MAKE) "+x+"\tAVR_FREQ="+z+" "+optlist[t]+" NO_START_APP_ON_POR=1 TIMEOUT=8 BAUD_RATE="+b+"\n\tmv $(PROGRAM)_"+x+".hex $(PROGRAM)_"+x+"_"+z+t+"_8sec.hex\nifndef PRODUCTION\n\tmv $(PROGRAM)_"+x+".lst $(PROGRAM)_"+x+"_"+z+t+"_8sec.lst\nendif\n\n",file=f2)
      if y<=1:
        if(optibootbuilds[x]["hasvoltdependance"]):
          listofbuilds.append(x+"at"+z+t+"_int")
          listofbuilds.append(x+"at"+z+t+"_int_8sec")
          print(x+"at"+z+t+"_int:\n\t$(MAKE) "+x+"\tAVR_FREQ="+z+" "+optlist[t]+" BAUD_RATE="+b+"\n\tmv $(PROGRAM)_"+x+".hex $(PROGRAM)_"+x+"_"+z+t+"_int.hex\nifndef PRODUCTION\n\tmv $(PROGRAM)_"+x+".lst $(PROGRAM)_"+x+"_"+z+t+"_int.lst\nendif\n\n",file=f2)
          print(x+"at"+z+t+"_int_8sec:\n\t$(MAKE) "+x+"\tAVR_FREQ="+z+" "+optlist[t]+" NO_START_APP_ON_POR=1 TIMEOUT=8 BAUD_RATE="+b+"\n\tmv $(PROGRAM)_"+x+".hex $(PROGRAM)_"+x+"_"+z+t+"_8sec_int.hex\nifndef PRODUCTION\n\tmv $(PROGRAM)_"+x+".lst $(PROGRAM)_"+x+"_"+z+t+"_8sec_int.lst\nendif\n\n",file=f2)
f2.close();
f3=open("makeall.tiny.bat","w")
for x in listofbuilds:
  print("call omake "+x,file=f3)
