/**
 ******************************************************************************
 * @Filename:	timersdriver.h
 * @Project: 	loraRC
 * @Author: 	Jose Barros
 * @Copyright (C) 2017 Jose Barros
 * @Email:  	josemanuelbarros@gmail.com
 *****************************************************************************/
/*
 * @License:
 * This file is part of loraRC.
 * loraRC is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * loraRC is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with loraRC.  If not, see <http://www.gnu.org/licenses/>.
 */

 #ifndef TimerDriver_h
 #define TimerDriver_h
 #include "Arduino.h"
 #include "util/atomic.h"

#define COMXA1  7
#define COMXA0  6
#define COMXB1  5
#define COMXB0  4
#define WGMX1   1
#define WGMX0   0
#define WGMX2   3
#define WGMX3   4
#define ICES1   6
#define ICNC1   7
#define CSX0    0
#define CSX1    1
#define CSX2    2

#define EDGE_FALLING false
#define EDGE_RISING true

class TimerDriver {
public:
  enum waveFormGenerator {
    WAVE_FORM_NORMAL = 0,                             //0

    WAVE_PWM_PHASE_CORRECT,                           //1
    WAVE_PWM_PHASE_CORRECT_8, //tim1                  //2
    WAVE_PWM_PHASE_CORRECT_9, //tim1                  //3
    WAVE_PWM_PHASE_CORRECT_10,//tim1                  //4
    WAVE_PWM_PHASE_CORRECT_OCR,                       //5
    WAVE_PWM_PHASE_CORRECT_ICR,//tim1                 //6

    WAVE_CTC_OCR,                                     //7
    WAVE_CTC_ICR,//tim1                               //8
    WAVE_FAST_PWM,                                    //9
    WAVE_FAST_PWM_8, //tim1                           //10
    WAVE_FAST_PWM_9, //tim                            //11
    WAVE_FAST_PWM_10,//tim1                           //12
    WAVE_FAST_PWM_OCR,                                //13
    WAVE_FAST_PWM_ICR,//tim1                          //14

    WAVE_PWM_PHASE_FREQUENCY_CORRECT_OCR,//TIMER_1    //15
    WAVE_PWM_PHASE_FREQUENCY_CORRECT_ICR,//TIMER_1    //16
};

  enum compareMode {
    COMPARE_NORMAL, //0
    COMPARE_TOGGLE, //1
    COMPARE_CLEAR,   //2
    COMPARE_SET    //3
  };
  enum timer { TIMER_0, TIMER_1, TIMER_2};
  enum timer_clock {
    CLOCK_NONE = 0,               //0
    CLOCK_1,                      //1
    CLOCK_1_8,                    //2
    CLOCK_1_32, //TIMER_2         //3
    CLOCK_1_64,                   //4
    CLOCK_1_128, //TIMER_2        //5
    CLOCK_1_256,                  //6
    CLOCK_1_1024,                 //7
    CLOCK_EXT_FALLING,//!=TIMER_2 //8
    CLOCK_EXT_RISING//!=TIMER_2   //9
  };
  enum timer_interrupt { INT_ON_OVERFLOW = 1, INT_ON_COMPARE_A = 2, INT_ON_COMPARE_B = 4, INT_ON_INPUT_CAPTURE = 32};//TIMSK0
  struct timerConfig{
    timer timerID;
    waveFormGenerator waveForm;
    compareMode compareA;
    compareMode compareB;
    timer_clock tclock;
    bool useInputCaptureNoiseCanceler;//tim1
    bool inputCaptureEdgeSelect;//tim1
  };
  TimerDriver();
  void init(timerConfig cfg);
  void enableInterrupts(uint8_t interrupts);
  void disableInterrupts(uint8_t interrupts);
  void setOutputCompareValueA(uint16_t value);
  void setOutputCompareValueB(uint16_t value);
  void setInputCapture(uint16_t value);
  uint16_t getOutputCompareValueA();
  uint16_t getOutputCompareValueB();
  uint16_t getInputCapture();
  uint16_t getTimerValue();
  static void on_compareAinterrupt_priv(uint8_t ix);
  static void on_compareBinterrupt_priv(uint8_t ix);
  static void on_overflow_interrupt_priv(uint8_t ix);
  static void on_input_capture_interrupt_priv(uint8_t ix);
  virtual void on_compareAinterrupt();
  virtual void on_compareBinterrupt();
  virtual void on_overflow_interrupt();
  virtual void on_input_capture_interrupt();
private:
  static TimerDriver* s_pin[3];
  volatile uint8_t *TCCRA;
  volatile uint8_t *TCCRB;
  volatile uint8_t *TCNTL;//timer counter
  volatile uint8_t *TCNTH;
  volatile uint8_t *OCRAL;//output compare A
  volatile uint8_t *OCRAH;
  volatile uint8_t *OCRBL;//output compare B
  volatile uint8_t *OCRBH;
  volatile uint8_t *TIMSK;//interrupt mask
  volatile uint8_t *TIFR;//interrupt flag
  volatile uint8_t *ICRL;//input capture register
  volatile uint8_t *ICRH;
  uint8_t enabledInterrupts;
  TimerDriver* m_next;
};

 #endif
