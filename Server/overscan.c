#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define	GET_OVERSCAN		0x0004000a
#define	SET_OVERSCAN		0x0004800a

static int file_desc;

/* 
 * send property message to mailbox using ioctl
 */
static int mailbox_property(int file_desc, void *buf)
{
   int return_value = ioctl(file_desc, _IOWR(100, 0, char *), buf);

   /* ioctl error of some kind */ 
   if (return_value < 0) {
      close(file_desc);
   }

   return return_value;
}

int overscan_get() {
   unsigned int property[32];
   memset(property, 0, sizeof(property));
   property[0] = 10 * sizeof(int);
   property[2] = GET_OVERSCAN; 
   property[3] = 0x00000010; 
   mailbox_property(file_desc, property);
   return property[5];
}

void overscan_set(int value)
{
   unsigned int property[32];
   memset(property, 0, sizeof(property));
   property[0] = 10 * sizeof(int);
   property[2] = SET_OVERSCAN;
   property[3] = 0x00000010;
   property[4] = 0x00000010;
   property[5] = value;
   property[6] = value;
   property[7] = value;
   property[8] = value;
   mailbox_property(file_desc, property);
}

int
overscan_init() {
   file_desc = open("/dev/vcio", 0);
   if (file_desc == -1) return -1;
   return 0;
}

