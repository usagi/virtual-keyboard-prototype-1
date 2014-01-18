#pragma once

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
    if(a.total() != b.total())
      throw std::logic_error("a.total() != b.total()");
    
    cv::Mat r(a.rows, a.cols, a.type());
    
    const auto ia = a.begin<T_a_pixel>();
    const auto ea = a.end  <T_a_pixel>();
    const auto ib = b.begin<T_b_pixel>();
    const auto ir = r.begin<T_a_pixel>();
    
    auto op = [&](T_a_pixel ap, const T_b_pixel bp)
    {
      auto p = &ap.x;
      const auto e = p + (sizeof(T_a_pixel) / sizeof(typename T_a_pixel::value_type));
      while(p < e)
        *p++ *= bp;
      return std::move(ap);
    };
    
    std::transform(ia, ea, ib, ir, op);
    
    return std::move(r);
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
  
  /*
  // in : cv::Mat<CV_32FC3(HSV96)>
  // out: [ H[value([0.f-360.f)), count], S[value([0.f-1.f]), count], V[value[0-255], count] >
  inline std::array<std::map<float, size_t>, 3> cv_mat_hsv_hist_from_HSV96(const cv::Mat& m)
  {
    std::array<std::map<float, size_t>, 3> r;
    
    using value_type = std::array<float, 3>;
    const auto i = m.begin<value_type>();
    const auto e = i + m.total();
    
    std::for_each(i, e, [&](const value_type& v){
      for(size_t n = 0; n < 3; ++n)
      {
        auto& r_ = r[n];
        auto  v_ = v[n];
        
        if(r_.find(v_) != std::end(r_))
          ++r_[v_];
        else
          r_.emplace(v_, 1);
      }
    });
    
    return std::move(r);
  }
  */
  /*
  // in : cv::Mat<CV_8UC3(BGR24)>
  // out: [ H[value([0.f-360.f)), count], S[value([0.f-1.f]), count], V[value[0-255], count] >
  inline std::array<std::map<float, size_t>, 3> cv_mat_hsv_hist_from_BGR24(const cv::Mat& m)
  {
    cv::Mat hsv;
    m.convertTo(hsv, CV_32F);
    cv::cvtColor(hsv, hsv, CV_BGR2HSV);
    return cv_mat_hsv_hist_from_HSV96(hsv);
  }
  
  // in : cv::Mat<CV_8UC3(BGR24)>
  // out: [ B[value([0-255]), count], G[value([0-255]), count], R[value[0-255], count] >
  inline std::array<std::array<size_t, std::numeric_limits<uint8_t>::max() + 1>, 3> cv_mat_bgr_hist_from_BGR24(const cv::Mat& m)
  {
    using result_t = std::array
    < std::array
      < size_t
      , std::numeric_limits<uint8_t>::max() + 1
      >
    , 3
    >;
    
    auto r = result_t();
    
    using value_type = std::array<uint8_t, 3>;
    const auto i = m.begin<value_type>();
    const auto e = i + m.total();
    
    std::for_each(i, e, [&](const value_type& v){
      for(size_t n = 0; n < 3; ++n)
        ++r[n][v[n]];
    });
    
    return std::move(r);
  }
  */
  /*
  inline void write_file_hsv_hist
  ( const std::array<std::map<float, size_t>, 3>& hsv_hist
  , const std::string& filename_h = "hist_h.txt"
  , const std::string& filename_s = "hist_s.txt"
  , const std::string& filename_v = "hist_v.txt"
  )
  {
    const std::array<std::string, 3> filenames{{filename_h, filename_s, filename_v}};
    for(size_t n = 0; n < hsv_hist.size(); ++n)
    {
      std::ofstream f(filenames[n]);
      for(const auto v: hsv_hist[n])
        f << v.first << '\t' << v.second << '\n';
    }
  }
  
  inline void write_file_bgr_hist
  ( const std::array<std::array<size_t, std::numeric_limits<uint8_t>::max() + 1>, 3>& bgr_hist
  , const std::string& filename = "hist_bgr.txt"
  )
  {
    for(size_t n = 0; n < bgr_hist.size(); ++n)
    {
      std::ofstream f(filename);
      for(size_t n = 0; n < std::numeric_limits<uint8_t>::max() + 1; ++n)
        f << n << "\t" << int(bgr_hist[0][n]) << '\t' << int(bgr_hist[1][n]) << '\t'<< int(bgr_hist[2][n]) << '\n';
    }
  }
  */
}
