/*!
 * @file Adafruit_Fingerprint.cpp
 *
 * @mainpage Adafruit Fingerprint Sensor Library
 *
 * @section intro_sec Introduction
 *
 * This is a library for our optical Fingerprint sensor
 *
 * Designed specifically to work with the Adafruit Fingerprint sensor
 * ----> http://www.adafruit.com/products/751
 *
 * These displays use TTL Serial to communicate, 2 pins are required to
 * interface
 * Adafruit invests time and resources providing this open source code,
 * please support Adafruit and open-source hardware by purchasing
 * products from Adafruit!
 *
 * @section author Author
 *
 * Written by Limor Fried/Ladyada for Adafruit Industries.
 *
 * @section license License
 *
 * BSD license, all text above must be included in any redistribution
 *
 */

#include "Adafruit_Fingerprint.h"

//#define FINGERPRINT_DEBUG

/*!
 * @brief Gets the command packet
 */
#define GET_CMD_PACKET(...)                                                    \
  uint8_t data[] = {__VA_ARGS__};                                              \
  Adafruit_Fingerprint_Packet packet(FINGERPRINT_COMMANDPACKET, sizeof(data),  \
                                     data);                                    \
  writeStructuredPacket(packet);                                               \
  uint16_t timeout = data[0] ==                                                \
    FINGERPRINT_REGMODEL ? TIMEOUT_CREATEMODEL : DEFAULTTIMEOUT;               \
  if (getStructuredPacket(&packet, timeout) != FINGERPRINT_OK)                 \
    return FINGERPRINT_PACKETRECIEVEERR;                                       \
  if (packet.type != FINGERPRINT_ACKPACKET)                                    \
    return FINGERPRINT_PACKETRECIEVEERR;

/*!
 * @brief Sends the command packet
 */
#define SEND_CMD_PACKET(...)                                                   \
  GET_CMD_PACKET(__VA_ARGS__);                                                 \
  return packet.data[0];

/***************************************************************************
 PUBLIC FUNCTIONS
 ***************************************************************************/

#if defined(__AVR__) || defined(ESP8266) || defined(FREEDOM_E300_HIFIVE1)
/**************************************************************************/
/*!
    @brief  Instantiates sensor with Software Serial
    @param  ss Pointer to SoftwareSerial object
    @param  password 32-bit integer password (default is 0)
*/
/**************************************************************************/
Adafruit_Fingerprint::Adafruit_Fingerprint(SoftwareSerial *ss,
                                           uint32_t password) {
  thePassword = password;
  theAddress = 0xFFFFFFFF;

  hwSerial = NULL;
  swSerial = ss;
  mySerial = swSerial;
}
#endif

#ifdef ARDUINO_ARCH_RP2040
/**************************************************************************/
/*!
    @brief  Instantiates sensor with SerialUART
    @param  hs Pointer to SerialUART object
    @param  password 32-bit integer password (default is 0)

*/
/**************************************************************************/
Adafruit_Fingerprint::Adafruit_Fingerprint(SerialUART *hs,
                                           uint32_t password) {
  thePassword = password;
  theAddress = 0xFFFFFFFF;

#if defined(__AVR__) || defined(ESP8266) || defined(FREEDOM_E300_HIFIVE1)
  swSerial = NULL;
#endif
  uartSerial = hs;
  mySerial = uartSerial;
}
#else
/**************************************************************************/
/*!
    @brief  Instantiates sensor with Hardware Serial
    @param  hs Pointer to HardwareSerial object
    @param  password 32-bit integer password (default is 0)

*/
/**************************************************************************/
Adafruit_Fingerprint::Adafruit_Fingerprint(HardwareSerial *hs,
                                           uint32_t password) {
  thePassword = password;
  theAddress = 0xFFFFFFFF;

#if defined(__AVR__) || defined(ESP8266) || defined(FREEDOM_E300_HIFIVE1)
  swSerial = NULL;
#endif
  hwSerial = hs;
  mySerial = hwSerial;
}
#endif


