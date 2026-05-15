#include <common.h>
#include <encoder/stream-encoder.h>
#include <iostream>

int StreamEncoder::avio_write_packet(void *opaque, const uint8_t *buf, int buf_size)
{
    auto* muxed_buffer = static_cast<std::vector<uint8_t>*>(opaque);
    muxed_buffer->insert(muxed_buffer->end(), buf, buf + buf_size);
    return buf_size;
}

bool StreamEncoder::init(const EncoderConfig& config)
{
    _config = config;

    const AVCodec* codec = avcodec_find_encoder_by_name("libx264");
    if (!codec)
    {
        std::cerr << "libx264 codec not found! Check vcpkg.json features.\n";
        return false;
    }

    _codec_context = avcodec_alloc_context3(codec);
    if (!_codec_context)
        return false;

    _codec_context->width = config.width;
    _codec_context->height = config.height;
    _codec_context->time_base = { 1, static_cast<int>(config.fps) };
    _codec_context->framerate = { static_cast<int>(config.fps), 1 };
    _codec_context->pix_fmt = AV_PIX_FMT_YUV420P;
    _codec_context->bit_rate = config.bitrate_kbps * 1000;
    
    if (_format_context && _format_context->oformat->flags & AVFMT_GLOBALHEADER) 
    {
        _codec_context->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    av_opt_set(_codec_context->priv_data, "preset", "ultrafast", 0);
    av_opt_set(_codec_context->priv_data, "tune", "zerolatency", 0);

    if (avcodec_open2(_codec_context, codec, nullptr) < 0)
    {
        std::cerr << "Could not open codec.\n";
        return false;
    }

    _frame = av_frame_alloc();
    _frame->format = _codec_context->pix_fmt;
    _frame->width  = _codec_context->width;
    _frame->height = _codec_context->height;
    av_frame_get_buffer(_frame, 0);

    _packet = av_packet_alloc();

    _sws_context = sws_getContext(
        config.width, config.height, AV_PIX_FMT_BGRA,
        config.width, config.height, AV_PIX_FMT_YUV420P,
        SWS_FAST_BILINEAR, nullptr, nullptr, nullptr
    );

    if (avformat_alloc_output_context2(&_format_context, nullptr, "mpegts", nullptr) < 0) 
    {
        std::cerr << "Failed to allocate format context\n";
        return false;
    }

    // 2. Создаем видеопоток внутри контейнера
    _video_stream = avformat_new_stream(_format_context, nullptr);
    if (!_video_stream) 
    {
        std::cerr << "Failed to create new stream\n";
        return false;
    }

    if (avcodec_parameters_from_context(_video_stream->codecpar, _codec_context) < 0) 
    {
        std::cerr << "Failed to copy codec parameters\n";
        return false;
    }

    const int avio_ctx_buffer_size = 4096;
    _avio_buffer = static_cast<uint8_t*>(av_malloc(avio_ctx_buffer_size));
    
    _avio_context = avio_alloc_context(
        _avio_buffer, avio_ctx_buffer_size, 1, 
        &_muxed_data,
        nullptr, &StreamEncoder::avio_write_packet, nullptr
    );

    _format_context->pb = _avio_context;
    _format_context->flags |= AVFMT_FLAG_CUSTOM_IO;

    if (avformat_write_header(_format_context, nullptr) < 0) 
    {
        std::cerr << "Error occurred when writing header\n";
        return false;
    }

    _frame_pts = 0;
    return true;
}

void StreamEncoder::release()
{
    if (_format_context) 
    {
        if (_avio_context) 
        {
            av_freep(&_avio_context->buffer);
            avio_context_free(&_avio_context);
        }

        avformat_free_context(_format_context);
        _format_context = nullptr;
    }

    if (_sws_context) { sws_freeContext(_sws_context); _sws_context = nullptr; }
    if (_frame) { av_frame_free(&_frame); }
    if (_packet) { av_packet_free(&_packet); }
    if (_codec_context) { avcodec_free_context(&_codec_context); }
}

std::vector<uint8_t> StreamEncoder::encode_h264_from_bgra(const cv::Mat& frame)
{
    _muxed_data.clear();

    if (frame.empty() || !_codec_context || !_sws_context)
        return _muxed_data;

    cv::Mat continuous_frame;
    if (frame.isContinuous())
        continuous_frame = frame;
    else
        frame.copyTo(continuous_frame);

    const uint8_t* in_data[1] = { continuous_frame.data };
    int in_linesize[1] = { static_cast<int>(continuous_frame.step[0]) };

    sws_scale(_sws_context, in_data, in_linesize, 0, _config.height, _frame->data, _frame->linesize);

    _frame->pts = _frame_pts++;

    int ret = avcodec_send_frame(_codec_context, _frame);
    if (ret < 0)
        return _muxed_data;

    while (ret >= 0)
    {
        ret = avcodec_receive_packet(_codec_context, _packet);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break; 
        else if (ret < 0)
        {
            std::cerr << "Error during encoding!\n";
            break;
        }

        av_packet_rescale_ts(_packet, _codec_context->time_base, _video_stream->time_base);
        _packet->stream_index = _video_stream->index;

        av_interleaved_write_frame(_format_context, _packet);        
        av_packet_unref(_packet);
    }

    return _muxed_data;
}