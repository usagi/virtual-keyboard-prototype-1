#include "finger-detector.hxx"

namespace arisin
{
  namespace etupirka
  {
    void finger_detector_t::set_pre_bilateral(double d, double sc, double ss)
    {
      pre_bilateral_d_ = d;
      pre_bilateral_sc_ = sc;
      pre_bilateral_ss_ = ss;
      L(INFO, "set_pre_bilateral d, sc, ss: " << d << ", " << sc << ", " << ss);
    }
    
    void finger_detector_t::set_pre_morphology(int n)
    {
      pre_morphology_n_ = n;
      L(INFO, "set_pre_morphology n: " << n);
    }
    
    void finger_detector_t::set_hsv(float hsv_h_min, float hsv_h_max, float hsv_s_min, float hsv_s_max, float hsv_v_min, float hsv_v_max)
    {
      hsv_h_min_ = hsv_h_min;
      hsv_h_max_ = hsv_h_max;
      hsv_s_min_ = hsv_s_min;
      hsv_s_max_ = hsv_s_max;
      hsv_v_min_ = hsv_v_min;
      hsv_v_max_ = hsv_v_max;
      L(INFO, "set_hsv h-min, h-max, s-min, s-max, v-min, v-max: " << hsv_h_min << ", " << hsv_h_max << ", " << hsv_s_min << ", " << hsv_s_max << ", " << hsv_v_min << ", " << hsv_v_max << ", ");
    }
    
    void finger_detector_t::set_nail_morphology(int n)
    {
      nail_morphology_n_ = n;
      L(INFO, "set_nail_morphology n: " << n);
    }
    
    void finger_detector_t::set_nail_median_blur(int ksize)
    {
      if(ksize % 2 == 0)
        ++ksize;
      
      nail_median_blur_ksize_ = ksize;
      L(INFO, "set_nail_median_blur ksize: " << ksize);
    }
    
    void finger_detector_t::set_circles(double dp, double min_dist, double param_1, double param_2, int min_radius, int max_radius)
    {
      circles_dp_ = dp;
      circles_min_dist_ = min_dist;
      circles_param_1_ = param_1;
      circles_param_2_ = param_2;
      circles_min_radius_ = min_radius;
      circles_max_radius_ = max_radius;
      L(INFO, "set_circles dp, min_dist, param_1, param_2, min_radius, max_radius: " << dp << ", " << min_dist << ", " << param_1 << ", " << param_2 << ", " << min_radius << ", " << max_radius);
    }
    
    finger_detector_t::circles_t finger_detector_t::operator()(const cv::Mat& frame) const
    {
      cv::Mat bilateral_frame;
      bilateralFilter(frame, bilateral_frame, pre_bilateral_d_, pre_bilateral_sc_, pre_bilateral_ss_);
      
      cv::Mat morphology_frame;
      cv::morphologyEx(frame, morphology_frame, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1, -1), pre_morphology_n_);
      
      cv::Mat hsv_filtered_map;
      hsv_filtered_map = filter_hsv_from_BGR24
      ( morphology_frame
      , hsv_h_min_, hsv_h_max_
      , hsv_s_min_, hsv_s_max_
      , hsv_v_min_, hsv_v_max_
      );
      
      cv::Mat hsv_frame;
      hsv_frame = cv_mat_cross_with_unorm<cv::Point3_<uint8_t>, uint8_t>(morphology_frame, hsv_filtered_map);
      
      cv::Mat pre_nail_frame;
      {
        // hsv split and get h-channel frame
        cv::Mat h_channel_frame;
        {
          std::vector<cv::Mat> hsv_split_frame;
          cv::split(hsv_frame, hsv_split_frame);
          h_channel_frame = hsv_split_frame[0];
        }
        // normalize
        //cv::?
        
        // morphology: h-channel
        cv::Mat h_channel_morphology_frame;
        cv::morphologyEx(h_channel_frame, h_channel_morphology_frame, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1, -1), nail_morphology_n_);
        
        // median-blur: h-channel
        cv::medianBlur(h_channel_morphology_frame, pre_nail_frame, nail_median_blur_ksize_);
      }
      
      circles_t circles;
      {
        // circles detector
        cv::HoughCircles
        ( pre_nail_frame, circles, CV_HOUGH_GRADIENT
        , circles_dp_, circles_min_dist_
        , circles_param_1_, circles_param_2_
        , circles_min_radius_, circles_max_radius_
        );
        
        // circle filter
        boost::sort(circles, [](const cv::Vec3f& a, const cv::Vec3f& b){ return a[0] < b[0]; });
        const auto e = std::end(circles);
        auto t = std::begin(circles);
        for(auto i = std::begin(circles) + 1; i < e; ++i)
        {
          // i.x - i.r が t.x + t.r 以下にあるかチェック
          //   true : それは既存 t と比べて y 値が大きければ（より下にあれば） t と置換える
          //   false: それは次の t になる
          const auto i_left = (*i)[0] - (*i)[2];
          const auto t_right = (*t)[0] + (*t)[2];
          if( i_left <= t_right )
          {
            if( (*i)[1] > (*t)[1] )
              *t = *i;
          }
          else
            *++t = *i;
        }
        circles.resize(std::distance(std::begin(circles), t + 1));
      }
#ifndef NDEBUG
      for(const auto& circle: circles)
        L(INFO, "circle x, y, r: " << circle[0] << ", " << circle[1] << ", " << circle[2]);
#endif
      return circles;
    }
  }
}