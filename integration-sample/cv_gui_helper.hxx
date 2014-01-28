#pragma once

#include <memory>
#include <string>
#include <vector>
#include <stdexcept>
#include <unordered_map>

#include <boost/range/algorithm.hpp>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

namespace
{
  struct cv_gui_helper_t final
  {
    struct trackbar_data_t final
    {
      std::string name;
      std::unique_ptr<int> pvalue;
      int visual_ratio;
      float real_ratio;
      
      explicit trackbar_data_t(const std::string& trackbar_name, std::unique_ptr<int>&& pv, int visual_ratio_, float real_ratio_)
        : name(trackbar_name)
        , pvalue(std::move(pv))
        , visual_ratio(visual_ratio_)
        , real_ratio(real_ratio_)
      { }
    };
    
    using trackbars_t = std::unordered_map<int, trackbar_data_t>;
    
    struct window_data_t final
    {
      std::string name;
      trackbars_t trackbars;
      
      explicit window_data_t(const std::string& window_name)
        : name(window_name)
      { }
    };
    
    using windows_t = std::unordered_map<int, window_data_t>;
    
    template<class T>
    struct new_window_params_t final
    {
      T window_id;
      std::string window_name;
      int flags = 0;
      explicit new_window_params_t(const T window_id_, const std::string& window_name_, const int flags_ = CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO)
        : window_id(window_id_), window_name(window_name_), flags(flags_)
      {}
    };
    
    template<class T_trackbar_id, class T_window_id>
    struct new_trackbar_params_t final
    {
      T_trackbar_id trackbar_id;
      std::string trackbar_name;
      T_window_id window_id;
      float initial_value = 0;
      int max_value = 0;
      int visual_ratio;
      float real_ratio;
      explicit new_trackbar_params_t(const T_trackbar_id trackbar_id_, const std::string& trackbar_name_, const T_window_id window_id_, const float initial_value_, const int max_value_, const int visual_ratio_, const float real_ratio_)
        : trackbar_id(trackbar_id_), trackbar_name(trackbar_name_), window_id(window_id_), initial_value(initial_value_), max_value(max_value_), visual_ratio(visual_ratio_), real_ratio(real_ratio_)
      {}
    };
    
    template<class T>
    inline new_window_params_t<T> make_new_window_params(const T window_id, const std::string& window_name, const int flags = CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO)
    { return new_window_params_t<T>(window_id, window_name, flags); }
    
    template<class T_trackbar_id, class T_window_id>
    inline new_trackbar_params_t<T_trackbar_id, T_window_id>
    make_new_trackbar_params(T_trackbar_id trackbar_id, const std::string& trackbar_name, T_window_id window_id, float initial_value, int max_value, int visual_ratio = 1, float real_ratio = 1.l)
    { return new_trackbar_params_t<T_trackbar_id, T_window_id>(trackbar_id, trackbar_name, window_id, initial_value, max_value, visual_ratio, real_ratio); }
    
    template<class T>
    inline const std::string& window_name(const T id)
    { return windows.at(int(id)).name; }
    
    template<class T_trackbar_id, class T_window_id>
    inline std::tuple<const std::string&, const std::string&> trackbar_name(const T_trackbar_id trackbar_id, const T_window_id window_id)
    {
      const auto& window = windows.at(int(window_id));
      const auto& window_name = window.name;
      const auto& trackbar_name = window.trackbars.at(int(trackbar_id)).name;
      return std::tuple<const std::string&, const std::string&>(trackbar_name, window_name);
    }
    
    template<class T>
    inline void show(const T id, const cv::Mat& m)
    { cv::imshow(window_name(id), m); }
    
    inline void add_axes(cv::Mat& m)
    {
      cv::line(m, {0, int(m.rows * 0.5)}, {m.cols, int(m.rows * 0.5)}, {0, 0, 255});
      cv::line(m, {int(m.cols * 0.5), 0}, {int(m.cols * 0.5), m.rows}, {0, 255, 0});
      cv::line(m, {int(m.cols * 0.5), int(m.rows * 0.5)}, {int(m.cols * 0.5), int(m.rows * 0.5)}, {255, 0, 0});
    }
    
    template<class T>
    inline void resize(const T id, const int width, const int height)
    {
#if CV_MAJOR_VERSION < 2 || ( CV_MAJOR_VERSION == 2 && CV_MINOR_VERSION < 4)
      cvResizeWindow(window_name(id).data(), width, height);
#else
      cv::resizeWindow(window_name(id), width, height);
#endif
    }
    
    template<class T>
    inline void move(const T id, const int width, const int height)
    {
#if CV_MAJOR_VERSION < 2 || ( CV_MAJOR_VERSION == 2 && CV_MINOR_VERSION < 4)
      cvMoveWindow(window_name(id).data(), width, height);
#else
      cv::moveWindow(window_name(id), width, height);
#endif
    }
/*    
    template<class T>
    inline void present(const T id) const
    {
#if CV_MAJOR_VERSION < 2 || ( CV_MAJOR_VERSION == 2 && CV_MINOR_VERSION < 4)
      cvUpdateWindow(window_name(id).data());
#else
      cv::updateWindow(window_name(id));
#endif
    }
    
    inline void present() const
    {
      for(const auto& window: windows)
#if CV_MAJOR_VERSION < 2 || ( CV_MAJOR_VERSION == 2 && CV_MINOR_VERSION < 4)
        cvUpdateWindow(window.second.name.data());
#else
        cv::updateWindow(window.second.name);
#endif
    }
*/
    inline void new_windows() { }
    
