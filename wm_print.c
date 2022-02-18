#include "wm_print.h"
#include "wm_reports.h" //for now

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

int show_reports = 0;
int reports_truncated = 0;

uint64_t next_report_ts = 0;
uint64_t report_timeout_us = 500000;

int verbose_reports = 0;

void print_report(const uint8_t * buf, int len)
{
  struct timeval tv;
  int i;
  struct report_data * data = (struct report_data *)buf;
  uint64_t ts;

  if (len == 0) return;

  gettimeofday(&tv, NULL);
  ts = tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;

  if (buf[0] == 0xa2) //report from wii
  {
    printf("\e[2;37m%ld.%06ld     \e[1;31mWii:\e[0m ", tv.tv_sec, tv.tv_usec);
    printf("\e[33m%02x\e[0m ", buf[1]);

    switch (buf[1])
    {
      case 0x10:
      {
        struct report_rumble * rpt = (struct report_rumble *)data->buf;

        printf("(set rumble %u)", rpt->rumble & 1);
        break;
      }
      case 0x12: //data reporting mode
      {
        struct report_mode * rpt = (struct report_mode *)data->buf;

        printf("(set reporting mode %02x, cont: %u)", rpt->mode, rpt->continuous & 1);

        break;
      }
      case 0x11: //player LEDs
      {
        struct report_leds * rpt = (struct report_leds *)data->buf;

        printf("(set player leds %u %u %u %u)", rpt->led_1 & 1, rpt->led_2 & 1,
          rpt->led_3 & 1, rpt->led_4 & 1);
        break;
      }
      case 0x13:
      case 0x1a: //ir camera enable
      {
        struct report_ir_enable * rpt = (struct report_ir_enable *)data->buf;

        printf("%02x %02x ", buf[2], buf[3]);
        printf("(set ir cam enable %u)", rpt->enabled & 1);

        break;
      }
      case 0x14:
      case 0x19: //speaker enable
      {
        struct report_speaker_enable * rpt = (struct report_speaker_enable *)data->buf;

        printf("(set speaker enable %u)", !rpt->muted & 1);

        break;
      }
      case 0x15: //status information request
        printf("(status request)");

        break;
      case 0x16: //write memory
      {
        struct report_mem_write * rpt = (struct report_mem_write *)data->buf;

        printf("\x1B[35m%02x \x1B[32m%02x %02x %02x \x1B[36m%02x\033[0m ", buf[2], buf[3], buf[4], buf[5], buf[6]);
        for (i = 7; i < len; i++) printf("%02x ", buf[i]);

        if (rpt->source0 || rpt->source1)
        {
          switch (rpt->offset & 0xfe)
          {
            case 0xa2: printf("(write speaker register a2)"); break;
            case 0xa4: printf("(write ext register a4)"); break;
            case 0xa6: printf("(write wmp register a6)"); break;
            case 0xb0: printf("(write ir register b0)"); break;
          }
        }
        else
        {
          printf("(write eeprom)");
        }
        break;
      }
      case 0x17: //read memory
      {
        struct report_mem_read * rpt = (struct report_mem_read *)data->buf;

        printf("\x1B[35m%02x \x1B[32m%02x %02x %02x \x1B[36m%02x %02x\033[0m ", buf[2], buf[3], buf[4], buf[5], buf[6], buf[7]);

        if (rpt->source0 || rpt->source1)
        {
          switch (rpt->offset & 0xfe)
          {
            case 0xa2: printf("(read speaker register a2)"); break;
            case 0xa4: printf("(read ext register a4)"); break;
            case 0xa6: printf("(read wmp register a6)"); break;
            case 0xb0: printf("(read ir register b0)"); break;
          }
        }
        else
        {
          printf("(read eeprom)");
        }

        break;
      }
      case 0x18: //speaker data (not used)
        printf("(speaker data)");
        break;
      default: //??
        printf("(unknown report)");
        break;
    }

    printf("\e[0m\n");
  }
  else
  {
    if (buf[1] < 0x30 || show_reports)
    {
      if (ts >= next_report_ts)
      {
        printf("\e[2;37m%ld.%06ld \e[1;34mWiimote:\e[0m ", tv.tv_sec, tv.tv_usec);
        printf("\e[33m%02x\e[0m \e[0;34m%02x %02x\e[0m ", buf[1], buf[2], buf[3]);

        switch(buf[1])
        {
          case 0x22:
            printf("\e[0;35m%02x \e[0;31m%02x\e[0m ", buf[4], buf[5]);
            printf("(ack report: %02x, res: %02x)", buf[4], buf[5]);
            break;
          case 0x21:
            printf("\e[0;31m%02x \x1B[32m%02x %02x\e[0m ", buf[4], buf[5], buf[6]);
            for (i = 7; i < len; i++)
            {
              printf("%02x ", buf[i]);
            }
            printf("(memory output)");
            break;
          case 0x20:
            printf("\e[0;35m%02x\e[0m ", buf[4]);
            for (i = 5; i < len; i++)
            {
              printf("%02x ", buf[i]);
            }
            printf("(status report)");
            break;
          default:
            for (i = 4; i < len; i++)
            {
              printf("%02x ", buf[i]);
            }
        }

        printf("\e[0m\n");

        if (verbose_reports)
        {
          struct report_accelerometer * report_accel = (struct report_accelerometer *)(buf + 2);
          printf("  accel %02x %02x, %02x %02x, %02x %02x\n",
            report_accel->x, report_accel->buttons.accel_0 & 0x3,
            report_accel->y, (report_accel->buttons.accel_1 & 0x1) << 1,
            report_accel->z, (report_accel->buttons.accel_1 & 0x2));

          if (buf[1] == 0x33)
          {
            struct report_ir_ext * report_ir = (struct report_ir_ext *)(buf + 2 + 5);
            for (int i = 0; i < 4; i++)
            {
              printf("  object %d: %d, %d [%d]\n", i,
                (((unsigned int)report_ir->obj[i].x_hi & 0x3) << 8 | report_ir->obj[i].x_lo),
                (((unsigned int)report_ir->obj[i].y_hi & 0x3) << 8 | report_ir->obj[i].y_lo),
                report_ir->obj[i].size);
            }
          }

          if (buf[1] == 0x37)
          {
            struct report_ir_basic * report_ir = (struct report_ir_basic *)(buf + 2 + 5);
            printf("  object %d: %d, %d\n", 0,
              (((unsigned int)report_ir->x1_hi & 0x3) << 8 | report_ir->x1_lo),
              (((unsigned int)report_ir->y1_hi & 0x3) << 8 | report_ir->y1_lo));
            printf("  object %d: %d, %d\n", 1,
              (((unsigned int)report_ir->x2_hi & 0x3) << 8 | report_ir->x2_lo),
              (((unsigned int)report_ir->y2_hi & 0x3) << 8 | report_ir->y2_lo));
            printf("  object %d: %d, %d\n", 2,
              (((unsigned int)report_ir->x3_hi & 0x3) << 8 | report_ir->x3_lo),
              (((unsigned int)report_ir->y3_hi & 0x3) << 8 | report_ir->y3_lo));
            printf("  object %d: %d, %d\n", 3,
              (((unsigned int)report_ir->x4_hi & 0x3) << 8 | report_ir->x4_lo),
              (((unsigned int)report_ir->y4_hi & 0x3) << 8 | report_ir->y4_lo));
          }
        }

        reports_truncated = 0;
        next_report_ts = ts + report_timeout_us;
      }
    }
    else
    {
      if (!reports_truncated)
      {
        printf("                           \x1B[2;37mReporting 0x%x ", buf[1]);
        switch (buf[1])
        {
          case 0x30: // core buttons
            printf("(core buttons)");
            break;
          case 0x31: // core buttons + accelerometer
            printf("(core buttons + accelerometer)");
            break;
          case 0x32: // core buttons + 8 extension bytes
            printf("(core buttons + 8 extension bytes)");
            break;
          case 0x33: // core buttons + accelerometer + 12 ir bytes
            printf("(core buttons + accelerometer + 12 ir bytes)");
            break;
          case 0x34: // core buttons + accelerometer + 19 extension bytes
            printf("(core buttons + accelerometer + 19 extension bytes)");
            break;
          case 0x35: // core buttons + accelerometer + 16 extension bytes
            printf("(core buttons + accelerometer + 16 extension bytes)");
            break;
          case 0x36: // core buttons + 10 ir bytes + 9 extension bytes
            printf("(core buttons + 10 ir bytes + 9 extension bytes)");
            break;
          case 0x37: // core buttons + accelerometer + 10 ir bytes + 6 extension bytes
            printf("(core buttons + accelerometer + 10 ir bytes + 6 extension bytes)");
            break;
          case 0x3d: // 21 extension bytes
            printf("(21 extension bytes)");
            break;
          case 0x3e: // interleaved core buttons + accelerometer with 36 ir bytes pt I
            printf("(interleaved core buttons + accelerometer with 36 ir bytes)");
            break;
          case 0x3f: // interleaved core buttons + accelerometer with 36 ir bytes pt II
            printf("(interleaved core buttons + accelerometer with 36 ir bytes)");
            break;
        }
        printf("\e[0m\n");
        reports_truncated = 1;
      }
    }
  }
}
