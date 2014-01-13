#pragma once

#include <Magick++.h>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace
{
  

  constexpr const Magick::StorageType convert_from_cvmat_depth_to_magick_storage_type(const int src)
  {
    return (src == CV_8U)  ? Magick::CharPixel
        : (src == CV_64F) ? Magick::DoublePixel
        : (src == CV_32F) ? Magick::FloatPixel
  //     : (src == CV_32S) ? Magick::IntegerPixel
  //     : (src == CV_64S) ? Magick::LongPixel
  //     : (src == CV_16S) ? Magick::ShortPixel
        :                   MagickCore::UndefinedPixel
        ;
  }

  inline Magick::Image convert_to_magick_image(const cv::Mat& src, const std::string& src_colorspace = "BGR")
  {
    if(!src.isContinuous())
      throw std::runtime_error("abort convert; cv::Mat::isContinuous == false");
    
    return Magick::Image
      ( src.cols
      , src.rows
      , src_colorspace
      , convert_from_cvmat_depth_to_magick_storage_type(src.depth())
      , reinterpret_cast<void*>(src.data)
      );
  }

}
