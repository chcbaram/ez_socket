#include <iostream>
#include <thread>

#include "ez_hal.h"

using namespace ez;

#define DATA_BUF_MAX (2*1024*1024)

uint8_t data_buf[DATA_BUF_MAX];



std::thread read_thread;


ez_socket_t ez_server;
ez_socket_t ez_client;


int beginServer(void);
int beginClient(void);


int main(int argc, char *argv[])
{
  log("argc %d\n", argc);

  if (argc == 2 && strcmp(argv[1], "server") == 0)
  {
    log("begin server...\n");
    beginServer();
  }
  else if (argc == 2 && strcmp(argv[1], "client") == 0)
  {
    log("begin client...\n");
    beginClient();
  }
  else
  {
    log("wrong parameter\n");
  }

  return 0;
}


int beginServer(void)
{
  ez_err_t err_ret;
  int ret;

  err_ret = socketInit(&ez_server, EZ_SOCKET_SERVER, EZ_SOCKET_UDP);
  err_ret = socketCreate(&ez_server);
  err_ret = socketBind(&ez_server, NULL, 30000);
  if (err_ret == EZ_OK)
  {
    char readmsg[128];
    int rx_cnt = 0;
    int rx_len_sum = 0;


    read_thread = std::thread([=] 
    {
      while(1)
      {
        int rx_len;

        rx_len = socketRead(&ez_server, (uint8_t *)readmsg, 28);
        if (rx_len > 0)
        {
          logInfo("socketRead OK, %s\n", readmsg);
        }
        else
        {
          logError("read\n");
        }
      }  
    });

    while(1)
    {
      char sendmsg[] = "This is message from Server";

      if (socketIsRemtoeIP(&ez_server) == true)
      {
        ret = socketWrite(&ez_server, (const uint8_t *)sendmsg, sizeof(sendmsg));      
        if (ret > 0)
        {
          log("wr_len : %d\n", ret);
        }
        else
        {
          logError("write\n");
        }
      }
      delay(1000);      
    }        
  }

  ret = socketClose(&ez_server);
  ret = socketDestroy(&ez_server);
  ret = socketDeInit(&ez_server);

  printf("exit..\n");

  return EZ_OK;
}

int beginClient(void)
{
  ez_err_t err_ret;
  int ret;

  err_ret = socketInit(&ez_server, EZ_SOCKET_CLIENT, EZ_SOCKET_UDP);
  err_ret = socketCreate(&ez_server);
  
  err_ret = socketConnect(&ez_server, "127.0.0.1", 30000);  
  if (err_ret == EZ_OK)
  {
    char readmsg[128];
    ez_ip_addr_t remote_ip;
    socketGetRemoteIP(&ez_server, &remote_ip);
    log("Remote IP : %s:%d\n", remote_ip.ip_addr, remote_ip.port);

    read_thread = std::thread([=] 
    {
      while(1)
      {
        int rx_len;

        rx_len = socketRead(&ez_server, (uint8_t *)readmsg, 28);
        if (rx_len > 0)
        {
          logInfo("socketRead OK, %s\n", readmsg);
        }
        else
        {
          logError("read\n");
        }
      }  
    });

    while(1)
    {
      char sendmsg[] = "This is message from Remote";

      ret = socketWrite(&ez_server, (const uint8_t *)sendmsg, sizeof(sendmsg));      
      if (ret > 0)
      {
        log("wr_len : %d\n", ret);
      }
      else
      {
        logError("write\n");
      }
      delay(1000);
    }
  }

  err_ret = socketClose(&ez_server);
  err_ret = socketDestroy(&ez_server);
  err_ret = socketDeInit(&ez_server);

  printf("exit..\n");

  return EZ_OK;
}
