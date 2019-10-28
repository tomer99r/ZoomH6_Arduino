#ifndef ZOOM_H6_H
#define ZOOM_H6_H
#include "Arduino.h"

enum Commands {
  Record        = 0x81,
  Play          = 0x82,
  SkipBackward  = 0x84,
  SkipForward   = 0x88,
  Stop          = 0x90,
  SecFunc       = 0x80,
};

enum SubCommands {
  Release     = 0x00,
  Channel_L   = 0x01,
  Channel_R   = 0x02,
  Channel_1   = 0x04,
  Channel_2   = 0x08,
  Channel_3   = 0x10,
  Channel_4   = 0x20,
  VolumeDown  = 0x40,
  VolumeUp    = 0x80,
};

enum StatusHigh {
  Record_on     = 0x01,
  Channel_L_on  = 0x20,
  Channel_R_on  = 0x08,
  Channel_1_on  = 0x02,
};

enum StatusLow {
  Channel_2_on  = 0x20,
  Channel_3_on  = 0x08,
  Channel_4_on  = 0x02,
};
class ZoomH6Remote {
 public:
  ZoomH6Remote(HardwareSerial* serial, Stream* debug = nullptr, uint32_t timeout = 40);
  ~ZoomH6Remote() = default;

  bool initialize();
  bool sendCommand(byte primary, byte secondary, uint32_t gap, byte* reply = nullptr);

 private:
  int8_t writeWithReply(byte* data, size_t dataLength, byte* reply, size_t replyLength);
  void printStatus(byte& high, byte& low);

  Stream* _debug;
  uint32_t _timeout;
  HardwareSerial* _serial;
};

#endif
