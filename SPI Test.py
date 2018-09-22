import spidev
spi = spidev.SpiDev()
spi.open(bus, device)

def spiBegin():
    spi.open(1,0)

def spiReadReg(int reg):
    writebytes(reg & 0x7F)
    byte = val 
