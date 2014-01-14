#include "camera-capture.hxx"

namespace arisin
{
  namespace etupirka
  {
    camera_capture_t::camera_capture_t(int top_camera_id__, int front_camera_id__, int width__, int height__)
      : top_camera_id_(top_camera_id__)
      , front_camera_id_(front_camera_id__)
      , width_(width__)
      , height_(height__)
    {
      L(INFO, "top-cam-id(" << top_camera_id__ << ") front-cam-id(" << front_camera_id__ << ") width(" << width__ << ") height(" << height__ << ")");
      
      captures[top].set(CV_CAP_PROP_FRAME_HEIGHT, height__);
      captures[top].set(CV_CAP_PROP_FRAME_WIDTH , width__ );
      L(INFO, "top-cam set height and width");
      
      captures[front].set(CV_CAP_PROP_FRAME_HEIGHT, height__);
      captures[front].set(CV_CAP_PROP_FRAME_WIDTH , width__ );
      L(INFO, "front-cam set height and width");
      
      captures[top].open(top_camera_id__);
      L(INFO, "top-cam opened");
      
      captures[front].open(front_camera_id__);
      L(INFO, "front-cam opened");
    }
    
    camera_capture_t::captured_frames_t camera_capture_t::operator()()
    {
      camera_capture_t::captured_frames_t r;
      
      cv::Mat tmp;
      
      captures[top] >> tmp;
      r.top = tmp.clone();
      L(INFO, "top-cam captured");
      
      captures[front] >> tmp;
      r.front = tmp.clone();
      L(INFO, "front-cam captured");
      
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