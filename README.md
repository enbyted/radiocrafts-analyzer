radiocrafts-analyzer
====================

Packet analyzer for Radiocrafts Tiny Mesh(tm) gateway.

Basicly what it does, or rather will do is pretty simple:
- Receive packets from gateway device, through RS232
- Send packets to gateway device (to network), through RS232
- Probably keep track of devices in network
- Provide a way to reconfigure devices and send serial data to them, maybe through sockets or something (so you could debug your programs)
- Provide a way to drive GPIO pins, react to user input and other things for testing purposes

Sounds like a plan, so let's get back to work :)

Also, please keep in mind that this software is written using ncurses library, so you need to have that one installed.
I'm testing it on Raspberry Pi, currently Raspian, but in some time I'll have to build a proper kernel for end device, so that'll be my test platform.

Also,
I know, the source code is ugly, by all means, but it's just a quick test tool. I'm publishing it just in case someone would need it and there aren't any tools like that for linux console.
