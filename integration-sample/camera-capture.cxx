#include "camera-capture.hxx"

namespace arisin
{
  namespace etupirka
  {
    camera_capture_t::camera_capture_t(const configuration_t& conf)
      : top_camera_id_(conf.camera_capture.top_camera_id)
      , front_camera_id_(conf.camera_capture.front_camera_id)
      , width_(conf.camera_capture.width)
      , height_(conf.camera_capture.height)
      , video_file_top_(conf.video_file_top)
      , video_file_front_(conf.video_file_front)
    {
      DLOG(INFO) << "top-cam-id: "       << top_camera_id_;
      DLOG(INFO) << "front-cam-id: "     << front_camera_id_;
      DLOG(INFO) << "width: "            << width_;
      DLOG(INFO) << "height: "           << height_;
      DLOG(INFO) << "video-file-top: "   << video_file_top_;
      DLOG(INFO) << "video-file-front: " << video_file_front_;
      
      if(conf.video_file_top.empty())
      {
        captures[top].set(CV_CAP_PROP_FRAME_HEIGHT, height_);
        captures[top].set(CV_CAP_PROP_FRAME_WIDTH , width_);
        DLOG(INFO) << "top-cam set height and width";
      }
      
      if(conf.video_file_front.empty())
      {
        captures[front].set(CV_CAP_PROP_FRAME_HEIGHT, height_);
        captures[front].set(CV_CAP_PROP_FRAME_WIDTH , width_);
        DLOG(INFO) << "front-cam set height and width";
      }
      
      if(conf.video_file_top.empty())
        captures[top].open(top_camera_id_);
      else
        captures[top].open(conf.video_file_top);
      
      if(!captures[top].isOpened())
        LOG(FATAL) << "top-cam can not opened";
      DLOG(INFO) << "top-cam opened";
      
      if(conf.video_file_front.empty())
        captures[front].open(front_camera_id_);
      else
        captures[front].open(conf.video_file_front);
        
      if(!captures[front].isOpened())
        LOG(FATAL) << "front-cam can not opened";
      DLOG(INFO) << "front-cam opened";
    }
    
    camera_capture_t::captured_frames_t camera_capture_t::operator()()
    {
      camera_capture_t::captured_frames_t r;
      
      cv::Mat tmp;
      
      captures[top] >> tmp;
      r.top = tmp.clone();
      DLOG(INFO) << "top-cam captured";
      
      captures[front] >> tmp;
      r.front = tmp.clone();
      DLOG(INFO) << "front-cam captured";
      
      if(!video_file_top_.empty() && captures[top].get(CV_CAP_PROP_POS_FRAMES) == captures[top].get(CV_CAP_PROP_FRAME_COUNT))
      {
        DLOG(INFO) << "top-cam to reload video file: " << video_file_top_;
        captures[top].release();
        captures[top].open(video_file_top_);
        if(!captures[top].isOpened())
          LOG(FATAL) << "top-cam can not opened";
      }
      
      if(!video_file_front_.empty() && captures[front].get(CV_CAP_PROP_POS_FRAMES) == captures[front].get(CV_CAP_PROP_FRAME_COUNT))
      {
        DLOG(INFO) << "front-cam to reload video file: " << video_file_front_;
        captures[front].release();
        captures[front].open(video_file_front_);
        if(!captures[front].isOpened())
          LOG(FATAL) << "front-cam can not opened";
      }
      
      return std::move(r);
    }
    
    const int camera_capture_t::top_camera_id() const
    { return top_camera_id_; }
    
    const int camera_capture_t::front_camera_id() const
    { return front_camera_id_; }
    
    const int camera_capture_t::width() const
    { return width_; }
    
    const int camera_capture_t::height() const
    { return height_; }

  }
}