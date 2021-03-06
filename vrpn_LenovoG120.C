// vrpn_LenovoG120.C: VRPN driver for Lenovo G120 wireless joystick/mouse


// There is a non-HID Linux-based driver for this device that has a capability
// not implemented in the HID interface.  It uses the input.h interface.
#if defined(linux) && !defined(VRPN_USE_HID)
#include <linux/input.h>
#endif

#include "vrpn_LenovoG120.h"
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
static const vrpn_uint16 vrpn_LenovoG120_VENDOR = 6127;
static const vrpn_uint16 vrpn_LenovoG120_PRODUCT = 24586;

vrpn_LenovoG120::vrpn_LenovoG120(const char *name, vrpn_Connection *c)
  : 
#if defined(VRPN_USE_HID)
   vrpn_HidInterface(_filter = new vrpn_HidNthMatchAcceptor(1, new vrpn_HidProductAcceptor(vrpn_LenovoG120_VENDOR, vrpn_LenovoG120_PRODUCT))),
#endif
  vrpn_Analog(name, c)
  , vrpn_Button(name, c)
{
  vrpn_Analog::num_channel = 2;
  vrpn_Button::num_buttons = 10;

  // Initialize the state of all the analogs and buttons
  memset(buttons, 0, sizeof(buttons));
  memset(lastbuttons, 0, sizeof(lastbuttons));
  memset(channel, 0, sizeof(channel));
  memset(last, 0, sizeof(last));

// There is a non-HID Linux-based driver for this device that has a capability
// not implemented in the HID interface.  It is implemented using the Event
// interface.
#if defined(linux) && !defined(VRPN_USE_HID)
  // Use the Event interface to open devices looking for the one
  // we want.  Call the acceptor with all the devices we find
  // until we get one that we want.
    fd = -1;
    FILE *f;
    int i = 0;

    // try to autodetect the device
    char *fname = (char *)malloc(1000*sizeof(char));
    while(i < 256) {
        sprintf(fname, "/dev/input/event%d", i++);
        f = fopen(fname, "r+b");
        if(f) {
          // We got an active device.  Fill in its values and see if it
          // is acceptable to the filter.
          struct input_devinfo ID;
          ioctl(fileno(f), EVIOCGID, &ID);
          vrpn_HIDDEVINFO info;
          info.product = ID.product;
          info.vendor = ID.vendor;
          if (_filter->accept(info)) {
            fd = fileno(f);
            set_led(1);
            break;
          } else {
            fclose(f);
          }
        }
    }

    if(!f) {
        perror("Could not open the device");
        exit(1);
    }

    free(fname);

    // turn the LED on
    set_led(1);
#endif
}

vrpn_LenovoG120::~vrpn_LenovoG120()
{
#if defined(linux) && !defined(VRPN_USE_HID)
	set_led(0);
#endif
        delete _filter;
}

#if defined(VRPN_USE_HID)

void vrpn_LenovoG120::on_data_received(size_t bytes, vrpn_uint8 *buffer)
{
  decodePacket(bytes, buffer);
}
#endif

void vrpn_LenovoG120::mainloop()
{
#if defined(VRPN_USE_HID)
	// Full reports are 7 bytes long.
	// XXX If we get a 2-byte report mixed in, then something is going to get
	// truncated.
	update();
#elif defined(linux) && !defined(VRPN_USE_HID)
    struct timeval zerotime;
    fd_set fdset;
    struct input_event ev;
    int i;

    zerotime.tv_sec = 0;
    zerotime.tv_usec = 0;

    FD_ZERO(&fdset);                      /* clear fdset              */
    FD_SET(fd, &fdset);                   /* include fd in fdset      */
    int moreData = 0;
    do {
        vrpn_noint_select(fd + 1, &fdset, NULL, NULL, &zerotime);
        moreData = 0;
        if (FD_ISSET(fd, &fdset)) {
            moreData = 1;
            if (vrpn_noint_block_read(fd, reinterpret_cast<char*>(&ev), sizeof(struct input_event)) != sizeof(struct input_event)) {
                send_text_message("Error reading from vrpn_LenovoG120", vrpn_Analog::timestamp, vrpn_TEXT_ERROR);
                if (d_connection) { d_connection->send_pending_reports(); }
                return;
            }
            switch (ev.type) {
                case EV_KEY:    // button movement
                    vrpn_gettimeofday((timeval *)&this->vrpn_Button::timestamp, NULL);
                    buttons[ev.code & 0x0ff] = ev.value;
                    break;
 
                case EV_REL:    // axis movement
                case EV_ABS:    // new kernels send more logical _ABS instead of _REL
                    vrpn_gettimeofday((timeval *)&this->vrpn_Analog::timestamp, NULL);
                    // Convert from short to int to avoid a short/double conversion
                    // bug in GCC 3.2.
                    i = ev.value;
                    channel[ev.code] = static_cast<double>(i)/400.0;           
                    break;
 
                default:
                    break;
            }
        }
        report_changes();
    } while (moreData == 1);
#endif

    server_mainloop();
    vrpn_gettimeofday(&_timestamp, NULL);
}

