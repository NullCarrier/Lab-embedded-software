/* ###################################################################
**     Filename    : main.cpp
**     Project     : Lab1
**     Processor   : MK70FN1M0VMJ12
**     Version     : Driver 01.01
**     Compiler    : GNU C Compiler
**     Date/Time   : 2015-07-20, 13:27, # CodeGen: 0
**     Abstract    :
**         Main module.
**         This module contains user's application code.
**     Settings    :
**     Contents    :
**         No public methods
**
** ###################################################################*/
/*!
** @file main.cpp
** @version 2.0
** @brief
**         Main module.
**         This module contains user's application code.
**  Copyright (c) UTS. All rights reserved.
*/
/*!
**  @addtogroup main_module main module documentation
**  @{
*/
// CPU module - contains low level hardware initialization routines
#include "Cpu.h"

// Simple OS
#include "critical.h"

//include packet module
#include "packet.h"

#include "LEDs.h"

//#include "Flash.h"

#include "PIT.h"

#include "RTC.h"

#include "I2C.h"

#include "accel.h"


const uint64_t BAUDRATE = 115200;

const uint8_t THREAD_STACK_SIZE = 100;

extern TFIFO TxFIFO;
extern TFIFO RxFIFO;

extern OS_ECB* TxfifoSemaphore;
extern OS_ECB* RxfifoSemaphore;

extern uint8_t Data;

static OS_ERROR Error; //Error for all possible ones in RTOS

//OS thread stacks
OS_THREAD_STACK(HandlePacketThreadStack, THREAD_STACK_SIZE);
OS_THREAD_STACK(RxThreadStack, THREAD_STACK_SIZE);
OS_THREAD_STACK(TxThreadStack, THREAD_STACK_SIZE);
OS_THREAD_STACK(InitThreadStack, THREAD_STACK_SIZE);


/* MODULE main */

//std::list<OS_TCB> Tcb;   //a empty Thread control block


class HandlePacket
{
  using F = void (void*); // a function type, not a pointer

  private:
    uint8_t priority;

  public:

   	HandlePacket(F* Thread, void* pData, const uint8_t prio):
   	priority(prio)
    {
   	  // Create HandlePacket thread
   	  Error = OS_ThreadCreate(Thread,
   		               pData,
   		               &HandlePacketThreadStack[THREAD_STACK_SIZE - 1],
					   prio); // Highest priority
    }

    enum Command
	{
      CMD_STARTUP = 0x04,
      CMD_TOWERVERSION = 0x09,
      CMD_TOWERNUMBER = 0x0B,
      CMD_MODE = 0x0A,
      CMD_SETTIME = 0x0C,
      CMD_TOWERMODE = 0x0D,
      CMD_ACCEL_VALUE = 0x10,
      CMD_ACK_STARTUP = 0x84,
      CMD_ACK_TOWERVERSION = 0x89,
      CMD_ACK_TOWERNUMBER = 0x8B,
      CMD_ACK_TOWERMODE = 0x8D,
      CMD_MYTOWERNUMBER = 0x9434
    };


  /*! @brief To handle startup packet
   *  @param packet the PacketVert2 object
   *  @return void
  */
    static void HandleStartupPacket(Packet_t &packet);

  /*! @brief To handle acknowledgement startup packet
   *  @param packet the PacketVert2 object
   *  @return void
  */
  static void HandleACKStartupPacket(Packet_t &packet);

  /*! @brief To handle tower version packet
   *  @param packet the PacketVert2 object
   *  @return void
  */
  static void HandleTowerVersionPacket(Packet_t &packet);

  /*! @brief To handle acknowledgement tower version packet
   *  @param packet the PacketVert2 object
   *  @return void
  */
  static void HandleACKTowerVersionPacket(Packet_t &packet);

  /*! @brief To handle tower number packet
   *  @param packet the PacketVert2 object
   *  @return void
  */
  static void HandleTowerNumberPacket(Packet_t &packet);

