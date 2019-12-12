#include <wiringPi.h>
#include <wiringPiSPI.h>

#define PI_CHANNEL 0		/* RPi has two channels */
#define SPI_CLK_HZ 32000000	/* SPI clock speed (Hz) */

/* GPIO pins in BCM numbering format, named by connected interface on
   e-paper module */
enum BCM_EPD_PINS
    { RST_PIN  = 17, DC_PIN   = 25,
      CS_PIN   =  8, BUSY_PIN = 24 };

int
spi_initialise();
{
    wiringPiSetupGpio();
    pinMode(RST_PIN, OUTPUT);
    pinMode(DC_PIN, OUTPUT);
    pinMode(CS_PIN, OUTPUT);
    pinMode(BUSY_PIN, INPUT);
    wiringPiSPISetup(PI_CHANNEL, SPI_CLK_HZ)
    return 0;
}

firmware_load()

int
main(int argc, int *argv[])
{
    get_gpio_interface();
    
    //spi_initialise();
    firmware_load();
    
    return 0;
}
