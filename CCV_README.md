To add support for the aimon controller, 2 files were added with HID code
to partially support the Aimon joystick:

  vrpn_Aimon.C
  vrpn_Aimon.h

These files were copied from one of the other HID devices as a template.  In
addition, the following code needed to be modified to support the new files
added:

  CMakeLists.txt
  server_src/vrpn_Generic_server_object.C
  server_src/vrpn_Generic_server_object.h
  server_src/vrpn.cfg
