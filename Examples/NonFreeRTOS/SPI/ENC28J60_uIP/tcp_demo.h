#ifndef __TCP_DEMO_H__
#define __TCP_DEMO_H__

/* Since this file will be included by uip.h, we cannot include uip.h
   here. But we might need to include uipopt.h if we need the u8_t and
   u16_t datatypes. */

#include "uipopt.h"
#include "enc28j60_uip.h"

enum
{
  STATE_CMD     = 0,
  STATE_TX_TEST = 1,
  STATE_RX_TEST = 2
};

struct tcp_demo_appstate
{
  u8_t state;
  u8_t *textptr;
  int textlen;
};

typedef struct tcp_demo_appstate uip_tcp_appstate_t;

#ifndef UIP_APPCALL
#define UIP_APPCALL tcp_server_appcall
#endif

void tcp_demo_appcall(void);

#endif
