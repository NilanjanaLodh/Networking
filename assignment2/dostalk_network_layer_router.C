#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>



#define CR 0x0D
#define LF 0x0A
#define MAX_PACK 100
#define MAX_BUF 50

/**
PACKET format

0 ****next hop_mac(6)**** |6 *****source_mac(6)****** |12 **type(2)** |14 ***dest_ip(2)*** |16 **source(2)** |18 ***data**.....


**/
unsigned char SWINT[2]={0x60 , 0x70};
unsigned char router_mac0[] = "\x08\x00\x27\x1c\xe6\x54"; // mac of router
unsigned char router_mac1[] = "\x08\x00\x27\xdd\x6d\x32"; // mac of router .. side 2 
unsigned char routing_table[3][20][6];


//unsigned char source_ip[2]={1,1}; /// format - {network,host}
unsigned char dest_ip[2];
unsigned char dest_mac[6];

unsigned char out_packet[MAX_PACK];
unsigned char c[2];
int handle , in_packet_len , i;
unsigned char in_packet[MAX_PACK];

unsigned char type[]="\xff\xff";

unsigned char broadcast_mac[] = "\xff\xff\xff\xff\xff\xff";
unsigned char buffer[MAX_BUF];
int buflen;
int delimcount=0;
unsigned char tmp;


void replace_packet_headers(unsigned char* smac , unsigned char* dmac);
void send_packet(int i);
int access_type(int i);
int release_type(int i);
void setup_routing_table();


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
		if(memcmp(in_packet, broadcast_mac , 6)==0)
		{
			return;
		}

		memcpy(out_packet+12, in_packet+12 , in_packet_len-12);
		dest_ip[0]=in_packet[14];
		dest_ip[1]=in_packet[15];
		memcpy(dest_mac,&routing_table[dest_ip[0]][dest_ip[1]][0],6);
		if(memcmp(in_packet, router_mac0 , 6)==0)
		{
			cprintf("routing packet from 0 -- > 1\n\r");
			replace_packet_headers(router_mac1 , dest_mac);
			cprintf("packet being sent to ");
			for(i=0;i<6;i++)
			{
				cprintf("%02x : ", out_packet[i]);
			}
			putch('\r');
			putch('\n');
			send_packet(1);
			return;
		}

		if(memcmp(in_packet, router_mac1 , 6)==0)
		{
			cprintf("routing packet from 1 -- > 0\n\r");
			replace_packet_headers(router_mac0 , dest_mac);
			for(i=0;i<6;i++)
			{
				cprintf("%02x : ", out_packet[i]);
			}
			putch('\r');
			putch('\n');
			send_packet(0);
			return;
		}


		return;
	}
}


int main()
{
	setup_routing_table();
	access_type(0);
	access_type(1);
	buflen =0;

	getchar();
	getchar();
	//Press enter twice to end

	release_type(0);
	release_type(1);
	printf("Exiting routing ..\n\r");
	return 0;
}

void replace_packet_headers(unsigned char* smac , unsigned char* dmac)
{

	memcpy(out_packet,dmac, 6); //set the destination mac
	memcpy(out_packet+6, smac, 6); //set the source mac

}



void send_packet(int i)
{
       //	cprintf("sending packet via NIC %d\r\n",i);
	union REGS inregs,outregs;
	struct SREGS segregs;
	cprintf("sending packet via NIC %d\r\n",i);
	segread(&segregs);
	inregs.h.ah = 4;
	segregs.ds = FP_SEG(out_packet);
	inregs.x.si = FP_OFF(out_packet);
	inregs.x.cx = 64;
	int86x(SWINT[i],&inregs,&outregs,&segregs);
}

int access_type(int i)
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
	int86x(SWINT[i],&inregs,&outregs,&segregs);
	printf("Carry Flag Access Type %d\n",outregs.x.cflag);
	printf("Handle %d\n",outregs.x.ax);
	handle = outregs.x.ax;
	return outregs.x.cflag;
}

int release_type(int i)
{
	union REGS inregs,outregs;
	struct SREGS segregs;
	inregs.h.ah=3;
	inregs.x.bx=handle;
	int86x(SWINT[i],&inregs,&outregs,&segregs);
	printf("Carry Flag Release Type %d\n",outregs.x.cflag);

	return 0;
}

void setup_routing_table()
{
	memcpy(&routing_table[1][1][0],"\x08\x00\x27\xe4\xee\xd6",6);
	memcpy(&routing_table[1][2][0],"\x08\x00\x27\xA6\xCE\xE7",6);
	memcpy(&routing_table[2][1][0],"\x08\x00\x27\x52\xa8\xad",6);
	memcpy(&routing_table[2][2][0],"\x08\x00\x27\x23\x90\xBB",6);
}