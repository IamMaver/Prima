#include "prima.h"
/** Function-validators for different commands */
int isValidPowerControl(uint32_t length, const uint8_t *data)
{
    if (length != 1 || (data[0] != 0 && data[0] != 1))
    {
        return 0;
    }
    return 1;
}

int isValidNetworkLabel(uint32_t length, const uint8_t *data)
{
    if (length == 0 || length > 255)
    {
        return 0;
    }
    return 1;
}

/** Function-processors for different commands */
void processPowerControl(uint32_t length, const uint8_t *data)
{
    if (data[0] == 0)
    {
        fprintf(stderr, "Power ON\n");
    }
    else
    {
        fprintf(stderr, "Power OFF\n");
    }
}

void processNetworkLabel(uint32_t length, const uint8_t *data)
{
    char networkLabel[256];
    memcpy(networkLabel, data, length);
    networkLabel[length] = 0;
    fprintf(stderr, "Network label = %s\n", networkLabel);
}
