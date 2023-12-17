/**************************************************************************/
/*!
    @file     NTAGType4.cpp
    @author   Steef
    @license  BSD
*/
/**************************************************************************/
#include "NTAGType4.h"

#define NFC_FORUM_TAG_TYPE_4 ("NFC Forum Type 4")

NTAGType4::NTAGType4(Adafruit_PN532 &nfcModule) 
{
    _nfcModule = &nfcModule;
}

NTAGType4::~NTAGType4() 
{
}

NfcTag NTAGType4::read(byte *uid, unsigned int uidLength) {
  //Type4 Read Steps: 
  // 1) inListPassiveTarget (already done at card detecting, so no need to do this here)
  // 2) isoSelectNTAGType4Application
  // 3) isoSelectNTAGType4CCFile
  // 4) isoReadNTAGType4CCFile
  // 5) isoSelectNTAGType4NdefFile
  // 6) isoReadNTAGType4NdefLength
  // 7) isoReadNTAGType4NdefFile

  if (!isoSelectNTAGType4Application(*_nfcModule)) {
    return emptyTag(uid, uidLength);
  }

  if (!isoSelectNTAGType4CCFile(*_nfcModule)) {
    return emptyTag(uid, uidLength);
  }

  if (!isoReadNTAGType4CCFile(*_nfcModule)) {
    return emptyTag(uid, uidLength);
  }

  if (!isoSelectNTAGType4NdefFile(*_nfcModule)) {
    return emptyTag(uid, uidLength);
  }

  uint8_t ndefFileLenght;
  if (!isoReadNTAGType4NdefLength(*_nfcModule, &ndefFileLenght)) {
    return emptyTag(uid, uidLength);
  }

  NdefMessage ndefMessage = isoReadNTAGType4NdefFile(*_nfcModule, ndefFileLenght);
  return NfcTag(uid, uidLength, NFC_FORUM_TAG_TYPE_4, ndefMessage);
}

NfcTag NTAGType4::emptyTag(uint8_t *uid, uint8_t uidLength) {
    NdefMessage message = NdefMessage();
    message.addEmptyRecord();
    return NfcTag(uid, uidLength, NFC_FORUM_TAG_TYPE_4, message);
}

// Select the NDEF Tag Application
// CMD APDU: 00 A4 04 00 07 D2 76 00 00 85 01 01 00
// RSP APDU: 90 00
bool NTAGType4::isoSelectNTAGType4Application(Adafruit_PN532 &nfc) {
  #ifdef NDEF_USE_SERIAL
  Serial.println(__FUNCTION__);
  #endif
  uint8_t selectCmd[13] = { 0x00, 0xA4, 0x04, 0x00, 0x07, 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01, 0x00 };
  uint8_t selectCmdLen = sizeof(selectCmd);
  uint8_t response[3];
  uint8_t responseLen = sizeof(response);
  bool success = false;

  success = nfc.inDataExchange(selectCmd, selectCmdLen, response, &responseLen);

#ifdef NDEF_USE_SERIAL
  Serial.println(F("response:"));
  Adafruit_PN532::PrintHexChar(response, responseLen);
#endif
  if (success) {
    // Check if the response indicates success
    if (response[responseLen - 2] == 0x90 && response[responseLen - 1] == 0x00) {
      #ifdef NDEF_USE_SERIAL
      Serial.println("Select NTAGType4 Application successfully!");
      #endif
      return true;
    } else {
      #ifdef NDEF_USE_SERIAL
      Serial.println("Select NTAGType4 Application failed!");
      #endif
      return false;
    }
  } else {
    #ifdef NDEF_USE_SERIAL
    Serial.println("Communication with the NTAGType4 failed!");
    #endif
    return false;
  }
}

