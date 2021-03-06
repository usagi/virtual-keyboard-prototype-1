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
      long double real_ratio;
      
      explicit trackbar_data_t(const std::string& trackbar_name, std::unique_ptr<int>&& pv, int visual_ratio_, long double real_ratio_)
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
      long double initial_value = 0;
      int max_value = 0;
      int visual_ratio;
      long double real_ratio;
      explicit new_trackbar_params_t(const T_trackbar_id trackbar_id_, const std::string& trackbar_name_, const T_window_id window_id_, const long double initial_value_, const int max_value_, const int visual_ratio_, const long double real_ratio_)
        : trackbar_id(trackbar_id_), trackbar_name(trackbar_name_), window_id(window_id_), initial_value(initial_value_), max_value(max_value_), visual_ratio(visual_ratio_), real_ratio(real_ratio_)
      {}
    };
    
    template<class T>
    new_window_params_t<T> make_new_window_params(const T window_id, const std::string& window_name, const int flags = CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO)
    { return new_window_params_t<T>(window_id, window_name, flags); }
    
    template<class T_trackbar_id, class T_window_id>
    new_trackbar_params_t<T_trackbar_id, T_window_id>
    make_new_trackbar_params(T_trackbar_id trackbar_id, const std::string& trackbar_name, T_window_id window_id, long double initial_value, int max_value, int visual_ratio = 1, long double real_ratio = 1.l)
    { return new_trackbar_params_t<T_trackbar_id, T_window_id>(trackbar_id, trackbar_name, window_id, initial_value, max_value, visual_ratio, real_ratio); }
    
    template<class T>
    const std::string& window_name(const T id)
    { return windows.at(int(id)).name; }
    
    template<class T_trackbar_id, class T_window_id>
    std::tuple<const std::string&, const std::string&> trackbar_name(const T_trackbar_id trackbar_id, const T_window_id window_id)
    {
      const auto& window = windows.at(int(window_id));
      const auto& window_name = window.name;
      const auto& trackbar_name = window.trackbars.at(int(trackbar_id)).name;
      return std::tuple<const std::string&, const std::string&>(trackbar_name, window_name);
    }
    
    template<class T>
    void show(const T id, const cv::Mat& m, const bool show_axes, const bool cloning = false)
    {
      if(show_axes)
        show_with_axes(id, m, cloning);
      else
        show(id, m);
    }
    
    template<class T>
    void show(const T id, const cv::Mat& m)
    { cv::imshow(window_name(id), m); }
    
    template<class T>
    void show_with_axes(const T id, const cv::Mat& m, bool cloning = false)
    {
      auto m_axes = cloning ? m.clone() : m;
      cv::line(m_axes, {0, int(m_axes.rows * 0.5)}, {m_axes.cols, int(m_axes.rows * 0.5)}, {0, 0, 255});
      cv::line(m_axes, {int(m_axes.cols * 0.5), 0}, {int(m_axes.cols * 0.5), m_axes.rows}, {0, 255, 0});
      cv::line(m_axes, {int(m_axes.cols * 0.5), int(m_axes.rows * 0.5)}, {int(m_axes.cols * 0.5), int(m_axes.rows * 0.5)}, {255, 0, 0});
      show(id, m_axes);
    }
    
    void new_windows() { }
    
    template<class T_head, class... T_tails>
    void new_windows(const T_head& head, T_tails... tails)
    {
      new_window(head);
      new_windows(tails...);
    }
    
    template<class T>
    void new_window(const new_window_params_t<T>& new_window_params)
    {
      if(boost::find_if(windows, [&](const windows_t::value_type& v){ return v.second.name == new_window_params.window_name; }) != std::end(windows))
        throw std::logic_error(std::string("window name is already used: ") + new_window_params.window_name);
      
      cv::namedWindow(new_window_params.window_name, new_window_params.flags);
      
      windows.emplace(int(new_window_params.window_id), window_data_t(new_window_params.window_name));
    }
    
    void new_trackbars() { }
    
    template<class T_head, class... T_tails>
    void new_trackbars(const T_head& head, T_tails... tails)
    {
      new_trackbar(head);
      new_trackbars(tails...);
    }
    
    template<class T_trackbar_id, class T_window_id>
    void new_trackbar(const new_trackbar_params_t<T_trackbar_id, T_window_id>& new_trackbar_params)
    {
      auto& target_window = windows.at(int(new_trackbar_params.window_id));
      
      auto& target_trackbars = target_window.trackbars;
      
      if(boost::find_if(target_trackbars, [&](const trackbars_t::value_type& v){ return v.second.name == new_trackbar_params.trackbar_name; }) != std::end(target_trackbars))
        throw std::logic_error(std::string("trackbar name is already used: ") + new_trackbar_params.trackbar_name);
      
      std::unique_ptr<int> pv(new int(new_trackbar_params.initial_value * new_trackbar_params.visual_ratio));
      cv::createTrackbar(new_trackbar_params.trackbar_name, window_name(new_trackbar_params.window_id), pv.get(), new_trackbar_params.max_value * new_trackbar_params.visual_ratio, nullptr, nullptr);
      target_trackbars.emplace(int(new_trackbar_params.trackbar_id), trackbar_data_t(new_trackbar_params.trackbar_name, std::move(pv), new_trackbar_params.visual_ratio, new_trackbar_params.real_ratio));
    }
    
    template<class T_trackbar_id, class T_window_id>
    int trackbar(const T_trackbar_id trackbar_id, const T_window_id window_id)
    { return *windows.at(int(window_id)).trackbars.at(int(trackbar_id)).pvalue; }
    
    template<class T, class T_trackbar_id, class T_window_id>
    T trackbar(const T_trackbar_id trackbar_id, const T_window_id window_id)
    {
      const auto& t = windows.at(int(window_id)).trackbars.at(int(trackbar_id));
      return T(*t.pvalue) / T(t.visual_ratio) * T(t.real_ratio);
    }
    
    template<class T_trackbar_id, class T_window_id>
    void trackbar(const T_trackbar_id trackbar_id, const T_window_id window_id, int value, bool trackbar_refresh = true)
    {
      *windows.at(int(window_id)).trackbars.at(int(trackbar_id)).pvalue = value;
      
      const auto names = trackbar_name(trackbar_id, window_id);
      
      if(trackbar_refresh)
        cv::setTrackbarPos(std::get<0>(names), std::get<1>(names), value);
    }
    
    bool wait_key_not(char key, int fps) const
    { return (cv::waitKey(fps) & 0xff) != key; }
    
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
