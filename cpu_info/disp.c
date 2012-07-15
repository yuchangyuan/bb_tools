#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define WIDTH 128
#define HEIGHT 64

#define SZ (WIDTH * HEIGHT)

static int fd = -1;
static uint8_t *ptr = (uint8_t *)-1;
static int ref = 0;

uint8_t *open_disp(int *width, int * height)
{
    if ((int)ptr > 0) {
        ++ref;
        *width = WIDTH;
        *height = HEIGHT;
        return ptr;
    }

    fd = open("/dev/fb0", O_RDWR);
    if (fd < 0) { fprintf(stderr, "ERROR"); }


    ptr = (uint8_t *)mmap(0, SZ, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if ((int)ptr == -1) {
        printf("Fout: gefaald bij het mappen van de framebuffer device in het geheugen.\n");
            return NULL;
    }

    // printf("ptr = %p\n", ptr);

    *width = WIDTH;
    *height = HEIGHT;

    ref++;

    return ptr;
}


void close_disp()
{
    --ref;
    if (ref > 0) return;

    munmap(ptr, SZ);
    ptr = (uint8_t *)-1;
    close(fd);
}
