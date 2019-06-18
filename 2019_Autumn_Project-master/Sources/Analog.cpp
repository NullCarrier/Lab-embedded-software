/*
 * Analog.cpp
 *
 *  Created on: 14 Jun 2019
 *      Author: paul
 */

#include "Analog.h"

namespace Analog
{

  std::vector<int32_t> Analog_t::inputSinusoid;


  Analog_t::Analog_t(const uint32_t clock)
  {
    Analog_Init(clock);
  }


  bool Analog_t::GetSample()
  {
     return Analog_Get(0, &adcReading); // get a value from ADC
  }


  void Analog_t::GetVoltage()
  {
    int64_t read;
    int32_t tempData;

    if (this->GetSample() )
    {
      read = adcReading * 65536; //Convert into 32Q16

      tempData = (read * resolutionAD) >> 16; //Convert adcReading into actual voltage
    }

    //Put one sample into vector
    if (inputSinusoid.size() < SIZE)
      inputSinusoid.push_back(tempData);

  }


  bool Analog_t::PutSample(const int16_t value)
  {
    int16_t inputValue = value;

    //Convert into discrete level
    inputValue *= resolutionDA;

    return Analog_Put(0, inputValue);
  }


  uint16_t&& Analog_t::GetFrequency()
  {

  }





}