  /*! @brief To handle acknowledgement tower number packet
   *  @param packet the PacketVert2 object
   *  @return void
  */
  static void HandleACKTowerNumberPacket(Packet_t &packet);

  /*! @brief To handle tower mode packet
   *  @param packet the PacketVert2 object
   *  @return void
  */
  static void HandleTowerModePacket(Packet_t &packet);

  /*! @brief To handle acknowledgement tower mode packet
   *  @param packet the PacketVert2 object
   *  @return void
  */
  static void HandleACKTowerModePacket(Packet_t &packet);

  /*! @brief To send 4 packets initially and the repsonse for startup protocol
   *  @param packet the PacketVert2 object
   *  @return void
  */
  static void InitResponsePacket(Packet_t &packet);

 /*! @brief To decide how to send packet depending on packet command ID
   *  @param packet the PacketVert2 object
   *  @return void
  */
  static void HandleCommandPacket(Packet_t &packet);

 /*! @brief To handle set time packet
   *  @param packet the PacketVert2 object
   *  @return void
  */
  static void SetTimePacket(Packet_t &packet);

  /*! @brief To select Poll or Interrupt mode
     *  @param packet the Packet_t object
     *  @return void
    */
  static void SetModePacket(Packet_t &packet);

  /*! @brief Destructor for deleting thread when object has not existed
   *
   *
  */
  ~HandlePacket()
  {
    OS_ThreadDelete(priority); // delete thread in destructor , has not assigned priority yet
  }

};

void HandlePacket::HandleStartupPacket(Packet_t &packet)
{
 // Assgin value for startup command according to packet protocol
  Packet_Parameter1 = Packet_Parameter2 = Packet_Parameter3 = 0;

  packet.Packet_t::PacketPut(Packet_Command, Packet_Parameter1, Packet_Parameter1, Packet_Parameter3); //send it to FIFO
}

void HandlePacket::HandleTowerVersionPacket(Packet_t &packet)
{
  // Assgin value for towerversion command according to packet protoco
  Packet_Parameter1 = 0x76;
  Packet_Parameter2 = 0x01;
  Packet_Parameter3 = 0;

  packet.Packet_t::PacketPut(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3); // send it to FIFO
}

void HandlePacket::HandleTowerNumberPacket(Packet_t &packet)
{
// Assgin value for towernumber command according to packet protocol
  Packet_Parameter1 = 0x01;
  Packet_Parameter2 = 0x94;
  Packet_Parameter3 = 0x34;

  packet.Packet_t::PacketPut(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3); //send it to FIFO
}

void HandlePacket::HandleTowerModePacket(Packet_t &packet)
{
  Packet_Parameter1 = 0x01;
  Packet_Parameter2 = 0x01;
  Packet_Parameter3 = 0;

  packet.Packet_t::PacketPut(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);
}

void HandlePacket::SetTimePacket(Packet_t &packet)
{
  RTC::RTC_t rtc;

  rtc.RTC_Set(Packet_Parameter1, Packet_Parameter2, Packet_Parameter3);

  packet.Packet_t::PacketPut(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3); //send it to FIFO
}

/*
void HandlePacket::SetModePacket(Packet_t &packet)
{
  //Select mode
  if (Packet_Parameter2 == 0)
  Accelerometer.SetMode(Accel::POLL);
  else
  Accelerometer.SetMode(Accel::INT);

  //packet.Packet_t::PacketPut(Packet_Command, Packet_Parameter1, Packet_Parameter2, Packet_Parameter3); //send it to FIFO
}*/



