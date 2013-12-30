#include <iostream>
#include <chrono>
#include <thread>
#include <boost/program_options.hpp>

#if defined(_WIN64) || defined(_WIN32)
#elif __APPLE__
#elif defined(__linux) || defined(__unix) || defined(__posix)
  #include <fcntl.h>
  //#include <linux/input.h>
  #include <linux/uinput.h>
#endif

enum class os_target: unsigned
{ Unknown = 0x00
, Windows
, OSX
, Linux_UNIX_POSIX
};

constexpr const char* to_string(const os_target a)
{
  return
    a == os_target::Linux_UNIX_POSIX
    ? "Linux_UNIX_POSIX"
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
#elif __APPLE__
  os_target::OSX
#elif defined(__linux) || defined(__unix) || defined(__posix)
  os_target::Linux_UNIX_POSIX
#else
  os_target::Unknown
#endif
  ;

constexpr auto default_keycode     = KEY_A;
constexpr auto default_wait_in_sec = 3;
constexpr auto default_device_name       = "virtual-keyboard-prototype-1";
constexpr auto default_device_vendor_id  = 0x0000;
constexpr auto default_device_product_id = 0x0000;
constexpr auto default_device_version    = 0;

constexpr auto version_info = "vertual-keyboard-prototype-1/keyboard-writer\n"
                              "version 0.0.0"
                              ;

struct writer_base_t
{ virtual void operator()(const int keycode) = 0; };


#if defined(_WIN64) || defined(_WIN32)
#elif __APPLE__
#elif defined(__linux) || defined(__unix) || defined(__posix)
struct writer_t final
  : writer_base_t
{
  writer_t()
  { initialize(); }
  
  ~writer_t()
  { finalize(); }
  
  void operator()(const int keycode) override
  {
    send_event(EV_KEY, keycode, 1);
    send_event(EV_KEY, keycode, 0);
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
};
#endif

void write_invoke(const int keycode, const int wait)
{
  writer_t w;
  
  for (auto wait_ = wait; wait_ > 0; --wait_)
  {
    std::cout
      << "wait... " << wait_ << " [sec]" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
  
  std::cout
    << "invoke key press: keycode(" << keycode << ") " << std::endl;
  w(keycode);
}

boost::program_options::variables_map option(const int& ac, const char* const * const  av)
{
  using namespace boost::program_options;
  
  options_description description("オプション");
  description.add_options()
    ("help,h"   , "ヘルプ")
    ("keycode,k", value<int>()->default_value(default_keycode)    , "キーコード")
    ("wait,w"   , value<int>()->default_value(default_wait_in_sec), "キーイベント発生までのウェイト[秒]")
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

int main (const int ac, const char* const * const av) try
{
  auto vm = option(ac, av);
  if(!vm.count("help") && !vm.count("version"))
    write_invoke(vm["keycode"].as<int>(), vm["wait"].as<int>());
}
catch (const std::exception& e)
{ std::cerr << e.what() << "\n"; }
