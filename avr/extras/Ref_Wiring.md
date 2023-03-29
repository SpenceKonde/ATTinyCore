# Minimal Wiring Requirements

These are the bare minimum requirements to bring the chip to life and program it using the Arduino IDE. The 0.1uf capacitors are *not* optional - you must use these or the parts will behave erratically, and may not work at all; these should be placed as close to the chip as possible.

* Vcc pin(s) connected to supply voltage (1.8-5.5v depending on part, clock speed, and fuse settings).
  * Make sure that you have not set BOD to a voltage lower than the nominal supply voltage if BOD is enabled!
  * **All** Vcc pins must be connected to power.
* Gnd pin(s) connected to Ground
  * **All** Gnd pins must be connected to ground.
* 0.1uf ceramic capacitor connected between Vcc and Gnd pins, as close to the part as practical.
  * If part has multiple Vcc pins, there **must** be 0.1uf cap from each one to ground, as close to the part as practical.
* If part has an AVcc pin, that **must** be connected to Vcc - see optional inductor note below.
* If part has an AGnd pin, that **must** be connected to Gnd.
* If part has an AVcc pin and an AGnd pin, 0.1uf ceramic capacitor between AVcc and AGnd, right next to the chip.
* If part has an AVcc pin but no AGnd pin, 0.1uf cerapic capacitor between AVcc and Gnd, right next to the chip.
* Somewhere on the breadboard or PCB, there should be a larger capacitor between Vcc and Gnd.
  * This is called "Board level" decoupling. It is particularly important when the wires to the board are either very long, or very short and the device is plugged directly into a low impedance power supply.
    * If a regulator is in use, the output capacitor of the regulator will serve as board level decoupling at Vcc, however.
    * If a regulator is in use, an input capacitor should be installed as well. The requirements for this should be listed in the datasheet for the regulator - this varies from "0.1uF ceramic" to "10uF Tantalum or 47uF electrolytic"; take this specification seriously. This capacitor, if large enough, counts as board level decoupling for for V<sub>in</sub>.
  * The capacitor used for board level decoupling should be at an absolute bare minimum 1uF, and I would not recommend anything smaller than 4.7uF.
    * Any kind of cap is acceptable here, particularly for larger values.
    * Warning: Ceramic capacitors exhibit significant DC-bias dependence of capacitance. A 4.7uF 0402 capacitor might indeed be 4.7uF at 1V DC bias. At 5V, it will be 1-2uF only! See note below.
    * For typical applications 4.7 or 10uf ceramic caps are acceptable (and by far the cheapest).
    * For especially long wires, or if large or fast changing loads are powered by the same supply, larger values should be used (preferably in parallel with a smaller ceramic).
    * Values as high as 1000uF may be needed if this is powered from the same supply as a long string of 2812 LEDs.
* Symptoms of insufficient decoupling are unexpected resets when the load suddenly increases due to:
  * turning on one or more LEDs.
  * (in extreme cases) erasing or writing the flash.

## Optional Wiring

### For improved ADC accuracy (87, 167, 261, 461, 861 only)
* If there is an AVcc pin and you are really concerned about ADC accuracy, instead of directly connecting it to Vcc, connect it through a Ferrite Bead inductor. 10uH has been recommended. This is rarely done.
  * The upcoming Ultramini Classic boards from my tindie store will feature pads and a cuttablebridge so that this can be done if desired.
  * This is probably the most important on the x61, whose "big feature" is that it has either the best or second best ADC features on any tinyAVR and is better than the ADC on most megaAVRs (it has a preponderance of differential ADC channels, with gain up to 32x, and it's a true differential ADC.

### Autoreset (if using serial)
It is often useful to have the board reset when most tools open the serial port (though your finer console applications, like the one I use, HTerm, let you control this).

* 10k resistor between Reset pin and Vcc
* 0.1uf capacitor between Reset pin and pin header that you can connect to the DTR pin of your serial adapter
* Diode (any) between Reset and Vcc, band towards Vcc.

Additionally in cases like these it is highly useful to have the standard FTDI 6-pin header as well: The pinout of this is:
1. Gnd
2. Not used
3. Vcc
4. Rx (this will go to Tx of serial adapter)
5. Tx (this will go to Rx of serial adapter)
6. DTR

Note that this is the layout of pins least likely to cause damage if connected backwards, a trait shared with the ISP header pinout.

## Ceramic Capacitor caveats for board level decoupling
Ceramic capacitors are awesome. Most people try to avoid using any other kind, and with good reason. They are one of the cheapest things around (until physical size exceeds 1210), they're not polarized, they don't dry out and fail completely over time like aluminum electrolytics do, they don't smoke or catch fire when abused with excessive or reversed voltages like tantalum caps do.  The capacitance scales down better than electroltics (which don't get much smaller below around 1uF). They also do not pose the ethical concerns of tantalum capacitors.

