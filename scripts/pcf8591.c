#include <bcm2835.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
    char buffer[1];  // Mudança: Declaração do buffer como 'char'
    unsigned char i;

    if (!bcm2835_init()) return 1;

    bcm2835_i2c_begin();
    bcm2835_i2c_setSlaveAddress(0x48);  // Endereço do PCF8591
    bcm2835_i2c_set_baudrate(10000);
    printf("start..........\n");

    while(1)
    {
        for (i = 0; i < 2; i++)
        {
            buffer[0] = i;
            bcm2835_i2c_write_read_rs(buffer, 1, buffer, 1);  // Corrigido: buffer como 'char'
            printf("AIN%d: %5.2f  ", i, (double)buffer[0] * 3.3 / 255);
        }
        printf("\n");
        bcm2835_delay(100);
    }

    bcm2835_i2c_end();
    bcm2835_close();
    return 0;
}
