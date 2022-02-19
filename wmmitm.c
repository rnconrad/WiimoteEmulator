#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <sys/time.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdbool.h>
#include <string.h>
#include <poll.h>
#include <pthread.h>

#include "sdp.h"
#include "adapter.h"
#include "wm_print.h"

#define PSM_SDP 1
#define PSM_CTRL 0x11
#define PSM_INT 0x13

bdaddr_t host_device_bdaddr;
bdaddr_t wiimote_device_bdaddr;
bdaddr_t host_bdaddr;
bdaddr_t wiimote_bdaddr;

int sdp_fd, ctrl_fd, int_fd;
int wm_ctrl_fd, wm_int_fd;
int sock_sdp_fd, sock_ctrl_fd, sock_int_fd;

extern int show_reports;

static int has_host = 0;
static int is_connected = 0;

//signal handler to break out of main loop
static int running = 1;
void sig_handler(int sig)
{
  running = 0;
}

int create_socket()
{
  int fd;
  struct linger l = { .l_onoff = 1, .l_linger = 5 };
  int opt = 0;

  fd = socket(AF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
  if (fd < 0)
  {
    return -1;
  }

  if (setsockopt(fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0)
  {
    close(fd);
    return -1;
  }

  if (setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &opt, sizeof(opt)) < 0)
  {
    close(fd);
    return -1;
  }

  if (setsockopt(fd, SOL_L2CAP, L2CAP_LM, &opt, sizeof(opt)) < 0)
  {
    close(fd);
    return -1;
  }

  return fd;
}

int l2cap_connect(bdaddr_t device_bdaddr, bdaddr_t bdaddr, int psm)
{
  int fd;
  struct sockaddr_l2 addr;

  fd = create_socket();
  if (fd < 0)
  {
    return -1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.l2_family = AF_BLUETOOTH;
  addr.l2_psm    = htobs(psm);
  addr.l2_bdaddr = device_bdaddr;

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    close(fd);
    return -1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.l2_family = AF_BLUETOOTH;
  addr.l2_psm    = htobs(psm);
  addr.l2_bdaddr = bdaddr;

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    close(fd);
    return -1;
  }

  return fd;
}

int l2cap_listen(bdaddr_t device_bdaddr, int psm)
{
  int fd;
  struct sockaddr_l2 addr;

  fd = create_socket();
  if (fd < 0)
  {
    return -1;
  }

  memset(&addr, 0, sizeof(addr));
  addr.l2_family = AF_BLUETOOTH;
  addr.l2_psm = htobs(psm);
  addr.l2_bdaddr = device_bdaddr;

  if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    close(fd);
    return -1;
  }

  if (listen(fd, 1) < 0)
  {
    close(fd);
    return -1;
  }

  return fd;
}

int listen_for_connections()
{
#ifdef SDP_SERVER
  sock_sdp_fd = l2cap_listen(host_device_bdaddr, PSM_SDP);
  if (sock_sdp_fd < 0)
  {
    printf("can't listen on psm %d: %s\n", PSM_SDP, strerror(errno));
    return -1;
  }
#endif

  sock_ctrl_fd = l2cap_listen(host_device_bdaddr, PSM_CTRL);
  if (sock_ctrl_fd < 0)
  {
    printf("can't listen on psm %d: %s\n", PSM_CTRL, strerror(errno));
    return -1;
  }

  sock_int_fd = l2cap_listen(host_device_bdaddr, PSM_INT);
  if (sock_int_fd < 0)
  {
    printf("can't listen on psm %d: %s\n", PSM_INT, strerror(errno));
    return -1;
  }

  return 0;
}

int accept_connection(int socket_fd, bdaddr_t * bdaddr)
{
  int fd;
  struct sockaddr_l2 addr;
  socklen_t opt = sizeof(addr);

  fd = accept(socket_fd, (struct sockaddr *)&addr, &opt);
  if (fd < 0)
  {
    return -1;
  }

  if (bdaddr != NULL)
  {
    *bdaddr = addr.l2_bdaddr;
  }

  return fd;
}

int connect_to_host()
{
  ctrl_fd = l2cap_connect(host_device_bdaddr, host_bdaddr, PSM_CTRL);
  if (ctrl_fd < 0)
  {
    printf("can't connect to host psm %d: %s\n", PSM_CTRL, strerror(errno));
    return -1;
  }

  int_fd = l2cap_connect(host_device_bdaddr, host_bdaddr, PSM_INT);
  if (int_fd < 0)
  {
    printf("can't connect to host psm %d: %s\n", PSM_INT, strerror(errno));
    return -1;
  }

  return 0;
}

