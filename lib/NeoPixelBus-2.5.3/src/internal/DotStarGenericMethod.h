/*-------------------------------------------------------------------------
NeoPixel library helper functions for DotStars using general Pins (APA102).

Written by Michael C. Miller.

I invest time and resources providing this open source code,
please support me by dontating (see https://github.com/Makuna/NeoPixelBus)

-------------------------------------------------------------------------
This file is part of the Makuna/NeoPixelBus library.

NeoPixelBus is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

NeoPixelBus is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with NeoPixel.  If not, see
<http://www.gnu.org/licenses/>.
-------------------------------------------------------------------------*/

#pragma once

// must also check for arm due to Teensy incorrectly having ARDUINO_ARCH_AVR set
#if defined(ARDUINO_ARCH_AVR) && !defined(__arm__)
#include "TwoWireBitBangImpleAvr.h"
#else
#include "TwoWireBitBangImple.h"
#endif


template<typename T_TWOWIRE> class DotStarMethodBase
{
public:
	DotStarMethodBase(uint8_t pinClock, uint8_t pinData, uint16_t pixelCount, size_t elementSize) :
        _sizePixels(pixelCount * elementSize),
		_sizeEndFrame((pixelCount + 15) / 16), // 16 = div 2 (bit for every two pixels) div 8 (bits to bytes)
		_wire(pinClock, pinData)
    {
        _pixels = (uint8_t*)malloc(_sizePixels);
        memset(_pixels, 0, _sizePixels);
    }

	DotStarMethodBase(uint16_t pixelCount, size_t elementSize) :
		DotStarMethodBase(SCK, MOSI, pixelCount, elementSize)
	{
	}

    ~DotStarMethodBase()
    {
        free(_pixels);
    }

    bool IsReadyToUpdate() const
    {
        return true; // dot stars don't have a required delay
    }

#if defined(ARDUINO_ARCH_ESP32)
	void Initialize(int8_t sck, int8_t miso, int8_t mosi, int8_t ss)
	{
		_wire.begin(sck, miso, mosi, ss);
	}
#endif

    void Initialize()
    {
		_wire.begin();
    }

    // void Update(bool)
    // {
	// 	const uint8_t startFrame[4] = { 0x00 };

	// 	_wire.beginTransaction();

    //     // start frame
	// 	_wire.transmitBytes(startFrame, sizeof(startFrame));
        
    //     // data
	// 	_wire.transmitBytes(_pixels, _sizePixels);
        
    //     // end frame 
	// 	// one bit for every two pixels with no less than 1 byte
	// 	for (size_t endFrameByte = 0; endFrameByte < _sizeEndFrame; endFrameByte++)
	// 	{
	// 		_wire.transmitByte(0xff);
	// 	}

	// 	_wire.endTransaction();
    // }

    //MY METHOD
    void Update(bool)
    {
	// start frame
        _wire.beginTransaction();
        
        uint8_t newarray[4][_sizePixels/4];
		memset(&newarray,0,sizeof(newarray));

		uint8_t org_index;
		for(uint8_t pixelst=0;pixelst<_sizePixels/4;pixelst++){
			for(uint8_t bytet=0;bytet<4;bytet++){ //skip first byte
				newarray[bytet][pixelst] = _pixels[org_index++];
			}
		}
        // Map 1D {RGBxRGBxRGBx} array into 2D 
        //memcpy(newarray,_pixels,sizeof(char)*_sizePixels);
		for(uint8_t pixelst=0;pixelst<_sizePixels/4;pixelst++){
			for(uint8_t bytet=1;bytet<4;bytet++){ //skip first byte
				_wire.transmitByte(newarray[4-bytet][pixelst]);
			}
		}
        // set clock and data back to low between updates
        //digitalWrite(_pinData, LOW);
    }

    uint8_t* getPixels() const
    {
        return _pixels;
    };

    size_t getPixelsSize() const
    {
        return _sizePixels;
    };

private:
	const size_t   _sizePixels;   // Size of '_pixels' buffer below
	const size_t   _sizeEndFrame;

    

	T_TWOWIRE _wire;
    uint8_t* _pixels;       // Holds LED color values
};

typedef DotStarMethodBase<TwoWireBitBangImple> DotStarMethod;

#if !defined(__AVR_ATtiny85__)
#include "TwoWireSpiImple.h"
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeed20Mhz>> DotStarSpi20MhzMethod;
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeed10Mhz>> DotStarSpi10MhzMethod;
typedef DotStarMethodBase<TwoWireSpiImple<SpiSpeed2Mhz>> DotStarSpi2MhzMethod;
typedef DotStarSpi10MhzMethod DotStarSpiMethod;
#endif



