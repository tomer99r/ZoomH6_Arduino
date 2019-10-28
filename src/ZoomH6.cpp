#include "ZoomH6.h"
static const byte handshakeSeq[] = {0xC2, 0xE1, 0x31, 0x2E, 0x30, 0x30,0xA1,0x80,0x00 };
static const byte handshakeReply[] = {0x82, 0x83, 0x80, 0x81};

ZoomH6Remote::ZoomH6Remote(HardwareSerial* serial, Stream* debug, uint32_t timeout) :
    _serial(serial), _debug(debug), _timeout(timeout) {}

/*This will write the data to the serial, and return the expected number of replys
  in case of failure the result is negative */
int8_t ZoomH6Remote::writeWithReply(byte* data, size_t dataLength, byte* reply, size_t replyLength) {
  int8_t respCount = 0;

  for (int i = 0; i < dataLength; ++i) {
    _serial->write(data[i]);
  }

  uint32_t timeout = millis() + _timeout;
  do {
    if (_serial->available()) {
      byte resp = _serial->read();
      if (reply != nullptr) {
        reply[respCount % replyLength] = resp;
      }
      timeout = millis() + _timeout;
      respCount++;
    }

  } while (millis() <= timeout);//respCount < replyLength);

  return respCount;
}

bool ZoomH6Remote::initialize() {
    byte reply[3] = {0};

    if(*_serial) _serial->end();
    _serial->begin(2400, SERIAL_8N1);  // connection to Zoom.

    uint32_t timeout = millis() + 10000;
    while (!*_serial) {
      if(millis() > timeout) goto FAIL_TIME;
    }

    timeout = millis() + 10000;
    do {
        if(_serial->available()) {
            reply[0] = _serial->read();
        } else {
            _serial->write(0x00);
            delay(200);
        }
        if(millis() > timeout) return false;
    } while(reply[0] != 0x82);

    if (!writeWithReply(handshakeSeq, 1, reply, 1)) goto FAIL_TIME;
    if (reply[0] != handshakeReply[1]) goto FAIL_RES;

    if (!writeWithReply(&handshakeSeq[1], 5, reply, 1)) goto FAIL_TIME;
    if (reply[0] != handshakeReply[2]) goto FAIL_RES;

    if (!writeWithReply(&handshakeSeq[6], 1, reply, 1)) goto FAIL_TIME;
    if (reply[0] != handshakeReply[3]) goto FAIL_RES;

    writeWithReply(&handshakeSeq[7], 2, reply, 3); //clear status
    printStatus(reply[1], reply[2]);

    return true;
FAIL_TIME:
    if (_debug) _debug->println("Failed handshake - timeout");
    return false;
FAIL_RES:
    if (_debug) {
      for (int i = 0; i < 3; i++) {
        _debug->println(reply[i]);
      }
      _debug->println("Failed handshake - invalid return value");
    }
    return false;
}

void ZoomH6Remote::printStatus(byte& high, byte& low) {
  if (_debug) {
    if (high == 0x00 && low == 0x00) _debug->println("No Channel Record Ready!");

    if (high & StatusHigh::Record_on) _debug->println("Start Recording");
    if (high & StatusHigh::Channel_L_on) _debug->println("Left Channel Enabled");
    if (high & StatusHigh::Channel_R_on) _debug->println("Right Channel Enabled");
    if (high & StatusHigh::Channel_1_on) _debug->println("Channel 1 Enabled");

    if (low & StatusLow::Channel_2_on) _debug->println("Channel 2 Enabled");
    if (low & StatusLow::Channel_3_on) _debug->println("Channel 3 Enabled");
    if (low & StatusLow::Channel_4_on) _debug->println("Channel 4 Enabled");
  }
}

/* This method is used to transmit commands for the Zoom,
 *  primary - the primary command
 *  secondary - the sub command
 *  gap - the time to set between sending a command to releasing it (getting the status) in millis
 *  debug - verbose of result status to the debug serial
 */
bool ZoomH6Remote::sendCommand(byte primary, byte secondary, uint32_t gap, byte* reply) {
  byte command[2] = {primary, secondary};
  byte releaseCom[2] = {Commands::SecFunc, SubCommands::Release };

  if (!writeWithReply(command, 2, reply, 3)) return false;
  if (reply != nullptr) printStatus(reply[1], reply[2]);

  if (gap) delay(gap);

  if (!writeWithReply(releaseCom, 2, nullptr, 3)) return false;

  return true;
}
