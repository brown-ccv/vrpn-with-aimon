// vrpn_Aimon.C: VRPN driver for Aimon


#include "vrpn_BufferUtils.h"
#include "vrpn_Aimon.h"
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#if !defined(_WIN32)
#include <sys/ioctl.h>
#include <unistd.h>
#endif
#include <string.h>

typedef struct input_devinfo {
        vrpn_uint16 bustype;
        vrpn_uint16 vendor;
        vrpn_uint16 product;
        vrpn_uint16 version;
} XXX_should_have_been_in_system_includes;

// USB vendor and product IDs for the models we support
static const vrpn_uint16 vrpn_AIMON_VENDOR = 0x0866; //1133;
static const vrpn_uint16 vrpn_AIMON_PSELITE = 0x0001;

vrpn_Aimon::vrpn_Aimon(vrpn_HidAcceptor *filter,
                                   const char *name, vrpn_Connection *c)
  : vrpn_Button_Filter(name, c)
  , vrpn_Analog(name, c)
  , vrpn_HidInterface(_filter)
  , _filter(filter)
{
  vrpn_Analog::num_channel = 12;
  vrpn_Button::num_buttons = 9;

  // Initialize the state of all the analogs and buttons
  memset(buttons, 0, sizeof(buttons));
  memset(lastbuttons, 0, sizeof(lastbuttons));
  memset(channel, 0, sizeof(channel));
  memset(last, 0, sizeof(last));
}

vrpn_Aimon::~vrpn_Aimon()
{
        delete _filter;
}


void vrpn_Aimon::on_data_received(size_t bytes, vrpn_uint8 *buffer)
{
  decodePacket(bytes, buffer);
}

void vrpn_Aimon::mainloop()
{
	//vrpn_Analog::server_mainloop();
	//vrpn_Button::server_mainloop();

	update();
    server_mainloop();
    vrpn_gettimeofday(&_timestamp, NULL);
}

void vrpn_Aimon::report_changes(vrpn_uint32 class_of_service)
{
	vrpn_Analog::timestamp = _timestamp;
	vrpn_Button::timestamp = _timestamp;

	vrpn_Analog::report_changes(class_of_service);
	vrpn_Button::report_changes();
}

void vrpn_Aimon::report(vrpn_uint32 class_of_service)
{
	vrpn_Analog::timestamp = _timestamp;
	vrpn_Button::timestamp = _timestamp;

	vrpn_Analog::report(class_of_service);
	vrpn_Button::report_changes();
}

void vrpn_Aimon::decodePacket(size_t bytes, vrpn_uint8 *buffer)
{

	if (bytes < 49) {
		vrpn_gettimeofday(&_timestamp, NULL);
		send_text_message("Bad Packet length, expected 49 received less.", _timestamp, vrpn_TEXT_WARNING);
	}
	else {
		channel[0] = (float)(buffer[7] - 128) /-128.0;
		channel[1] = (float)(buffer[6] - 128) /128.0;
		channel[2] = (float)buffer[20]/256.0;
		channel[3] = (float)buffer[18]/256.0;

		channel[4] = (float)buffer[14]/256.0;
		channel[5] = (float)buffer[16]/256.0;
		channel[6] = (float)buffer[17]/256.0;
		channel[7] = (float)buffer[15]/256.0;

		channel[8] = (buffer[41] << 8) | buffer[42];
		channel[9] = (buffer[43] << 8) | buffer[44];
		channel[10] = (buffer[45] << 8) | buffer[46];
		channel[11] = (buffer[47] << 8) | buffer[48];

		buttons[0] = ( buffer[3] & (1<<2));
		buttons[1] = ( buffer[3] & (1));
		buttons[2] = ( buffer[2] & (1<<1));
		buttons[3] = ( buffer[2] & (1<<4));
		buttons[4] = ( buffer[2] & (1<<6));
		buttons[5] = ( buffer[2] & (1<<7));
		buttons[6] = ( buffer[2] & (1<<5));
		buttons[7] = ( buffer[2] & (1));
		buttons[8] = ( buffer[4] & (1));
	}
    report_changes();

}

vrpn_Aimon_Elite::vrpn_Aimon_Elite(const char *name, vrpn_Connection *c)
  : vrpn_Aimon(_filter = new vrpn_HidProductAcceptor(vrpn_AIMON_VENDOR, vrpn_AIMON_PSELITE), name, c)
{
}
