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
#include "wiimote.h"
#include "input.h"
#include "input_sdl.h"
#include "input_socket.h"
#include "adapter.h"
#include "wm_print.h"

#define PSM_SDP 1
#define PSM_CTRL 0x11
#define PSM_INT 0x13

bdaddr_t host_bdaddr;
int has_host = 0;

int sdp_fd, ctrl_fd, int_fd;
int sock_sdp_fd, sock_ctrl_fd, sock_int_fd;

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

int l2cap_connect(bdaddr_t bdaddr, int psm)
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
  addr.l2_bdaddr = bdaddr;

  if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
  {
    close(fd);
    return -1;
  }

  return fd;
}

int l2cap_listen(int psm)
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
  addr.l2_bdaddr = *BDADDR_ANY;

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
  sock_sdp_fd = l2cap_listen(PSM_SDP);
  if (sock_sdp_fd < 0)
  {
    printf("can't listen on psm %d: %s\n", PSM_SDP, strerror(errno));
    return -1;
  }
#endif

  sock_ctrl_fd = l2cap_listen(PSM_CTRL);
  if (sock_ctrl_fd < 0)
  {
    printf("can't listen on psm %d: %s\n", PSM_CTRL, strerror(errno));
    return -1;
  }

  sock_int_fd = l2cap_listen(PSM_INT);
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
  ctrl_fd = l2cap_connect(host_bdaddr, PSM_CTRL);
  if (ctrl_fd < 0)
  {
    printf("can't connect to host psm %d: %s\n", PSM_CTRL, strerror(errno));
    return -1;
  }

  int_fd = l2cap_connect(host_bdaddr, PSM_INT);
  if (int_fd < 0)
  {
    printf("can't connect to host psm %d: %s\n", PSM_INT, strerror(errno));
    return -1;
  }

  return 0;
}

void disconnect()
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

void print_usage(char *argv0)
{
  printf("usage: %s [ <wii-bdaddr> [ gui | unix <path> | ip <port> ] ]\n", argv0);
}

int main(int argc, char *argv[])
{
  struct input_source input_source;

  struct pollfd pfd[6];
  unsigned char buf[256];
  ssize_t len;

  struct wiimote_state state;

  int send_report_now = 1;
  int input_result;
  int failure = 0;

  if (argc > 1)
  {
    if (strcmp(argv[1], "pair") == 0)
    {
      // Act as if nothing given.
    }
    else if (bachk(argv[1]) >= 0)
    {
      str2ba(argv[1], &host_bdaddr);
      has_host = 1;
    }
    else
    {
        print_usage(*argv);
        return 1;
    }
  }
  if (argc <= 2 || strcmp(argv[2], "gui") == 0)
  {
    input_sdl_init();
    input_source = input_source_sdl;
  }
  else if (argc > 3 && strcmp(argv[2], "unix") == 0)
  {
    input_socket_init_unix_at_path(argv[3]);
    input_source = input_source_socket;
  }
  else if (argc > 3 && strcmp(argv[2], "ip") == 0)
  {
    input_socket_init_ip_on_port(argv[3]);
    input_source = input_source_socket;
  }
  else
  {
    print_usage(*argv);
    return 1;
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

#ifndef SDP_SERVER
  if (register_wiimote_sdp_record() < 0)
  {
    printf("failed to add Wiimote SDP record\n");
    restore_device();
    return 1;
  }
#endif

  wiimote_init(&state);

  if (has_host)
  {
    printf("connecting to host...\n");
    if (connect_to_host() < 0)
    {
      printf("couldn't connect\n");
      running = 0;
    }
    else
    {
      char straddr[18];
      ba2str(&host_bdaddr, straddr);
      printf("connected to %s\n", straddr);

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
      printf("listening for connections... (press wii's sync button)\n");
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

      pfd[5].events |= POLLOUT;
    }

    if (poll(pfd, 6, 20) < 0)
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

    if (pfd[5].revents & POLLIN)
    {
      len = recv(int_fd, buf, 32, MSG_DONTWAIT);
      if (len > 0)
      {
        print_report(buf, len);
        process_report(&state, buf, len);
      }
    }

    input_result = input_update(&state, &input_source);
    if (input_result)
    {
      running = 0;
      if (input_result == -2)
      {
        power_off_host(&host_bdaddr);
      }
      else
      {
        disconnect(&host_bdaddr);
      }
    }

    if (is_connected && send_report_now)
    {
      if (pfd[5].revents & POLLOUT)
      {
        len = generate_report(&state, buf);
        if (len > 0)
        {
          print_report(buf, len);
          send(int_fd, buf, len, MSG_DONTWAIT);
        }

        failure = 0;
      }
      else
      {
        if (++failure > 5)
        {
          printf("connection timed out, attemping to reconnect...\n");
          disconnect();
          is_connected = 0;
        }
      }
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

  disconnect();

  close(sock_sdp_fd);
  close(sock_ctrl_fd);
  close(sock_int_fd);

  restore_device();

#ifndef SDP_SERVER
  unregister_wiimote_sdp_record();
#endif

  wiimote_destroy(&state);
  input_source.unload();

  return 0;
}

