#include "NdefRecord.h"

NdefRecord::NdefRecord()
{
    //Serial.println("NdefRecord Constructor 1");
    _tnf = 0;
    _typeLength = 0;
    _payloadLength = 0;
    _idLength = 0;
    _type = (byte *)NULL;
    _payload = (byte *)NULL;
    _id = (byte *)NULL;
}

NdefRecord::NdefRecord(const NdefRecord& rhs)
{
    //Serial.println("NdefRecord Constructor 2 (copy)");

    _tnf = rhs._tnf;
    _typeLength = rhs._typeLength;
    _payloadLength = rhs._payloadLength;
    _idLength = rhs._idLength;
    _type = (byte *)NULL;
    _payload = (byte *)NULL;
    _id = (byte *)NULL;

    if (_typeLength)
    {
        _type = (byte*)malloc(_typeLength);
        memcpy(_type, rhs._type, _typeLength);
    }

    if (_payloadLength)
    {
        _payload = (byte*)malloc(_payloadLength);
        memcpy(_payload, rhs._payload, _payloadLength);
    }

    if (_idLength)
    {
        _id = (byte*)malloc(_idLength);
        memcpy(_id, rhs._id, _idLength);
    }

}

// TODO NdefRecord::NdefRecord(tnf, type, payload, id)

NdefRecord::~NdefRecord()
{
    //Serial.println("NdefRecord Destructor");
    if (_typeLength)
    {
        free(_type);
    }

    if (_payloadLength)
    {
        free(_payload);
    }

    if (_idLength)
    {
        free(_id);
    }
}

NdefRecord& NdefRecord::operator=(const NdefRecord& rhs)
{
    //Serial.println("NdefRecord ASSIGN");

    if (this != &rhs)
    {
        // free existing
        if (_typeLength)
        {
            free(_type);
        }

        if (_payloadLength)
        {
            free(_payload);
        }

        if (_idLength)
        {
            free(_id);
        }

        _tnf = rhs._tnf;
        _typeLength = rhs._typeLength;
        _payloadLength = rhs._payloadLength;
        _idLength = rhs._idLength;

        if (_typeLength)
        {
            _type = (byte*)malloc(_typeLength);
            memcpy(_type, rhs._type, _typeLength);
        }

        if (_payloadLength)
        {
            _payload = (byte*)malloc(_payloadLength);
            memcpy(_payload, rhs._payload, _payloadLength);
        }

        if (_idLength)
        {
            _id = (byte*)malloc(_idLength);
            memcpy(_id, rhs._id, _idLength);
        }
    }
    return *this;
}

// size of records in bytes
int NdefRecord::getEncodedSize()
{
    int size = 2; // tnf + typeLength
    if (_payloadLength > 0xFF)
    {
        size += 4;
    }
    else
    {
        size += 1;
    }

    if (_idLength)
    {
        size += 1;
    }

    size += (_typeLength + _payloadLength + _idLength);

    return size;
}

void NdefRecord::encode(byte *data, bool firstRecord, bool lastRecord)
{
    // assert data > getEncodedSize()

    uint8_t* data_ptr = &data[0];

    *data_ptr = getTnfByte(firstRecord, lastRecord);
    data_ptr += 1;

    *data_ptr = _typeLength;
    data_ptr += 1;

    if (_payloadLength <= 0xFF) {  // short record
        *data_ptr = _payloadLength;
        data_ptr += 1;
    } else { // long format
        // 4 bytes but we store length as an int
        data_ptr[0] = 0x0; // (_payloadLength >> 24) & 0xFF;
        data_ptr[1] = 0x0; // (_payloadLength >> 16) & 0xFF;
        data_ptr[2] = (_payloadLength >> 8) & 0xFF;
        data_ptr[3] = _payloadLength & 0xFF;
        data_ptr += 4;
    }

    if (_idLength)
    {
        *data_ptr = _idLength;
        data_ptr += 1;
    }

    //Serial.println(2);
    memcpy(data_ptr, _type, _typeLength);
    data_ptr += _typeLength;

    if (_idLength)
    {
        memcpy(data_ptr, _id, _idLength);
        data_ptr += _idLength;
    }
    
    memcpy(data_ptr, _payload, _payloadLength);
    data_ptr += _payloadLength;
}

byte NdefRecord::getTnfByte(bool firstRecord, bool lastRecord)
{
    int value = _tnf;

    if (firstRecord) { // mb
        value = value | 0x80;
    }

    if (lastRecord) { //
        value = value | 0x40;
    }

    // chunked flag is always false for now
    // if (cf) {
    //     value = value | 0x20;
    // }

    if (_payloadLength <= 0xFF) {
        value = value | 0x10;
    }

    if (_idLength) {
        value = value | 0x8;
    }

    return value;
}

byte NdefRecord::getTnf()
{
    return _tnf;
}

void NdefRecord::setTnf(byte tnf)
{
    _tnf = tnf;
}

unsigned int NdefRecord::getTypeLength()
{
    return _typeLength;
}

int NdefRecord::getPayloadLength()
{
    return _payloadLength;
}

unsigned int NdefRecord::getIdLength()
{
    return _idLength;
}

String NdefRecord::getType()
{
    char type[_typeLength + 1];
    memcpy(type, _type, _typeLength);
    type[_typeLength] = '\0'; // null terminate
    return String(type);
}

