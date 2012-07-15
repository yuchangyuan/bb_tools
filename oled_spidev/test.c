
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>


FILE *f_rst = NULL;
FILE *f_dc = NULL;

static void pabort(const char *s)
{
    perror(s);
    abort();
}

static const char *device = "/dev/spidev2.0";
static uint8_t mode = 0;
static uint8_t bits = 32;

static uint32_t speed = 48 * 1000 * 1000;

int fd = -1;

int init_spi()
{
    int ret;

    fd = open(device, O_RDWR);
    if (fd < 0)
        pabort("can't open device");

    /*
     * spi mode
     */
    ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
    if (ret == -1)
        pabort("can't set spi mode");

    ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
    if (ret == -1)
        pabort("can't get spi mode");

    /*
     * bits per word
     */
    ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't set bits per word");

    ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1)
        pabort("can't get bits per word");

    /*
     * max speed hz
     */
    ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't set max speed hz");

    ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
    if (ret == -1)
        pabort("can't get max speed hz");

    printf("spi mode: %d\n", mode);
    printf("bits per word: %d\n", bits);
    printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);


    f_rst = fopen("/sys/class/gpio/gpio111/value", "w");
    f_dc  = fopen("/sys/class/gpio/gpio117/value", "w");

    return ret;
}

static int dc_value = 0;

#define NOP 0xE3

void cmd32(char a1, char a2, char a3, char a4)
{
    char buf[4];

    buf[3] = a1;
    buf[2] = a2;
    buf[1] = a3;
    buf[0] = a4;

   if (dc_value) {
        fprintf(f_dc, "0");
        fflush(f_dc);
        dc_value = 0;
    }


   write(fd, buf, 4);
}



void reset()
{
    printf("reset\n");
    fprintf(f_rst, "0");
    fflush(f_rst);

    usleep(10 * 1000);

    fprintf(f_rst, "1");
    fflush(f_rst);

    fprintf(f_dc, "0");
    fflush(f_dc);
    dc_value = 0;
}

//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Instruction Setting
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Set_Start_Column(unsigned char d)
{
    cmd32(0x00 + d % 16, 0x10 + d / 16, NOP, NOP);
    // Set Lower Column Start Address for Page Addressing Mode
    //   Default => 0x00
    // Set Higher Column Start Address for Page Addressing Mode
    //   Default => 0x10
}


void Set_Addressing_Mode(unsigned char d)
{
    cmd32(0x20, d, NOP, NOP);
    // Set Memory Addressing Mode
    //   Default => 0x02
    //     0x00 => Horizontal Addressing Mode
    //     0x01 => Vertical Addressing Mode
    //     0x02 => Page Addressing Mode
}


void Set_Column_Address(unsigned char a, unsigned char b)
{
    cmd32(0x21, a, b, NOP);
    // Set Column Address
    //   Default => 0x00 (Column Start Address)
    //   Default => 0x7F (Column End Address)
}


void Set_Page_Address(unsigned char a, unsigned char b)
{
    cmd32(0x22, a, b, NOP);
    // Set Page Address
    //   Default => 0x00 (Page Start Address)
    //   Default => 0x07 (Page End Address)
}


void Set_Start_Line(unsigned char d)
{
    cmd32(0x40|d, NOP, NOP, NOP);
    // Set Display Start Line
    //   Default => 0x40 (0x00)
}


void Set_Contrast_Control(unsigned char d)
{
    cmd32(0x81, d, NOP, NOP);
    // Set Contrast Control
    //   Default => 0x7F
}


void Set_Charge_Pump(unsigned char d)
{
    cmd32(0x8D, 0x10|d, NOP, NOP);
    // Set Charge Pump
    //   Default => 0x10
    //     0x10 (0x00) => Disable Charge Pump
    //     0x14 (0x04) => Enable Charge Pump
}


void Set_Segment_Remap(unsigned char d)
{
    cmd32(0xA0|d, NOP, NOP, NOP);
    // Set Segment Re-Map
    //   Default => 0xA0
    //     0xA0 (0x00) => Column Address 0 Mapped to SEG0
    //     0xA1 (0x01) => Column Address 0 Mapped to SEG127
}


void Set_Entire_Display(unsigned char d)
{
    cmd32(0xA4|d, NOP, NOP, NOP);
    // Set Entire Display On / Off
    //   Default => 0xA4
    //     0xA4 (0x00) => Normal Display
    //     0xA5 (0x01) => Entire Display On
}


void Set_Inverse_Display(unsigned char d)
{
    cmd32(0xA6|d, NOP, NOP, NOP);
    // Set Inverse Display On/Off
    //   Default => 0xA6
    //     0xA6 (0x00) => Normal Display
    //     0xA7 (0x01) => Inverse Display On
}


void Set_Multiplex_Ratio(unsigned char d)
{
    cmd32(0xA8, d, NOP, NOP);
    // Set Multiplex Ratio
    //   Default => 0x3F (1/64 Duty)
}


void Set_Display_On_Off(unsigned char d)
{
    cmd32(0xAE|d, NOP, NOP, NOP);
    // Set Display On/Off
    //   Default => 0xAE
    //     0xAE (0x00) => Display Off
    //     0xAF (0x01) => Display On
}


