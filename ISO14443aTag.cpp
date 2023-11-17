/**************************************************************************/
/*!
    @file     ISO14443aTag.cpp
    @author   Steef
    @license  BSD
*/
/**************************************************************************/

#include "ISO14443aTag.h"

#define PRINT_HEX(num)       Serial.print(' '); Serial.print((num>>4)&0x0F, HEX); Serial.print(num&0x0F, HEX)

bool ISO14443aTag::parseISO14443aTag(uint8_t *apdu, uint8_t apduLength) {
  if (apduLength < 6) {
      #ifdef NDEF_USE_SERIAL
      Serial.println("Couldn't parse ISO14443a Tag info from APDU");
      #endif
      return false;
  }

  tagNumber = apdu[0];
  sensRes = apdu[2];
  sensRes <<= 8;
  sensRes |= apdu[3];
  selRes = apdu[4];
  
  nfcIdLength = apdu[5];
  nfcId = new uint8_t[nfcIdLength];
  if (apduLength < (5 + nfcIdLength + 1)) { 
    #ifdef NDEF_USE_SERIAL
    Serial.println("Couldn't parse ISO14443a Tag info from APDU");
    #endif
    return false;
  }
  for (uint8_t i = 0; i < nfcIdLength; i++) {
      nfcId[i] = apdu[6 + i];
  }

  type = guessTagType();

  //Set ATS (only implemented for Type 4 now)
  if (type == ISO14443aTag::Type::Type4) {
    atsLength = apdu[5 + nfcIdLength + 1];
    ats = new uint8_t[atsLength];
    if (apduLength < (5 + nfcIdLength + 1 + atsLength)) { 
      #ifdef NDEF_USE_SERIAL
      Serial.println("NO ATS info available to parse");
      #endif
      return true;
    }
    
    #ifdef NDEF_USE_SERIAL
    Serial.println("atsLength: " + String(atsLength) + "ats: ");
    #endif
    for (uint8_t i = 0; i < atsLength; i++) {
      ats[i] = apdu[6 + nfcIdLength + 1 + i];
      #ifdef NDEF_USE_SERIAL
      Serial.print(ats[i], HEX);
      #endif
    }
  } else {
    ats = nullptr;
    atsLength = 0;
  }
  return true;
}

ISO14443aTag::Type ISO14443aTag::guessTagType() {
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
      #ifdef NDEF_USE_SERIAL
      Serial.println("Mifare Classic Tag detected!");
      #endif
      return true;
    }
  }

  return false;
}

bool ISO14443aTag::isType2() {
  if (sensRes == 0x44 && selRes == 0x0) {
    #ifdef NDEF_USE_SERIAL
    Serial.println("Type 2 Tag detected!");
    #endif
    return true;
  }

  return false;
}

bool ISO14443aTag::isNtagType4() {
// ATQA 0x344 && SAK 0x20 - NFC Forum Type 4
  if (sensRes == 0x0344 | sensRes == 0x0304) {
    if(selRes == 0x20) {
      if (nfcId[0] == 0x04) {
        #ifdef NDEF_USE_SERIAL
        Serial.println("NTAG Type 4 detected!");
        #endif
        return true;
      }
    }
  }

  #ifdef NDEF_USE_SERIAL
  Serial.println("No NTAG Type 4 found!");
  #endif
  return false;
}

void ISO14443aTag::print() {
    #ifdef NDEF_USE_SERIAL
    Serial.println("-------------------------------------------------------");
    Serial.println("");
    
    Serial.print("tagNumber ="); PRINT_HEX(tagNumber); Serial.println("");
    Serial.print("sensRes ="); PRINT_HEX(sensRes); Serial.println("");
    Serial.print("selRes ="); PRINT_HEX(selRes); Serial.println("");
    Serial.print("nfcIdLength ="); PRINT_HEX(nfcIdLength); Serial.println("");
    
    if (nfcId != nullptr) {
      Serial.print("nfcId =");
      for (int i = 0; i < nfcIdLength; i++) {
          PRINT_HEX(nfcId[i]);
      }
    }
    
    if (ats != nullptr) {
      Serial.println("");
      Serial.print("atsLength = "); PRINT_HEX(atsLength); Serial.println("");
      Serial.print("ats = ");
      for (int i = 0; i < nfcIdLength; i++) {
        PRINT_HEX(ats[i]);
      }
    }
    
    Serial.println("");
    Serial.println("-------------------------------------------------------");
    #endif
}