// Select the CC File
// CMD APDU: 00 A4 00 0C 02 E1 03
// RSP APDU: 90 00
bool NTAGType4::isoSelectNTAGType4CCFile(Adafruit_PN532 &nfc) { 
  #ifdef NDEF_USE_SERIAL
  Serial.println(__FUNCTION__);
  #endif
  // ISOSelectFile Command, Select the CC File, length Data field: 7 bytes, Datafield: E1 03 (Application)
  uint8_t selectCmd[7] = { 0x00, 0xA4, 0x00, 0x0C, 0x02, 0xE1, 0x03 };
  uint8_t selectCmdLen = sizeof(selectCmd);
  uint8_t response[3];
  uint8_t responseLen = sizeof(response);
  bool success = false;

  success = nfc.inDataExchange(selectCmd, selectCmdLen, response, &responseLen);
  #ifdef NDEF_USE_SERIAL
  Serial.println(F("response:"));
  Adafruit_PN532::PrintHexChar(response, responseLen);
  #endif

  if (success) {
    // Check if the response indicates success
    if (response[responseLen - 2] == 0x90 && response[responseLen - 1] == 0x00) {
      #ifdef NDEF_USE_SERIAL
      Serial.println("Select NTAGType4 CC File successfully!");
      #endif
      return true;
    } else {
      #ifdef NDEF_USE_SERIAL
      Serial.println("Select NTAGType4 CC File failed!");
      #endif
      return false;
    }
  } else {
    #ifdef NDEF_USE_SERIAL
    Serial.println("Communication with the NTAGType4 failed!");
    #endif
    return false;
  }
}

// Read the CC File
// CMD APDU: 00 B0 00 00 0F
// (Example of a) RSP APDU: 00 0F 20 00 3B 00 34 04 06 E1 04 00 32 00 00
bool NTAGType4::isoReadNTAGType4CCFile(Adafruit_PN532 &nfc) {
  #ifdef NDEF_USE_SERIAL
  Serial.println(__FUNCTION__);
  #endif
  // ISOReadBinary command, Command, offset 0, target currently selected file, read until 0F (15 bytes)
  uint8_t selectCmd[5] = { 0x00, 0xB0, 0x00, 0x00, 0x0F };
  uint8_t selectCmdLen = sizeof(selectCmd);
  uint8_t response[18 + 2]; //(CCFilesize + status bytes)
  uint8_t responseLen = sizeof(response);
  bool success = false;

  success = nfc.inDataExchange(selectCmd, selectCmdLen, response, &responseLen);
  
  // Check if the response indicates success
  if (success && response[responseLen - 2] != 0x90 && response[responseLen - 1] != 0x00) {
    success = false;
    #ifdef NDEF_USE_SERIAL
    Serial.println("No success status received from tag");
    #endif
  }

  #ifdef NDEF_USE_SERIAL
  Serial.println(F("response:"));
  Adafruit_PN532::PrintHexChar(response, responseLen);
  #endif

  if (success) {
    #ifdef NDEF_USE_SERIAL
    Serial.println("Read NTAGType4 CC File successfully!");
    #endif
    // CCFile *ccFile = parseCCFile(response, &responseLen);
    return true;
  } else {
    #ifdef NDEF_USE_SERIAL
    Serial.println("Communication with the NTAGType4 failed!");
    #endif
    return false;
  }
}

// Select the NDEF File
// CMD APDU: 00 A4 00 0C 02 E1 04
// RSP APDU: 90 00
bool NTAGType4::isoSelectNTAGType4NdefFile(Adafruit_PN532 &nfc) {
  #ifdef NDEF_USE_SERIAL
  Serial.println(__FUNCTION__);
  #endif
  // ISOSelectFile command, select (MF, DF, EF) by identifier, No response data: no FCI should be returned, length Data field: 2 bytes, DataField: E1 04 (File ID for NDEF message, Length of expected response : 00 (no response data expected)
  uint8_t selectCmd[7] = { 0x00, 0xA4, 0x00, 0x0C, 0x02, 0xE1, 0x04 };
  uint8_t selectCmdLen = sizeof(selectCmd);
  uint8_t response[3];
  uint8_t responseLen = sizeof(response);
  bool success = false;

  success = nfc.inDataExchange(selectCmd, selectCmdLen, response, &responseLen);
  #ifdef NDEF_USE_SERIAL
  Serial.println(F("response:"));
  Adafruit_PN532::PrintHexChar(response, responseLen);
  #endif

  if (success) {
    // Check if the response indicates success
    if (response[responseLen - 2] == 0x90 && response[responseLen - 1] == 0x00) {
      #ifdef NDEF_USE_SERIAL
      Serial.println("Select NTAGType4 NDEF File successfully!");
      #endif
      // CCFile *ccFile = parseCCFile(response, &responseLen);
      return true;
    } else {
      #ifdef NDEF_USE_SERIAL
      Serial.println("Select NTAGType4 NDEF File failed!");
      #endif
      return false;
    }
  } else {
    #ifdef NDEF_USE_SERIAL
    Serial.println("Communication with the NTAGType4 failed!");
    #endif
    return false;
  }
}

