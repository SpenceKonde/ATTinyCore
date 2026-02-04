// VirtualWire.h
//
// Virtual Wire implementation for Arduino
// See the README file in this directory fdor documentation
// 
// Author: Mike McCauley (mikem@airspayce.com) DO NOT CONTACT THE AUTHOR DIRECTLY: USE THE LISTS
// Copyright (C) 2008 Mike McCauley
// $Id: VirtualWire.h,v 1.6 2013/02/14 22:02:11 mikem Exp mikem $

/// \mainpage VirtualWire library for Arduino
///
/// This is the Arduino VirtualWire library.
///
/// VirtualWire is an Arduino library that provides features to send short
/// messages, without addressing, retransmit or acknowledgment, a bit like UDP
/// over wireless, using ASK (amplitude shift keying). Supports a number of
/// inexpensive radio transmitters and receivers. All that is required is
/// transmit data, receive data and (for transmitters, optionally) a PTT
/// transmitter enable.
///
/// It is intended to be compatible with the RF Monolithics (www.rfm.com)
/// Virtual Wire protocol, but this has not been tested.
///
/// Does not use the Arduino UART. Messages are sent with a training preamble,
/// message length and checksum. Messages are sent with 4-to-6 bit encoding
/// for good DC balance, and a CRC checksum for message integrity.
///
/// Why not just use the Arduino UART connected directly to the
/// transmitter/receiver? As discussed in the RFM documentation, ASK receivers
/// require a burst of training pulses to synchronize the transmitter and
/// receiver, and also requires good balance between 0s and 1s in the message
/// stream in order to maintain the DC balance of the message. UARTs do not
/// provide these. They work a bit with ASK wireless, but not as well as this
/// code.
///
/// This library provides classes for 
/// - VirtualWire: unaddressed, unreliable messages
///
/// Example Arduino programs are included to show the main modes of use.
///
/// The version of the package that this documentation refers to can be downloaded 
/// from http://www.airspayce.com/mikem/arduino/VirtualWire/VirtualWire-1.15.zip
/// You can find the latest version at http://www.airspayce.com/mikem/arduino/VirtualWire
///
/// You can also find online help and disussion at http://groups.google.com/group/virtualwire
/// Please use that group for all questions and discussions on this topic. 
/// Do not contact the author directly, unless it is to discuss commercial licensing.
///
/// \par Supported Hardware
/// A range of communications hardware is supported. The ones listed blow are
/// available in common retail outlets in Australian and other countries for
/// under $10 per unit. Many other modules may also work with this software.
/// Runs on ATmega8/168 (Arduino Diecimila, Uno etc) and ATmega328 and possibly
/// others. Also runs on on Energia with MSP430G2553 / G2452 and Arduino with 
/// ATMega328 (courtesy Yannick DEVOS - XV4Y).
/// Also compiles and runs on ATtiny85 in Arduino environment, courtesy r4z0r7o3.
///
/// - Receivers
///  - RX-B1 (433.92MHz) (also known as ST-RX04-ASK)
/// - Transmitters: 
///  - TX-C1 (433.92MHz)
/// - Transceivers
///  - DR3100 (433.92MHz)
///
/// \par Installation
/// To install, unzip the library into the libraries sub-directory of your
/// Arduino application directory. Then launch the Arduino environment; you
/// should see the library in the Sketch->Import Library menu, and example
/// code in
/// File->Sketchbook->Examples->VirtualWire menu.
///
/// \par Open Source Licensing GPL V2
///
/// This is the appropriate option if you want to share the source code of your
/// application with everyone you distribute it to, and you also want to give them
/// the right to share who uses it. If you wish to use this software under Open
/// Source Licensing, you must contribute all your source code to the open source
/// community in accordance with the GPL Version 2 when your application is
/// distributed. See http://www.gnu.org/copyleft/gpl.html
/// 
/// \par Commercial Licensing
///
/// This is the appropriate option if you are creating proprietary applications
/// and you are not prepared to distribute and share the source code of your
/// application. Contact info@airspayce.com for details.
///
/// \par Revision History
/// \version 1.0 Original release
/// 
/// \version 1.1 2008-06-24
/// 	Now can compile for atmega8
///	Reported by creatrope
/// \version 1.2 2009-03-30
///	Fixed a problem that prevented compiling with arduino-0015
///	Reported by Jaime Castro
/// \version 1.3 2009-04-01
///	Fixed a compatibility problem with ATMEGA328 of the new arduino
///	Now use SIGNAL(TIMER1_COMPA_vect) instead of ISR(SIG_OUTPUT_COMPARE1A)
///	as discussed in
///	http://www.arduino.cc/cgi-bin/yabb2/YaBB.pl?num=1237714550/11
///	and reported by Jaime Castro.
/// \version 1.4 2010-01-29
///	Added vx_tx_active(), suggested by Alan Burlison.
/// \version 1.5 2011-09-09
///	Added vx_tx_active() function.
/// \version 1.6 2012-01-10
///	Fixed a problem where the receiver was always reenabled after
///	transmission. Reported by David Bath
/// \version 1.9 2012-02-07 Documentation updates
///	Documentation updates
/// \version 1.10 Updated CHANGES file with changes since 1.4.
/// \version 1.11 Converted documentation to Doxygen. Moved CHANGES log to this version history.
///     Ensure vw_rx_pin is not accessed unless receiver is enabled
/// \version 1.12 Compiles and runs on on Energia with MSP430G2553 / G2452 and Arduino with ATMega328. 
///     Patches contributed by Yannick DEVOS - XV4Y
/// \version 1.13 util/crc16.h needed for compiling on  Energia with MSP430G2553 / G2452 was accidentally
///     left out of the distribution
/// \version 1.14 Added support ATtiny85 on Arduino, patch provided by r4z0r7o3.
/// \version 1.15 Updated author and distribution location details to airspayce.com
///
/// \par Implementation Details
/// See: http://www.airspayce.com/mikem/arduino/VirtualWire.pdf
///
/// \par Performance
/// See: http://www.airspayce.com/mikem/arduino/VirtualWire.pdf
///
/// \par Connections
/// See: http://www.airspayce.com/mikem/arduino/VirtualWire.pdf
///
/// \file VirtualWire.h
/// \brief VirtualWire API
///
/// To use the VirtualWire library, you must have
/// \code
/// #include <VirtualWire.h>
/// \endcode
/// At the top of your sketch.
/// 

