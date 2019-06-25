
/*! @file UART.h
 *
 *  @brief I/O routines for UART communications on the TWR-K70F120M.
 *
 *  This contains the functions for operating the UART (serial port).
 *
 *  @author Chao Li
 *  @date 07/05/2019
 *  Copyright (c) Chao Li. All rights reserved.
 */

#ifndef UART_H
#define UART_H

// new types
#include "type_cpp.h"

// _EI() _DI(),
#include "PE_Types.h"

// include FIFO module
#include "FIFO.h"

// involve mask for all registers
#include "MK70F12.h"

#include "OS_cpp.h" //critical section


const uint8_t THREAD_STACK_SIZE_RT = 120;

OS_THREAD_STACK(RxThreadStack, THREAD_STACK_SIZE_RT );
OS_THREAD_STACK(TxThreadStack, THREAD_STACK_SIZE_RT );


static void RxThread(void* pData);

static void TxThread(void* pData);

static TFIFO TxFIFO;
static TFIFO RxFIFO;

static OS_ECB* RxfifoSemaphore;
static OS_ECB* TxfifoSemaphore;

static uint8_t RxData;

class UART_t
{
  private:
    uint32_t baudRate;  /*!< baudRate The desired baud rate in bits/sec. */
    uint32_t moduleClk;  /*!< moduleClk The module clock rate in Hz. */
    OS_ERROR error;

  protected:
  /*! @brief Sets up the UART interface before first use.
   *
   *  @return bool - TRUE if the UART was successfully initialized.
   */
    bool Init() const;

  /*! @brief Constructor for the UART interface before first use.
     *
     *  @param rate The desired baud rate in bits/sec.
     *  @param clock The module clock rate in Hz.
     *
     */
    UART_t(const uint32_t rate, const uint32_t clock):
    baudRate{rate}, moduleClk{clock}
    {
      if (this->Init() )
      {
        error = OS_ThreadCreate(RxThread,
    	      	                0,
    	      	                &RxThreadStack[THREAD_STACK_SIZE_RT - 1],
    	      	                7);

        error = OS_ThreadCreate(TxThread,
            	      	                0,
            	      	                &TxThreadStack[THREAD_STACK_SIZE_RT - 1],
            	      	                8);


      }

    }

    UART_t() = default;
  
  public:
  /*! @brief Put a byte in the transmit FIFO if it is not full.
   *
   *  @param rxData The byte to be placed in the transmit FIFO.
   *  @return bool - TRUE if the data was placed in the transmit FIFO.
   *  @note Assumes that UART_Init has been called.
   */
    bool InChar(uint8_t &rxData);

  /*! @brief Put a byte in the transmit FIFO if it is not full.
   *
   *  @param txData The byte to be placed in the transmit FIFO.
   *  @return bool - TRUE if the data was placed in the transmit FIFO.
   *  @note Assumes that UART_Init has been called.
   */
    bool OutChar(const uint8_t txData);
};

/*! @brief Receiving thread
       *
       *  @param pData might not use but is required by the OS to create a thread.
       *
       */
  void RxThread(void* pData)
 {
    for (;;)
    {
  	  OS_SemaphoreWait(RxfifoSemaphore, 0); //suspend the thread until next time it has been siginified
      RxFIFO.Put(RxData); // let the receiver to send a byte of data to RxFIFO

     // OS_SemaphoreSignal(ShareFIFOSemaphore); //Release a remaphore
    }
 }



  /*! @brief Transimission thread
       *
       *  @param pData might not use but is required by the OS to create a thread.
       *
       */
  void TxThread(void* pData)
 {
    uint8_t data = 0;

    for (;;)
    {
  	OS_SemaphoreWait(TxfifoSemaphore, 0); //suspend the thread until next time it has been siginified

      if (!TxFIFO.Get(data) )
  	  UART2_C2 &= ~UART_C2_TIE_MASK; // Disarm the UART output
  	else
  	{
  	  UART2_D = data;
  	  UART2_C2 |= UART_C2_TIE_MASK;// Arm output device
  	}

    }

 }


/*! @brief Poll the UART status register to try and receive and/or transmit one character.
 *
 *  @return void
 *  @note Assumes that UART_Init has been called.
 */
//void UART_Poll(void);

/*! @brief Interrupt service routine for the UART.
 *
 *  @note Assumes the transmit and receive FIFOs have been initialized.
 */
void __attribute__ ((interrupt)) UART_ISR(void);

#endif

