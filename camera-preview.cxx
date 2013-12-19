#include <array>
#include <vector>
#include <boost/program_options.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

constexpr auto default_width  = 320;
constexpr auto default_height = 240;
constexpr auto default_fps    = 30;
constexpr auto default_write  = true;
constexpr auto default_show   = true;
constexpr auto version_info   = "vertual-keyboard-prototype-1/camera-preview\n";

void run
( const std::vector<int>& ids
, const int width
, const int height
, const int fps
, const bool write
, const bool show
)
{
  const auto ms_per_frame = 1000 / fps;

  std::vector<cv::VideoCapture> caps;
  caps.reserve(ids.size());
  
  std::vector<std::string> infos;
  infos.reserve(ids.size());
  
  std::vector<cv::VideoWriter> writers;
  if(write)
    writers.reserve(ids.size());

  for(size_t n = 0; n < ids.size(); ++n)
  {
    const auto id = ids.at(n);
    
    caps.emplace_back(id);
    auto& cap = caps.at(n);
    
    cap.set(CV_CAP_PROP_FRAME_WIDTH , width );
    cap.set(CV_CAP_PROP_FRAME_HEIGHT, height);
    
    const auto info
      = std::string("camera #") + std::to_string(n)
      + "device #" + std::to_string(id)
      + " W: " + std::to_string(width)
      + " H: " + std::to_string(height)
      + " / " + std::to_string(fps) + " [fps]"
      ;

    if(!cap.isOpened())
      throw std::runtime_error(info);

    std::cout << info << std::endl;

    cv::namedWindow(info, CV_WINDOW_AUTOSIZE|CV_WINDOW_FREERATIO);

    if(write)
      writers.emplace_back
        ( std::string("c")+std::to_string(n)+"d"+std::to_string(id)+".avi"
        , CV_FOURCC('X','V','I','D')
        , fps
        , cv::Size(width, height)
        );

    infos.emplace_back(std::move(info));
  }

  do
  {
    for(size_t n = 0; n < ids.size(); ++n)
    {
      auto& cap = caps.at(n);
      const auto& info = infos.at(n);
      
      cv::Mat frame;
      cap >> frame;
      
      if(write)
        writers.at(n) << frame;

      if(show)
        cv::imshow(info, frame);
    
    }
  }
  while(cv::waitKey(ms_per_frame) < 0);
}

boost::program_options::variables_map option(const int& ac, const char* const * const  av)
{
  using namespace boost::program_options;
  std::vector<std::string> a;
  options_description description("オプション");
  description.add_options()
    ("ids,i"    , value<std::vector<int>>()->multitoken()  , "camera id array (eg.: -i 0 1 2)")
    ("write"    , value<bool>()->default_value(default_write), "write to file (eg.: --write false)")
    ("show"     , value<bool>()->default_value(default_show) , "show (eg.: --show false)")
    ("width,W"  , value<int>()->default_value(default_width) , "width [px]")
    ("height,H" , value<int>()->default_value(default_height), "height [px]")
    ("fps,F"    , value<int>()->default_value(default_fps)   , "fps [frames/sec]")
    ("help,h"  , "ヘルプ")
    ("version,v", "バージョン情報")
    ;
  
  variables_map vm;
  store(parse_command_line(ac, av, description), vm);
  notify(vm);
  
  if(vm.count("help"))
    std::cout << description << std::endl;
  if(vm.count("version"))
    std::cout << version_info << std::endl;
  
  return vm;
}

int main(const int ac, const char* const * const av)
try
{
  auto vm = option(ac, av);
  if(!vm.count("help") && !vm.count("version"))
    run
      ( vm["ids"].as<std::vector<int>>()
      , vm["width"].as<int>()
      , vm["height"].as<int>()
      , vm["fps"].as<int>()
      , vm["write"].as<bool>()
      , vm["show"].as<bool>()
      );
}
catch(const std::exception& e)
{ std::cerr << e.what() << "\n"; }
