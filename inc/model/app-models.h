#ifndef APP_MODELS_H_
#define APP_MODELS_H_

#include <log/log.h>
#include <vector>
#include <string>

#include <nlohmann/json.hpp>

class AppModels
{
public:
    struct StreamConfig
    {
        enum class CaptureTarget { DISPLAY, WINDOW };
        enum class StreamTarget { WEB, LOOPBACK };
        using CaptureSources = std::vector<std::string>;

        CaptureTarget   capture_target;
        StreamTarget    stream_target;

        CaptureSources  capture_sources;
        uint            selected_source_idx;

        uint            target_fps;
        uint            target_br_kbps;
    };
    struct NetworkConfigTx
    {
        std::string stream_id;
        std::string user_name;
        std::string user_pwd;

        std::string srt_passphrase;

        std::string server_ip;
        uint        server_port;
    };
    struct NetworkConfigRx
    {
        std::string stream_id;
        std::string user_name;
        std::string user_pwd;

        std::string srt_passphrase;

        std::string server_ip;
        uint        server_port;
    };
    struct LogEntry 
    {
        LogKind     level;
        std::string text;
    };

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(NetworkConfigTx, stream_id, user_name, user_pwd, srt_passphrase, server_ip, server_port);
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(NetworkConfigRx, stream_id, user_name, user_pwd, srt_passphrase, server_ip, server_port);
};

#endif /* APP_MODELS_H_ */