#ifndef STREAM_ENCODER_H_
#define STREAM_ENCODER_H_

#include <vector>
#include <cstdint>
#include <opencv2/opencv.hpp>

// FFmpeg is a C library, so we must include it inside extern "C"
extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

struct EncoderConfig
{
    unsigned int width = 1280;
    unsigned int height = 720;
    unsigned int fps = 30;
    unsigned int bitrate_kbps = 2500;
};

class StreamEncoder
{
public:
    StreamEncoder() = default;
    ~StreamEncoder() { release(); }

    StreamEncoder(const StreamEncoder&) = delete;
    StreamEncoder& operator=(const StreamEncoder&) = delete;

    bool init(const EncoderConfig& config);
    void release();

    // Takes a raw BGRA frame from OpenCV and returns an encoded H.264 packet
    std::vector<uint8_t> encode_frame(const cv::Mat& frame);

private:
    AVCodecContext* _codec_context = nullptr;
    AVFrame* _frame = nullptr;
    AVPacket* _packet = nullptr;
    SwsContext* _sws_context = nullptr;

    int             _frame_pts = 0;
    EncoderConfig   _config;
};

#endif /* STREAM_ENCODER_H_ */