/*! @file UART.cpp
 *
 *  @brief I/O routines for UART communications on the TWR-K70F120M.
 *
 *  This contains the functions for operating the UART (serial port).
 *
 *  @author Chao Li
 *  @date 07/05/2019
 *  Copyright (c) Chao Li. All rights reserved.
 */
#ifndef IDMT_H_
#define IDMT_H_

#include "type_cpp.h"

#include "Flash.h"

#include "FixPoint32.h"

#include <utility>

namespace IDMT
{

typedef struct
{
  const float A = 0.02;
const uint8_t AVI = 1;
const uint8_t AEI = 2;
const float KI = 0.14;
const float KVI = 13.5;
const uint8_t KEI = 80;
}characteristic;



class IDMT_t
{
  private:
    static uint8_t* setting;
    static uint16union_t* nBTrip;


  public:
    IDMT_t();
    
    void Set(uint8_t slope);

    void GetSetting(uint8_t &slope);

    int16_t&& GetCurrent(int32_t &magV);

    bool GetNbTrip(uint16_t &NbTrip);

    //the return value is in 32Q16 with uni: sec
    uint32_t&& GetTripTime( uint16_t &current);
};



}




#endif
