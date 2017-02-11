#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>


#define SWINT 0x60
#define CR 0x0D
#define LF 0x0A
#define MAX_PACK 100
#define MAX_BUF 50

/**
PACKET format

0 ****next hop_mac(6)**** |6 *****source_mac(6)****** |12 **type(2)** |14 ***dest_ip(2)*** |16 **source(2)** |18 ***data**.....


**/
///change this to user input later
unsigned char router_mac[] = "\x08\x00\x27\x1c\xe6\x54"; // mac of router
unsigned char my_ip[2]={1,1}; /// format - {network,host}

/**
unsigned char router_mac[] = "\x08\x00\x27\xdd\x6d\x32"; // mac of router .. side 2 
unsigned char my_ip[2]={2,1}; /// format - {network,host}
**/
unsigned char dest_ip[2];

unsigned char out_packet[MAX_PACK];
unsigned char c[2];
int handle , in_packet_len , i;
unsigned char in_packet[MAX_PACK];
unsigned char my_mac[6];
unsigned char type[]="\xff\xff";

unsigned char broadcast_mac[] = "\xff\xff\xff\xff\xff\xff";
unsigned char buffer[MAX_BUF];
int buflen;
int delimcount=0;
unsigned char tmp;


void get_my_mac();
void fill_packet_headers();
void fill_msg(unsigned char *msg , int length);
void send_packet();
int access_type();
int release_type();


void interrupt receiver(bp, di, si, ds, es, dx, cx, bx, ax, ip, cs, flags)
unsigned short bp, di, si, ds, es, dx, cx, bx, ax, ip, cs, flags;
{
	if(ax==0)
	{
		if(cx>MAX_PACK)
		{
			es=0;
			di=0;
			return;
		}

		es=FP_SEG(in_packet);// where to store .. give it the address!
		di=FP_OFF(in_packet);
		in_packet_len=cx;
	}

	else
	{
		///first filter out Broadcasts and messages not meant for itself
		if(memcmp(in_packet, my_mac , 6)!=0)
		{
		 //	cprintf("Not meant for me :(");
		//	putch('\r'); putch('\n');
			return;
		}

		cprintf("%d.%d > ", in_packet[16] , in_packet[17]);
		for(i=18;i<in_packet_len;i++)
		{
			if(in_packet[i]==0x00)
				break;
			putch(in_packet[i]);
		}
		putch('\r'); putch('\n');
	       //	cprintf("Done printing msg\n");
		return;
	}
}


int main()
{
	get_my_mac();
	input_dest_ip();
	fill_packet_headers();
	access_type();
	buflen =0;
	//Press enter twice to end
	
	while(1)
	{
		tmp=getchar();
		if(tmp==LF || tmp==CR)
		{
			if(delimcount>=1)
				break;

			if(buflen==0)
				delimcount++;


				fill_msg(buffer,buflen);
				buflen=0;
				send_packet();

		}
		else
		{
			delimcount=0;
			buffer[buflen]=tmp;
			buflen++;
		}
	}

	release_type();
	printf("Exting  ..\n\r");
	return 0;
}

void get_my_mac()
{
	union REGS inregs,outregs;
	struct SREGS segregs;
	char far *bufptr;
	segread(&segregs);
	bufptr = (char far *)my_mac;
	segregs.es = FP_SEG(bufptr);
	inregs.x.di = FP_OFF(my_mac);
	inregs.x.cx = 6;
	inregs.h.ah = 6;
	int86x(SWINT, &inregs, &outregs, &segregs);
}

void fill_packet_headers()
{

	memcpy(out_packet,router_mac, 6); //set the destination mac
	memcpy(out_packet+6, my_mac, 6); //set the source mac
	memcpy(out_packet+12, type, 2); //set the type
	memcpy(out_packet+14, dest_ip ,2 );//destination ip
	memcpy(out_packet+16, my_ip , 2);// source ip
}

void fill_msg(unsigned char *msg, int length)
{
	memcpy(out_packet+18,msg, length);
	//zero out the others

	for(i=length+18;i<MAX_BUF-4;i++)
		out_packet[i]=0;

}

void send_packet()
{
	union REGS inregs,outregs;
	struct SREGS segregs;
	segread(&segregs);
	inregs.h.ah = 4;
	segregs.ds = FP_SEG(out_packet);
	inregs.x.si = FP_OFF(out_packet);
	inregs.x.cx = MAX_PACK;
	int86x(SWINT,&inregs,&outregs,&segregs);
}

int access_type()
{
	union REGS inregs,outregs;
	struct SREGS segregs;
	inregs.h.al=1; //if_class
	inregs.x.bx=-1;///try changing this
	inregs.h.dl=0; //if_number
	inregs.x.cx=0; //typelen =0
	inregs.h.ah=2; //interrupt number
	segregs.es=FP_SEG(receiver);
	inregs.x.di=FP_OFF(receiver);
	c[0]=0xff;
	c[1]=0xff;
	segregs.ds=FP_SEG(c);
	inregs.x.si=FP_OFF(c);
	int86x(SWINT,&inregs,&outregs,&segregs);
	printf("Carry Flag Access Type %d\n",outregs.x.cflag);
	printf("Handle %d\n",outregs.x.ax);
	handle = outregs.x.ax;
	return outregs.x.cflag;
}

int release_type()
{
	union REGS inregs,outregs;
	struct SREGS segregs;
	inregs.h.ah=3;
	inregs.x.bx=handle;
	int86x(SWINT,&inregs,&outregs,&segregs);
	printf("Carry Flag Release Type %d\n",outregs.x.cflag);

	return 0;
}


int input_dest_ip()
{
	printf("Enter destination ip : ");
	dest_ip[0]=getchar()-'0';
	getchar();
	dest_ip[1]=getchar()-'0';
	getchar();
	getchar();
}

