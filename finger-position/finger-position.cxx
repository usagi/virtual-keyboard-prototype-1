//#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <chrono>
#include <functional>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <limits>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <boost/range/algorithm.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "chrono_helper.hxx"
#include "cv_mat_helper.hxx"
#include "cv_mat_filter.hxx"
#include "cv_gui_helper.hxx"
#include "cv_video_helper.hxx"

int main(const int an, const char* const* const as)
{
  if(an < 2)
    throw std::runtime_error("invalid argument, too few.");
  
  const std::string video_filename(as[1]);
  
  const auto frames_fps = cv_video_helper_t::load_to_vector(video_filename);
  const auto& frames = std::get<0>(frames_fps);
  const auto& fps    = std::get<1>(frames_fps);
  
  using master_pixel_element_t = uint8_t;
  using master_pixel_t = cv::Point3_<master_pixel_element_t>;
  
  std::cout << "video filename          : " << video_filename << "\n"
            << "      first frame width : " << frames.at(0).cols << " [px]\n"
            << "                  height: " << frames.at(0).rows << " [px]\n"
            << "      fps               : " << fps << " [fps]\n"
            << "      frame count       : " << frames.size() << " [#]\n"
            << "      on memory size    : " << frames.size() * sizeof(master_pixel_t) * frames.at(0).total() << " [bytes]\n"
            << "      time              : " << frames.size() / fps << " [sec]\n"
            ;
  
  auto& cv_gui_helper = cv_gui_helper_t::instance();
  
  enum class window { master, bilateral, morphology, hsv, nail, output };
  
  cv_gui_helper.new_windows
  ( cv_gui_helper.make_new_window_params( window::master    , "0: master")
  , cv_gui_helper.make_new_window_params( window::bilateral , "0 --> 1: master --> bilateral" )
  , cv_gui_helper.make_new_window_params( window::morphology, "1 --> 2: bilateral --> morphology" )
  , cv_gui_helper.make_new_window_params( window::hsv       , "2 --> 3: morphology --> hsv-filter")
  , cv_gui_helper.make_new_window_params( window::nail      , "3 --> 4: hsv-filter --> nail")
  , cv_gui_helper.make_new_window_params( window::output    , "4 --> 5: nail --> output")
  );
  
  enum class trackbar
  // for master
  { auto_play
  , frame_position
  , show_axes
  // for bilateral
  , diameter, sigma_color, sigma_space
  // for morphology
  , morphology_repeat
  // for hsv-filter
  , h_min, h_max, s_min, s_max, v_min, v_max
  // for nail(pre)
  , nail_morphology, nail_median_blur
  // for nail(hough-circle)
  , nail_circle_dp, nail_circle_min_dist
  , nail_circle_param_1, nail_circle_param_2
  , nail_circle_min_radius, nail_circle_max_radius
  };
  
  cv_gui_helper.new_trackbars
  // master
  ( cv_gui_helper.make_new_trackbar_params( trackbar::auto_play     , "auto play     "     , window::master, 1, 1)
  , cv_gui_helper.make_new_trackbar_params( trackbar::frame_position, "frame position", window::master, 0, frames.size())
  , cv_gui_helper.make_new_trackbar_params( trackbar::show_axes     , "show axis     "     , window::master, 1, 1)
  // test
  , cv_gui_helper.make_new_trackbar_params( trackbar::diameter   , "diameter    (x100)"   , window::bilateral, 16, 127, 100)
  , cv_gui_helper.make_new_trackbar_params( trackbar::sigma_color, "sigma color (x100)", window::bilateral, 72, 127, 100)
  , cv_gui_helper.make_new_trackbar_params( trackbar::sigma_space, "sigma space (x100)", window::bilateral, 16, 127, 100)
  // morphology
  , cv_gui_helper.make_new_trackbar_params( trackbar::morphology_repeat, "morphology repeat", window::morphology, 5, 15)
  // hsv-filter
  , cv_gui_helper.make_new_trackbar_params( trackbar::h_min, "H min   (x100)", window::hsv, 315.87  , 720, 100)
  , cv_gui_helper.make_new_trackbar_params( trackbar::h_max, "H max   (x100)", window::hsv, 356.36  , 720, 100)
  , cv_gui_helper.make_new_trackbar_params( trackbar::s_min, "S min (x10000)", window::hsv,   0.2992,   1, 10000)
  , cv_gui_helper.make_new_trackbar_params( trackbar::s_max, "S max (x10000)", window::hsv,   0.7049,   1, 10000)
  , cv_gui_helper.make_new_trackbar_params( trackbar::v_min, "V min         ", window::hsv, 120     , 255)
  , cv_gui_helper.make_new_trackbar_params( trackbar::v_max, "V max         ", window::hsv, 255     , 255)
  // nail (pre)
  , cv_gui_helper.make_new_trackbar_params( trackbar::nail_morphology , "morphology repeat ", window::nail, 5, 15)
  , cv_gui_helper.make_new_trackbar_params( trackbar::nail_median_blur, "median-blur kernel", window::nail, 13, 35)
  // nail (hough-circle)
  , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circle_dp        , "hough-circle dp"        , window::nail,   1,  16)
  , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circle_min_dist  , "hough-circle min dist"  , window::nail,   8,  64)
  , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circle_param_1   , "hough-circle param-1"   , window::nail, 100, 200)
  , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circle_param_2   , "hough-circle param-2"   , window::nail,   8, 200)
  , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circle_min_radius, "hough-circle min radius", window::nail,   4, 48)
  , cv_gui_helper.make_new_trackbar_params( trackbar::nail_circle_max_radius, "hough-circle max radius", window::nail,  12, 48)
  );

  do
  {
    if(cv_gui_helper.trackbar<bool>(trackbar::auto_play, window::master))
    {
      cv_gui_helper.trackbar
      ( trackbar::frame_position, window::master
      , (cv_gui_helper.trackbar(trackbar::frame_position, window::master) + 1) % frames.size()
      );
    }
    
    const auto frame_number = cv_gui_helper.trackbar(trackbar::frame_position, window::master);
    
    const auto& master_frame = frames[frame_number];
    
    std::cout << "current frame: " <<  std::setw(6) << std::right << frame_number << "\n";
    
    if(frame_number < 1)
    {
      std::cout << "skip\n";
      continue;
    }
    
    cv::Mat bilateral_frame;
    print_time
    ( [&]()
    {
      const auto d  = cv_gui_helper.trackbar<double>(trackbar::diameter, window::bilateral);
      const auto sc = cv_gui_helper.trackbar<double>(trackbar::sigma_color, window::bilateral);
      const auto ss = cv_gui_helper.trackbar<double>(trackbar::sigma_space, window::bilateral);
      bilateralFilter(master_frame, bilateral_frame, d, sc, ss);
    }
    , "morphology"
    );
    
    cv::Mat morphology_frame;
    print_time
    ( [&](){ cv::morphologyEx(master_frame, morphology_frame, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1, -1), cv_gui_helper.trackbar(trackbar::morphology_repeat, window::morphology)); }
    , "morphology"
    );
    
    cv::Mat hsv_filtered_map;
    print_time([&]()
    {
      hsv_filtered_map = filter_hsv_from_BGR24
      ( morphology_frame
      , cv_gui_helper.trackbar<float>(trackbar::h_min, window::hsv), cv_gui_helper.trackbar<float>(trackbar::h_max, window::hsv)
      , cv_gui_helper.trackbar<float>(trackbar::s_min, window::hsv), cv_gui_helper.trackbar<float>(trackbar::s_max, window::hsv)
      , cv_gui_helper.trackbar<float>(trackbar::v_min, window::hsv), cv_gui_helper.trackbar<float>(trackbar::v_max, window::hsv)
      );
    }
    , "generate filter hsv map with controller values"
    );
    
    cv::Mat hsv_frame;
    print_time
    ( [&](){ hsv_frame = cv_mat_cross_with_unorm<cv::Point3_<uint8_t>, uint8_t>(morphology_frame, hsv_filtered_map); }
    , "synth hsv filtered frame"
    );
    
    cv::Mat pre_nail_frame;
    print_time
    ( [&]()
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
      auto param_morphorogy = cv_gui_helper.trackbar(trackbar::nail_morphology, window::nail);
      cv::morphologyEx(h_channel_frame, h_channel_morphology_frame, cv::MORPH_OPEN, cv::Mat(), cv::Point(-1, -1), param_morphorogy);
      
      // median-blur: h-channel
      auto param_median_blur = cv_gui_helper.trackbar(trackbar::nail_median_blur, window::nail);
      if(param_median_blur % 2 == 0)
        cv_gui_helper.trackbar(trackbar::nail_median_blur, window::nail, ++param_median_blur);
      cv::medianBlur(h_channel_morphology_frame, pre_nail_frame, param_median_blur);
    }, "split h-channel and morphology and median"
    );
    
    std::vector<cv::Vec3f> circles;
    print_time
    ( [&]()
    {
      // circles detector
      auto dp         = cv_gui_helper.trackbar(trackbar::nail_circle_dp        , window::nail);
      auto min_dist   = cv_gui_helper.trackbar(trackbar::nail_circle_min_dist  , window::nail);
      auto param_1    = cv_gui_helper.trackbar(trackbar::nail_circle_param_1   , window::nail);
      auto param_2    = cv_gui_helper.trackbar(trackbar::nail_circle_param_2   , window::nail);
      auto min_radius = cv_gui_helper.trackbar(trackbar::nail_circle_min_radius, window::nail);
      auto max_radius = cv_gui_helper.trackbar(trackbar::nail_circle_max_radius, window::nail);
      cv::HoughCircles(pre_nail_frame, circles, CV_HOUGH_GRADIENT, dp, min_dist, param_1, param_2, min_radius, max_radius);
      
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
    , "detect nail with hough-circle detector"
    );
    
    cv::Mat nail_frame;
    cv::Mat output_frame;
    print_time
    ( [&]()
    {
      // nail_frame from gray to color
      cv::cvtColor(pre_nail_frame, nail_frame, CV_GRAY2BGR);
      // output frame
      output_frame = master_frame.clone();
      // render and print position
      std::cout
        << "=== detected finger position list ===\n"
        << "  X, Y, R\n"
        ;
      for(const auto v : circles)
      {
        std::cout << v[0] << ", " << v[1] << ", " << v[2] << "\n";
        const cv::Point center(int(std::round(v[0])), int(std::round(v[1])));
        const auto      radius = int(std::round(v[2]));
        for(auto frame: {nail_frame, output_frame})
        {
          cv::circle(frame, center,      2, cv::Scalar(0,0xff,0), -1, 8, 0);
          cv::circle(frame, center, radius, cv::Scalar(0,0,0xff),  3, 8, 0);
        }
      }
      std::cout << "=====================================\n";
    }
    , "render detected circles"
    );

    print_time
    ( [&]()
    {
      const auto show_axes = cv_gui_helper.trackbar<bool>(trackbar::show_axes, window::master);
      cv_gui_helper.show(window::master    , master_frame    , show_axes, true);
      cv_gui_helper.show(window::bilateral , bilateral_frame , show_axes);
      cv_gui_helper.show(window::morphology, morphology_frame, show_axes);
      cv_gui_helper.show(window::hsv       , hsv_frame       , show_axes);
      cv_gui_helper.show(window::nail      , nail_frame      , show_axes);
      cv_gui_helper.show(window::output    , output_frame    , show_axes);
    }
    , "show(with axes): master, output"
    );
  }
  while(cv_gui_helper.wait_key_not('\x1b', fps));
}