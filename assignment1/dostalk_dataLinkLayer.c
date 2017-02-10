#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>


#define SWINT 0x60
#define CR 0x0D
#define LF 0x0A
#define MAX_IN_PACK 200
#define MAX_BUF 50

///change this to user input later
//unsigned char dest_mac[] = "\x08\x00\x27\x52\xA8\xAD"; // mac of clone
unsigned char dest_mac[] = "\x08\x00\x27\xe4\xee\xd6"; // mac of actual
unsigned char out_packet[64];
unsigned char c[2];
int handle , in_packet_len , i;
unsigned char in_packet[MAX_IN_PACK];
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
		if(cx>MAX_IN_PACK)
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
		///first filter out Broadcasts
		if(memcmp(in_packet, broadcast_mac , 6)==0)
		{
		 //	cprintf("BC\n");
		//	putch('\r'); putch('\n');
			return;
		}

		for(i=6;i<12;i++)
		{
			cprintf("%02x:", in_packet[i]);
		}
		cprintf(" > ");
		for(i=14;i<in_packet_len;i++)
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

	memcpy(out_packet,dest_mac, 6); //set the destination mac
	memcpy(out_packet+6, my_mac, 6); //set the source mac
	memcpy(out_packet+12, type, 2); //set the type
}

void fill_msg(unsigned char *msg, int length)
{
	memcpy(out_packet+14,msg, length);
	//zero out the others

	for(i=length+14;i<60;i++)
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
	inregs.x.cx = 64;
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

