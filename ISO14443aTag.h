/**************************************************************************/
/*!
    @file     ISO14443aTag.h
    @author   Steef
    @license  BSD
*/
/**************************************************************************/


#ifndef ISO14443aTag_h
#define ISO14443aTag_h

#include <Arduino.h>

struct ISO14443aTag {

    public:
    enum class Type {
        MifareClassic = 1,
        Type2 = 2,
        Type4 = 4, 
        Unknown = 0
    };

    uint8_t tagNumber;
    uint16_t sensRes;
    uint8_t selRes;
    uint8_t nfcIdLength;
    uint8_t *nfcId;
    uint8_t atsLength;
    uint8_t *ats;
    Type type;

    ISO14443aTag() {
        tagNumber = 0;
        sensRes = 0x0000;
        selRes = 0x00;
        nfcIdLength = 0x00;
        nfcId = nullptr;
        atsLength = 0x00;
        ats = nullptr;
        type = Type::Unknown;
    }

    ~ISO14443aTag() {
        delete nfcId;
        delete ats;
    }

    void parseISO14443aTag(uint8_t *apdu, uint8_t apduLength);
    Type guessTagType();
    void print();

    private:
    bool isMifareClassic();
    bool isType2();
    bool isNtagType4();
};
#endif