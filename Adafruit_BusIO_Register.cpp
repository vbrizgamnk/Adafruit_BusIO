#include <Adafruit_BusIO_Register.h>

Adafruit_BusIO_Register::Adafruit_BusIO_Register(Adafruit_I2CDevice *i2cdevice, uint16_t reg_addr, 
						 uint8_t width, uint8_t bitorder, uint8_t address_width) {
  _i2cdevice = i2cdevice;
  _spidevice = NULL;
  _addrwidth = address_width;
  _address = reg_addr;
  _bitorder = bitorder;
  _width = width;
}

Adafruit_BusIO_Register::Adafruit_BusIO_Register(Adafruit_SPIDevice *spidevice, uint16_t reg_addr, 
						 Adafruit_BusIO_SPIRegType type,
						 uint8_t width, uint8_t bitorder, uint8_t address_width) {
  _spidevice = spidevice;
  _spiregtype = type;
  _i2cdevice = NULL;
  _addrwidth = address_width;
  _address = reg_addr;
  _bitorder = bitorder;
  _width = width;
}

Adafruit_BusIO_Register::Adafruit_BusIO_Register(Adafruit_I2CDevice *i2cdevice, Adafruit_SPIDevice *spidevice, 
						 uint16_t reg_addr, Adafruit_BusIO_SPIRegType type,
						 uint8_t width, uint8_t bitorder, uint8_t address_width) {
  _spidevice = spidevice;
  _i2cdevice = i2cdevice;
  _spiregtype = type;
  _addrwidth = address_width;
  _address = reg_addr;
  _bitorder = bitorder;
  _width = width;
}


bool Adafruit_BusIO_Register::write(uint8_t *buffer, uint8_t len) {

  uint8_t addrbuffer[2] = {(uint8_t)(_address & 0xFF), (uint8_t)(_address>>8)};

  if (_i2cdevice) {
    return _i2cdevice->write(buffer, len, true, addrbuffer, _addrwidth);
  }
  if (_spidevice) {
    if (_spiregtype == ADDRBIT8_HIGH_TOREAD) {
      addrbuffer[0] &= ~0x80;
    }
    return _spidevice->write( buffer, len, addrbuffer, _addrwidth);
  }
  return false;
}

bool Adafruit_BusIO_Register::write(uint32_t value, uint8_t numbytes) {
  if (numbytes == 0) {
    numbytes = _width;
  }
  if (numbytes > 4) {
    return false;
  }

  for (int i=0; i<numbytes; i++) {
    if (_bitorder == LSBFIRST) {
      _buffer[i] = value & 0xFF;
    } else {
      _buffer[numbytes-i-1] = value & 0xFF;
    }
    value >>= 8;
  }
  return write(_buffer, numbytes);
}

// This does not do any error checking! returns 0xFFFFFFFF on failure
uint32_t Adafruit_BusIO_Register::read(void) {
  if (! read(_buffer, _width)) {
    return -1;
  }

  uint32_t value = 0;

   for (int i=0; i < _width; i++) {
     value <<= 8;
     if (_bitorder == LSBFIRST) {
       value |= _buffer[_width-i-1];
     } else {
       value |= _buffer[i];
     }
   }

   return value;
}


bool Adafruit_BusIO_Register::read(uint8_t *buffer, uint8_t len) {
  uint8_t addrbuffer[2] = {(uint8_t)(_address & 0xFF), (uint8_t)(_address>>8)};

  if (_i2cdevice) {
    return _i2cdevice->write_then_read(addrbuffer, _addrwidth, buffer, len);
  }
  if (_spidevice) {
    if (_spiregtype == ADDRBIT8_HIGH_TOREAD) {
      addrbuffer[0] |= 0x80;
    }
    return _spidevice->write_then_read(addrbuffer, _addrwidth, buffer, len);
  }
  return false;
}

bool Adafruit_BusIO_Register::read(uint16_t *value) {
  if (! read(_buffer, 2)) {
    return false;
  }

  if (_bitorder == LSBFIRST) {
    *value = _buffer[1];
    *value <<= 8;
    *value |= _buffer[0];
  } else {
    *value = _buffer[0];
    *value <<= 8;
    *value |= _buffer[1];
  }
  return true;
}

bool Adafruit_BusIO_Register::read(uint8_t *value) {
  if (! read(_buffer, 1)) {
    return false;
  }

  *value = _buffer[0];
  return true;
}

void Adafruit_BusIO_Register::print(Stream *s) {
  uint32_t val = read();
  s->print("0x"); s->print(val, HEX);
}

void Adafruit_BusIO_Register::println(Stream *s) {
  print(s);
  s->println();
}


Adafruit_BusIO_RegisterBits::Adafruit_BusIO_RegisterBits(Adafruit_BusIO_Register *reg, uint8_t bits, uint8_t shift) {
  _register = reg;
  _bits = bits;
  _shift = shift;
}

uint32_t Adafruit_BusIO_RegisterBits::read(void) {
  uint32_t val = _register->read();
  val >>= _shift;
  return val & ((1 << (_bits+1)) - 1);
}

void Adafruit_BusIO_RegisterBits::write(uint32_t data) {
  uint32_t val = _register->read();

  // mask off the data before writing
  uint32_t mask = (1 << (_bits+1)) - 1;
  data &= mask;

  mask <<= _shift;
  val &= ~mask;      // remove the current data at that spot
  val |= data << _shift; // and add in the new data
  
  _register->write(val, _register->width());
}

