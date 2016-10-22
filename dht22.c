#include "wiringPi.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/resource.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#define MAX_TIMINGS	  86

int DHT_PIN = 0;

int read_dht_data(float *h, float *t)
{
  uint8_t j = 0, i;
  int data[5] = { 0, 0, 0, 0, 0 };

  pinMode(DHT_PIN, OUTPUT);
  digitalWrite(DHT_PIN, LOW);
  delay(18);
  digitalWrite(DHT_PIN, HIGH);
  delayMicroseconds(10);

  pinMode(DHT_PIN, INPUT);

  uint8_t laststate	= HIGH;
  unsigned int lasttime = micros();

  for(i = 0; i < MAX_TIMINGS; i++) {
    int state;
    unsigned int current;

    while (true) {
      state = digitalRead(DHT_PIN);
      current = micros();
      if (current - lasttime > 200) {
        goto finish;
      }
      if (laststate != state) {
        break;
      }
    }

    /* ignore first 3 transitions */
    if ((i >= 4) && (i % 2 == 0)) {
      /* shove each bit into the storage bytes */
      data[j / 8] <<= 1;
      if ((current - lasttime) > 60)
      data[j / 8] |= 1;
      j++;
    }

    laststate = state;
    lasttime = current;
  }

  finish:

  if (j < 40) {
    fprintf(stderr, "Not enough bits: %d\n", j);
    return -1;
  }

  if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
    fprintf(stderr, "Invalid CRC\n");
    return -2;
  }

  if (h) {
    *h = (float)((data[0] << 8) + data[1]) / 10;
    if (*h > 100) {
      *h = data[0];	// for DHT11
    }
  }

  if (t) {
    *t = (float)(((data[2] & 0x7F) << 8) + data[3]) / 10;
    if (*t > 125) {
      *t = data[2];	// for DHT11
    }
    if (data[2] & 0x80)	{
      *t = -*t;
    }
  }
  return 0;
}

void setprio() {
  id_t pid = getpid();
  int ret = setpriority(PRIO_PROCESS, pid, 10000);
  if (ret < 0) {
    fprintf(stderr, "Failed to set prio: %d\n", errno);
  }
}

void usage(const char *cmd) {
  fprintf(stderr, "Usage: %s [-d <delaymsec>] [-r <retry>] [-p <pin>] [-s all|hum|temp]\n", cmd);
  exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
  int delaymsec = 0;
  int retry = 2;
  int whattoshow = 0;

  while (true) {
    switch (getopt(argc, argv, "d:r:p:s:")) {
      case -1:
      goto done;

      case 'd':
      delaymsec = atoi(optarg);
      break;

      case 'r':
      retry = atoi(optarg);
      break;

      case 'p':
      DHT_PIN = atoi(optarg);
      break;

      case 's':
      if (!strcmp(optarg, "all")) {
        whattoshow = 0;
      } else if (!strcmp(optarg, "hum")) {
        whattoshow = 1;
      } else if (!strcmp(optarg, "temp")) {
        whattoshow = 2;
      } else {
        usage(argv[0]);
      }
      break;

      default: /* '?' */
      usage(argv[0]);
      break;
    }
  }

  done:

  if (DHT_PIN <= 0) {
    fprintf(stderr, "Missing DHT PIN. Go to: http://joey.hazlett.us/pine64/pine64_pins.html\n");
    usage(argv[0]);
  }

  if (wiringPiSetup()) {
    fprintf(stderr, "Failed to configure GPIO\n");
    usage(argv[0]);
  }

  setprio();

  while (1) {
    float h = 0, c = 0;

    for(int i = 0; i <= retry; ++i) {
      if(read_dht_data(&h, &c) == 0) {
        switch(whattoshow) {
          case 0:
          printf("Humidity: %.1f %%\n", h);
          printf("Temperature: %.1f *C\n", c);
          break;

          case 1:
          printf("%.1f\n", h);
          break;

          case 2:
          printf("%.1f\n", c);
          break;
        }


        break;
      }
    }

    if(delaymsec > 0) {
      delay(delaymsec);
    } else {
      break;
    }
  }

  return(0);
}
