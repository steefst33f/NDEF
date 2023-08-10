/**************************************************************************/
/*!
    @file     NTAGType4.cpp
    @author   Steef
    @license  BSD
*/
/**************************************************************************/

#include <Wire.h>
#include <PN532_I2C.h>
#include <NdefMessage.h>
#include <NfcAdapter.h>
#include <PN532_debug.h>
#include <ISO14443aTag.h>

#define NFC_FORUM_TAG_TYPE_4 ("NFC Forum Type 4")

NTAGType4::NTAGType4(PN532 &nfcModule) 
{
    DMSG_STR(__FUNCTION__);
    _nfcModule = &nfcModule;
}

NTAGType4::~NTAGType4() 
{
    DMSG_STR(__FUNCTION__);
}

NfcTag NTAGType4::read() {
  Serial.println(__FUNCTION__);

  uint8_t uid[16];
  uint8_t uidLength;
  uint8_t apdu[64];
  uint8_t apduLength;

  Serial.println("Reading..");  

  if (!_nfcModule->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, apdu, &apduLength)) {
    Serial.println("*****************-done with card-****************");  
    return emptyTag(uid, uidLength);
  }

  ISO14443aTag tag = ISO14443aTag();
  tag.parseISO14443aTag(apdu, apduLength);

  if (!_nfcModule->inListPassiveTarget()) {
    Serial.println("*****************-done with card-****************");  
    return emptyTag(uid, uidLength);
  }

  if (!isoSelectNTAGType4Application(*_nfcModule)) {
    Serial.println("*****************-done with card-****************");  
    return emptyTag(uid, uidLength);
  }

  Serial.println("---------------------------------------------");

  if (!isoSelectNTAGType4CCFile(*_nfcModule)) {
    Serial.println("*****************-done with card-****************");  
    return emptyTag(uid, uidLength);
  }

  Serial.println("---------------------------------------------");

  if (!isoReadNTAGType4CCFile(*_nfcModule)) {
    Serial.println("*****************-done with card-****************");  
    return emptyTag(uid, uidLength);
  }

  Serial.println("---------------------------------------------");

  if (!isoSelectNTAGType4NdefFile(*_nfcModule)) {
    Serial.println("*****************-done with card-****************");  
    return emptyTag(uid, uidLength);
  }

  Serial.println("---------------------------------------------");

  uint8_t ndefFileLenght;
  if (!isoReadNTAGType4NdefLength(*_nfcModule, &ndefFileLenght)) {
    Serial.println("*****************-done with card-****************");  
    return emptyTag(uid, uidLength);
  }

  Serial.println("---------------------------------------------");

  NdefMessage ndefMessage = isoReadNTAGType4NdefFile(*_nfcModule, ndefFileLenght);
  return NfcTag(uid, uidLength, NFC_FORUM_TAG_TYPE_4, ndefMessage);

  Serial.println("*****************-done with card-****************");  
}

NfcTag NTAGType4::emptyTag(uint8_t *uid, uint8_t uidLength) {
    NdefMessage message = NdefMessage();
    message.addEmptyRecord();
    return NfcTag(uid, uidLength, NFC_FORUM_TAG_TYPE_4, message);
}

// Select the NDEF Tag Application
// CMD APDU: 00 A4 04 00 07 D2 76 00 00 85 01 01 00
// RSP APDU: 90 00
bool NTAGType4::isoSelectNTAGType4Application(PN532 &nfc) {
  Serial.println(__FUNCTION__);

  uint8_t selectCmd[13] = { 0x00, 0xA4, 0x04, 0x00, 0x07, 0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01, 0x00 };
  uint8_t selectCmdLen = sizeof(selectCmd);
  uint8_t response[256];
  uint8_t responseLen = sizeof(response);
  bool success = false;

  success = nfc.inDataExchange(selectCmd, selectCmdLen, response, &responseLen);

  if (success) {
    // Check if the response indicates success
    if (response[responseLen - 2] == 0x90 && response[responseLen - 1] == 0x00) {
      Serial.println("Select NTAGType4 Application successfully!");
      return true;
    } else {
      Serial.println("Select NTAGType4 Application failed!");
      return false;
    }
  } else {
    Serial.println("Communication with the NTAGType4 failed!");
    return false;
  }
}

// Select the CC File
// CMD APDU: 00 A4 00 0C 02 E1 03
// RSP APDU: 90 00
bool NTAGType4::isoSelectNTAGType4CCFile(PN532 &nfc) {
  Serial.println(__FUNCTION__);
 
  // ISOSelectFile Command, Select the CC File, length Data field: 7 bytes, Datafield: E1 03 (Application)
  uint8_t selectCmd[7] = { 0x00, 0xA4, 0x00, 0x0C, 0x02, 0xE1, 0x03 };
  uint8_t selectCmdLen = sizeof(selectCmd);
  uint8_t response[256];
  uint8_t responseLen = sizeof(response);
  bool success = false;

  success = nfc.inDataExchange(selectCmd, selectCmdLen, response, &responseLen);

  if (success) {
    // Check if the response indicates success
    if (response[responseLen - 2] == 0x90 && response[responseLen - 1] == 0x00) {
      Serial.println("Select NTAGType4 CC File successfully!");
      return true;
    } else {
      Serial.println("Select NTAGType4 CC File failed!");
      return false;
    }
  } else {
    Serial.println("Communication with the NTAGType4 failed!");
    return false;
  }
}

