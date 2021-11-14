static uint16 generateCRC(uint8_t pData[], uint16 messageLength)
{
    uint16 crc = 0xFFFFU;
    if((pData != NULL) && (messageLength >= 2U))
    {
        uint16 i = 0U;       /*byte counter*/
        uint8 j = 0U;       /*bit counter*/

        for(i=0U;i<(uint16)((messageLength-2U) & 0xFFFFU);i++)
        {
            crc ^= (uint16)pData[i];
            for(j=8U;j!=0U;j--)
            {
                if((crc & 0x0001U) != 0U)
                {
                    crc >>= 1;
                    crc ^= 0xA001U;
                }
                else
                {
                    crc >>= 1;
                }
            }
        }
    }
    return crc;
}
