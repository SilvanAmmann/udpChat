import time
import smbus
i2c_bus = smbus.SMBus(1) # Get I2C bus
i2c_address = 0x44       # SHT31 address
def temp():
    # start measurement command: 0x2c06
    i2c_bus.write_i2c_block_data(i2c_address, 0x2C, [0x06])
    time.sleep(0.5)  # wait 500 ms

    # Read data back from 0x00(00), 6 bytes
    # Temp MSB, Temp LSB, Temp CRC, Humididty MSB, Humidity LSB, Humidity CRC
    data = i2c_bus.read_i2c_block_data(i2c_address, 0x00, 6)

    # Convert the data
    temp = data[0] * 256 + data[1]
    cTemp = -45 + (175 * temp / 65535.0)
    fTemp = -49 + (315 * temp / 65535.0)
    humidity = 100 * (data[3] * 256 + data[4]) / 65535.0

    # Output data to console
    print("Temperature in Celsius: %.2f C" %cTemp)
    print("Temperature in Fahrenheit: %.2f F" %fTemp)
    print("Relative Humidity: %.2f %%RH" %humidity)
    return cTemp