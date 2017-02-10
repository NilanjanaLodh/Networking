This is the README file for the dostalk_dataLinkLayer application.

**What is it?**
	It is a simple talk application developed for DOS operating system. This doesn't use any existing protocols like TCP/IP and simulates *data-link layer* activity.  It simply exchanges packets with the computer with the mentioned MAC address with the help of the packet driver.
	Why DOS you ask :P ? 
	

 - DOS doesn't come with networking support built-in. So you need to go into the basics and develop the application with bare minimum. In this case , just on top of the packet driver. So you really get to learn the basics!
 - This also happened to be one of our Networking lab assignments.


**Prerequisites**
 

 - Packet driver for your NIC for DOS. Find the one you need here:
   http://www.georgpotthast.de/sioux/packet.htm

**How to build and run?**

Change the destination MAC ID by editing this line in `dostalk_dataLinkLayer.c`  to give the correct destination MAC ID:

    unsigned char dest_mac[] = "\x08\x00\x27\xe4\xee\xd6"; 

Compile `dostalk_dataLinkLayer.c` on DOS with the compiler mode set to huge.

Repeat the above steps on the other computer.

Run `dostalk_dataLinkLayer.exe` on both the machines.

Enjoy :)
 


 


> Written with [StackEdit](https://stackedit.io/).