/**************************************************************************/
/*!
    @brief  Instantiates sensor with a stream for Serial
    @param  serial Pointer to a Stream object
    @param  password 32-bit integer password (default is 0)

*/
/**************************************************************************/

Adafruit_Fingerprint::Adafruit_Fingerprint(Stream *serial, uint32_t password) {

  thePassword = password;
  theAddress = 0xFFFFFFFF;

  hwSerial = NULL;
#if defined(__AVR__) || defined(ESP8266) || defined(FREEDOM_E300_HIFIVE1)
  swSerial = NULL;
#endif
  mySerial = serial;
}

/**************************************************************************/
/*!
    @brief  Initializes serial interface and baud rate
    @param  baudrate Sensor's UART baud rate (usually 57600, 9600 or 115200)
*/
/**************************************************************************/
void Adafruit_Fingerprint::begin(uint32_t baudrate, int8_t rxPin, int8_t txPin) {
  delay(1000); // one second delay to let the sensor 'boot up'

#if defined(ARDUINO_ARCH_RP2040)
  if (uartSerial) {
    uartSerial->setRX(rxPin);
    uartSerial->setTX(txPin);
    uartSerial->begin(baudrate);
  }
#else
  if (hwSerial)
    hwSerial->begin(baudrate, 134217756U, rxPin, txPin);
#endif
#if defined(__AVR__) || defined(ESP8266) || defined(FREEDOM_E300_HIFIVE1)
  if (swSerial)
    swSerial->begin(baudrate);
#endif
}

/**************************************************************************/
/*!
    @brief  Closes serial interface
*/
/**************************************************************************/
void Adafruit_Fingerprint::close() {
#if defined(ARDUINO_ARCH_RP2040)
  if (uartSerial) {
    uartSerial->end();
  }
#else
  if (hwSerial)
    hwSerial->end();
#endif
#if defined(__AVR__) || defined(ESP8266) || defined(FREEDOM_E300_HIFIVE1)
  if (swSerial)
    swSerial->end();
#endif
}

/**************************************************************************/
/*!
    @brief  Verifies the sensors' access password (default password is
   0x0000000). A good way to also check if the sensors is active and responding
    @returns True if password is correct
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::verifyPassword(void) {
  uint8_t p = checkPassword();
  if (p == FINGERPRINT_OK) {   
    return getParameters(); // properly configure the object by reading system parameters
  }
  return p;
}

/**************************************************************************/
/*!
    @brief  Verifies the sensors' access password (default password is
   0x0000000). A good way to also check if the sensors is active and responding
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
    @returns <code>FINGERPRINT_PASSFAIL</code> on wrong password
*/
/**************************************************************************/

uint8_t Adafruit_Fingerprint::checkPassword(void) {
  GET_CMD_PACKET(FINGERPRINT_VERIFYPASSWORD, (uint8_t)(thePassword >> 24),
                 (uint8_t)(thePassword >> 16), (uint8_t)(thePassword >> 8),
                 (uint8_t)(thePassword & 0xFF));
  return packet.data[0];
}

