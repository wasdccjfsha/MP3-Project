#ifndef LABSPI_H
#define LABSPI_H
#include "LPC17xx.h"




class LabSPI
{
private:

public:
    enum FrameModes
    {
	SPI,
	Micro
    };

    enum Peripheral
    {
        SSP0 = 0,
        SSP1 = 1
    };

    bool init(Peripheral peripheral);

    uint8_t transfer(uint8_t send);

    LabSPI();
    ~LabSPI();
};

#endif
