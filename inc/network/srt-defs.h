#ifndef SRT_DEFS_H_
#define SRT_DEFS_H_

enum class SrtLatePktDrop
{
    DISABLE     = 0,
    ENABLE      = 1
};

enum class SrtEncryption 
{
    NONE        = 0,
    AES_128     = 16,
    AES_192     = 24,
    AES_256     = 32
};

#endif /* SRT_DEFS_H_ */