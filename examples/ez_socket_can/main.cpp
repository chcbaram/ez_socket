#include <iostream>
#include <thread>

#include "hw.h"
#include "ez_hal.h"


using namespace ez;

#define DATA_BUF_MAX (2*1024*1024)

uint8_t data_buf[DATA_BUF_MAX];



std::thread read_thread;


ez_socket_t ez_socket;

cmd_can_t cmd_can;
cmd_can_driver_t cmd_can_driver;
uint8_t cmd_can_buf[1024*1024];
qbuffer_t cmd_can_q;

int beginServer(void);
int beginClient(void);

uint32_t canDriverAvailable(void);
bool     canDriverFlush(void);
uint8_t  canDriverRead(void);
uint32_t canDriverWrite(uint8_t *p_data, uint32_t length);






int main(int argc, char *argv[])
{  
  log("argc %d\n", argc);

  cmd_can_driver.available = canDriverAvailable;
  cmd_can_driver.flush = canDriverFlush;
  cmd_can_driver.read = canDriverRead;
  cmd_can_driver.write = canDriverWrite;

  cmdCanInit(&cmd_can, &cmd_can_driver);
  cmdCanOpen(&cmd_can);

  qbufferCreate(&cmd_can_q, cmd_can_buf, 1024*1024);


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

  err_ret = socketInit(&ez_socket, EZ_SOCKET_SERVER, EZ_SOCKET_UDP);
  err_ret = socketCreate(&ez_socket);
  err_ret = socketBind(&ez_socket, NULL, 30000);
  if (err_ret == EZ_OK)
  {
    read_thread = std::thread([=] 
    {
      uint8_t rx_buf[512];
      uint32_t heart_cnt = 0;

      while(1)
      {
        int rx_len;

        rx_len = socketRead(&ez_socket, (uint8_t *)rx_buf, 512);
        if (rx_len > 0)
        {
          qbufferWrite(&cmd_can_q, rx_buf, rx_len);
        }

        if (cmdCanReceivePacket(&cmd_can) == true)
        {
          if (cmd_can.rx_packet.type == PKT_TYPE_CMD)
          {
            log("Receive Cmd : 0x%02X\n", cmd_can.rx_packet.cmd);
            switch(cmd_can.rx_packet.cmd)
            {
              case PKT_CMD_PING:
                memcpy(cmd_can.packet.data, &heart_cnt, sizeof(heart_cnt));
                cmdCanSendResp(&cmd_can, cmd_can.rx_packet.cmd, cmd_can.packet.data, sizeof(heart_cnt));
                heart_cnt++;
                break;

              default:
                break;
            }
          }
          if (cmd_can.rx_packet.type == PKT_TYPE_PING)
          {
            log("Receive Type Ping\n");
          }
          if (cmd_can.rx_packet.type == PKT_TYPE_CAN)
          {
            can_msg_t can_msg;

            memcpy(&can_msg, cmd_can.rx_packet.data, sizeof(can_msg));

            log("rx can msg id:0x%X, ext:%d dlc:%d, ", can_msg.id, can_msg.id_type, can_msg.length);
            for (int i=0; i<can_msg.length; i++)
            {
              log("0x%02X, ", can_msg.data[i]);
            }
            log("\n");
          }
        }        
      }  
    });
    
    uint32_t can_cnt = 0;
    while(1)
    {
      can_msg_t can_msg;

      can_msg.id  = 0x123;
      can_msg.id_type = CAN_EXT;
      can_msg.frame = CAN_CLASSIC;
      can_msg.frame_type = CAN_FRAME_TYPE_DATA;      
      can_msg.length = 4;
      memcpy(can_msg.data, &can_cnt, 4);
      can_cnt++;

      cmdCanSendType(&cmd_can, PKT_TYPE_CAN, (uint8_t *)&can_msg, sizeof(can_msg));      
      ez::delay(1000);
    }        
  }

  ret = socketClose(&ez_socket);
  ret = socketDestroy(&ez_socket);
  ret = socketDeInit(&ez_socket);

  printf("exit..\n");

  return EZ_OK;
}

int beginClient(void)
{
  ez_err_t err_ret;
  int ret;

  err_ret = socketInit(&ez_socket, EZ_SOCKET_CLIENT, EZ_SOCKET_UDP);
  err_ret = socketCreate(&ez_socket);
  
  err_ret = socketConnect(&ez_socket, "127.0.0.1", 30000);  
  if (err_ret == EZ_OK)
  {
    char readmsg[128];
    ez_ip_addr_t remote_ip;
    socketGetRemoteIP(&ez_socket, &remote_ip);
    log("Remote IP : %s:%d\n", remote_ip.ip_addr, remote_ip.port);

    read_thread = std::thread([=] 
    {
      uint8_t rx_buf[512];

      while(1)
      {
        int rx_len;

        rx_len = socketRead(&ez_socket, (uint8_t *)rx_buf, 512);
        if (rx_len > 0)
        {
          qbufferWrite(&cmd_can_q, rx_buf, rx_len);
        }

        if (cmdCanReceivePacket(&cmd_can) == true)
        {
          if (cmd_can.rx_packet.type == PKT_TYPE_RESP)
          {
            if (cmd_can.rx_packet.cmd == PKT_CMD_PING)
            {
              uint32_t heart_bit = 0;            
              memcpy(&heart_bit, cmd_can.rx_packet.data, 4);
              log("rx packet : 0x%X, %d\n", cmd_can.rx_packet.cmd, heart_bit);   
            }
          }
          if (cmd_can.rx_packet.type == PKT_TYPE_CAN)
          {
            can_msg_t can_msg;

            memcpy(&can_msg, cmd_can.rx_packet.data, sizeof(can_msg));

            log("rx can msg id:0x%X, dlc:%d, ", can_msg.id, can_msg.length);
            for (int i=0; i<can_msg.length; i++)
            {
              log("0x%02X, ", can_msg.data[i]);
            }
            log("\n");
          }
        }        
      }  
    });

    while(1)
    {
      cmdCanSendCmd(&cmd_can, PKT_CMD_PING, NULL, 0);
      ez::delay(100);
    }
  }

  err_ret = socketClose(&ez_socket);
  err_ret = socketDestroy(&ez_socket);
  err_ret = socketDeInit(&ez_socket);

  printf("exit..\n");

  return EZ_OK;
}




uint32_t canDriverAvailable(void)
{
  return qbufferAvailable(&cmd_can_q);
}

bool canDriverFlush(void)
{
  qbufferFlush(&cmd_can_q);
  return true;
}

uint8_t canDriverRead(void)
{
  uint8_t ret;

  qbufferRead(&cmd_can_q, &ret, 1);
  return ret;
}

uint32_t canDriverWrite(uint8_t *p_data, uint32_t length)
{
  int ret = 0;

  if (socketIsRemtoeIP(&ez_socket))
  {
    ret = socketWrite(&ez_socket, (const uint8_t *)p_data, length);  
  }

  if (ret < 0) ret = 0;

  return ret;
}