void vrpn_LenovoG120::report_changes(vrpn_uint32 class_of_service)
{
	vrpn_Analog::timestamp = _timestamp;
	vrpn_Button::timestamp = _timestamp;

	vrpn_Analog::report_changes(class_of_service);
	vrpn_Button::report_changes();
}

void vrpn_LenovoG120::report(vrpn_uint32 class_of_service)
{
	vrpn_Analog::timestamp = _timestamp;
	vrpn_Button::timestamp = _timestamp;

	vrpn_Analog::report(class_of_service);
	vrpn_Button::report_changes();
}

#if defined(linux) && !defined(VRPN_USE_HID)
int vrpn_LenovoG120::set_led(int led_state)
{
  struct input_event event;
  int ret;

  event.type  = EV_LED;
  event.code  = LED_MISC;
  event.value = led_state;

  ret = write(fd, &event, sizeof(struct input_event));
  if (ret < 0) {
    perror ("setting led state failed");
  }
  return ret < sizeof(struct input_event);
}
#endif

// Swap the endian-ness of the 2-byte entry in the buffer.
// This is used to make the little-endian int 16 values
// returned by the device into the big-endian format that is
// expected by the VRPN unbuffer routines.

static void swap_endian2(char *buffer)
{
	char c;
	c = buffer[0]; buffer[0] = buffer[1]; buffer[1] = c;
}

#if defined(VRPN_USE_HID)
void vrpn_LenovoG120::decodePacket(size_t bytes, vrpn_uint8 *buffer)
{
	int btn;
	//float valuator[2];
	for (btn = 0; btn < 6; btn++) {
		vrpn_uint8 mask;
		mask = 1 << (btn % 8);
		buttons[btn] = ( buffer[17] & mask) != 0;
	}
	for (btn = 0; btn < 4; btn++) {
		vrpn_uint8 mask;
		mask = 1 << (btn % 8);
		buttons[btn+6] = ( buffer[18] & mask) != 0;
	}

	channel[0] = (((float)buffer[13]-58.0)/70.0)-1.0;
	channel[1] = (((float)buffer[14]-58.0)/-70.0)+1.0;

	report_changes();

		//printf("Buttons: %d %d %d %d %d %d | %d %d %d %d\n", button[0], button[1], button[2], 
		//	button[3], button[4], button[5], button[6], button[7], button[8], button[9]);
		//printf("Valuators: %f %f\n", valuator[0], valuator[1]);

  // Decode all full reports.
  // Full reports for all of the pro devices are 7 bytes long (the first
  // byte is the report type, because this device has multiple ones the
  // HIDAPI library leaves it in the report).
/*  for (size_t i = 0; i < bytes / 7; i++) {
    vrpn_uint8 *report = buffer + (i * 7);

    // There are three types of reports.  Parse whichever type
    // this is.
    char  report_type = report[0];
    char *bufptr = static_cast<char *>(static_cast<void*>(&report[1]));
    vrpn_int16 temp;
    const float scale = static_cast<float>(1.0/400.0);
    switch (report_type)  {
      case 1:
        swap_endian2(bufptr); vrpn_unbuffer(const_cast<const char **>(&bufptr), &temp);
        channel[0] = temp * scale;
        if (channel[0] < -1.0) { channel[0] = -1.0; }
        if (channel[0] > 1.0) { channel[0] = 1.0; }
        swap_endian2(bufptr); vrpn_unbuffer(const_cast<const char **>(&bufptr), &temp);
        channel[1] = temp * scale;
        if (channel[1] < -1.0) { channel[1] = -1.0; }
        if (channel[1] > 1.0) { channel[1] = 1.0; }
        swap_endian2(bufptr); vrpn_unbuffer(const_cast<const char **>(&bufptr), &temp);
        channel[2] = temp * scale;
        if (channel[2] < -1.0) { channel[2] = -1.0; }
        if (channel[2] > 1.0) { channel[2] = 1.0; }
        break;

      case 2:
        swap_endian2(bufptr); vrpn_unbuffer(const_cast<const char **>(&bufptr), &temp);
        channel[3] = temp * scale;
        if (channel[3] < -1.0) { channel[3] = -1.0; }
        if (channel[3] > 1.0) { channel[3] = 1.0; }
        swap_endian2(bufptr); vrpn_unbuffer(const_cast<const char **>(&bufptr), &temp);
        channel[4] = temp * scale;
        if (channel[4] < -1.0) { channel[4] = -1.0; }
        if (channel[4] > 1.0) { channel[4] = 1.0; }
        swap_endian2(bufptr); vrpn_unbuffer(const_cast<const char **>(&bufptr), &temp);
        channel[5] = temp * scale;
        if (channel[5] < -1.0) { channel[5] = -1.0; }
        if (channel[5] > 1.0) { channel[5] = 1.0; }
        break;

      case 3: { // Button report
        int btn;

        for (btn = 0; btn < vrpn_Button::num_buttons; btn++) {
            vrpn_uint8 *location, mask;
            location = report + 1 + (btn / 8);
            mask = 1 << (btn % 8);
            buttons[btn] = ( (*location) & mask) != 0;
        }
        break;
      }

      default:
        vrpn_gettimeofday(&_timestamp, NULL);
        send_text_message("Unknown report type", _timestamp, vrpn_TEXT_WARNING);
    }
    // Report this event before parsing the next.
    report_changes();
  } */
}
#endif
