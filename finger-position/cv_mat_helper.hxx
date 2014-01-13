#pragma once

#include <array>
#include <map>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace
{

  inline const std::string cvmat_depth_to_string(const cv::Mat& m)
  {
    const auto depth = m.depth();
    switch(depth)
    {
  #define CASE(x) \
      case x: return #x;
      CASE(CV_8U) CASE(CV_8S)
      CASE(CV_16U) CASE(CV_16S)
      CASE(CV_32S) CASE(CV_32F)
      CASE(CV_64F)
      CASE(CV_USRTYPE1)
  #undef CASE
    }
    return std::string("unknown(") + std::to_string(depth) + ")";
  }

  inline const std::string cvmat_type_to_string(const cv::Mat& m)
  {
    const auto type = m.type();
    switch(type)
    {
  #define CASE(x) \
      case x: return #x;
      CASE(CV_8UC1) CASE(CV_8UC2) CASE(CV_8UC3) CASE(CV_8UC4)
      CASE(CV_8SC1) CASE(CV_8SC2) CASE(CV_8SC3) CASE(CV_8SC4)
      CASE(CV_16UC1) CASE(CV_16UC2) CASE(CV_16UC3) CASE(CV_16UC4)
      CASE(CV_16SC1) CASE(CV_16SC2) CASE(CV_16SC3) CASE(CV_16SC4)
      CASE(CV_32SC1) CASE(CV_32SC2) CASE(CV_32SC3) CASE(CV_32SC4)
      CASE(CV_32FC1) CASE(CV_32FC2) CASE(CV_32FC3) CASE(CV_32FC4)
      CASE(CV_64FC1) CASE(CV_64FC2) CASE(CV_64FC3) CASE(CV_64FC4)
  #undef CASE
      default:
        for(size_t n = 5 ; n < 256; ++n)
          if(type == CV_8UC(n))
            return std::string("CV_8UC(") + std::to_string(n) + ")";
          else if(type == CV_8SC(n))
            return std::string("CV_8SC(") + std::to_string(n) + ")";
          else if(type == CV_16UC(n))
            return std::string("CV_16UC(") + std::to_string(n) + ")";
          else if(type == CV_16SC(n))
            return std::string("CV_16SC(") + std::to_string(n) + ")";
          else if(type == CV_32SC(n))
            return std::string("CV_32SC(") + std::to_string(n) + ")";
          else if(type == CV_32FC(n))
            return std::string("CV_32FC(") + std::to_string(n) + ")";
          else if(type == CV_64FC(n))
            return std::string("CV_64FC(") + std::to_string(n) + ")";
    }
    return std::string("unknown(") + std::to_string(type) + ")";
  }

  inline void print_cvmat_info(const cv::Mat& cvmat)
  {
    std::cerr
      << "type         : " << cvmat_type_to_string(cvmat) << " [-]\n"
      << "depth        : " << cvmat_depth_to_string(cvmat) << " [-]\n"
      << "channels     : " << cvmat.channels() << " [#]\n"
      << "cols         : " << cvmat.cols << " [#]\n"
      << "rows         : " << cvmat.rows << " [#]\n"
      << "element size : " << cvmat.elemSize() << " [bytes]\n"
      << "channel size : " << cvmat.elemSize1() << " [bytes]\n"
      << "row size     : " << cvmat.step1() << " [#]\n"
      << "row size     : " << cvmat.step << " [bytes]\n"
      << "data size    : " << size_t(cvmat.rows) * cvmat.step << " [bytes]\n"
      << "is continuous: " << std::boolalpha << cvmat.isContinuous() << "\n"
      ;
  }
}
