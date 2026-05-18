#ifndef SRT_DEFS_H_
#define SRT_DEFS_H_

enum class SrtDeliveryOrder 
{
    OUT_OF_ORDER    = 0,
    IN_ORDER        = 1 
};

enum class SrtMessageTtl : int 
{
    TTL_INFINITE    = -1,
    TTL_100MS       = 100,
    TTL_1000MS      = 1000
};

enum class SrtLatePktDrop
{
    DISABLE         = 0,
    ENABLE          = 1
};

enum class SrtEncryption 
{
    NONE            = 0,
    AES_128         = 16,
    AES_192         = 24,
    AES_256         = 32
};

#endif /* SRT_DEFS_H_ */