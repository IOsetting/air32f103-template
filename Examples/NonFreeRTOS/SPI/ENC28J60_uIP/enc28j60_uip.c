#include "enc28j60_uip.h"

#include <stdio.h>
#include "enc28j60.h"
#include "uip.h"

extern struct uip_eth_addr uip_ethaddr;
static unsigned char mymac[6] = {0x04,0x02,0x35,0x00,0x00,0x01};

void etherdev_init(void)
{
  u8 i;
	/*initialize enc28j60*/
	enc28j60Init(mymac);

	for (i = 0; i < 6; i++) {
		uip_ethaddr.addr[i] = mymac[i];
	}
  // LED: 0x476 is PHLCON LEDA=links status, LEDB=receive/transmit
	enc28j60PhyWrite(PHLCON, 0x476);
  // Change clkout from 6.25MHz to 12.5MHz
	enc28j60clkout(2); 
}

void etherdev_send(u8 *p_char, u16 length)
{
	enc28j60PacketSend(length, p_char);
}

u16 etherdev_read(u8 *p_char)
{
	return enc28j60PacketReceive(1500, p_char);
}

void tcp_server_appcall(void)
{
	switch(uip_conn->lport)  {
		// case HTONS(80):
		// 	httpd_appcall(); 
		// 	break;
		case HTONS(1200):
		  tcp_demo_appcall(); 
			break;
    default:
      printf("Unknown packet\r\n");
	}
}

/*
                etherdev_poll()

    This function will read an entire IP packet into the uip_buf.
    If it must wait for more than 0.5 seconds, it will return with
    the return value 0. Otherwise, when a full packet has been read
    into the uip_buf buffer, the length of the packet is returned.
*/
u16 etherdev_poll(void)
{
	u16 bytes_read = 0;
	return bytes_read;
}
