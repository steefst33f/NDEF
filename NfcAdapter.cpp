#include "NfcAdapter.h"

NfcAdapter::NfcAdapter(Adafruit_PN532* nfcModule)
{
    _nfcModule = nfcModule;
    _tag = ISO14443aTag();
}

NfcAdapter::~NfcAdapter(void)
{
}

bool NfcAdapter::begin(boolean verbose)
{
    _nfcModule->begin();

    uint32_t versiondata = _nfcModule->getFirmwareVersion();

    if (! versiondata)
    {
    #ifdef NDEF_USE_SERIAL
        Serial.print(F("Didn't find PN53x board"));
    #endif
       return false;
    }

    if (verbose)
    {
    #ifdef NDEF_USE_SERIAL
        Serial.print(F("Found chip PN5")); Serial.println((versiondata>>24) & 0xFF, HEX);
        Serial.print(F("Firmware ver. ")); Serial.print((versiondata>>16) & 0xFF, DEC);
        Serial.print('.'); Serial.println((versiondata>>8) & 0xFF, DEC);
    #endif
    }
    // configure board to read RFID tags
    _nfcModule->SAMConfig();

    return true;
}

boolean NfcAdapter::isTagPresent(unsigned long timeout)
{
    return _nfcModule->readPassiveTargetID(PN532_MIFARE_ISO14443A, _uid, &_uidLength, _apdu, &_apduLength, timeout, true);
}

void NfcAdapter::wakeUp() {
    return _nfcModule->wakeup();
}

boolean NfcAdapter::powerDownMode() {
    // return _nfcModule->powerDownMode();
}

bool NfcAdapter::identifyTag() {
    return _tag.parseISO14443aTag(_apdu, _apduLength);
}

boolean NfcAdapter::tagPresent(unsigned long timeout)
{
    uint8_t success;
    success = _nfcModule->readPassiveTargetID(PN532_MIFARE_ISO14443A, _uid, &_uidLength, _apdu, &_apduLength, timeout, true);

    if (success) {
        _tag.parseISO14443aTag(_apdu, _apduLength);
    } else {
        _tag = ISO14443aTag();
    }
    
    return success;
}

void NfcAdapter::releaseTag()
{
    _nfcModule->inRelease(); //releases all the listed tags from the nfc modules memory and puts it in low Vbat mode and turns off the RF!
    _tag = ISO14443aTag();
    #ifdef NDEF_USE_SERIAL
        Serial.printf("Released tag(s)");
    #endif
}

boolean NfcAdapter::erase()
{
    NdefMessage message = NdefMessage();
    message.addEmptyRecord();
    return write(message);
}

boolean NfcAdapter::format()
{
    boolean success;
    #ifdef NDEF_SUPPORT_MIFARE_CLASSIC
    if (_tag.type == ISO14443aTag::Type::MifareClassic)
    {
        MifareClassic mifareClassic = MifareClassic(*_nfcModule);
        success = mifareClassic.formatNDEF(_uid, _uidLength);
    }
    else
    #endif
    {
    #ifdef NDEF_USE_SERIAL
        Serial.print(F("Unsupported Tag."));
    #endif
        success = false;
    }
    return success;
}

boolean NfcAdapter::clean()
{
    #ifdef NDEF_SUPPORT_MIFARE_CLASSIC
    if (_tag.type == ISO14443aTag::Type::MifareClassic)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Cleaning Mifare Classic"));
        #endif
        MifareClassic mifareClassic = MifareClassic(*_nfcModule);
        return mifareClassic.formatMifare(_uid, _uidLength);
    }
    else
    #endif
    if (_tag.type == ISO14443aTag::Type::Type2)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Cleaning Mifare Ultralight"));
        #endif
        MifareUltralight ultralight = MifareUltralight(*_nfcModule);
        return ultralight.clean();
    }
    else
    {
        #ifdef NDEF_USE_SERIAL
        Serial.print(F("No driver for card type "));Serial.println(static_cast<int>(_tag.type));
        #endif
        return false;
    }
}

NfcTag NfcAdapter::read()
{
    #ifdef NDEF_SUPPORT_MIFARE_CLASSIC
    if (_tag.type == ISO14443aTag::Type::MifareClassic)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Reading Mifare Classic"));
        #endif
        MifareClassic mifareClassic = MifareClassic(*_nfcModule);
        return mifareClassic.read(_uid, _uidLength);
    }
    else
    #endif
    if (_tag.type == ISO14443aTag::Type::Type2)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Reading Mifare Ultralight"));
        #endif
        MifareUltralight ultralight = MifareUltralight(*_nfcModule);
        return ultralight.read(_uid, _uidLength);
    }
    else if (_tag.type == ISO14443aTag::Type::Type4)
    {
        #ifdef NDEF_DEBUG
        Serial.print(F("Reading NTAG424"));
            #endif
        NTAGType4 ntag424 = NTAGType4(*_nfcModule);
        return ntag424.read(_uid, _uidLength);
    }
    else if (_tag.type == ISO14443aTag::Type::Unknown)
    {
        #ifdef NDEF_DEBUG
        Serial.print(F("Cant read tag of unknown type"));
        #endif
        return NfcTag(_uid, _uidLength);
    }
    else
    {
        #ifdef NDEF_DEBUG
        Serial.print(F("No driver for card type "));//Serial.println(type);
        #endif
        // TODO should set type here
        return NfcTag(_uid, _uidLength);
    }
}

boolean NfcAdapter::write(NdefMessage& ndefMessage)
{
    boolean success;
    #ifdef NDEF_SUPPORT_MIFARE_CLASSIC
    if (_tag.type == ISO14443aTag::Type::MifareClassic)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Writing Mifare Classic"));
        #endif
        MifareClassic mifareClassic = MifareClassic(*_nfcModule);
        success = mifareClassic.write(ndefMessage, _uid, _uidLength);
    }
    else
    #endif
    if (_tag.type == ISO14443aTag::Type::Type2)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Writing Mifare Ultralight"));
        #endif
        MifareUltralight mifareUltralight = MifareUltralight(*_nfcModule);
        success = mifareUltralight.write(ndefMessage, _uid, _uidLength);
    }
    else if (_tag.type == ISO14443aTag::Type::Unknown)
    {
        #ifdef NDEF_USE_SERIAL
        Serial.print(F("Can not determine tag type"));
        #endif
        success = false;
    }
    else
    {
        #ifdef NDEF_USE_SERIAL
        Serial.print(F("No driver for card type "));Serial.println(static_cast<int>(_tag.type));
        #endif
        success = false;
    }

    return success;
}

ISO14443aTag NfcAdapter::getInlistedTag() {
    return _tag;
}
