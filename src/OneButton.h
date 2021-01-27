// -----
// OneButton.h - Library for detecting button clicks, doubleclicks and long
// press pattern on a single button. This class is implemented for use with the
// Arduino environment. Copyright (c) by Matthias Hertel,
// http://www.mathertel.de This work is licensed under a BSD style license. See
// http://www.mathertel.de/License.aspx More information on:
// http://www.mathertel.de/Arduino
// -----
// 02.10.2010 created by Matthias Hertel
// 21.04.2011 transformed into a library
// 01.12.2011 include file changed to work with the Arduino 1.0 environment
// 23.03.2014 Enhanced long press functionalities by adding longPressStart and
// longPressStop callbacks
// 21.09.2015 A simple way for debounce detection added.
// 14.05.2017 Debouncing improvements.
// 25.06.2018 Optional third parameter for deactivating pullup.
// 26.09.2018 Anatoli Arkhipenko: Included solution to use library with other
// sources of input.
// 26.09.2018 Initialization moved into class declaration.
// 26.09.2018 Jay M Ericsson: compiler warnings removed.
//
// 29.01.2020 ShaggyDog18: optimized by using switch() instead of multiple if()-s; PARAM_FUNC functions are optional to save space
// 12.02.2020 ShaggyDog18: Modification Log:
// - modified the State Machine (still the same number of states), maintained full compatibility with initial Author development
// - introduced new functions: multiClickFunc() for 3+ clicks; getNumberClicks() to return number of clicks;
// - optimized - changed some types of variables (f.e.: bool _buttonPressed, uint8_t _state) to compact the code 
// - modified SimpleOneButton example to test more functions incl. new multiClickFunc() and getNumberClicks() functions
// -----

#pragma once

#include "Arduino.h"

//#define PARAM_FUNC		// uncomment in case need calling functions with parameters

// ----- Callback function types -----

extern "C" {
typedef void (*callbackFunction)(void);
#ifdef PARAM_FUNC
  typedef void (*parameterizedCallbackFunction)(void*);
#endif
}

#define attachTripleClick attachMultiClick  // for compatibility with previous version
#define attachPress attachClick  // for compatibility with previous library version


class OneButton {
public:
  // ----- Constructor -----
  OneButton();

  OneButton(const int16_t pin, const bool activeLow = true, const bool pullupActive = true);

  // ----- Set runtime parameters -----
  /**
   * set # millisec after safe click is assumed.
   */
  inline void setDebounceTicks(const uint16_t ticks){ _debounceTicks = ticks; }

  /**
   * set # millisec after single click is assumed.
   */
  inline void setClickTicks(const uint16_t ticks){ _clickTicks = ticks; }

  /**
   * set # millisec after press is assumed.
   */
  inline void setPressTicks(const uint16_t ticks){ _pressTicks = ticks; }

  /**
   * Attach an event to be called when a single click is detected.
   * @param newFunction
   */
  inline void attachClick(callbackFunction newFunction){ _clickFunc = newFunction; }
  #ifdef PARAM_FUNC
    void attachClick(parameterizedCallbackFunction newFunction, void* parameter);
  #endif

  /**
   * Attach an event to be called after a double click is detected.
   * @param newFunction
   */
  inline void attachDoubleClick(callbackFunction newFunction){ _doubleClickFunc = newFunction; }
  #ifdef PARAM_FUNC
    void attachDoubleClick(parameterizedCallbackFunction newFunction, void* parameter);
  #endif

  /**
   * Attach an event to be called after a triple and more clicks are detected.
   * @param newFunction
   */
  inline void attachMultiClick(callbackFunction newFunction){ _multiClickFunc = newFunction; }
  #ifdef PARAM_FUNC
    void attachMultiClick(parameterizedCallbackFunction newFunction, void* parameter);
  #endif  

  /**
   * @deprecated Replaced by longPressStart, longPressStop, and duringLongPress.
   * @param newFunction
   */
  // defined it as alias to attachClick()
  //inline void attachPress(callbackFunction newFunction){ _pressFunc = newFunction; }

  /**
   * Attach an event to fire as soon as the button is pressed down.
   * @param newFunction
   */
  inline void attachPressStart(callbackFunction newFunction){ _pressStartFunc = newFunction; }

  /**
   * Attach an event to fire when the button is pressed and held down.
   * @param newFunction
   */
  inline void attachLongPressStart(callbackFunction newFunction){ _longPressStartFunc = newFunction; }
  #ifdef PARAM_FUNC
    void attachLongPressStart(parameterizedCallbackFunction newFunction, void* parameter);
  #endif

