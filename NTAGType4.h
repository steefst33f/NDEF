/**************************************************************************/
/*!
    @file     NTAGType4.h
    @author   Steef
    @license  BSD
*/
/**************************************************************************/

#ifndef __NTAGTYPE4_H__
    #define __NTAGTYPE4_H__

    #include <PN532.h>
    #include <NfcTag.h>

    #define NFC_FORUM_TAG_TYPE_4 ("NFC Forum Type 4")

    class NTAGType4 {

        public:
            NTAGType4(PN532 &nfcModule);
            ~NTAGType4();

            // NTAGType4 command functions
            bool isoSelectFile(uint8_t p1, uint8_t p2, uint8_t lc, uint8_t *data, uint8_t *rAdpu, uint8_t *rAdpuLength);
            bool isoSelectFileNDEF(uint8_t *rAdpu, uint8_t *rAdpuLength); //TODO: replace with below
            bool isoReadBinary(uint8_t *rAdpu, uint8_t *rAdpuLength);
            NfcTag read();

        private:
            PN532 *_nfcModule;

            bool isoSelectNTAGType4Application(PN532 &nfc);
            bool isoSelectNTAGType4CCFile(PN532 &nfc);
            bool isoReadNTAGType4CCFile(PN532 &nfc);
            bool isoSelectNTAGType4NdefFile(PN532 &nfc);
            bool isoReadNTAGType4NdefLength(PN532 &nfc, uint8_t *ndefFileLength);
            NdefMessage isoReadNTAGType4NdefFile(PN532 &nfc, uint8_t ndefFileLength);

            NfcTag emptyTag(uint8_t *uid, uint8_t uidLength);

    };

#endif