### Ceramic capacitors age
The first is that as are used or stored at temperatures below 130 C, the capacitance slowly falls. As those temperatures are generally exceeded during reflow, this will "deage" the capacitors. The specified capacitance is typically based on the expected capacitance some number of hours (ex, 1000), and is specified as a percentage per "decade hour" (that is, it is logarithmic - if it falls 3% per decade hour, it will be 12% lower after 1000 hours - so a freshly deaged 10uF cap with those aging specifications would have a 11.2uF capacitance immediately after deaging, 10.6uF ten hours later, 10.3uF 4 days later, and 10uf 42 days later, and a year after, 9.7uF. This is rarely relevant due to the logarithmic aging curve, but [Kemet has an excellent document](https://forum.digikey.com/uploads/short-url/isxVFxLPtAwHC4BP6avmfPszBTQ.pdf) on it.

### Ceramic capacitors and "DC-bias"
The second is that capacitance depends on the DC-bias (ie, applied DC voltage). A cap at it's maximum rated voltage might have 90% of it's nominal capacitance, in which case you likely don't care. Or it might have 10% of it's rated capacitance, maybe even less. In that case you most certainly do care! If you had a 4.7uF input cap for your regulator, you might think you were all set, but if you didn't check the DC bias properties, you could be in for a nasty surprise when it's assembled: You find that regulator is providing poor load regulation, causing malfunctions. Had you checked the DC bias curve, you might have anticipated this - what would you expect when the datasheet specified a 4.7uF input cap, but you used one that provided a mere 0.5uF at V<sub>in</sub>?

Obviously, capacitor makers don't really like to talk about this - but they do have to tell people somewhere, so that angry customers can be pointed to some document or tool. Thus, you'll rarely see it in the excuse for a datasheet typically supplied for capacitors (this has very little information). Most capacitor companies will provide a tool on their website allowing you to get graphs of the C vs V<sub>DC bias</sub>, and in many cases search the manufacturer's products based on required capacitance at a specified DC bias. TDK (whose high capacitance ceramic caps are, for the most part, unusually good - though they are aware of this and their prices higher) and Murata (which seems to have fallen a bit behind the curve here) both provide nice tools for this (I like TDK's tool better). Samsung, another leader in the field (they're usually between Murata and TDK in this regard, and usually cheaper than both because of their incredible scale) has a mediocre tool on their website. Other large manufacturers likewise have such tools. Small manufacturers sometimes do NOT - if their capacitors are at risk of having relevant DC-bias effects, they should be avoided.

#### What factors determine this?
There are 3 major factors: The nominal capacitance and the physical size (including the height - we care about physical volume) are major factors, and the table below is a good heuristic for knowing when this may be an issue. However, these are not the only factors - the manufacturing process for some companies is better than others, and it will vary with the temperature dependence code, and series of capacitors from a given company. This table below applies to capacitors with a temperature code starting with X (eg X7R). Note that while there are, as I said, differences between the different temperature codes, these are not strongly biased in either direction. The same company might sell X5R caps that are terrible with DC bias, and ones that are good, in the same package size and their X7R caps may be better or worse than them. So unless you know you're in safe territory, you've gotta look it up. Surprisingly, there is relatively little impact from the maximum rated voltage - you will often (on tools that let you superimpose several graphs) see the 25V part's curve exactly trace the curve of the 16V one, and just extend it longer.

#### Capacitor vs DC-Bias Risk Chart**
Columns are are the maximum size that you can confidently use without needing check the detailed specs, rows are the voltage that the part will be used at in operation. At sizes 1 increment below the listed size, it is sometimes possible to find caps that retain a good deal of capacitance, but they are significantly outnumbered by ones that dont. . At the listed size, you'll usually be okay, but if you really care about the capacitance, you should check the graphs from the manufacturer. For large value capacitors, TDK usually wins. For smaller values TDK's advantage shrinks significantly. Thicker capacitors are usually better than thinner ones with the same footprint.

| Voltage | 0.1uF | 1.0uF | 4.7uF | 10 uF | 22 uF | 47 uF | 100 uF |
|---------|-------|-------|-------|-------|-------|-------|--------|
|  3.3V   | 0402  | 0603  | 0805  | 0805* | 1206* | 1210* | 1812   |
|  5.0V   | 0402  | 0603  | 0805* | 1206* | 1210  | 2220  | 2220*  |
| 12.0V   | 0603  | 1206  | 1210  | 1812  | 2220  | 2220* | nope   |
| 20.0V   | 0603* | 1210  | 2220  | 2200* | 2220* | nope  | no way |

`*` - indicates that while parts of this size that are good up to that voltage are common, a non-negligible portion of them are not. In this situation, assuming capacitors of higher capacitance in the same package size are available (for example, you want a 4.7uF cap at 5V. There aren't many of those that have >4uF capacitance at that DC bias.... but if you instead used a 0805 10uF capacitor, though it would have lost even more capacitance as a percentage, the remaining capacitance at 5V bias is likely to exceed 4.7 uF).

As you can see, as the voltage is increased, things rapidly get worse. This should not come as a great surprise. Charge is physical, and like anything, storing more of it takes more space (remember that a 1 Farad capacitor at 1 Volt stores 1 Coulomb of charge, while one at 5V stores 5 coulombs. If you've already packed a bunch of charge into a capacitor (like packing anything into any container), you'd expect that as it fills up, the same amount of voltage (or pressure for objects in containers) won't be able to force as much charge into it. This is also conceptually consistent with the formula for the stored *energy* in a capacitor, which is proportional to the capacitance times the square of the voltage - Doubling the capacitance at a constant voltage is easier than doubling the voltage at a given capacitance.

Sizes of ceramic caps above 1210 are often quite expensive - they lose their price advantage over tantalum caps. They are also not viewed favorably by many engineers, because of a perception that they are prone to failure due to board flex leading to cracking. Peer reviewed studies have debated how serious of an issue this is. People try to avoid using large capacitors. It is often morecommon to see several lower value 1206 or 1210 caps in parallel (this is very common on DC-DC converters, for example, where the input voltage might range to 25v. They might then put a pair of 22uF 1206 or 1210 caps; these might be good for a regulator that needed a minimum of 4.7uF or 10uF of input capacitance (respectively). )


#### Tantalum caps involve ethical concerns?
Uh, yeah, they do. tantalum is predominantly mined in southern and central Africa - a land of great mineral wealth, and considerable human suffering. The or, known colloquially as "Coltan" is considered a conflict mineral. Some is mined ethically. Some is mined by children or at gunpoint. Some of the otherwise legitimately mined or is stolen by organized crime rings. When prices have spiked, it has been economically viable to *fly heavyass tantalum or (and it is a notably dense mineral) out of the rural areas in helicopters*. The illegally produced material is sold to fund various militias, while proceeds from the legally produced stuff are often skimmed off by the kleptocrats running those nations; whether a militia, organized crime ring, or kleptocrat ends up with the money, it will likely be used to fund the endless civil wars that continue to immiserate the populations of those countries. So tantalum caps, while they offer stable capacitance over a wide voltage range, and they have a size advantage at highcapacitance (starting around 100uF, lower for higher voltage caps), have an an ethical shadow cast over them. And they occasionally "explode" - though this concern has been largely solved since their introduction, at least if they aren't abused. Also, "explode" is a very dramatic way of putting it. They'll definitely smoke, and occasionally briefly ignite (though all I could get is smoke when I was intentionally trying to see one of those "exploding capacitors").

Of course, everyone uses unethically sourced stuff every day. A non-negligible portion of gold on the international market (used for your ENIG surface treatment, and some bond wires in ICs) comes from "wildcat" or "artisanal" gold mining, which often involves improperly disposing of mercury and cyanide compounds and contaminates drinking water in third world nations. But ENIG is the de-facto standard if you want to go lead free; and lead solder in discarded electronics leaches lead in landfills, leading to groundwater contamination - worse still it's groundwater contamination in your country not some faraway land, which is why many countries have banned its use in most circumstances. If you look at the raw materials or end of life disposal of most products - even mundane things like food, clothing, and so on, it's hard to find one without some kind of ethical concern or another, and it's often difficult to be sure that whatever you got was sourced ethically.


## Additional requirements for programming over Serial using Optiboot
See the optional autoreset wiring recommendations above. These are no longer "optional".

## Once these parts are installed, you are ready to program your ATtiny
Well, if programming using Optiboot, the first "programming" you do will be using an ISP programmer to *install* optiboot.

See the [Programming Guide](Ref_Programming.md) for more information.
