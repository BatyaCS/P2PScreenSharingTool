#include <common.h>
#include <network\srt-environment.h>

#include <srt/srt.h>

SrtEnvironment::SrtEnvironment()
{
    if (srt_startup() < 0)
        throw std::runtime_error("Failed to initialize SRT library");
}

SrtEnvironment::~SrtEnvironment() 
{
    srt_cleanup();
}