#include <common.h>
#include <chrono>
#include <vector>

#include <network/srt-environment.h>
#include <app/application.h>

uint64_t GetTicksSinceStart() 
{
    static const auto start_time = std::chrono::steady_clock::now();
    auto current_time = std::chrono::steady_clock::now();
    
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
    return static_cast<uint64_t>(elapsed.count());
}

EXTERN_C uint logger_timestamp()
{
    return static_cast<uint>(GetTicksSinceStart());
}

void std_logger(LogKind level, const char * fmt, va_list args)
{
    int len = std::vsnprintf(nullptr, 0, fmt, args);
    if (len < 0)
    {
        std::cout << "Incorrect log received for std cout!";
        return;
    }

    std::vector<char> buffer(len + 1);
    std::vsnprintf(buffer.data(), buffer.size(), fmt, args);
    std::cout << buffer.data();
}

int main()
{
    GetTicksSinceStart();

    // Init logger
    ::set_logger(&std_logger);

    // Init libraries
    SrtEnvironment srt_environment;

    // Init App
    Application app;

    if (!app.init())
        return -1;

    app.run();

    return 0;
}