int connect_to_wiimote()
{
  wm_ctrl_fd = l2cap_connect(wiimote_device_bdaddr, wiimote_bdaddr, PSM_CTRL);
  if (wm_ctrl_fd < 0)
  {
    printf("can't connect to wiimote psm %d: %s\n", PSM_CTRL, strerror(errno));
    return -1;
  }

  wm_int_fd = l2cap_connect(wiimote_device_bdaddr, wiimote_bdaddr, PSM_INT);
  if (wm_int_fd < 0)
  {
    printf("can't connect to wiimote psm %d: %s\n", PSM_INT, strerror(errno));
    return -1;
  }

  return 0;
}

void disconnect_from_host()
{
  shutdown(sdp_fd, SHUT_RDWR);
  shutdown(ctrl_fd, SHUT_RDWR);
  shutdown(int_fd, SHUT_RDWR);

  close(sdp_fd);
  close(ctrl_fd);
  close(int_fd);

  sdp_fd = 0;
  ctrl_fd = 0;
  int_fd = 0;
}

void disconnect_from_wiimote()
{
  shutdown(wm_ctrl_fd, SHUT_RDWR);
  shutdown(wm_int_fd, SHUT_RDWR);

  close(wm_ctrl_fd);
  close(wm_int_fd);

  wm_ctrl_fd = 0;
  wm_int_fd = 0;
}

