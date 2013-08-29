/*
 * Copyright (c) 2010 by Cristian Maglie <c.maglie@bug.st>
 * SPI Master library for arduino.
 *
 * This file is free software; you can redistribute it and/or modify
 * it under the terms of either the GNU General Public License version 2
 * or the GNU Lesser General Public License version 2.1, both as
 * published by the Free Software Foundation.
 */

#include "pins_arduino.h"
#include "SPI.h"

SPIClass SPI;

#if defined(__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
// see http://gammon.com.au/spi
#	define DI   0  // D0, pin 5  Data In
#	define DO   1  // D1, pin 6  Data Out (this is *not* MOSI)
#	define USCK 2  // D2, pin 7  Universal Serial Interface clock
#	define SS   3  // D3, pin 2  Slave Select
#elif defined(__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
// these depend on the core used (check pins_arduino.h)
// this is for jeelabs' one (based on google-code core)
#	define DI   4   // PA6
#	define DO   5   // PA5
#	define USCK 6   // PA4
#	define SS   3   // PA7
#endif

void SPIClass::begin() {

#if defined(SPCR)

  // Set SS to high so a connected chip will be "deselected" by default
  digitalWrite(SS, HIGH);

  // When the SS pin is set as OUTPUT, it can be used as
  // a general purpose output port (it doesn't influence
  // SPI operations).
  pinMode(SS, OUTPUT);

  // Warning: if the SS pin ever becomes a LOW INPUT then SPI
  // automatically switches to Slave, so the data direction of
  // the SS pin MUST be kept as OUTPUT.
  SPCR |= _BV(MSTR);
  SPCR |= _BV(SPE);

  // Set direction register for SCK and MOSI pin.
  // MISO pin automatically overrides to INPUT.
  // By doing this AFTER enabling SPI, we avoid accidentally
  // clocking in a single bit since the lines go directly
  // from "input" to SPI control.  
  // http://code.google.com/p/arduino/issues/detail?id=888
  pinMode(SCK, OUTPUT);
  pinMode(MOSI, OUTPUT);

#else

  digitalWrite(SS, HIGH);
  pinMode(USCK, OUTPUT);
  pinMode(DO, OUTPUT);
  pinMode(SS, OUTPUT);
  pinMode(DI, INPUT);
  USICR = _BV(USIWM0);

#endif
}

byte SPIClass::transfer(byte b) {
#if defined(SPCR)

  SPDR = b;
  while (!(SPSR & _BV(SPIF)))
    ;
  return SPDR;

#else

  USIDR = b;
  USISR = _BV(USIOIF);
  do
    USICR = _BV(USIWM0) | _BV(USICS1) | _BV(USICLK) | _BV(USITC);
  while ((USISR & _BV(USIOIF)) == 0);
  return USIDR;

#endif
}

void SPIClass::end() {
#if defined(SPCR)

  SPCR &= ~_BV(SPE);

#endif
}

void SPIClass::setBitOrder(uint8_t bitOrder)
{
#if defined(SPCR)

  if(bitOrder == LSBFIRST) {
    SPCR |= _BV(DORD);
  } else {
    SPCR &= ~(_BV(DORD));
  }

#endif
}

void SPIClass::setDataMode(uint8_t mode)
{
#if defined(SPCR)

  SPCR = (SPCR & ~SPI_MODE_MASK) | mode;

#endif
}

void SPIClass::setClockDivider(uint8_t rate)
{
#if defined(SPCR)

  SPCR = (SPCR & ~SPI_CLOCK_MASK) | (rate & SPI_CLOCK_MASK);
  SPSR = (SPSR & ~SPI_2XCLOCK_MASK) | ((rate >> 2) & SPI_2XCLOCK_MASK);

#endif
}

