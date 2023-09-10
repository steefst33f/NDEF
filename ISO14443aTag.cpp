/**************************************************************************/
/*!
    @file     ISO14443aTag.cpp
    @author   Steef
    @license  BSD
*/
/**************************************************************************/

#include "ISO14443aTag.h"
#include "PN532_debug.h"

void ISO14443aTag::parseISO14443aTag(uint8_t *apdu, uint8_t apduLength) {
    Serial.println(__FUNCTION__);

    if (apduLength < 6) {
        Serial.println("Couldn't parse ISO14443a Tag info from APDU");
        return;
    }

    tagNumber = apdu[0];
    sensRes = apdu[2];
    sensRes <<= 8;
    sensRes |= apdu[3];
    selRes = apdu[4];
    nfcIdLength = apdu[5];
    nfcId = new uint8_t[nfcIdLength];

    for (uint8_t i = 0; i < nfcIdLength; i++) {
        nfcId[i] = apdu[6 + i];
    }

    atsLength = apdu[5 + nfcIdLength + 1];
    ats = new uint8_t[atsLength];
    for (uint8_t i = 0; i < atsLength; i++) {
        ats[i] = apdu[6 + nfcIdLength + 1 + i];
    }

    type = guessTagType();
}

ISO14443aTag::Type ISO14443aTag::guessTagType() {
    Serial.println(__FUNCTION__);

    if (isMifareClassic()) {
        return ISO14443aTag::Type::MifareClassic;
    } 

    if (isType2()) {
        return ISO14443aTag::Type::Type2;
    } 

    if (isNtagType4()) {
        return ISO14443aTag::Type::Type4;
    } 

    return ISO14443aTag::Type::Unknown;
}

bool ISO14443aTag::isMifareClassic() {
// (ATQA 0x4 && SAK 0x8) || (ATQA 0x44 && SAK 0x8) - Mifare Classic
  if (sensRes == 0x44 | sensRes == 0x40) {
    if(selRes == 0x80) {
      return true;
    }
  }

  return false;
}

bool ISO14443aTag::isType2() {
  if (sensRes == 0x44 && selRes == 0x0) {
    return true;
  }

  return false;
}

bool ISO14443aTag::isNtagType4() {
// ATQA 0x344 && SAK 0x20 - NFC Forum Type 4
  if (sensRes == 0x0344 | sensRes == 0x0304) {
    if(selRes == 0x20) {
      if (nfcId[0] == 0x04) {
        Serial.println("NTAG Type 4 detected!");
        return true;
      }
    }
  }

  Serial.println("No NTAG Type 4 found!");
  return false;
}

void ISO14443aTag::print() {
    Serial.println("-------------------------------------------------------");
    Serial.println("");
    
    Serial.print("tagNumber ="); DMSG_HEX(tagNumber); Serial.println("");
    Serial.print("sensRes ="); DMSG_HEX(sensRes); Serial.println("");
    Serial.print("selRes ="); DMSG_HEX(selRes); Serial.println("");
    Serial.print("nfcIdLength ="); DMSG_HEX(nfcIdLength); Serial.println("");
    
    Serial.print("nfcId =");
    for (int i = 0; i < nfcIdLength; i++) {
        DMSG_HEX(nfcId[i]);
    }
    
    Serial.println("");
    Serial.print("atsLength = "); DMSG_HEX(atsLength); Serial.println("");
    Serial.print("ats = ");
    for (int i = 0; i < nfcIdLength; i++) {
        DMSG_HEX(ats[i]);
    }
    
    Serial.println("");
    Serial.println("-------------------------------------------------------");
}