// Handling packet protocol (Tower to PC)
void HandlePacket::HandleCommandPacket(Packet_t &packet)
{
  switch (Packet_Command)
  {

  // for specific command. Startup needs to send 3 packets
    case CMD_STARTUP:
    InitResponsePacket(packet);
    break;
    case CMD_ACK_STARTUP:
    HandleACKStartupPacket(packet);
    break;
    case CMD_TOWERVERSION:
    HandleTowerVersionPacket(packet);//only responce once for version
    break;
    case CMD_ACK_TOWERVERSION:
    HandleACKTowerVersionPacket(packet);
    break;
    case CMD_TOWERNUMBER:
    HandleTowerNumberPacket(packet);//only responce once for number
    break;
  case CMD_ACK_TOWERNUMBER:
  HandleACKTowerNumberPacket(packet);
  break;
  case CMD_TOWERMODE:
  HandleTowerModePacket(packet);
  break;
  case CMD_ACK_TOWERMODE:
  HandleACKTowerModePacket(packet);
  break;
  case CMD_SETTIME:
  SetTimePacket(packet);
  break;
  case CMD_MODE:
  //SetModePacket(packet);
  break;

  }

}

void HandlePacket::InitResponsePacket(Packet_t &packet)
{
  // Send tower startup packet
  HandleStartupPacket(packet);

  // Send tower version packet
  Packet_Command = HandlePacket::CMD_TOWERVERSION;
  HandleTowerVersionPacket(packet);

  // Send tower number packet
  Packet_Command = HandlePacket::CMD_TOWERNUMBER;
  HandleTowerNumberPacket(packet);

  // Send tower mode packet
  Packet_Command = HandlePacket::CMD_TOWERMODE;
  HandleTowerModePacket(packet);
}

void HandlePacket::HandleACKStartupPacket(Packet_t &packet)
{
  // Send tower startup packet

  Packet_Command = HandlePacket::CMD_STARTUP;

  InitResponsePacket(packet);

  // Send ack tower startup packet

  Packet_Command = HandlePacket::CMD_ACK_STARTUP; // to modify the packet command ID

  HandleStartupPacket(packet); // to send ack pakcet

}

void HandlePacket::HandleACKTowerVersionPacket(Packet_t &packet)
{
  // Send tower version packet

  Packet_Command = HandlePacket::CMD_TOWERVERSION;


  HandleTowerVersionPacket(packet);

  // Send ack tower version packet

  Packet_Command = HandlePacket::CMD_ACK_TOWERVERSION;


  HandleTowerVersionPacket(packet);
}

void HandlePacket::HandleACKTowerNumberPacket(Packet_t &packet)
{
  // Send tower number pacHandlePacketket

  Packet_Command = HandlePacket::CMD_TOWERNUMBER;

  HandleTowerVersionPacket(packet);

  // Send ack tower version packet

  Packet_Command = HandlePacket::CMD_ACK_TOWERNUMBER;


  HandleTowerNumberPacket(packet);
}


void HandlePacket::HandleACKTowerModePacket(Packet_t &packet)
{
  // Send tower mode packet

  Packet_Command = HandlePacket::CMD_TOWERMODE;

  HandleTowerVersionPacket(packet);

  // Send ack tower mode packet
  Packet_Command = HandlePacket::CMD_ACK_TOWERMODE;

  HandleTowerModePacket(packet);
}
/*
void SendAccelPacket()
{
  //local variable for axis: x y z
  Accel::TAccelData data;
  static uint8_t dataX;
  static uint8_t dataY;
  static uint8_t dataZ;

  //Read from accelerometer
  Accelerometer.ReadXYZ(data.bytes);

  //if data is different, then  packet should be sent
  if ((dataX != data.axes.x) || (dataY != data.axes.y) || (dataZ != data.axes.z))
  {

   //To store previous value
   dataX = data.axes.x;
   dataY = data.axes.y;
   dataZ = data.axes.z;

   Packet.Packet_t::PacketPut(HandlePacket::CMD_ACCEL_VALUE, dataX, dataY, dataZ); //send it to FIFO
  }
}*/


static void InitModulesThread(void* pData)
{
  critical section;

  Packet_t packet(BAUDRATE, CPU_BUS_CLK_HZ);

  //Initialize PIT module
 // PIT::PIT_t pit(CPU_BUS_CLK_HZ, 0);
//  pit.Set(500, true); // Set period 500ms

  // Initialize RTC module
//  RTC::RTC_t rtc(0);

  LED_t led(LED_t::ORANGE);
  led.On();

  //delete this thread
  OS_ThreadDelete(OS_PRIORITY_SELF);
}



