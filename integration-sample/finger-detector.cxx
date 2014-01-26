#include "finger-detector.hxx"

#include <algorithm>
#include <memory>
#include <limits>
#include <cassert>

namespace
{
  // in[0]: source
  // in[1]: unorm_map
  // out  : cross-ed result
  template<class T_a_pixel, class T_b_pixel>
  cv::Mat cv_mat_cross_with_unorm(const cv::Mat& a, const cv::Mat& b)
  {
    //assert(a.total() == b.total())
    
    cv::Mat r(a.rows, a.cols, a.type());
    
    //assert(a.isContinuous() && b.isContinuous() && r.isContinuous());
          auto ia = reinterpret_cast<typename T_a_pixel::value_type*>(a.data);
    const auto ea = ia + a.total() * a.elemSize();
          auto ib = reinterpret_cast<T_b_pixel*>(b.data);
          auto ir = reinterpret_cast<typename T_a_pixel::value_type*>(r.data);
    
    while(ia < ea)
    {
      const auto ea2 = ia + sizeof(T_a_pixel) / sizeof(typename T_a_pixel::value_type);
      while(ia < ea2)
        *ir++ = *ia++ * *ib;
      ib++;
    }
    
    return r;
  }
  
  // in : cv::Mat<CV_32FC3(HSV96)>
  // out: cv::Mat<CV_8UC1(B1)>
  cv::Mat filter_hsv_from_HSV96
  ( const cv::Mat& src
  , const float h_min, const float h_max
  , const float s_min, const float s_max
  , const float v_min, const float v_max
  )
  {
    cv::Mat dst(src.rows, src.cols, CV_8UC1);
    
    using result_element_t = uint8_t;
    using pixel_t = cv::Point3f;
    
    const auto isrc = src.begin<pixel_t>();
    const auto esrc = src.end  <pixel_t>();
    auto       idst = dst.begin<result_element_t>();
    
    std::transform(isrc, esrc, idst, [&](const pixel_t& p)
    {
      return
      (
        ( ( p.x >= h_min && p.x <= h_max ) || ( h_max > 360.f &&  ( p.x >= h_min || p.x <= h_max - 360.f ) ) )
        &&  p.y >= s_min && p.y <= s_max
        &&  p.z >= v_min && p.z <= v_max
      )
        ? 1 // std::numeric_limits<result_element_t>::max
        : 0 // std::numeric_limits<result_element_t>::min
        ;
    });
    
    return std::move(dst);
  }
  
  // in : cv::Mat<CV_8UC3(BGR24)>
  // out: cv::Mat<CV_8UC1(B1)>
  cv::Mat filter_hsv_from_BGR24
  ( const cv::Mat& src
  , const float h_min, const float h_max
  , const float s_min, const float s_max
  , const float v_min, const float v_max
  )
  {
    cv::Mat hsv;
    src.convertTo(hsv, CV_32F);
    cv::cvtColor(hsv, hsv, CV_BGR2HSV);
    return filter_hsv_from_HSV96(hsv, h_min, h_max, s_min, s_max, v_min, v_max);
  }
}

namespace arisin
{
  namespace etupirka
  {
    finger_detector_t::finger_detector_t(const configuration_t& conf, bool is_top)
    {
      DLOG(INFO) << "ctor";
      set(conf, is_top);
    }
    
    void finger_detector_t::set(const configuration_t& c, bool is_top)
    {
      DLOG(INFO) << "is_top: " << is_top;
      set
      ( is_top
          ? c.finger_detector_top
          : c.finger_detector_front
      );
    }
    
    void finger_detector_t::set(const configuration_t::finger_detector_configuration_t& c)
    {
      set_pre_bilateral(c.pre_bilateral_d, c.pre_bilateral_sc, c.pre_bilateral_ss);
      set_pre_morphology(c.pre_morphology_n);
      set_hsv(c.hsv_h_min, c.hsv_h_max, c.hsv_s_min, c.hsv_s_max, c.hsv_v_min, c.hsv_v_max);
      set_nail_morphology(c.nail_morphology_n);
      set_nail_median_blur(c.nail_median_blur_ksize);
      set_circles(c.circles_dp, c.circles_min_dist, c.circles_param_1, c.circles_param_2, c.circles_min_radius, c.circles_max_radius);
    }
    
    void finger_detector_t::set_pre_bilateral(double d, double sc, double ss)
    {
      pre_bilateral_d_ = d;
      pre_bilateral_sc_ = sc;
      pre_bilateral_ss_ = ss;
      DLOG(INFO) << "set_pre_bilateral d, sc, ss: " << d << ", " << sc << ", " << ss;
    }
    
    void finger_detector_t::set_pre_morphology(int n)
    {
      pre_morphology_n_ = n;
      DLOG(INFO) << "set_pre_morphology n: " << n;
    }
    
    void finger_detector_t::set_hsv(float hsv_h_min, float hsv_h_max, float hsv_s_min, float hsv_s_max, float hsv_v_min, float hsv_v_max)
    {
      hsv_h_min_ = hsv_h_min;
      hsv_h_max_ = hsv_h_max;
      hsv_s_min_ = hsv_s_min;
      hsv_s_max_ = hsv_s_max;
      hsv_v_min_ = hsv_v_min;
      hsv_v_max_ = hsv_v_max;
      DLOG(INFO) << "set_hsv h-min, h-max, s-min, s-max, v-min, v-max: " << hsv_h_min << ", " << hsv_h_max << ", " << hsv_s_min << ", " << hsv_s_max << ", " << hsv_v_min << ", " << hsv_v_max << ", ";
    }
    
    void finger_detector_t::set_nail_morphology(int n)
    {
      nail_morphology_n_ = n;
      DLOG(INFO) << "set_nail_morphology n: " << n;
    }
    
    void finger_detector_t::set_nail_median_blur(int ksize)
    {
      if(ksize % 2 == 0)
      {
        LOG(WARNING) << "ksize(" << ksize << ") is not even number, fix to " << ksize + 1;
        ++ksize;
      }
      
      nail_median_blur_ksize_ = ksize;
      DLOG(INFO) << "set_nail_median_blur ksize: " << ksize;
    }
    
    void finger_detector_t::set_circles(double dp, double min_dist, double param_1, double param_2, int min_radius, int max_radius)
    {
      if(dp < 1)
      {
        LOG(WARNING) << "dp(" << dp << ") cannot set less than 1, fix to 1";
        dp = 1;
      }
      
      if(min_dist < 1)
      {
        LOG(WARNING) << "min_dist(" << min_dist << ") cannot set less than 1, fix to 1";
        min_dist = 1;
      }
      
      circles_dp_ = dp;
      circles_min_dist_ = min_dist;
      circles_param_1_ = param_1;
      circles_param_2_ = param_2;
      circles_min_radius_ = min_radius;
      circles_max_radius_ = max_radius;
      DLOG(INFO) << "set_circles dp, min_dist, param_1, param_2, min_radius, max_radius: " << dp << ", " << min_dist << ", " << param_1 << ", " << param_2 << ", " << min_radius << ", " << max_radius;
    }
    
    const cv::Mat& finger_detector_t::effected_frame() const
    { return pre_nail_frame; }
    
    finger_detector_t::circles_t finger_detector_t::operator()(const cv::Mat& frame)
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
      
      //cv::Mat pre_nail_frame;
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
        DLOG(INFO) << "circle x, y, r: " << circle[0] << ", " << circle[1] << ", " << circle[2];
#endif
      return circles;
    }
  }
}