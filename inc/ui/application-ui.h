#ifndef APPLICATION_UI_H_
#define APPLICATION_UI_H_

#include <ui/video-preview-widget.h>
#include <model/app-view-model.h>
#include <functional>

#include <GLFW/glfw3.h>
#include <d3d11.h>

class ApplicationUI
{
    static constexpr uint TARGET_FPS_MIN = 10;
    static constexpr uint TARGET_FPS_MAX = 60;

    static constexpr uint TARGET_BITRATE_MIN = 500;
    static constexpr uint TARGET_BITRATE_MAX = 10000;

public:
    struct UiConfig
    {
        std::string window_title;

        uint width;
        uint height;
    };

    using StartStopStreamCallback = std::function<void()>;
    using StartStopRxCallback = std::function<void()>;
    using SourcesUpdateCallback = std::function<void()>;

    ApplicationUI() = default;
    ~ApplicationUI() { shutdown(); }

    bool render(AppViewModel& view);

    bool init(const UiConfig& config);
    void shutdown();

    ID3D11Device* get_d3d11_device() const { return _pd3dDevice; }

    void set_web_texture(void * srv, uint w, uint h) { _web_srv = srv; _web_w = w; _web_h = h; }
    void set_loopback_texture(void * srv, uint w, uint h) { _loopback_srv = srv; _loopback_w = w; _loopback_h = h; }

    void set_start_stop_stream_callback(StartStopStreamCallback callback) { _start_stop_stream_callback = callback; }
    void set_start_stop_rx_callback(StartStopRxCallback callback) { _start_stop_rx_callback = callback; }
    void set_sources_update_callback(SourcesUpdateCallback callback) { _sources_update_callback = callback; }

private:
    bool render_broadcaster_tab(AppViewModel& view);
    bool render_web_preview_tab(AppViewModel& view);

    void render_capture_settings(AppViewModel& view);
    void render_encoding_settings(AppViewModel& view);
    void render_network_tx_settings(AppViewModel& view);
    void render_network_rx_settings(AppViewModel& view);

    void render_log_window(AppViewModel& view);

    bool CreateDeviceD3D(HWND hWnd);
    void CleanupDeviceD3D();
    void CreateRenderTarget();
    void CleanupRenderTarget();

    GLFWwindow *                _window = nullptr;
    ID3D11Device *              _pd3dDevice = nullptr;
    ID3D11DeviceContext *       _pd3dDeviceContext = nullptr;
    IDXGISwapChain *            _pSwapChain = nullptr;
    ID3D11RenderTargetView *    _mainRenderTargetView = nullptr;

    VideoPreviewWidget          _web_preview_widget;
    VideoPreviewWidget          _loopback_preview_widget;

    void *                      _web_srv = nullptr;
    void *                      _loopback_srv = nullptr;

    uint                        _web_w = 0, _web_h = 0;
    uint                        _loopback_w = 0, _loopback_h = 0;
   
    StartStopStreamCallback     _start_stop_stream_callback;
    StartStopRxCallback         _start_stop_rx_callback;
    SourcesUpdateCallback       _sources_update_callback;

    bool                        _scroll_to_bottom = false;
};

#endif /* APPLICATION_UI_H_ */