#ifndef APPLICATION_H_
#define APPLICATION_H_

#include <ui/application-ui.h>
#include <network/srt-transmitter.h>

#include <capturer/hw-video-capturer.h>
#include <encoder/hw-stream-encoder.h>

#include <mutex>
#include <fstream>
#include <vector>

class Application
{
public:
    Application() = default;
    ~Application() { cleanup(); }

    bool init();
    void cleanup();

    void run();

private:
    using StreamTarget = ApplicationUI::UiStreamConfig::StreamTarget;

    bool start_streaming();
    void stop_streaming();

    void handle_frame_captured(ID3D11Texture2D* tex, ID3D11Device* dev);
    void handle_frame_received(ID3D11Texture2D* tex, ID3D11Device* dev) {}

    void save_frame_for_loopback(ID3D11Texture2D* tex, ID3D11Device* dev);

    // UI Handlers
    void handle_start_stop_stream();
    void handle_start_stop_preview();
    void handle_sources_update();

    ApplicationUI   _ui;

    HwVideoCapturer _capturer;    
    HwStreamEncoder _encoder;
    SrtTransmitter  _srt_sender;

    std::mutex      _loopback_mutex;
    uint            _loopback_w = 0, _loopback_h = 0;

    ID3D11ShaderResourceView* _current_loopback_srv = nullptr;
    ID3D11ShaderResourceView* _srv_to_release = nullptr;

    bool            _is_stream_enabled = false;
    bool            _is_preview_enabled = false;
};

#endif /* APPLICATION_H_ */