/**************************************************************************/
/*!
    @brief  Get the sensors parameters, fills in the member variables
    status_reg, system_id, capacity, security_level, device_addr, packet_len
    and baud_rate
    @returns True if password is correct
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::getParameters(void) {
  GET_CMD_PACKET(FINGERPRINT_READSYSPARAM);

  status_reg = ((uint16_t)packet.data[1] << 8) | packet.data[2];
  system_id = ((uint16_t)packet.data[3] << 8) | packet.data[4];
  capacity = ((uint16_t)packet.data[5] << 8) | packet.data[6];
  security_level = ((uint16_t)packet.data[7] << 8) | packet.data[8];
  device_addr = ((uint32_t)packet.data[9] << 24) |
                ((uint32_t)packet.data[10] << 16) |
                ((uint32_t)packet.data[11] << 8) | (uint32_t)packet.data[12];
  packet_len = ((uint16_t)packet.data[13] << 8) | packet.data[14];
  if (packet_len == 0) {
    packet_len = 32;
  } else if (packet_len == 1) {
    packet_len = 64;
  } else if (packet_len == 2) {
    packet_len = 128;
  } else if (packet_len == 3) {
    packet_len = 256;
  }
  baud_rate = (((uint16_t)packet.data[15] << 8) | packet.data[16]) * 9600;

  return packet.data[0];
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to take an image of the finger pressed on surface
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_NOFINGER</code> if no finger detected
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
    @returns <code>FINGERPRINT_IMAGEFAIL</code> on imaging error
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::getImage(void) {
  SEND_CMD_PACKET(FINGERPRINT_GETIMAGE);
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to convert image to feature template
    @param slot Location to place feature template (put one in 1 and another in
   2 for verification to create model)
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_IMAGEMESS</code> if image is too messy
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
    @returns <code>FINGERPRINT_FEATUREFAIL</code> on failure to identify
   fingerprint features
    @returns <code>FINGERPRINT_INVALIDIMAGE</code> on failure to identify
   fingerprint features
*/
uint8_t Adafruit_Fingerprint::image2Tz(uint8_t slot) {
  SEND_CMD_PACKET(FINGERPRINT_IMAGE2TZ, slot);
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to take two print feature template and create a
   model
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
    @returns <code>FINGERPRINT_ENROLLMISMATCH</code> on mismatch of fingerprints
*/
uint8_t Adafruit_Fingerprint::createModel(void) {
  SEND_CMD_PACKET(FINGERPRINT_REGMODEL);
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to store the calculated model for later matching
    @param   location The model location #
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
    @returns <code>FINGERPRINT_FLASHERR</code> if the model couldn't be written
   to flash memory
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
uint8_t Adafruit_Fingerprint::storeModel(uint16_t location) {
  SEND_CMD_PACKET(FINGERPRINT_STORE, 0x01, (uint8_t)(location >> 8),
                  (uint8_t)(location & 0xFF));
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to load a fingerprint model from flash into buffer 1
    @param   location The model location #
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
uint8_t Adafruit_Fingerprint::loadModel(uint16_t location) {
  SEND_CMD_PACKET(FINGERPRINT_LOAD, 0x01, (uint8_t)(location >> 8),
                  (uint8_t)(location & 0xFF));
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to transfer 256-byte fingerprint template from the
   buffer to the UART
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
uint8_t Adafruit_Fingerprint::getModel(void) {
  SEND_CMD_PACKET(FINGERPRINT_UPLOAD, 0x01);
}

/**************************************************************************/
/*!
    @brief   read template data from the sensor
    @returns a buffer where the template's bufsize bytes are stored
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::get_template_buffer(int bufsize, uint8_t ref_buf[])
{
  if (getModel() != FINGERPRINT_OK) {
    Serial.println("getModel failed");
    return false; // check if buffer 1 is ready to be retrieved
  }

  int div = ceil(bufsize / packet_len);
  int rcv_bt_len = (packet_len + 11) * div; // data packet contains 11 extra bytes(first->2-header,4-address,1-type,2-length,last->2-checksum) excpet the main data
  uint8_t bytesReceived[rcv_bt_len];
  memset(bytesReceived, 0xff, rcv_bt_len);
  uint32_t starttime = millis() == 0 ? 1 : millis();
  int i = 0;
  while (i < rcv_bt_len && (millis() - starttime) < 5000)
  {
    if (mySerial->available())
    {
      starttime = millis() == 0 ? 1 : millis();
      bytesReceived[i++] = mySerial->read();
    }
  }
  if (i != rcv_bt_len)
    return FINGERPRINT_TIMEOUT;
  memset(ref_buf, 0xff, bufsize);
  for (int m = 0; m < div; m++)
  { // filtering data packets
    uint8_t stat = bytesReceived[(m * (packet_len + 11)) + 6];
    if (stat != FINGERPRINT_DATAPACKET && stat != FINGERPRINT_ENDDATAPACKET)
      return FINGERPRINT_BADPACKET;
    memcpy(ref_buf + (m * packet_len), bytesReceived + (m * (packet_len + 11)) + 9, packet_len);
  }
  return FINGERPRINT_OK;
}

/**************************************************************************/
/*!
    @brief   makes the buffer ready to download from upper computer
    @returns status
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::downloadModel(uint8_t buffer_no) {
  SEND_CMD_PACKET(FINGERPRINT_DOWNLOAD, buffer_no);
}

/**************************************************************************/
/*!
    @brief   tries to write the template data to sensor buffer 1
    @returns true/false (successful or not)
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::write_template_to_sensor(int temp_Size, const uint8_t ref_buf[])
{
  uint8_t p = downloadModel(0x01);
  if (p != FINGERPRINT_OK) {
    Serial.println("downloadModel failed");
    return p; // check if buffer 1 is ready to be loaded
  }

  //delay(2000);

  int div = ceil(temp_Size / packet_len);
  uint8_t data[packet_len];
  // memset(data, 0xff, packet_len);
  Adafruit_Fingerprint_Packet t_packet(FINGERPRINT_DATAPACKET, packet_len, data);
  for (int i = 0; i < div; i++) {
    if (i == (div - 1))
      t_packet.type = FINGERPRINT_ENDDATAPACKET;

    memcpy(t_packet.data, ref_buf + (packet_len * i), packet_len);
    writeStructuredPacket(t_packet);

    //delay(1000);
  }

  return FINGERPRINT_OK;
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to delete a model in memory
    @param   location The model location #
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
    @returns <code>FINGERPRINT_FLASHERR</code> if the model couldn't be written
   to flash memory
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
uint8_t Adafruit_Fingerprint::deleteModel(uint16_t location) {
  SEND_CMD_PACKET(FINGERPRINT_DELETE, (uint8_t)(location >> 8),
                  (uint8_t)(location & 0xFF), 0x00, 0x01);
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to delete ALL models in memory
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_BADLOCATION</code> if the location is invalid
    @returns <code>FINGERPRINT_FLASHERR</code> if the model couldn't be written
   to flash memory
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
uint8_t Adafruit_Fingerprint::emptyDatabase(void) {
  SEND_CMD_PACKET(FINGERPRINT_EMPTY);
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to search the current slot 1 fingerprint features to
   match saved templates. The matching location is stored in <b>fingerID</b> and
   the matching confidence in <b>confidence</b>
    @returns <code>FINGERPRINT_OK</code> on fingerprint match success
    @returns <code>FINGERPRINT_NOTFOUND</code> no match made
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::fingerFastSearch(void) {
  // high speed search of slot #1 starting at page 0x0000 and page #0x00A3
  GET_CMD_PACKET(FINGERPRINT_HISPEEDSEARCH, 0x01, 0x00, 0x00, 0x00, 0xA3);
  fingerID = 0xFFFF;
  confidence = 0xFFFF;

  fingerID = packet.data[1];
  fingerID <<= 8;
  fingerID |= packet.data[2];

  confidence = packet.data[3];
  confidence <<= 8;
  confidence |= packet.data[4];

  return packet.data[0];
}

/**************************************************************************/
/*!
    @brief   Check whether the sensor is normal
    @returns <code>FINGERPRINT_OK</code> if state is normal
    @returns <code>FINGERPRINT_ABNORMAL</code> if state is abnormal
*/
uint8_t Adafruit_Fingerprint::checkSensor(void) {
  SEND_CMD_PACKET(FINGERPRINT_CHECKSENSOR);
}

/**************************************************************************/
/*!
    @brief   Control the built in LED
    @param on True if you want LED on, False to turn LED off
    @returns <code>FINGERPRINT_OK</code> on success
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::LEDcontrol(bool on) {
  if (on) {
    SEND_CMD_PACKET(FINGERPRINT_LEDON);
  } else {
    SEND_CMD_PACKET(FINGERPRINT_LEDOFF);
  }
}

/**************************************************************************/
/*!
    @brief   Control the built in Aura LED (if exists). Check datasheet/manual
    for different colors and control codes available
    @param control The control code (e.g. breathing, full on)
    @param speed How fast to go through the breathing/blinking cycles
    @param coloridx What color to light the indicator
    @param count How many repeats of blinks/breathing cycles
    @returns <code>FINGERPRINT_OK</code> on fingerprint match success
    @returns <code>FINGERPRINT_NOTFOUND</code> no match made
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::LEDcontrol(uint8_t control, uint8_t speed,
                                         uint8_t coloridx, uint8_t count) {
  SEND_CMD_PACKET(FINGERPRINT_AURALEDCONFIG, control, speed, coloridx, count);
}

/**************************************************************************/
/*!
    @brief   Ask the sensor to search the current slot fingerprint features to
   match saved templates. The matching location is stored in <b>fingerID</b> and
   the matching confidence in <b>confidence</b>
   @param slot The slot to use for the print search, defaults to 1
    @returns <code>FINGERPRINT_OK</code> on fingerprint match success
    @returns <code>FINGERPRINT_NOTFOUND</code> no match made
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::fingerSearch(uint8_t slot) {
  // search of slot starting thru the capacity
  GET_CMD_PACKET(FINGERPRINT_SEARCH, slot, 0x00, 0x00, (uint8_t)(capacity >> 8),
                 (uint8_t)(capacity & 0xFF));

  fingerID = 0xFFFF;
  confidence = 0xFFFF;

  fingerID = packet.data[1];
  fingerID <<= 8;
  fingerID |= packet.data[2];

  confidence = packet.data[3];
  confidence <<= 8;
  confidence |= packet.data[4];

  return packet.data[0];
}

/**************************************************************************/
/*!
    @brief   Ask the sensor for the number of templates stored in memory. The
   number is stored in <b>templateCount</b> on success.
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::getTemplateCount(void) {
  GET_CMD_PACKET(FINGERPRINT_TEMPLATECOUNT);

  templateCount = packet.data[1];
  templateCount <<= 8;
  templateCount |= packet.data[2];

  return packet.data[0];
}

/**************************************************************************/
/*!
    @brief   read template index table from the sensor. The
   template indices are stored in <b>templates</b> on success.
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::getTemplateIndices() {
  uint8_t p = getParameters();
  if (p != FINGERPRINT_OK)
    return p;

  p = getTemplateCount();
  if (p != FINGERPRINT_OK)
    return p;

  // clear template indices
  memset(templates, 0, sizeof(templates));
  
  //Serial.printf("capacity:%d,", capacity);
  //uint8_t t = ceil(capacity / (double)256);
  //Serial.printf("t:%d,", t);

  uint16_t count = 0;
  for (uint8_t j = 0; j < ceil(capacity / (double)256); ++j) {
    //Serial.printf("for-j:%d,", j);
    GET_CMD_PACKET(FINGERPRINT_TEMPLATEREAD, j);

    if (packet.data[0] == FINGERPRINT_OK) {
      for (uint8_t i = 0; i < 32; ++i) {
        uint8_t byte = packet.data[i + 1];
        //Serial.printf("byte%d,", byte);
        for (uint8_t bit = 0; bit < 8; ++bit) {
          if (bitRead(byte, bit)) { //if (byte & (1 << bit)) {
            templates[count] = (i * 8) + bit + (j * 256);
            //Serial.printf("bit%d,", templates[count]);
            count++;
          }
        }
      }
    } else {
      return FINGERPRINT_PACKETRECIEVEERR;
    }
  }

  return FINGERPRINT_OK;
}

uint8_t Adafruit_Fingerprint::writeNotepad(uint8_t page, uint8_t content[]) {
  SEND_CMD_PACKET(FINGERPRINT_WRITENOTEPAD, page,
    content[0], content[1], content[2], content[3], content[4], content[5], content[6], content[7],
    content[8], content[9], content[10], content[11], content[12], content[13], content[14], content[15],
    content[16], content[17], content[18], content[19], content[20], content[21], content[22], content[23],
    content[24], content[25], content[26], content[27], content[28], content[29], content[30], content[31]);
}

uint8_t Adafruit_Fingerprint::readNotepad(uint8_t page, uint8_t content[]) {
  GET_CMD_PACKET(FINGERPRINT_READNOTEPAD, page);

  if (packet.data[0] == FINGERPRINT_OK) {
    for (uint8_t i = 0; i < 32; ++i) {
      content[i] = packet.data[i + 1];
    }
    return FINGERPRINT_OK;
  } else {
    return FINGERPRINT_PACKETRECIEVEERR;
  }
}

/**************************************************************************/
/*!
    @brief   Set the password on the sensor (future communication will require
   password verification so don't forget it!!!)
    @param   password 32-bit password code
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::setPassword(uint32_t password) {
  SEND_CMD_PACKET(FINGERPRINT_SETPASSWORD, (uint8_t)(password >> 24),
                  (uint8_t)(password >> 16), (uint8_t)(password >> 8),
                  (uint8_t)(password & 0xFF));
}

/**************************************************************************/
/*!
    @brief   Writing module registers
    @param   regAdd 8-bit address of register
    @param   value 8-bit value will write to register
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
    @returns <code>FINGERPRINT_ADDRESS_ERROR</code> on register address error
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::writeRegister(uint8_t regAdd, uint8_t value) {

  SEND_CMD_PACKET(FINGERPRINT_WRITE_REG, regAdd, value);
}

/**************************************************************************/
/*!
    @brief   Change UART baudrate
    @param   baudrate 8-bit Uart baudrate
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::setBaudRate(uint8_t baudrate) {

  return (writeRegister(FINGERPRINT_BAUD_REG_ADDR, baudrate));
}

/**************************************************************************/
/*!
    @brief   Change security level
    @param   level 8-bit security level
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::setSecurityLevel(uint8_t level) {

  return (writeRegister(FINGERPRINT_SECURITY_REG_ADDR, level));
}

/**************************************************************************/
/*!
    @brief   Change packet size
    @param   size 8-bit packet size
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_PACKETRECIEVEERR</code> on communication error
*/
/**************************************************************************/
uint8_t Adafruit_Fingerprint::setPacketSize(uint8_t size) {

  return (writeRegister(FINGERPRINT_PACKET_REG_ADDR, size));
}
/**************************************************************************/
/*!
    @brief   Helper function to process a packet and send it over UART to the
   sensor
    @param   packet A structure containing the bytes to transmit
*/
/**************************************************************************/

void Adafruit_Fingerprint::writeStructuredPacket(
    const Adafruit_Fingerprint_Packet &packet) {

  mySerial->write((uint8_t)(packet.start_code >> 8));
  mySerial->write((uint8_t)(packet.start_code & 0xFF));
  mySerial->write(packet.address[0]);
  mySerial->write(packet.address[1]);
  mySerial->write(packet.address[2]);
  mySerial->write(packet.address[3]);
  mySerial->write(packet.type);

  uint16_t wire_length = packet.length + 2;
  mySerial->write((uint8_t)(wire_length >> 8));
  mySerial->write((uint8_t)(wire_length & 0xFF));

#ifdef FINGERPRINT_DEBUG
  Serial.print("-> 0x");
  Serial.print((uint8_t)(packet.start_code >> 8), HEX);
  Serial.print(", 0x");
  Serial.print((uint8_t)(packet.start_code & 0xFF), HEX);
  Serial.print(", 0x");
  Serial.print(packet.address[0], HEX);
  Serial.print(", 0x");
  Serial.print(packet.address[1], HEX);
  Serial.print(", 0x");
  Serial.print(packet.address[2], HEX);
  Serial.print(", 0x");
  Serial.print(packet.address[3], HEX);
  Serial.print(", 0x");
  Serial.print(packet.type, HEX);
  Serial.print(", 0x");
  Serial.print((uint8_t)(wire_length >> 8), HEX);
  Serial.print(", 0x");
  Serial.print((uint8_t)(wire_length & 0xFF), HEX);
#endif

  uint16_t sum = ((wire_length) >> 8) + ((wire_length)&0xFF) + packet.type;
  for (uint8_t i = 0; i < packet.length; i++) {
    mySerial->write(packet.data[i]);
    sum += packet.data[i];
#ifdef FINGERPRINT_DEBUG
    Serial.print(", 0x");
    Serial.print(packet.data[i], HEX);
#endif
  }

  mySerial->write((uint8_t)(sum >> 8));
  mySerial->write((uint8_t)(sum & 0xFF));

#ifdef FINGERPRINT_DEBUG
  Serial.print(", 0x");
  Serial.print((uint8_t)(sum >> 8), HEX);
  Serial.print(", 0x");
  Serial.println((uint8_t)(sum & 0xFF), HEX);
#endif

  return;
}

/**************************************************************************/
/*!
    @brief   Helper function to receive data over UART from the sensor and
   process it into a packet
    @param   packet A structure containing the bytes received
    @param   timeout how many milliseconds we're willing to wait
    @returns <code>FINGERPRINT_OK</code> on success
    @returns <code>FINGERPRINT_TIMEOUT</code> or
   <code>FINGERPRINT_BADPACKET</code> on failure
*/
/**************************************************************************/
uint8_t
Adafruit_Fingerprint::getStructuredPacket(Adafruit_Fingerprint_Packet *packet,
                                          uint16_t timeout) {
  uint8_t byte;
  uint16_t idx = 0, timer = 0;

#ifdef FINGERPRINT_DEBUG
  Serial.print("max. ");
  Serial.print(timeout);
  Serial.print(" ms: <- ");
#endif

  while (true) {
    while (!mySerial->available()) {
      delay(1);
      timer++;
      if (timer >= timeout) {
#ifdef FINGERPRINT_DEBUG
        Serial.println("Timed out");
#endif
        return FINGERPRINT_TIMEOUT;
      }
    }
#ifdef FINGERPRINT_DEBUG
    Serial.printf("(%u ms)", timer);
#endif
    byte = mySerial->read();
#ifdef FINGERPRINT_DEBUG
    Serial.print("0x");
    Serial.print(byte, HEX);
    Serial.print(", ");
#endif
    switch (idx) {
    case 0:
      if (byte != (FINGERPRINT_STARTCODE >> 8))
        continue;
      packet->start_code = (uint16_t)byte << 8;
      break;
    case 1:
      packet->start_code |= byte;
      if (packet->start_code != FINGERPRINT_STARTCODE)
        return FINGERPRINT_BADPACKET;
      break;
    case 2:
    case 3:
    case 4:
    case 5:
      packet->address[idx - 2] = byte;
      break;
    case 6:
      packet->type = byte;
      break;
    case 7:
      packet->length = (uint16_t)byte << 8;
      break;
    case 8:
      packet->length |= byte;
      break;
    default:
      packet->data[idx - 9] = byte;
      if ((idx - 8) == packet->length) {
#ifdef FINGERPRINT_DEBUG
        Serial.println(" OK ");
#endif
        return FINGERPRINT_OK;
      }
      break;
    }
    idx++;
    if ((idx + 9) >= sizeof(packet->data)) {
      return FINGERPRINT_BADPACKET;
    }
  }
  // Shouldn't get here so...
  return FINGERPRINT_BADPACKET;
}

uint8_t Adafruit_Fingerprint::checkScannerBooted() {
  Serial.print("checkScannerBooted:");
  while(mySerial->available() > 0) {
    uint8_t byte = mySerial->read();
    Serial.println(byte);
    if (byte == FINGERPRINT_BOOTED)
      return FINGERPRINT_OK;
  }

  return FINGERPRINT_ABNORMAL;
}