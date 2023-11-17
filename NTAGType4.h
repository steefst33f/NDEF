/**************************************************************************/
/*!
    @file     NTAGType4.h
    @author   Steef
    @license  BSD
*/
/**************************************************************************/

#ifndef __NTAGTYPE4_H__
    #define __NTAGTYPE4_H__

    // #include <PN532.h>
    #include <Adafruit_PN532.h>
    #include "NfcTag.h"
    #include "NdefMessage.h"

    #define NFC_FORUM_TAG_TYPE_4 ("NFC Forum Type 4")

    class NTAGType4 {

        public:
            NTAGType4(Adafruit_PN532 &nfcModule);
            ~NTAGType4();

            // NTAGType4 command functions
            bool isoSelectFile(uint8_t p1, uint8_t p2, uint8_t lc, uint8_t *data, uint8_t *rAdpu, uint8_t *rAdpuLength);
            bool isoSelectFileNDEF(uint8_t *rAdpu, uint8_t *rAdpuLength); //TODO: replace with below
            bool isoReadBinary(uint8_t *rAdpu, uint8_t *rAdpuLength);
            NfcTag read(byte *uid, unsigned int uidLength);

        private:
            Adafruit_PN532 *_nfcModule;

            bool isoSelectNTAGType4Application(Adafruit_PN532 &nfc);
            bool isoSelectNTAGType4CCFile(Adafruit_PN532 &nfc);
            bool isoReadNTAGType4CCFile(Adafruit_PN532 &nfc);
            bool isoSelectNTAGType4NdefFile(Adafruit_PN532 &nfc);
            bool isoReadNTAGType4NdefLength(Adafruit_PN532 &nfc, uint8_t *ndefFileLength);
            NdefMessage isoReadNTAGType4NdefFile(Adafruit_PN532 &nfc, uint8_t ndefFileLength);

            NfcTag emptyTag(uint8_t *uid, uint8_t uidLength);
    };

#endif