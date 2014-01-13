#pragma once

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace
{
  struct cv_video_helper_t final
  {
    using frames_t = std::vector<cv::Mat>;
    using frames_fps_t = std::tuple<frames_t, int>;
    
    static frames_fps_t load_to_vector(const std::string& video_filename)
    {
      auto video = cv::VideoCapture(video_filename);
      
      if(!video.isOpened())
        throw std::runtime_error("cannot open video file.");
      
      const int fps = int(video.get(CV_CAP_PROP_FPS));
      
      const size_t frame_count = video.get(CV_CAP_PROP_FRAME_COUNT);
      if(frame_count == 0)
        throw std::runtime_error("cannot get frame count");
      
      frames_t frames(frame_count);
      
      for(auto& frame : frames)
      {
        cv::Mat source;
        video >> source;
        source.copyTo(frame);
      }
      
      return make_tuple(std::move(frames), std::move(fps));
    }
    
  private:
    cv_video_helper_t();
  };
}
