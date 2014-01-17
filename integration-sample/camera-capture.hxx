#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "configuration.hxx"
#include "logger.hxx"

namespace arisin
{
  namespace etupirka
  {
    class camera_capture_t final
    {
    public:
      struct captured_frames_t
      {
        cv::Mat top;
        cv::Mat front;
      };
      
    private:
      static constexpr size_t top   = 0;
      static constexpr size_t front = 1;
      
      std::array<cv::VideoCapture, 2> captures;
      
      int top_camera_id_;
      int front_camera_id_;
      int width_;
      int height_;
      std::string video_file_top_;
      std::string video_file_front_;
      
    public:
      camera_capture_t(const configuration_t& conf);
      captured_frames_t operator()();
      const int top_camera_id() const;
      const int front_camera_id() const;
      const int width() const;
      const int height() const;
    };
  }
}