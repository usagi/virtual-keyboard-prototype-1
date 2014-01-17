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
    {
      DLOG(INFO) << "top-cam-id(" << top_camera_id_ << ") front-cam-id(" << front_camera_id_ << ") width(" << width_ << ") height(" << height_ << ")";
      
      captures[top].set(CV_CAP_PROP_FRAME_HEIGHT, height_);
      captures[top].set(CV_CAP_PROP_FRAME_WIDTH , width_);
      DLOG(INFO) << "top-cam set height and width";
      
      captures[front].set(CV_CAP_PROP_FRAME_HEIGHT, height_);
      captures[front].set(CV_CAP_PROP_FRAME_WIDTH , width_);
      DLOG(INFO) << "front-cam set height and width";
      
      captures[top].open(top_camera_id_);
      DLOG(INFO) << "top-cam opened";
      
      captures[front].open(front_camera_id_);
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