#ifndef VRPN_AIMON_H
#define VRPN_AIMON_H

#include "vrpn_HumanInterface.h"
#include "vrpn_Button.h"
#include "vrpn_Analog.h"

// Device drivers for the Aimon OS ELITE


// Exposes two VRPN device classes: Button and Analog.
// Analogs are mapped to the six channels, each in the range (-1..1).



#if defined(VRPN_USE_HID)
class VRPN_API vrpn_Aimon: public vrpn_Button_Filter, public vrpn_Analog, protected vrpn_HidInterface {
public:
  vrpn_Aimon(vrpn_HidAcceptor *filter,
                   const char *name, vrpn_Connection *c = 0);
  virtual ~vrpn_Aimon();

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


class VRPN_API vrpn_Aimon_Elite: public vrpn_Aimon {
public:
  vrpn_Aimon_Elite(const char *name, vrpn_Connection *c = 0);
  virtual ~vrpn_Aimon_Elite() {};


protected:
};
#endif
// end of VRPN_AIMON_H
#endif

