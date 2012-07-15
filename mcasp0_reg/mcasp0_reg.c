#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdlib.h>

#define MCASP0_CFG_ADDR 0x48038000L

struct reg_def {
    uint32_t off;
    char *name;
};

struct reg_def reg_list[] = {
    { 0x10, "PFUNC" },
    { 0x14, "PDIR" },
    { 0x18, "PDOUT" },
    { 0x1C, "PDIN" },
    { 0x1C, "PDSET" },
    { 0x20, "PDCLR" },
    { 0x44, "GBLCTL" },
    { 0x48, "AMUTE" },
    { 0x4C, "DLBCTL" },
    { 0x50, "DITCTL" },
    { 0x60, "RGBLCTL" },
    { 0x64, "RMASK" },
    { 0x68, "RFMT" },
    { 0x6C, "AFSRCTL" },
    { 0x70, "ACLKRCTL" },
    { 0x74, "AHCLKRCTL" },
    { 0x78, "RTDM" },
    { 0x7C, "RINTCTL" },
    { 0x80, "RSTAT" },
    { 0x84, "RSLOT" },
    { 0x88, "RCLKCHK" },
    { 0x8C, "REVTCTL" },
    { 0xA0, "XGBLCTL" },
    { 0xA4, "XMASK" },
    { 0xA8, "XFMT" },
    { 0xAC, "AFSXCTL" },
    { 0xB0, "ACLKXCTL" },
    { 0xB4, "AHCLKXCTL" },
    { 0xB8, "XTDM" },
    { 0xBC, "XINTCTL" },
    { 0xC0, "XSTAT" },
    { 0xC4, "XSLOT" },
    { 0xC8, "XCLKCHK" },
    { 0xCC, "XEVTCTL" },
    { 0x180, "SRCTL0" },
    { 0x184, "SRCTL1" },
    { 0x188, "SRCTL2" },
    { 0x18C, "SRCTL3" },
};


int main(int argc, char **argv)
{
    int32_t *ptr;
    int fd = -1;
    int i;

    if ((fd = open("/dev/mem", O_RDWR)) < 0) {
        printf("Error opening /dev/mem\n");
        exit(1);
    }

    ptr = (int32_t *)mmap(0, getpagesize(),
        PROT_READ | PROT_WRITE,
        MAP_FILE | MAP_SHARED,
        fd, MCASP0_CFG_ADDR);

    if (ptr < 0) {
        perror("mmap failed: ");
        exit(1);
    }

    // NOTE: bus should work before read
    for (i = 0; i < sizeof(reg_list) / sizeof(struct reg_def); ++i) {
    	int j = 0;

        int off = reg_list[i].off;
        printf("%03x: %08x - %s\n", off, ptr[off / 4], reg_list[i].name);
        fflush(stdout);

	for (j = 1; j + 1 < argc; j += 2) {
		if (!strcmp(reg_list[i].name, argv[j])) {
			int val;

			if (sscanf(argv[j+1], "%x", &val) == 1) {
				ptr[off / 4] = val;
				printf("\t %08x - modified\n", val);
			}
			else {
				printf("\t parse faile: %s\n", argv[j+1]);
			}
		}
	}
    }

    munmap(ptr, getpagesize());

    return 0;
}
