#ifndef RECORD_H
#define RECORD_H

#define MAXKEY 128
#define MAXVALUE 2048

#define ISDEL (1 << 32 - 1)

class Record
{
public:
    int CRC;
    int keyLen;
    int valueLen;
    char* key;
    char* value;
};

#endif
