# TeensyXTKeyboard
This is a converter for hooking up a PC XT keyboard using USB.
It is based loosely on 
https://github.com/kesrut/tinyPS2toXTArduino/blob/master/ps_keyboard.ino
Hacked to bits by: Geoffrey Borggaard

Since this keyboard doesn't really have arrow keys, I've added a non-intuitive
mode for turning the keypad into arrow keys when you hit the NUM LOCK key.  When
it is in this mode, the light on the teensy should go on.

There are certainly bugs.  I have noticed after typing on this for a while, it
sometimes needs to be unplugged and plugged back in to start working again.

