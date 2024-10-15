#include "prima.h"

int decodeTLV(const uint8_t *buffer, size_t buf_size, TLVCommand *tlvCmd)
{
    if (buf_size < HEADER_TL)
    {
        return -1; 
    }
    memcpy(&tlvCmd->tag, buffer, HEADER_T);
    memcpy(&tlvCmd->length, buffer + HEADER_T, HEADER_L);

    tlvCmd->tag = ntohl(tlvCmd->tag);
    tlvCmd->length = ntohl(tlvCmd->length);

    if (buf_size < HEADER_TL + tlvCmd->length)
    {
        return -1;
    }

    tlvCmd->value = malloc(tlvCmd->length);
    if (!tlvCmd->value)
    {
        return -1;
    }

    memcpy(tlvCmd->value, buffer + HEADER_TL, tlvCmd->length);
    return 0;
}

void freeTLV(TLVCommand *tlvCmd)
{
    free(tlvCmd->value);
}