    template<class T_head, class... T_tails>
    inline void new_windows(const T_head& head, T_tails... tails)
    {
      new_window(head);
      new_windows(tails...);
    }
    
    template<class T>
    inline void new_window(const new_window_params_t<T>& new_window_params)
    {
      if(boost::find_if(windows, [&](const windows_t::value_type& v){ return v.second.name == new_window_params.window_name; }) != std::end(windows))
        throw std::logic_error(std::string("window name is already used: ") + new_window_params.window_name);
      
      cv::namedWindow(new_window_params.window_name, new_window_params.flags);
      
      windows.emplace(int(new_window_params.window_id), window_data_t(new_window_params.window_name));
    }
    
    inline void new_trackbars() { }
    
    template<class T_head, class... T_tails>
    inline void new_trackbars(const T_head& head, T_tails... tails)
    {
      //DLOG(INFO) << "new_trackbars start";
      //DLOG(INFO) << "call new_trackbar(head)";
      new_trackbar(head);
      //DLOG(INFO) << "call new_trackbars(tails...)";
      new_trackbars(tails...);
      //DLOG(INFO) << "new_trackbars end";
    }
    
    template<class T_trackbar_id, class T_window_id>
    inline void new_trackbar(const new_trackbar_params_t<T_trackbar_id, T_window_id>& new_trackbar_params)
    {
      //DLOG(INFO) << "new_trackbar";
      
      auto& target_window = windows.at(int(new_trackbar_params.window_id));
      //DLOG(INFO) << "target_window address: " << &target_window;
      //DLOG(INFO) << "target_window name: " << target_window.name;
      
      auto& target_trackbars = target_window.trackbars;
      //DLOG(INFO) << "target_trackbars size: " << target_trackbars.size();
      
      if(boost::find_if(target_trackbars, [&](const trackbars_t::value_type& v){ return v.second.name == new_trackbar_params.trackbar_name; }) != std::end(target_trackbars))
        throw std::logic_error(std::string("trackbar name is already used: ") + new_trackbar_params.trackbar_name);
      //DLOG(INFO) << "can use the trackbar name";
      
      std::unique_ptr<int> pv(new int(new_trackbar_params.initial_value * new_trackbar_params.visual_ratio));
      //DLOG(INFO) << "generate pv: " << *pv;
      cv::createTrackbar(new_trackbar_params.trackbar_name, window_name(new_trackbar_params.window_id), pv.get(), new_trackbar_params.max_value * new_trackbar_params.visual_ratio, nullptr, nullptr);
      //DLOG(INFO) << "trackbar created";
      target_trackbars.emplace(int(new_trackbar_params.trackbar_id), trackbar_data_t(new_trackbar_params.trackbar_name, std::move(pv), new_trackbar_params.visual_ratio, new_trackbar_params.real_ratio));
      //DLOG(INFO) << "emplaced";
    }
    
    template<class T_trackbar_id, class T_window_id>
    inline int trackbar(const T_trackbar_id trackbar_id, const T_window_id window_id)
    { return *windows.at(int(window_id)).trackbars.at(int(trackbar_id)).pvalue; }
    
    template<class T, class T_trackbar_id, class T_window_id>
    inline T trackbar(const T_trackbar_id trackbar_id, const T_window_id window_id)
    {
      const auto& t = windows.at(int(window_id)).trackbars.at(int(trackbar_id));
      return T(*t.pvalue) / T(t.visual_ratio) * T(t.real_ratio);
    }
    
    template<class T_trackbar_id, class T_window_id>
    inline void trackbar(const T_trackbar_id trackbar_id, const T_window_id window_id, const int value, const bool trackbar_refresh = true)
    {
      *windows.at(int(window_id)).trackbars.at(int(trackbar_id)).pvalue = value;
      
      const auto names = trackbar_name(trackbar_id, window_id);
      
      if(trackbar_refresh)
        cv::setTrackbarPos(std::get<0>(names), std::get<1>(names), value);
    }
    
    template<class T ,class T_trackbar_id, class T_window_id>
    inline void trackbar(const T_trackbar_id trackbar_id, const T_window_id window_id, const T value, const bool trackbar_refresh = true)
    {
      const auto& t = windows.at(int(window_id)).trackbars.at(int(trackbar_id));
      *t.pvalue = value * t.visual_ratio / t.real_ratio;
      
      const auto names = trackbar_name(trackbar_id, window_id);
      
      if(trackbar_refresh)
        cv::setTrackbarPos(std::get<0>(names), std::get<1>(names), *t.pvalue);
    }
    
    inline bool wait_key_not(char key, int wait_in_ms) const
    { return (cv::waitKey(wait_in_ms) & 0xff) != key; }
    
    static cv_gui_helper_t& instance()
    {
      static cv_gui_helper_t i;
      return i;
    }
    
  private:
    windows_t windows;
    
    cv_gui_helper_t(){}
    cv_gui_helper_t(const cv_gui_helper_t&) = delete;
    cv_gui_helper_t(cv_gui_helper_t&&) = delete;
    void operator=(const cv_gui_helper_t&) = delete;
    void operator=(cv_gui_helper_t&&) = delete;
  };
}
