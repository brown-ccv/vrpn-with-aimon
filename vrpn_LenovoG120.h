#ifndef VRPN_LenovoG120_H
#define VRPN_LenovoG120_H

#include "vrpn_HumanInterface.h"
#include "vrpn_Button.h"
#include "vrpn_Analog.h"

// Device drivers for the Lenovo G120 wireless joystick and mouse
// devices, connecting to them as HID devices (USB).

// Exposes two VRPN device classes: Button and Analog.
// Analogs are mapped to the six channels, each in the range (-1..1).

#if defined(VRPN_USE_HID)
class VRPN_API vrpn_LenovoG120: public vrpn_Button, public vrpn_Analog, protected vrpn_HidInterface {
public:
  vrpn_LenovoG120(const char *name, vrpn_Connection *c = 0);
  virtual ~vrpn_LenovoG120();

  virtual void mainloop();

protected:
  // Set up message handlers, etc.
  void on_data_received(size_t bytes, vrpn_uint8 *buffer);

  virtual void decodePacket(size_t bytes, vrpn_uint8 *buffer);
  struct timeval _timestamp;
  vrpn_HidAcceptor *_filter;

  // Send report iff changed
  void report_changes (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
  // Send report whether or not changed
  void report (vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
  // NOTE:  class_of_service is only applied to vrpn_Analog
  //  values, not vrpn_Button or vrpn_Dial
};
#else   // not _WIN32
class VRPN_API vrpn_LenovoG120: public vrpn_Button, public vrpn_Analog {
public:
  vrpn_LenovoG120(vrpn_HidAcceptor *filter, unsigned num_buttons,
                   const char *name, vrpn_Connection *c = 0);
  virtual ~vrpn_LenovoG120();

  virtual void mainloop();

protected:
  struct timeval _timestamp;
  vrpn_HidAcceptor *_filter;
  int fd;

  // Send report iff changed
  void report_changes(vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
  // Send report whether or not changed
  void report(vrpn_uint32 class_of_service = vrpn_CONNECTION_LOW_LATENCY);
  // NOTE:  class_of_service is only applied to vrpn_Analog
  //  values, not vrpn_Button or vrpn_Dial

// There is a non-HID Linux-based driver for this device that has a capability
// not implemented in the HID interface.
#if defined(linux) && !defined(VRPN_USE_HID)
  int set_led(int led_state);
#endif
};
#endif  // not _WIN32

// end of VRPN_LenovoG120_H
#endif

