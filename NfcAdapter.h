#ifndef NfcAdapter_h
#define NfcAdapter_h

#include <Adafruit_PN532.h>
#include "NfcTag.h"
#include "Ndef.h"

// Drivers
#include "MifareClassic.h"
#include "MifareUltralight.h"
#include "NTAGType4.h"
#include "ISO14443aTag.h"

#define TAG_TYPE_MIFARE_CLASSIC (0)
#define TAG_TYPE_1 (1)
#define TAG_TYPE_2 (2)
#define TAG_TYPE_3 (3)
#define TAG_TYPE_4 (4)
#define TAG_TYPE_UNKNOWN (99)

#define IRQ   (2)
#define RESET (3)  // Not connected by default on the NFC Shield

class NfcAdapter {
    public:
        NfcAdapter(Adafruit_PN532* nfcModule);

        ~NfcAdapter(void);
        bool begin(boolean verbose=true);
        bool powerDownMode();
        boolean isTagPresent(unsigned long timeout=0);
        bool identifyTag();
        boolean tagPresent(unsigned long timeout=0); // tagAvailable
        NfcTag read();
        boolean write(NdefMessage& ndefMessage);
        // erase tag by writing an empty NDEF record
        boolean erase();
        // format a tag as NDEF
        boolean format();
        // reset tag back to factory state
        boolean clean();
        // release all tag(s) (So we clear all stored data and can scan an other one) Also puts the nfc module in low Vbat mode and turns off the RF!
        void releaseTag();

        // Get info of the inlisted tag
        ISO14443aTag getInlistedTag();
    private:
        Adafruit_PN532* _nfcModule = nullptr;
        ISO14443aTag _tag;

        uint8_t _uid[7];
        uint8_t _uidLength;
        uint8_t _apdu[64];
        uint8_t _apduLength;
};

#endif