#ifndef VirtualWire_h
#define VirtualWire_h

#include <stdlib.h>
#if defined(ARDUINO)
 #if ARDUINO >= 100
  #include <Arduino.h>
 #else
  #include <wiring.h>
 #endif
#elif defined(__MSP430G2452__) || defined(__MSP430G2553__) // LaunchPad specific
 #include "legacymsp430.h"
 #include "Energia.h"
#else // error
 #error Platform not defined
#endif

// These defs cause trouble on some versions of Arduino
#undef abs
#undef double
#undef round

/// Maximum number of bytes in a message, counting the byte count and FCS
#define VW_MAX_MESSAGE_LEN 30

/// The maximum payload length
#define VW_MAX_PAYLOAD VW_MAX_MESSAGE_LEN-3

/// The size of the receiver ramp. Ramp wraps modulu this number
#define VW_RX_RAMP_LEN 160

/// Number of samples per bit
#define VW_RX_SAMPLES_PER_BIT 8

// Ramp adjustment parameters
// Standard is if a transition occurs before VW_RAMP_TRANSITION (80) in the ramp,
// the ramp is retarded by adding VW_RAMP_INC_RETARD (11)
// else by adding VW_RAMP_INC_ADVANCE (29)
// If there is no transition it is adjusted by VW_RAMP_INC (20)
/// Internal ramp adjustment parameter
#define VW_RAMP_INC (VW_RX_RAMP_LEN/VW_RX_SAMPLES_PER_BIT)
/// Internal ramp adjustment parameter
#define VW_RAMP_TRANSITION VW_RX_RAMP_LEN/2
/// Internal ramp adjustment parameter
#define VW_RAMP_ADJUST 9
/// Internal ramp adjustment parameter
#define VW_RAMP_INC_RETARD (VW_RAMP_INC-VW_RAMP_ADJUST)
/// Internal ramp adjustment parameter
#define VW_RAMP_INC_ADVANCE (VW_RAMP_INC+VW_RAMP_ADJUST)