static void RxThread(void* pData)
{
  for (;;)
  {
	OS_SemaphoreWait(RxfifoSemaphore, 0); //suspend the thread until next time it has been siginified
    RxFIFO.Put(Data); // let the receiver to send a byte of data to RxFIFO

  }
}


static void TxThread(void* pData)
{
  uint8_t data{0};

  for (;;)
  {
	OS_SemaphoreWait(TxfifoSemaphore, 0); //suspend the thread until next time it has been siginified

    if (!TxFIFO.Get(data) )
	  UART2_C2 &= ~UART_C2_TIE_MASK; // Disarm the UART output
	else
	{
	  UART2_D = data;
	}


  }

}

/*
 //function description
static void PITThread(void* argu)
{
  static LED_t led; //local object for LED

  for (;;)
  {
	    //wait....


  }
  //  Led.Color(LED_t::GREEN);
  //  Led.Toggle();


}


static void RTCThread(void* argu)
{

  uint8_t hours, mins, sec;
  RTC::RTC_t rtc;

  static LED_t led; //local object for LED

  for (;;)
  {



  }

  led.Color(LED_t::YELLOW);
  led.Toggle();

  rtc.RTC_Get(hours, mins, sec);

//Packet.Packet_t::PacketPut(HandlePacket::CMD_SETTIME, hours, mins, sec); //send it to FIFO
//SendAccelPacket();
}
*/



/*! @brief using member function as the thread of sending Packet
     *
     *  @param pData might not use but is required by the OS to create a thread.
     *
     */
static void HandlePacketThread(void* pData)
{
  // initialize the packet module
	Packet_t packet;

  for (;;)
  {
    if (packet.Packet_t::PacketGet())
      HandlePacket::HandleCommandPacket(packet);

  }

}



/*lint -save  -e970 Disable MISRA rule (6.3) checking. */
int main(void)
/*lint -restore Enable MISRA rule (6.3) checking. */
{
  /* Write your local variable definition here */

  __DI();//Disable interrupt

  //Initialize OS without flashing the orange LED
  OS_Init(CPU_CORE_CLK_HZ, 0);

  /*** Processor Expert internal initialization. DON'T REMOVE THIS CODE!!! ***/
  PE_low_level_init();
  /*** End of Processor Expert internal initialization.                    ***/


  /* Write your code here */

  Error = OS_ThreadCreate(InitModulesThread,
                  0,
                  &InitThreadStack[THREAD_STACK_SIZE - 1],
    		      0);

  Error = OS_ThreadCreate(RxThread,
                          0,
                          &RxThreadStack[THREAD_STACK_SIZE - 1],
  		                  2);

  Error = OS_ThreadCreate(TxThread,
                            0,
                            &TxThreadStack[THREAD_STACK_SIZE - 1],
    		                3);

  // Create sending packet thread with highest priority
  static HandlePacket packetThread(HandlePacketThread, 0, 5);

  __EI(); //Enable the interrupt

  // Start multithreading - never returns!
  OS_Start();


  /*** Don't write any code pass this line, or it will be deleted during code generation. ***/
  /*** RTOS startup code. Macro PEX_RTOS_START is defined by the RTOS component. DON'T MODIFY THIS CODE!!! ***/
  #ifdef PEX_RTOS_START
    PEX_RTOS_START();                  /* Startup of the selected RTOS. Macro is defined by the RTOS component. */
  #endif
  /*** End of RTOS startup code.  ***/
  /*** Processor Expert end of main routine. DON'T MODIFY THIS CODE!!! ***/
  for(;;){}
  /*** Processor Expert end of main routine. DON'T WRITE CODE BELOW!!! ***/
} /*** End of main routine. DO NOT MODIFY THIS TEXT!!! ***/

/* END main */
/*!
** @}
*/
/*
** ###################################################################
**
**     This file was created by Processor Expert 10.5 [05.21]
**     for the Freescale Kinetis series of microcontrollers.
**
** ###################################################################
*/