// Read the CC File
// CMD APDU: 00 B0 00 00 0F
// (Example of a) RSP APDU: 00 0F 20 00 3B 00 34 04 06 E1 04 00 32 00 00
bool NTAGType4::isoReadNTAGType4CCFile(PN532 &nfc) {
  Serial.println(__FUNCTION__);
  // ISOReadBinary command, Command, offset 0, target currently selected file, read until 0F (15 bytes)
  uint8_t selectCmd[5] = { 0x00, 0xB0, 0x00, 0x00, 0x0F };
  uint8_t selectCmdLen = sizeof(selectCmd);
  uint8_t response[256];
  uint8_t responseLen = sizeof(response);
  bool success = false;

  success = nfc.inDataExchange(selectCmd, selectCmdLen, response, &responseLen);

  if (success) {
    Serial.println("Read NTAGType4 CC File successfully!");
    // CCFile *ccFile = parseCCFile(response, &responseLen);
    return true;
  } else {
    Serial.println("Communication with the NTAGType4 failed!");
    return false;
  }
}

// Select the NDEF File
// CMD APDU: 00 A4 00 0C 02 E1 04
// RSP APDU: 90 00
bool NTAGType4::isoSelectNTAGType4NdefFile(PN532 &nfc) {
  Serial.println(__FUNCTION__);
  // ISOSelectFile command, select (MF, DF, EF) by identifier, No response data: no FCI should be returned, length Data field: 2 bytes, DataField: E1 04 (File ID for NDEF message, Length of expected response : 00 (no response data expected)
  uint8_t selectCmd[7] = { 0x00, 0xA4, 0x00, 0x0C, 0x02, 0xE1, 0x04 };
  uint8_t selectCmdLen = sizeof(selectCmd);
  uint8_t response[256];
  uint8_t responseLen = sizeof(response);
  bool success = false;

  success = nfc.inDataExchange(selectCmd, selectCmdLen, response, &responseLen);

  if (success) {
    // Check if the response indicates success
    if (response[responseLen - 2] == 0x90 && response[responseLen - 1] == 0x00) {
      Serial.println("Select NTAGType4 NDEF File successfully!");
      // CCFile *ccFile = parseCCFile(response, &responseLen);
      return true;
    } else {
      Serial.println("Select NTAGType4 NDEF File failed!");
      return false;
    }
  } else {
    Serial.println("Communication with the NTAGType4 failed!");
    return false;
  }
}

// Read the Length of the NDEF File
// CMD APDU: 00 B0 00 00 02
// (Example of a) RSP APDU: 00 4E 90 00
bool NTAGType4::isoReadNTAGType4NdefLength(PN532 &nfc, uint8_t *ndefFileLength) {
  Serial.println(__FUNCTION__);
  // ISOReadBinary command, offset 0, target currently selected file, length file 2 bytes (this indicates read the length of the NDEF File)
  uint8_t selectCmd[5] = { 0x00, 0xB0, 0x00, 0x00, 0x02 };
  uint8_t selectCmdLen = sizeof(selectCmd);
  uint8_t response[256];
  uint8_t responseLen = sizeof(response);
  bool success = false;

  success = nfc.inDataExchange(selectCmd, selectCmdLen, response, &responseLen);

  if (success) {
    Serial.println("Read NTAGType4 NDEF Lenght successfully!");
    // Check if the response indicates success
    if (response[responseLen - 2] == 0x90 && response[responseLen - 1] == 0x00) {
      Serial.println("Read NTAGType4 NDEF File Length succesfully!");
      *ndefFileLength = response[responseLen - 3];
      Serial.print("Read NTAGType4 NDEF File Length = ");
      Serial.print(' '); Serial.print((*ndefFileLength>>4)&0x0F, HEX); Serial.println(*ndefFileLength&0x0F, HEX);
      return true;
    } else {
      Serial.println("Select NTAGType4 NDEF File failed!");
      return false;
    }
  } else {
    Serial.println("Communication with the NTAGType4 failed!");
    return false;
  }
}

// Read the NDEF File
// CMD APDU: 00 B0 00 02 4E
// (Example of a) RSP APDU: 00 00 D0 00 00 90 00 (returns: Empty NDEF file, status: Success)
NdefMessage NTAGType4::isoReadNTAGType4NdefFile(PN532 &nfc, uint8_t ndefFileLength) {
  Serial.println(__FUNCTION__);
  // ISOReadBinary command, target currently selected file, offset is read starting from byte 2, length 0x4E is read read 3 bytes from the NDEF File
  uint8_t selectCmd[5] = { 0x00, 0xB0, 0x00, 0x02, ndefFileLength };
  uint8_t selectCmdLen = sizeof(selectCmd);
  uint8_t response[256];
  uint8_t responseLen = sizeof(response);
  bool success = false;

  success = nfc.inDataExchange(selectCmd, selectCmdLen, response, &responseLen);
  if (success) {
    Serial.println("Read NTAGType4 NDEF File successfully!");
  } else {
    Serial.println("Communication with the NTAGType4 failed!");
    NdefMessage message = NdefMessage();
    message.addEmptyRecord();
    return message;
  }
  return NdefMessage(reinterpret_cast<byte*>(response),(int)responseLen);  
}
