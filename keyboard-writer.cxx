#include <iostream>
#include <chrono>
#include <thread>
#include <boost/program_options.hpp>

#include <WonderRabbitProject/key.hxx>

#if defined(_WIN64) || defined(_WIN32)
  //#include ????
#elif defined(__APPLE__)
  #include "/System/Library/Frameworks/CoreGraphics.framework/Versions/A/Headers/CGEvent.h"
  #include "/System/Library/Frameworks/CoreGraphics.framework/Versions/A/Headers/CGEventSource.h"
  #include "/System/Library/Frameworks/CoreGraphics.framework/Versions/A/Headers/CGEventTypes.h"
  #include "/System/Library/Frameworks/CoreGraphics.framework/Versions/A/Headers/CGRemoteOperation.h"
#elif defined(__linux)
  #include <fcntl.h>
#else
  static_assert(false, "the system is not support");
#endif

namespace app
{

enum class os_target: unsigned
{ Unknown = 0x00
, Windows
, OSX
, Linux
};

constexpr const char* to_string(const os_target a)
{
  return
    a == os_target::Linux
    ? "Linux"
    : a == os_target::OSX
      ? "OSX"
      : a == os_target::Windows
        ? "Windows"
        : "Unknown"
    ;
}

constexpr auto os_target_mode = 
#if defined(_WIN64) || defined(_WIN32)
  os_target::Windows
#elif defined(__APPLE__)
  os_target::OSX
#elif defined(__linux)
  os_target::Linux
#else
  os_target::Unknown
#endif
  ;

constexpr auto default_key_name          = "a";
constexpr auto default_wait_in_sec       = 3;
constexpr auto default_device_name       = "virtual-keyboard-prototype-1";
constexpr auto default_device_vendor_id  = 0x0000;
constexpr auto default_device_product_id = 0x0000;
constexpr auto default_device_version    = 0;

constexpr auto version_info = "vertual-keyboard-prototype-1/keyboard-writer\n"
                              "version 0.0.0"
                              ;

struct writer_base_t
{ virtual void operator()(const int code) = 0; };

struct writer_t final
  : writer_base_t
{
#if defined(_WIN64) || defined(_WIN32)
#elif __APPLE__
  void operator()(const int code) override
  {
    if(!WonderRabbitProject::key::key_helper_t::instance().is_valid(code))
      throw std::runtime_error("invalid key code");
    
    constexpr auto key_press_code   = true;
    constexpr auto key_release_code = false;
    
    const auto keycode = CGKeyCode(code);
    
    auto event_source = CGEventSourceCreate(kCGEventSourceStateHIDSystemState);
    
    auto key_press_event   = CGEventCreateKeyboardEvent(event_source, keycode, key_press_code);
    auto key_release_event = CGEventCreateKeyboardEvent(event_source, keycode, key_release_code);
    
    //CGEventSetFlags(key_press_event, kCGEventFlagMaskShift);
    //CGEventSetFlags(key_release_event, kCGEventFlagMaskShift);
    
    constexpr auto event_tap_location = kCGHIDEventTap;
    
    CGEventPost(event_tap_location, key_press_event);
    CGEventPost(event_tap_location, key_release_event);
    
    CFRelease(key_press_event);
    CFRelease(key_release_event);
  }
#elif defined(__linux) || defined(__unix) || defined(__posix)
  writer_t()
  { initialize(); }
  
  ~writer_t()
  { finalize(); }
  
  void operator()(const int code) override
  {
    if(!WonderRabbitProject::key::key_helper_t::instance().is_valid(code))
      throw std::runtime_error("invalid key code");
    send_event(EV_KEY, code, 1);
    send_event(EV_KEY, code, 0);
    send_event(EV_SYN, SYN_REPORT, 0);
  }
  
protected:
  void initialize()
  {
    initialize_open_device();
    initialize_create_device();
  }
  
  void initialize_open_device()
  {
    fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fd < 0)
    { std::cerr << "/dev/uinput open error"; exit(0); }
  }
  
  void initialize_create_device()
  {
    if(ioctl(fd, UI_SET_EVBIT , EV_KEY) < 0)
    { std::cerr << "create device ioctl UI_SET_EVIT error"; exit(0); }
    
    auto set_keybit = [&](const int code)
    {
      if(ioctl(fd, UI_SET_KEYBIT, code) < 0)
      { std::cerr << "create device ioctl UI_KEYBIT error"; exit(0); }
    };

    for(auto code = 0; code < 256; ++code)
      set_keybit(code);
    
    struct uinput_user_dev dev;
    memset(&dev, 0, sizeof(dev));
    
    snprintf(dev.name, UINPUT_MAX_NAME_SIZE, default_device_name);
    dev.id.bustype = BUS_USB;
    dev.id.vendor  = default_device_vendor_id;
    dev.id.product = default_device_product_id;
    dev.id.version = default_device_version;
    
    if(write(fd, &dev, sizeof(dev)) < 0)
    { std::cerr << "write error"; exit(0); }
    if(ioctl(fd, UI_DEV_CREATE) < 0)
    { std::cerr << "ioctl error"; exit(0); }
  }
  
  void finalize()
  {
    if(fd >= 0)
    {
      finalize_uinput_device();
      close(fd);
    }
  }
  
  void finalize_uinput_device()
  {
    if (ioctl(fd, UI_DEV_DESTROY) < 0)
    { std::cerr << "finalize error"; exit(0); }
  }
  
  void send_event(const int type, const int code, const int value) const
  {
    struct input_event e;
    memset(&e, 0, sizeof(e));
    
    gettimeofday(&e.time, nullptr);
    
    e.type = type;
    e.code = code;
    e.value = value;
    
    if(write(fd, &e, sizeof(e)) < 0)
    { std::cerr << "send event write error"; exit(0); }
  }
  
  int fd;
#endif
};

void write_invoke(const int code, const int wait)
{
  writer_t w;
  
  for (auto wait_ = wait; wait_ > 0; --wait_)
  {
    std::cout
      << "wait... " << wait_ << " [sec]" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  
  auto kh = WonderRabbitProject::key::key_helper_t::instance();
  
  std::cout
    << "invoke key press: key = "
    << kh.name(code)
    << "(" << code << (kh.is_valid(code) ? "" : " [INVALID]") << ") " << std::endl;
  w(code);
}

boost::program_options::variables_map option(const int& ac, const char* const * const  av)
{
  using namespace boost::program_options;
  
  options_description description("オプション");
  description.add_options()
    ("help,h"   , "ヘルプ")
    ("key,k"    , value<std::string>()->default_value(default_key_name), "キーの名前")
    ("wait,w"   , value<int>()->default_value(default_wait_in_sec)     , "キーイベント発生までのウェイト[秒]")
    ("version,v", "バージョン情報")
    ;
  
  variables_map vm;
  store(parse_command_line(ac, av, description), vm);
  notify(vm);
  
  if(vm.count("help"))
    std::cout << description << std::endl;
  if(vm.count("version"))
    std::cout
      << version_info
      << " - " << to_string(os_target_mode)
      << std::endl;
  
  return vm;
}

}

int main (const int ac, const char* const * const av) try
{
  auto vm = app::option(ac, av);
  if(!vm.count("help") && !vm.count("version"))
    app::write_invoke(WonderRabbitProject::key::key_helper_t::instance().code(vm["key"].as<std::string>()), vm["wait"].as<int>());
}
catch (const std::exception& e)
{ std::cerr << e.what() << "\n"; }