int main(int argc, char *argv[])
{
  struct pollfd pfd[8];

  unsigned char buf[256];
  ssize_t len;
  unsigned char in_buf[256];
  ssize_t in_buf_len = 0;
  unsigned char out_buf[256];
  ssize_t out_buf_len = 0;

  int failure = 0;

  int enable_report_printing = 0;
  show_reports = 1;

  if (argc > 1)
  {
    if (bachk(argv[1]) < 0)
    {
      printf("usage: %s <wiimote-bdaddr> <wii-bdaddr>\n", *argv);
      return 1;
    }

    str2ba(argv[1], &wiimote_bdaddr);

    if (argc > 2)
    {
      if (bachk(argv[2]) < 0)
      {
        printf("usage: %s <wiimote-bdaddr> <wii-bdaddr>\n", *argv);
        return 1;
      }

      str2ba(argv[2], &host_bdaddr);
      has_host = 1;
    }
  }

  //set up unload signals
  signal(SIGINT, sig_handler);
  signal(SIGTERM, sig_handler);
  signal(SIGHUP, sig_handler);

  if (set_up_device(NULL) < 0)
  {
    printf("failed to set up Bluetooth device\n");
    return 1;
  }

  if (get_device_bdaddr(0, &host_device_bdaddr) < 0)
  {
    printf("failed to get host Bluetooth adapter address\n");
    restore_device();
    return 1;
  }

  if (get_device_bdaddr(1, &wiimote_device_bdaddr) < 0)
  {
    printf("failed to get Wiimote Bluetooth adapter address\n");
    printf("Warning: two Bluetooth adapters are required for proper functionality\n");
    wiimote_device_bdaddr = host_device_bdaddr;
  }

#ifndef SDP_SERVER
  if (register_wiimote_sdp_record() < 0)
  {
    printf("failed to add Wiimote SDP record\n");
    restore_device();
    return 1;
  }
#endif

  printf("connecting to wiimote... (press wiimote's sync button)\n");

  while (!bacmp(&wiimote_bdaddr, BDADDR_ANY))
  {
    if (failure++ > 3)
    {
      printf("couldn't find a wiimote to connect to\n");
      restore_device();
      return 1;
    }

    find_wiimote(&wiimote_bdaddr);
  }

  if (connect_to_wiimote() < 0)
  {
    printf("failed to connect to wiimote\n");
    restore_device();
    return 1;
  }

  if (has_host)
  {
    printf("connecting to host...\n");
    if (connect_to_host() < 0)
    {
      printf("couldn't connect to host\n");
      running = 0;
    }
    else
    {
      char straddr[18];
      ba2str(&host_bdaddr, straddr);
      printf("connected to host %s\n", straddr);

      is_connected = 1;
    }
  }
  else
  {
    if (listen_for_connections() < 0)
    {
      printf("couldn't listen\n");
      running = 0;
    }
    else
    {
      printf("listening for host connections... (press wii's sync button)\n");
    }
  }

  while (running)
  {
    memset(&pfd, 0, sizeof(pfd));

    pfd[0].fd = sock_sdp_fd;
    pfd[1].fd = sock_ctrl_fd;
    pfd[2].fd = sock_int_fd;

    pfd[3].fd = sdp_fd;

    pfd[4].fd = ctrl_fd;
    pfd[5].fd = int_fd;

    pfd[6].fd = wm_ctrl_fd;
    pfd[7].fd = wm_int_fd;

    if (!is_connected)
    {
      pfd[0].events = POLLIN;
      pfd[1].events = POLLIN;
      pfd[2].events = POLLIN;

      pfd[3].events = POLLIN | POLLOUT;
    }
    else
    {
      pfd[4].events = POLLIN;
      pfd[5].events = POLLIN;
      pfd[6].events = POLLIN;
      pfd[7].events = POLLIN;

      if (in_buf_len > 0)
      {
        pfd[5].events |= POLLOUT;
      }
      if (out_buf_len > 0)
      {
        pfd[7].events |= POLLOUT;
      }
    }

    if (poll(pfd, 8, 10) < 0)
    {
      printf("poll error\n");
      break;
    }

    if (pfd[4].revents & POLLERR)
    {
      printf("error on ctrl psm\n");
      break;
    }
    if (pfd[5].revents & POLLERR)
    {
      printf("error on data psm\n");
      break;
    }
    if (pfd[6].revents & POLLERR)
    {
      printf("error on ctrl psm\n");
      break;
    }
    if (pfd[7].revents & POLLERR)
    {
      printf("error on data psm\n");
      break;
    }

    if (pfd[0].revents & POLLIN)
    {
      sdp_fd = accept_connection(pfd[0].fd, NULL);
      if (sdp_fd < 0)
      {
        printf("error accepting sdp connection\n");
        break;
      }
    }
    if (pfd[1].revents & POLLIN)
    {
      ctrl_fd = accept_connection(pfd[1].fd, NULL);
      if (ctrl_fd < 0)
      {
        printf("error accepting ctrl connection\n");
        break;
      }
    }
    if (pfd[2].revents & POLLIN)
    {
      int_fd = accept_connection(pfd[2].fd, &host_bdaddr);
      if (int_fd < 0)
      {
        printf("error accepting int connection\n");
        break;
      }

      char straddr[18];
      ba2str(&host_bdaddr, straddr);
      printf("connected to %s\n", straddr);

      is_connected = 1;
      has_host = 1;
    }

    if (pfd[3].revents & POLLIN)
    {
      len = recv(sdp_fd, buf, 32, MSG_DONTWAIT);
      if (len > 0)
      {
        sdp_recv_data(buf, len);
      }
    }
    if (pfd[3].revents & POLLOUT)
    {
      len = sdp_get_data(buf);
      if (len > 0)
      {
        send(sdp_fd, buf, len, MSG_DONTWAIT);
      }
    }

    if (is_connected)
    {
      if (out_buf_len == 0 && (pfd[5].revents & POLLIN))
      {
        out_buf_len = recv(int_fd, out_buf, 32, MSG_DONTWAIT);
        if (enable_report_printing)
        {
          print_report(out_buf, out_buf_len);
        }
      }
      if (pfd[5].revents & POLLOUT)
      {
        if (in_buf_len > 0)
        {
          send(int_fd, in_buf, in_buf_len, MSG_DONTWAIT);
          in_buf_len = 0;
        }

        failure = 0;
      }
      // else
      // {
      //   if (++failure > 3)
      //   {
      //     printf("connection timed out, attemping to reconnect...\n");
      //     disconnect_from_host();
      //     is_connected = 0;
      //   }

      //   usleep(20*1000*1000);
      // }
    }

    if (in_buf_len == 0 && (pfd[7].revents & POLLIN))
    {
      in_buf_len = recv(wm_int_fd, in_buf, 32, MSG_DONTWAIT);
      if (enable_report_printing)
      {
        print_report(in_buf, in_buf_len);
      }
    }
    if (out_buf_len > 0 && (pfd[7].revents & POLLOUT))
    {
      send(wm_int_fd, out_buf, out_buf_len, MSG_DONTWAIT);
      out_buf_len = 0;
    }

    if (has_host && !is_connected)
    {
      if (connect_to_host() < 0)
      {
        usleep(500*1000);
      }
      else
      {
        printf("connected to host\n");
        is_connected = 1;
      }
    }
  }

  printf("cleaning up...\n");

  disconnect_from_host();
  disconnect_from_wiimote();

  close(sock_sdp_fd);
  close(sock_ctrl_fd);
  close(sock_int_fd);

  restore_device();

#ifndef SDP_SERVER
  unregister_wiimote_sdp_record();
#endif

  return 0;
}