// Read the Length of the NDEF File
// CMD APDU: 00 B0 00 00 02
// (Example of a) RSP APDU: 00 4E 90 00
bool NTAGType4::isoReadNTAGType4NdefLength(Adafruit_PN532 &nfc, uint8_t *ndefFileLength) {
  #ifdef NDEF_USE_SERIAL
  Serial.println(__FUNCTION__);
  #endif
  // ISOReadBinary command, offset 0, target currently selected file, length file 2 bytes (this indicates read the length of the NDEF File)
  uint8_t selectCmd[5] = { 0x00, 0xB0, 0x00, 0x00, 0x02 };
  uint8_t selectCmdLen = sizeof(selectCmd);
  uint8_t response[5];
  uint8_t responseLen = sizeof(response);
  bool success = false;

  success = nfc.inDataExchange(selectCmd, selectCmdLen, response, &responseLen);
  #ifdef NDEF_USE_SERIAL
  Serial.println(F("response:"));
  Adafruit_PN532::PrintHexChar(response, responseLen);
  #endif

  if (success) {
    #ifdef NDEF_USE_SERIAL
    Serial.println("Read NTAGType4 NDEF Lenght successfully!");
    #endif
    // Check if the response indicates success
    if (response[responseLen - 2] == 0x90 && response[responseLen - 1] == 0x00) {
      *ndefFileLength = response[responseLen - 3];
      #ifdef NDEF_USE_SERIAL
      Serial.println("Read NTAGType4 NDEF File Length succesfully!");
      Serial.print("Read NTAGType4 NDEF File Length = ");
      Serial.print(' '); Serial.print((*ndefFileLength>>4)&0x0F, HEX); Serial.println(*ndefFileLength&0x0F, HEX);
      #endif
      return true;
    } else {
      #ifdef NDEF_USE_SERIAL
      Serial.println("Select NTAGType4 NDEF File failed!");
      #endif
      return false;
    }
  } else {
    #ifdef NDEF_USE_SERIAL
    Serial.println("Communication with the NTAGType4 failed!");
    #endif
    return false;
  }
}

// Read the NDEF File
// CMD APDU: 00 B0 00 02 4E
// (Example of a) RSP APDU: 00 00 D0 00 00 90 00 (returns: Empty NDEF file, status: Success)
NdefMessage NTAGType4::isoReadNTAGType4NdefFile(Adafruit_PN532 &nfc, uint8_t ndefFileLength) {
  #ifdef NDEF_USE_SERIAL
  Serial.println(__FUNCTION__);
  #endif
  // ISOReadBinary command, target currently selected file, offset is read starting from byte 2, length 0x4E is read read 3 bytes from the NDEF File
  uint8_t selectCmd[5] = { 0x00, 0xB0, 0x00, 0x02, ndefFileLength };
  uint8_t selectCmdLen = sizeof(selectCmd);
  uint8_t response[ndefFileLength + 2];  //+2 for status bytes
  uint8_t responseLen = sizeof(response);
  bool success = false;

  success = nfc.inDataExchange(selectCmd, selectCmdLen, response, &responseLen);
  #ifdef NDEF_USE_SERIAL
  Serial.println(F("response:"));
  Adafruit_PN532::PrintHexChar(response, responseLen);
  #endif

  // Check if the response indicates success
  if (success && response[responseLen - 2] != 0x90 && response[responseLen - 1] != 0x00) {
    success = false;
    #ifdef NDEF_USE_SERIAL
    Serial.println("No success status received from tag");
    #endif
  }

  if (success) {
    #ifdef NDEF_USE_SERIAL
    Serial.println("Read NTAGType4 NDEF File successfully!");
    #endif
  } else {
    #ifdef NDEF_USE_SERIAL
    Serial.println("Communication with the NTAGType4 failed!");
    #endif
    NdefMessage message = NdefMessage();
    message.addEmptyRecord();
    return message;
  }
  return NdefMessage(reinterpret_cast<byte*>(response),(int)responseLen);  
}
