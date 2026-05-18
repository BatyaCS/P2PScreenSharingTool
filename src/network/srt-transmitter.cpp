#include <common.h>

#include <winsock2.h>
#include <ws2tcpip.h>
#include <srt/srt.h>

#include <network/srt-transmitter.h>
#include <algorithm>

SrtTransmitter::SrtTransmitter()
    : _socket(SRT_INVALID_SOCK)
{}

bool SrtTransmitter::is_connected() const 
{ 
    return _socket != SRT_INVALID_SOCK; 
}

bool SrtTransmitter::open_connection(const NetworkConfig& config) 
{
    close_connection();

    _socket = srt_create_socket();
    if (_socket == SRT_INVALID_SOCK)
    {
        LOG_ERROR("Failed to create SRT socket to open connection!\n");
        return false;
    }

    srt_setsockopt(_socket, 0, SRTO_STREAMID, config.stream_id.c_str(), static_cast<int>(config.stream_id.length()));
    
    if (!config.stream_pdw.empty()) 
    {
        const int key_len = static_cast<int>(config.encryption);
        const int pwd_len = static_cast<int>(config.stream_pdw.length());
        
        if (pwd_len > key_len)
        {
            LOG_ERROR("Incorrect password length is set to open connection!\n");
            return false;
        }
        
        srt_setsockopt(_socket, 0, SRTO_PBKEYLEN, &key_len, sizeof(key_len));
        srt_setsockopt(_socket, 0, SRTO_PASSPHRASE, config.stream_pdw.c_str(), pwd_len);
    }

    const int buf_size = static_cast<int>(config.buffer_size);
    srt_setsockopt(_socket, 0, SRTO_SNDBUF, &buf_size, sizeof(buf_size));

    const int pkt_drop = static_cast<int>(config.packet_drop);
    srt_setsockopt(_socket, 0, SRTO_TLPKTDROP, &pkt_drop, sizeof(pkt_drop));

    const int latency_ms = static_cast<int>(config.stream_latency_ms);
    srt_setsockopt(_socket, 0, SRTO_LATENCY, &latency_ms, sizeof(latency_ms));
    
    sockaddr_in sa{};
        
    sa.sin_family = AF_INET;
    sa.sin_port = htons(config.port);
    inet_pton(AF_INET, config.ip.c_str(), &sa.sin_addr);

    if (SRT_ERROR == srt_connect(_socket, (struct sockaddr*)&sa, sizeof(sa))) 
    {
        LOG_ERROR("Failed to init SRT connect! Error: %s\n", srt_getlasterror_str());
        
        close_connection();
        return false;
    }

    LOG("SRT Connection opened to %s:%u!\n", config.ip.c_str(), config.port);
    return true;
}

bool SrtTransmitter::close_connection()
{
    if (_socket == SRT_INVALID_SOCK)
        return true;

    if (SRT_ERROR == srt_close(_socket))
    {
        LOG_ERROR("Failed to close SRT connection on socket %d\n");
        return false;
    }

    _socket = SRT_INVALID_SOCK;
    return true;
}

void SrtTransmitter::send(const std::vector<uint8_t>& data) 
{
    if (_socket == SRT_INVALID_SOCK || data.empty()) 
        return;

    std::lock_guard<std::mutex> lock(_send_mutex);

    const size_t max_payload = 1316;

    size_t bytes_sent = 0;
    size_t total_size = data.size();

    while (bytes_sent < total_size) 
    {

        const size_t bytes_left = total_size - bytes_sent;
        const size_t bytes_to_send = std::min(max_payload, bytes_left);
        
        int res = srt_sendmsg(_socket, 
                             (const char*)(data.data() + bytes_sent), 
                             (int)bytes_to_send, 
                             -1, 0);
        
        if (res == SRT_ERROR) {
            std::cerr << "SRT Send Chunk Error: " << srt_getlasterror_str() << std::endl;
            break;
        }
        
        bytes_sent += bytes_to_send;
    }
}
