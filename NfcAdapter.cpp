#include <NfcAdapter.h>
#include <NTAG424.h>
#include <ISO14443aTag.h>

NfcAdapter::NfcAdapter(PN532Interface &interface)
{
    shield = new PN532(interface);
    tag = ISO14443aTag();
}

NfcAdapter::~NfcAdapter(void)
{
    delete shield;
}

void NfcAdapter::begin(boolean verbose)
{
    shield->begin();

    uint32_t versiondata = shield->getFirmwareVersion();

    if (! versiondata)
    {
#ifdef NDEF_USE_SERIAL
        Serial.print(F("Didn't find PN53x board"));
#endif
        while (1); // halt
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
    shield->SAMConfig();
}

boolean NfcAdapter::tagPresent(unsigned long timeout)
{
    uint8_t success;
    uint8_t uid[16];
    uint8_t uidLength;
    uint8_t apdu[64];
    uint8_t apduLength;

    if (timeout == 0)
    {
        success = shield->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, apdu, &apduLength);
    }
    else
    {
        success = shield->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, apdu, &apduLength, timeout);
    }
    tag.parseISO14443aTag(apdu, apduLength);
    return success;
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
    if (tag.type == ISO14443aTag::Type::MifareClassic)
    {
        MifareClassic mifareClassic = MifareClassic(*shield);
        success = mifareClassic.formatNDEF(uid, uidLength);
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
    if (tag.type == ISO14443aTag::Type::MifareClassic)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Cleaning Mifare Classic"));
        #endif
        MifareClassic mifareClassic = MifareClassic(*shield);
        return mifareClassic.formatMifare(uid, uidLength);
    }
    else
#endif
    if (tag.type == ISO14443aTag::Type::Type2)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Cleaning Mifare Ultralight"));
        #endif
        MifareUltralight ultralight = MifareUltralight(*shield);
        return ultralight.clean();
    }
    else
    {
#ifdef NDEF_USE_SERIAL
        Serial.print(F("No driver for card type "));Serial.println(static_cast<int>(tag.type));
#endif
        return false;
    }
}

NfcTag NfcAdapter::read()
{
#ifdef NDEF_SUPPORT_MIFARE_CLASSIC
    if (tag.type == ISO14443aTag::Type::MifareClassic)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Reading Mifare Classic"));
        #endif
        MifareClassic mifareClassic = MifareClassic(*shield);
        return mifareClassic.read(uid, uidLength);
    }
    else
#endif
    if (tag.type == ISO14443aTag::Type::Type2)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Reading Mifare Ultralight"));
        #endif
        MifareUltralight ultralight = MifareUltralight(*shield);
        return ultralight.read(uid, uidLength);
    }
    else if (tag.type == ISO14443aTag::Type::Type4)
    {
#ifdef NDEF_USE_SERIAL
        Serial.print(F("Reading NTAG424"));
#endif
        NTAGType4 ntag424 = NTAGType4(*shield);
        return ntag424.read();
    }
    else if (tag.type == ISO14443aTag::Type::Unknown)
    {
#ifdef NDEF_USE_SERIAL
        Serial.print(F("Can not determine tag type"));
#endif
        return NfcTag(uid, uidLength);
    }
    else
    {
        // Serial.print(F("No driver for card type "));Serial.println(type);
        // TODO should set type here
        return NfcTag(uid, uidLength);
    }
}

boolean NfcAdapter::write(NdefMessage& ndefMessage)
{
    boolean success;
#ifdef NDEF_SUPPORT_MIFARE_CLASSIC
    if (tag.type == ISO14443aTag::Type::MifareClassic)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Writing Mifare Classic"));
        #endif
        MifareClassic mifareClassic = MifareClassic(*shield);
        success = mifareClassic.write(ndefMessage, uid, uidLength);
    }
    else
#endif
    if (tag.type == ISO14443aTag::Type::Type2)
    {
        #ifdef NDEF_DEBUG
        Serial.println(F("Writing Mifare Ultralight"));
        #endif
        MifareUltralight mifareUltralight = MifareUltralight(*shield);
        success = mifareUltralight.write(ndefMessage, uid, uidLength);
    }
    else if (tag.type == ISO14443aTag::Type::Unknown)
    {
#ifdef NDEF_USE_SERIAL
        Serial.print(F("Can not determine tag type"));
#endif
        success = false;
    }
    else
    {
#ifdef NDEF_USE_SERIAL
        Serial.print(F("No driver for card type "));Serial.println(static_cast<int>(tag.type));
#endif
        success = false;
    }

    return success;
}