/// Outgoing message bits grouped as 6-bit words
/// 36 alternating 1/0 bits, followed by 12 bits of start symbol
/// Followed immediately by the 4-6 bit encoded byte count, 
/// message buffer and 2 byte FCS
/// Each byte from the byte count on is translated into 2x6-bit words
/// Caution, each symbol is transmitted LSBit first, 
/// but each byte is transmitted high nybble first
#define VW_HEADER_LEN 8

// Cant really do this as a real C++ class, since we need to have 
// an ISR
extern "C"
{
    /// Set the digital IO pin to be for transmit data. 
    /// This pin will only be accessed if
    /// the transmitter is enabled
    /// \param[in] pin The Arduino pin number for transmitting data. Defaults to 12.
    extern void vw_set_tx_pin(uint8_t pin);

    /// Set the digital IO pin to be for receive data.
    /// This pin will only be accessed if
    /// the receiver is enabled
    /// \param[in] pin The Arduino pin number for receiving data. Defaults to 11.
    extern void vw_set_rx_pin(uint8_t pin);

    // Set the digital IO pin to enable the transmitter (press to talk, PTT)'
    /// This pin will only be accessed if
    /// the transmitter is enabled
    /// \param[in] pin The Arduino pin number to enable the transmitter. Defaults to 10.
    extern void vw_set_ptt_pin(uint8_t pin);

    /// By default the PTT pin goes high when the transmitter is enabled.
    /// This flag forces it low when the transmitter is enabled.
    /// \param[in] inverted True to invert PTT
    extern void vw_set_ptt_inverted(uint8_t inverted);

    /// Initialise the VirtualWire software, to operate at speed bits per second
    /// Call this one in your setup() after any vw_set_* calls
    /// Must call vw_rx_start() before you will get any messages
    /// \param[in] speed Desired speed in bits per second
    extern void vw_setup(uint16_t speed);

    /// Start the Phase Locked Loop listening to the receiver
    /// Must do this before you can receive any messages
    /// When a message is available (good checksum or not), vw_have_message();
    /// will return true.
    extern void vw_rx_start();

    /// Stop the Phase Locked Loop listening to the receiver
    /// No messages will be received until vw_rx_start() is called again
    /// Saves interrupt processing cycles
    extern void vw_rx_stop();

    /// Returns the state of the
    /// transmitter
    /// \return true if the transmitter is active else false
    extern uint8_t vx_tx_active();

    /// Block until the transmitter is idle
    /// then returns
    extern void vw_wait_tx();

    /// Block until a message is available
    /// then returns
    extern void vw_wait_rx();

    /// Block until a message is available or for a max time
    /// \param[in] milliseconds Maximum time to wait in milliseconds.
    /// \return true if a message is available, false if the wait timed out.
    extern uint8_t vw_wait_rx_max(unsigned long milliseconds);

    /// Send a message with the given length. Returns almost immediately,
    /// and message will be sent at the right timing by interrupts
    /// \param[in] buf Pointer to the data to transmit
    /// \param[in] len Number of octetes to transmit
    /// \return true if the message was accepted for transmission, false if the message is too long (>VW_MAX_MESSAGE_LEN - 3)
    extern uint8_t vw_send(uint8_t* buf, uint8_t len);

    // Returns true if an unread message is available
    /// \return true if a message is available to read
    extern uint8_t vw_have_message();

    // If a message is available (good checksum or not), copies
    // up to *len octets to buf.
    /// \param[in] buf Pointer to location to save the read data (must be at least *len bytes.
    /// \param[in,out] len Available space in buf. Will be set to the actual number of octets read
    /// \return true if there was a message and the checksum was good
    extern uint8_t vw_get_message(uint8_t* buf, uint8_t* len);
    
    /// Declare an external function to call when the timer overflows.
    /// \param[in] Funct Pointer to the function to call when timer overflows (eg: Software PWM management function)
    extern void vw_declare_timer_Ovf_funct(void (*Funct)(void));

}

/// @example client.pde
/// Client side of simple client/server pair using VirtualWire

/// @example server.pde
/// Server side of simple client/server pair using VirtualWire

/// @example transmitter.pde
/// Transmitter side of simple one-way transmitter->receiver pair using VirtualWire

/// @example receiver.pde
/// Transmitter side of simple one-way transmitter->receiver pair using VirtualWire

#endif
