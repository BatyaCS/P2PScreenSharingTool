#include <iostream>
#include <mutex>
#include <fstream>
#include <opencv2/opencv.hpp>

#include <capturer/video-capturer.h>
#include <encoder/stream-encoder.h>

int main()
{
    VideoCapturer capturer;
    StreamEncoder  encoder;

    // 1. Setup encoder configuration
    EncoderConfig enc_config;
    enc_config.width = 2560;
    enc_config.height = 1440;
    enc_config.fps = 60;
    enc_config.bitrate_kbps = 5000;

    // Initialize the encoder
    if (!encoder.init(enc_config))
    {
        std::cerr << "Failed to initialize encoder!\n";
        return -1;
    }

    // 2. Setup capturer configuration
    CaptureConfig cap_config;
    cap_config.target = CaptureTarget::MONITOR;

    // Capturer still needs OutputConfig to know target dimensions and capture rate
    OutputConfig out_config;
    out_config.width = enc_config.width;
    out_config.height = enc_config.height;
    out_config.target_fps = enc_config.fps;

    // 3. Open binary file for writing the raw H.264 stream
    std::ofstream output_file("test_frames.h264", std::ios::binary | std::ios::out);
    if (!output_file.is_open())
    {
        std::cerr << "Failed to open output file!\n";
        return -1;
    }

    std::atomic<int> frame_count{0};
    const int max_frames = 300; // We want exactly 300 frames (~1 second of video)

    // 4. Define the callback
    FrameCallback callback = [&](const cv::Mat& frame)
    {
        // Stop processing if we reached the target frame count
        if (frame_count.load() >= max_frames)
        {
            return;
        }

        // Encode the raw OpenCV frame into H.264 packets
        std::vector<uint8_t> packet = encoder.encode_frame(frame);
        
        if (!packet.empty())
        {
            // Write bytes directly to the file
            output_file.write(reinterpret_cast<const char*>(packet.data()), packet.size());
            
            std::cout << "Encoded frame " << (frame_count.load() + 1) << "/" << max_frames 
                      << " | Size: " << packet.size() << " bytes\n";
        }

        frame_count++;
    };

    // 5. Start the capture process
    std::cout << "Starting capture and encoding...\n";
    if (!capturer.start(cap_config, out_config, callback))
    {
        std::cerr << "Failed to start capturer!\n";
        return -1;
    }

    // 6. Wait in the main thread until 30 frames are processed
    while (frame_count.load() < max_frames && capturer.is_running())
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    // 7. Cleanup
    capturer.stop();
    output_file.close();

    std::cout << "Successfully saved " << max_frames << " frames to 'test_frames.h264'.\n";

    return 0;
}

// int main()
// {
//     VideoCapturer capturer;

//     auto windows = VideoCapturer::get_available_windows();
//     std::cout << "--- Available windows for capture ---\n";
//     for (size_t i = 0; i < windows.size(); ++i)
//     {
//         std::cout << "[" << i << "] HWND: " << windows[i].first << " | Title: " << windows[i].second << "\n";
//     }
//     std::cout << "-------------------------------------\n";

//     auto monitors = VideoCapturer::get_available_monitors();
//     std::cout << "--- Available Monitors ---\n";
//     for (size_t i = 0; i < monitors.size(); ++i)
//     {
//         std::cout << "[" << i << "] HMONITOR: " << monitors[i].first << " | " << monitors[i].second << "\n";
//     }
//     std::cout << "--------------------------\n";

//     CaptureConfig cap_config;
//     cap_config.target = CaptureTarget::MONITOR;

//     OutputConfig out_config;
//     out_config.width = 1280;
//     out_config.height = 720;
//     out_config.target_fps = 30;

//     // Shared resources between the capture thread and the main UI thread
//     cv::Mat display_frame;
//     std::mutex frame_mutex;

//     // 3. Define the callback
//     // We use a lambda to capture the shared frame and mutex by reference
//     FrameCallback callback = [&display_frame, &frame_mutex](const cv::Mat& frame)
//     {
//         // Lock the mutex before writing to the shared frame
//         std::lock_guard<std::mutex> lock(frame_mutex);
//         frame.copyTo(display_frame);
//     };

//     // 4. Start the capture
//     if (!capturer.start(cap_config, out_config, callback))
//     {
//         std::cerr << "Failed to start capturer!\n";
//         return -1;
//     }

//     std::cout << "Capture started. Press ESC in the video window to stop.\n";

//     // 5. Main UI loop
//     cv::namedWindow("Stream Preview", cv::WINDOW_NORMAL);
//     cv::resizeWindow("Stream Preview", 800, 600);

//     while (capturer.is_running())
//     {
//         cv::Mat local_frame;
        
//         // Safely read the latest frame from the capture thread
//         {
//             std::lock_guard<std::mutex> lock(frame_mutex);
//             if (!display_frame.empty())
//                 display_frame.copyTo(local_frame);
//         }

//         // Display the frame if it's not empty
//         if (!local_frame.empty())
//             cv::imshow("Stream Preview", local_frame);

//         // Wait for 30ms (~33 FPS) and check if ESC (key code 27) was pressed
//         if (cv::waitKey(30) == 27)
//             break;
//     }

//     // 6. Cleanup
//     capturer.stop();
//     cv::destroyAllWindows();
    
//     std::cout << "Capture stopped gracefully.\n";

//     return 0;
// }