  /**
   * Attach an event to fire as soon as the button is released after a long press.
   * @param newFunction
   */
  inline void attachLongPressStop(callbackFunction newFunction){ _longPressStopFunc = newFunction; }
  #ifdef PARAM_FUNC
    void attachLongPressStop(parameterizedCallbackFunction newFunction, void* parameter);
  #endif

  /**
   * Attach an event to fire periodically while the button is held down.
   * @param newFunction
   */
  inline void attachDuringLongPress(callbackFunction newFunction){ _duringLongPressFunc = newFunction; }
  #ifdef PARAM_FUNC
    void attachDuringLongPress(parameterizedCallbackFunction newFunction, void* parameter);
  #endif

  // ----- State machine functions -----
  /**
   * @brief Call this function every some milliseconds for checking the input
   * level at the initialized digital pin.
   */
  void tick(void);

  /**
   * @brief Call this function every time the input level has changed.
   * Using this function no digital input pin is checked because the current
   * level is given by the parameter.
   */
  void tick(bool level);
  
  /**
   * @return true if we are currently handling button press flow
   * (This allows power sensitive applications to know when it is safe to power down the main CPU)
   */
  inline bool isIdle() { return _state == WAIT_FOR_INITIAL_PRESS; }

  /**
   * Detect whether or not the button is currently inside a long press.
   * @return
   */
  inline bool isLongPressed() { return _state == LONG_PRESS; /*_isLongPressed;*/ }

  /**
   * Get the current number of ticks that the button has been held down for.
   * @return
   */
  inline int getPressedTicks() { return _stopTime - _startTime; }
  /**
   * Reset the button state machine.
   */
  void reset(void);

  /**
   * Get number of clicks for the multiple click case.// ShaggyDog: return number of clicks
   */
  inline uint8_t getNumberClicks(void) { return _nClicks; }

private:
  // These variables will hold functions acting as event source.
  callbackFunction _clickFunc = NULL;
  //callbackFunction _pressFunc = NULL; // old function, same as _clickFunc(); keep it for compatibility
  callbackFunction _doubleClickFunc = NULL;
  callbackFunction _multiClickFunc = NULL;	// new multiple click function
  callbackFunction _pressStartFunc = NULL;
  
  // long press set of functions
  callbackFunction _longPressStartFunc = NULL;
  callbackFunction _longPressStopFunc = NULL;
  callbackFunction _duringLongPressFunc = NULL;
	
  #ifdef PARAM_FUNC
    parameterizedCallbackFunction _paramClickFunc = NULL;
    void* _clickFuncParam = NULL;

    parameterizedCallbackFunction _paramDoubleClickFunc = NULL;
    void* _doubleClickFuncParam = NULL;

    parameterizedCallbackFunction _paramMultiClickFunc = NULL;
    void* _multiClickFuncParam = NULL;

    parameterizedCallbackFunction _paramLongPressStartFunc = NULL;
    void* _longPressStartFuncParam = NULL;

    parameterizedCallbackFunction _paramLongPressStopFunc = NULL;
    void* _longPressStopFuncParam;

    parameterizedCallbackFunction _paramDuringLongPressFunc = NULL;
    void* _duringLongPressFuncParam = NULL;
  #endif

  int16_t  _pin; // hardware pin number.
  uint16_t _debounceTicks = 50; // number of ticks for debounce times.
  uint16_t _clickTicks = 400;   // number of ticks that have to pass by before a click is detected.
  uint16_t _pressTicks = 800;   // number of ticks that have to pass by before a long button press is detected
  
  // These variables that hold information across the upcoming tick calls.
  // They are initialized once on program start and are updated every time the
  // tick function is called.
  unsigned long _startTime; // will be set in state 1
  unsigned long _stopTime; // will be set in state 2

  bool _buttonPressed = false;
  //bool _isLongPressed = false;

  uint8_t _nClicks = 0;	// ShaggyDog - count number of clicks  

  // define FiniteStateMachine
  enum stateMachine_t : uint8_t {
    WAIT_FOR_INITIAL_PRESS = 0, // 0
    DEBOUNCE_OR_LONG_PRESS,	  // 1
    DETECT_CLICK,  // 2
    COUNT_CLICKS,  // 3
    LONG_PRESS     // used to be 6, now is equal to 4
  } _state = WAIT_FOR_INITIAL_PRESS;
};