// this assumes the caller created type correctly
void NdefRecord::getType(uint8_t* type)
{
    memcpy(type, _type, _typeLength);
}

void NdefRecord::setType(const byte * type, const unsigned int numBytes)
{
    if(_typeLength)
    {
        free(_type);
    }

    _type = (uint8_t*)malloc(numBytes);
    memcpy(_type, type, numBytes);
    _typeLength = numBytes;
}

// assumes the caller sized payload properly
void NdefRecord::getPayload(byte *payload)
{
    memcpy(payload, _payload, _payloadLength);
}

void NdefRecord::setPayload(const byte * payload, const int numBytes)
{
    if (_payloadLength)
    {
        free(_payload);
    }

    _payload = (byte*)malloc(numBytes);
    memcpy(_payload, payload, numBytes);
    _payloadLength = numBytes;
}

String NdefRecord::getId()
{
    char id[_idLength + 1];
    memcpy(id, _id, _idLength);
    id[_idLength] = '\0'; // null terminate
    return String(id);
}

String NdefRecord::getUri()
{
    String uri = "";
    if (getType() == "U") {
        if (_payloadLength > 0) {
            switch (_payload[0]) {
                case 0x000: break;
                case 0x001: uri += "http://www."; break;
                case 0x002: uri += "https://www."; break;
                case 0x003: uri += "http://"; break;
                case 0x004: uri += "https://"; break;
                case 0x005: uri += "tel:"; break;
                case 0x006: uri += "mailto:"; break;
                case 0x007: uri += "ftp://anonymous:anonymous@"; break;
                case 0x008: uri += "ftp://ftp."; break;
                case 0x009: uri += "ftps://"; break;
                case 0x00A: uri += "sftp://"; break;
                case 0x00B: uri += "smb://"; break;
                case 0x00C: uri += "nfs://"; break;
                case 0x00D: uri += "ftp://"; break;
                case 0x00E: uri += "dav://"; break;
                case 0x00F: uri += "news:"; break;
                case 0x010: uri += "telnet://"; break;
                case 0x011: uri += "imap:"; break;
                case 0x012: uri += "rtsp://"; break;
                case 0x013: uri += "urn:"; break;
                case 0x014: uri += "pop:"; break;
                case 0x015: uri += "sip:"; break;
                case 0x016: uri += "sips:"; break;
                case 0x017: uri += "tftp:"; break;
                case 0x018: uri += "btspp://"; break;
                case 0x019: uri += "btl2cap://"; break;
                case 0x01A: uri += "btgoep://"; break;
                case 0x01B: uri += "tcpobex://"; break;
                case 0x01C: uri += "irdaobex://"; break;
                case 0x01D: uri += "file://"; break;
                case 0x01E: uri += "urn:epc:id:"; break;
                case 0x01F: uri += "urn:epc:tag:"; break;
                case 0x020: uri += "urn:epc:pat:"; break;
                case 0x021: uri += "urn:epc:raw:"; break;
                case 0x022: uri += "urn:epc:"; break;
                case 0x023: uri += "urn:nfc:"; break;
                default: break;
            }
            for (int i = 1; i < _payloadLength; ++i) {
                uri += (char)_payload[i];
            }
        }
    }
    return uri;
}

void NdefRecord::getId(byte *id)
{
    memcpy(id, _id, _idLength);
}

void NdefRecord::setId(const byte * id, const unsigned int numBytes)
{
    if (_idLength)
    {
        free(_id);
    }

    _id = (byte*)malloc(numBytes);
    memcpy(_id, id, numBytes);
    _idLength = numBytes;
}
#ifdef NDEF_USE_SERIAL

void NdefRecord::print()
{
    Serial.println(F("  NDEF Record"));
    Serial.print(F("    TNF 0x"));Serial.print(_tnf, HEX);Serial.print(" ");
    switch (_tnf) {
    case TNF_EMPTY:
        Serial.println(F("Empty"));
        break;
    case TNF_WELL_KNOWN:
        Serial.println(F("Well Known"));
        break;
    case TNF_MIME_MEDIA:
        Serial.println(F("Mime Media"));
        break;
    case TNF_ABSOLUTE_URI:
        Serial.println(F("Absolute URI"));
        break;
    case TNF_EXTERNAL_TYPE:
        Serial.println(F("External"));
        break;
    case TNF_UNKNOWN:
        Serial.println(F("Unknown"));
        break;
    case TNF_UNCHANGED:
        Serial.println(F("Unchanged"));
        break;
    case TNF_RESERVED:
        Serial.println(F("Reserved"));
        break;
    default:
        Serial.println();
    }
    Serial.print(F("    Type Length 0x"));Serial.print(_typeLength, HEX);Serial.print(" ");Serial.println(_typeLength);
    Serial.print(F("    Payload Length 0x"));Serial.print(_payloadLength, HEX);;Serial.print(" ");Serial.println(_payloadLength);
    if (_idLength)
    {
        Serial.print(F("    Id Length 0x"));Serial.println(_idLength, HEX);
    }
    Serial.print(F("    Type "));PrintHexChar(_type, _typeLength);
    // TODO chunk large payloads so this is readable
    Serial.print(F("    Payload "));PrintHexChar(_payload, _payloadLength);
    if (_idLength)
    {
        Serial.print(F("    Id "));PrintHexChar(_id, _idLength);
    }
    Serial.print(F("    Record is "));Serial.print(getEncodedSize());Serial.println(" bytes");

}
#endif
