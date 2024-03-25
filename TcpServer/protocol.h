#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef unsigned int uint;
typedef struct PDU
{
    uint uiPDULen;
    uint uiMsgType;
    char caData[64];
    uint uiMsgLen;
    int caMsg[];
}PDU;

PDU *mkPDU(uint uiMsgLen);

#endif // PROTOCOL_H
