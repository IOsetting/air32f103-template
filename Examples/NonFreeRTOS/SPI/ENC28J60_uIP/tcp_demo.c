#include "tcp_demo.h"

#include <string.h>
#include <stdio.h>
#include "air32f10x_conf.h"
#include "uip.h"

static void aborted(void);
static void timedout(void);
static void closed(void);
static void connected(void);
static void newdata(void);
static void acked(void);
static void senddata(void);

static u8 test_data[2048];

/**
 * callback function for UIP_APPCALL
 */
void tcp_demo_appcall(void)
{
  if (uip_aborted())
  {
    printf("uip_aborted!\r\n");
    aborted();
  }

  if (uip_timedout())
  {
    printf("uip_timedout!\r\n");
    timedout();
  }

  if (uip_closed())
  {
    printf("uip_closed!\r\n");
    closed();
  }

  if (uip_connected())
  {
    printf("uip_connected!\r\n");
    connected();
  }

  if (uip_acked())
  {
    acked();
  }

  if (uip_newdata())
  {
    newdata();
  }

  if (uip_rexmit() || uip_newdata() || uip_acked() || uip_connected() || uip_poll())
  {
    senddata();
  }
}

static void aborted(void)
{
  ;
}

static void timedout(void)
{
  ;
}

static void closed(void)
{
  ;
}

static void connected(void)
{

  struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;

  //memset(test_data, 0x55, 2048);

  s->state = STATE_CMD;
  s->textlen = 0;

  s->textptr = (u8_t *)"Connect STM32-FD Board Success!";
  s->textlen = strlen((char *)s->textptr);
  
  //senddata();
  
  //a[0]=0x0d; a[1]=0x0a;
  //s->textptr = a;
  //s->textlen = 2;

}

void TCP_Cmd(struct tcp_demo_appstate *s)
{
  uint8_t led;

  if ((uip_len == 7) && (memcmp("ledon ", uip_appdata, 6) == 0))
  {
    led = ((uint8_t *)uip_appdata)[6];
    if (led == '1')
    {
      s->textptr = (u8_t *)"Led 1 On!";
    }
    else if (led == '2')
    {
      s->textptr = (u8_t *)"Led 2 On!";
    }
    else if (led == '3')
    {
      s->textptr = (u8_t *)"Led 3 On!";
    }
    s->textlen = strlen((char *)s->textptr);
  }
  else if ((uip_len == 8) && (memcmp("ledoff ", uip_appdata, 7) == 0))
  {
    led = ((uint8_t *)uip_appdata)[7]; /* 操作的LED序号 */
    if (led == '1')
    {
      s->textptr = (u8_t *)"Led 1 Off!";
    }
    else if (led == '2')
    {
      s->textptr = (u8_t *)"Led 2 Off!";
    }
    else if (led == '3')
    {
      s->textptr = (u8_t *)"Led 3 Off!";
    }

    s->textlen = strlen((char *)s->textptr);
  }
  else if ((uip_len == 6) && (memcmp("txtest", uip_appdata, 6) == 0))
  {
    s->state = STATE_TX_TEST;

    s->textptr = test_data;
    s->textlen = 1400;
  }
  else if ((uip_len == 6) && (memcmp("rxtest", uip_appdata, 6) == 0))
  {
    s->state = STATE_RX_TEST;
    s->textptr = (u8_t *)"Ok";
    s->textlen = 2;
  }
  else
  {
    s->textptr = (u8_t *)"Unknow Command!\r\n";
    s->textlen = strlen((char *)s->textptr);
  }
}

static void newdata(void)
{
  struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;

  if (s->state == STATE_CMD)
  {
    printf("uip_newdata!\r\n");
    TCP_Cmd(s);
  }
  else if (s->state == STATE_TX_TEST)
  {
    if ((uip_len == 1) && (((uint8_t *)uip_appdata)[0] == 'A'))
    {
      ;
    }
    else
    {
      s->state = STATE_CMD;
      s->textlen = 0;
    }
  }
  else if (s->state == STATE_RX_TEST)
  {
    if ((uip_len == 4) && (memcmp("stop", uip_appdata, 4) == 0))
    {
      s->state = STATE_CMD;
      s->textlen = 0;
    }
    else
    {
      static int sLen;

      sLen = uip_len;
      s->textptr = (uint8_t *)&sLen;
      s->textlen = 4;
    }
  }
}

static void acked(void)
{
  struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;

  switch (s->state)
  {
    case STATE_CMD:
      s->textlen = 0;
      printf("uip_acked!\r\n");
      break;

    case STATE_TX_TEST:
      s->textptr = test_data;
      s->textlen = 1400;
      break;

    case STATE_RX_TEST:
      s->textlen = 0;
      break;
  }
}

static void senddata(void)
{
  struct tcp_demo_appstate *s = (struct tcp_demo_appstate *)&uip_conn->appstate;

  if (s->textlen > 0)
  {
    uip_send(s->textptr, s->textlen);
  }
}

void uip_log(char *m)
{
  printf("uIP log message: %s\r\n", m);
}