void Set_Start_Page(unsigned char d)
{
    cmd32(0xB0|d, NOP, NOP, NOP);
    // Set Page Start Address for Page Addressing Mode
    //   Default => 0xB0 (0x00)
}


void Set_Common_Remap(unsigned char d)
{
    cmd32(0xC0|d, NOP, NOP, NOP);
    // Set COM Output Scan Direction
    //   Default => 0xC0
    //     0xC0 (0x00) => Scan from COM0 to 63
    //     0xC8 (0x08) => Scan from COM63 to 0
}


void Set_Display_Offset(unsigned char d)
{
    cmd32(0xD3, d, NOP, NOP);
    // Set Display Offset
    //   Default => 0x00
}


void Set_Display_Clock(unsigned char d)
{
    cmd32(0xD5, d, NOP, NOP);
    // Set Display Clock Divide Ratio / Oscillator Frequency
    //   Default => 0x80
    //     D[3:0] => Display Clock Divider
    //     D[7:4] => Oscillator Frequency
}


void Set_Precharge_Period(unsigned char d)
{
    cmd32(0xD9, d, NOP, NOP);
    // Set Pre-Charge Period
    //   Default => 0x22 (2 Display Clocks [Phase 2] / 2 Display Clocks [Phase 1])
    //     D[3:0] => Phase 1 Period in 1~15 Display Clocks
    //     D[7:4] => Phase 2 Period in 1~15 Display Clocks
}


void Set_Common_Config(unsigned char d)
{
    cmd32(0xDA, 0x02|d, NOP, NOP);
    // Set COM Pins Hardware Configuration
    //   Default => 0x12 (0x10)
    //     Alternative COM Pin Configuration
    //     Disable COM Left/Right Re-Map
}


void Set_VCOMH(unsigned char d)
{
    cmd32(0xDB, d, NOP, NOP);
    // Set VCOMH Deselect Level
    //   Default => 0x20 (0.77*VCC)
}



//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Show Regular Pattern (Full Screen)
//-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
void Fill_RAM1(char *buf)
{
    int i;

    Set_Start_Page(0);
    Set_Start_Column(0x00);

    if (!dc_value) {
        fprintf(f_dc, "1");
        fflush(f_dc);
        dc_value = 1;
    }

    // NOTE: set large will block, ~700us
    for (i = 0; i < 8; ++i) {
        write(fd, buf, 128);
    }

}


void Fill_RAM2(char *buf)
{
    int ret;
    int i;

    struct spi_ioc_transfer tr_list[8];

    for (i = 0; i < 8; ++i) {
        tr_list[i].tx_buf = (unsigned long)(buf + i * 128);
        tr_list[i].rx_buf = 0;
        tr_list[i].len = 128;
        tr_list[i].delay_usecs = 0;
        tr_list[i].speed_hz = 0;
        tr_list[i].bits_per_word = 0;
    }


    Set_Start_Page(0);
    Set_Start_Column(0x00);


    if (!dc_value) {
        fprintf(f_dc, "1");
        fflush(f_dc);
        dc_value = 1;
    }

    // ~500us
    ret = ioctl(fd, SPI_IOC_MESSAGE(8), tr_list);
}

void Fill_RAM(char *buf)
{
    Fill_RAM2(buf);
}



void OLED_Init_I()                // VCC Generated by Internal DC/DC Circuit
{
    char buf[1024];

    reset();

    Set_Display_On_Off(0x00);        // Display Off (0x00/0x01)
    Set_Display_Clock(0xf1);        // Set Clock as 100 Frames/Sec
    Set_Multiplex_Ratio(0x3F);        // 1/64 Duty (0x0F~0x3F)
    Set_Display_Offset(0x00);        // Shift Mapping RAM Counter (0x00~0x3F)
    Set_Start_Line(0x00);            // Set Mapping RAM Display Start Line (0x00~0x3F)
    Set_Charge_Pump(0x04);            // Enable Embedded DC/DC Converter (0x00/0x04)
    Set_Addressing_Mode(0x00);        // Set Page Addressing Mode (0x00/0x01/0x02)
    Set_Segment_Remap(0x01);        // Set SEG/Column Mapping (0x00/0x01)
    Set_Common_Remap(0x08);            // Set COM/Row Scan Direction (0x00/0x08)
    Set_Common_Config(0x10);        // Set Sequential Configuration (0x00/0x10)
    Set_Contrast_Control(0x80);    // Set SEG Output Current
    Set_Precharge_Period(0xf1);        // Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
    Set_VCOMH(0x40);            // Set VCOM Deselect Level
    Set_Entire_Display(0x00);        // Disable Entire Display On (0x00/0x01)
    Set_Inverse_Display(0x00);        // Disable Inverse Display On (0x00/0x01)

    memset(buf, 0, 1024);
    Fill_RAM(buf);                // Clear Screen

    Set_Display_On_Off(0x01);        // Display On (0x00/0x01)
}


//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
//  Main Program
//=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=


int main()
{
    char buf[1024];

    init_spi();
    OLED_Init_I();

    memset(buf, 0, 1024);

    while (1) {
        int j;

        for (j = 0; j < 1024; ++j) buf[j] = rand() & 0xff;

        Fill_RAM(buf);

        usleep(10*1000);
    }

    return 0;
}
