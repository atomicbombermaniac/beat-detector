# beat-detector
Quick hack to detect beats in songs in order to control things like lights (strobe) or the planet.

This project as an adaptation from the original at:
https://damian.pecke.tt/2015/03/02/beat-detection-on-the-arduino.html
MANY THANKS to Damian for computing the IIR values and for the code.
I decided to upload my variation so that maybe one night, it will be helpful to someone in dire quick need as it was for me Damian's code.

The most important difference is that there is no more need for the external potentiometer to set the beat detection level.
Software "automagically" -tries-to- detect that threshold.

WARNING: It is garbage at off-beat stuff, so no  Aphex Twin or other wierd alien stuff.
The closer the signal is to simple house music, the better, but mostly any stable rhytm for dancing works wonders.


This code runs on chinese clone (~2$) of Arduino NANO and can be ported to any fast enough junk. Probably works on sand.
Uses "line" signal level (from laptop or phone output jack), which MUST be connected through a capacitor ( ~100nF) to the input of the ADC (analog input) 
channel you desire. Any signal that is about 2Vpp max should do. That same ADC input must be be also connected with a resistor to GND and with another 
resistor to VREF pin. About 100K should be ideal.


The 5  "VU-meter" leds are not mandatory, but are useful IRL.

I used this to control a 1.5KW Xenon strobe light. Worked wonders. Almost felt like the pandemic was over. 
That was 'till the strobe ran out of chinesium and kicked the bucket the next night.

Read the code and the comments inside for more info.

