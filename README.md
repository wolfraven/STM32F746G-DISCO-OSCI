# STM32F746G-DISCO-OSCI – recording into "Wave file" on SD-Card
The intention for this project:
- Getting in touch with the STM32F7xx device family
- Evaluation of STemWIN for GUI development

So I bought my first "STM32F746G-DISCO"...
...and nothing is better for evaluation and whirl up potential problems more, than 
making a meaningful project:

For the evaluation of the STemWin stuff I tried first to rebuild the "Demonstration", 
that can be found in the relating folder of the CubeMX repository. Here for example 
the folder path for the current used Version V1.14.0:
"...../STM32Cube/Repository/STM32Cube_FW_F7_V1.14.0/Projects/STM32746G-Discovery/
Demonstrations/StemWin".

I prefer to use native Eclipse CDT, so I configured it up as project for this IDE.
This implementation is done with Eclipse Oxygen, CDT and the GNU MCU tool chain.
The "apps" meue structure of the demonstration is nice and a good base to hook in 
useful functionality (apps).
After getting it running, the idea was to adapt the famous "OSZI" project for the 
STM32F429I-DISCO as such an app for the F7-DISCO. The original source for the F4-OSZI 
can be found here:
http://mikrocontroller.bplaced.net/wordpress/?page_id=752
Or here:
https://github.com/noahchense/STM32F429i-Disco_OSC.

A basic adaption, using LL drivers is done.
User interface is now chaged to a GUI with permanent buttons, calling specified 
configuration or selection dialogs.   

Another intention to have a closer look on the STM32F746G-DISCO – beside the larger
display – was the SD Card socked on board!
Most valuable to get an oscilloscope recording functionality on a SD card.

Now the big question: How to write the OSCI data to disk?
Why not trying the "WAVE format", before developing an own exotic one?!
Using the wave format for recording allow to use "existing" audio processing programs 
e.g. for "visualisation".

Also "working" with the recorded data is easy.
"Matlab" and "Octave" can load the created "exotic" wave files with their build-in 
"audio..." functions directly. No need for special import programming or conversion.

Some example "waves" and a sample script "osci.m" included in the folde "DOKU/Matlab".

A more detailed description with pictures can be found in "STM32F746-DISCO-OSCI.pdf".


