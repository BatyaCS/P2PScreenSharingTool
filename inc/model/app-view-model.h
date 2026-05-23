#ifndef APP_VIEW_MODEL_H_
#define APP_VIEW_MODEL_H_

#include <model/app-models.h>
#include <vector>

#include <mutex>
#include <atomic>

class AppViewModel 
{
public:
    using StreamConfig = AppModels::StreamConfig;
    using NetworkConfigTx = AppModels::NetworkConfigTx;
    using NetworkConfigRx = AppModels::NetworkConfigRx;
    using Logs = std::vector<AppModels::LogEntry>;

    StreamConfig        stream_config{};

    NetworkConfigTx     network_tx{};
    NetworkConfigRx     network_rx{};

    Logs                logs;
    std::mutex          logs_mutex;
    
    std::atomic<bool>   is_broadcasting{false};
    std::atomic<bool>   is_watching{false};
};

#endif /* APP_VIEW_MODEL_H_ */