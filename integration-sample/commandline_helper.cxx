#include "commandline_helper.hxx"

namespace
{
  constexpr size_t h(const char* const  str, int n = 0)
  { return !str[n] ? 5381 : (h(str, n+1)*33) ^ str[n]; }
}

namespace arisin
{
  namespace etupirka
  {
    configuration_t commandline_helper_t::interpret(const std::vector<std::string>& arguments)
    {
#ifndef NDEBUG
      for(size_t n = 0; n < arguments.size(); ++n)
        L(INFO, "arguments[" << n << "]: " << arguments[n]);
#endif
      
      configuration_t conf;
      
      // ToDo: auto conf = commandline_helper_t::load_default();
      
      // ToDo: commandline_helper_t::load_file(conf);
      
      for(auto i = std::begin(arguments), e = std::end(arguments); i < e; ++i)
      {
        
        switch(h(i->data()))
        {
          case h("-m"):
          case h("--mode"):
            try
            {
              switch(h((++i)->data()))
              {
                case h("main"    ): conf.mode = mode_t::main;     break;
                case h("reciever"): conf.mode = mode_t::reciever; break;
                default: conf.mode = mode_t::none;
              }
            }
            catch(const std::exception& e) { L(ERROR, "catch exception: " << e.what()); }
            continue;
            
          case h("-t"):
          case h("--camera-capture/top-camera-id"):
            try { conf.camera_capture.top_camera_id = std::stoi(*++i); }
            catch(const std::exception& e) { L(ERROR, "catch exception: " << e.what()); }
            continue;
            
          case h("-f"):
          case h("--camera-capture/front-camera-id"):
            try { conf.camera_capture.front_camera_id = std::stoi(*++i); }
            catch(const std::exception& e) { L(ERROR, "catch exception: " << e.what()); }
            continue;
            
          case h("-W"):
          case h("--camera-capture/width"):
            try { conf.camera_capture.width = std::stoi(*++i); }
            catch(const std::exception& e) { L(ERROR, "catch exception: " << e.what()); }
            continue;
            
          case h("-H"):
          case h("--camera-capture/height"):
            try { conf.camera_capture.height = std::stoi(*++i); }
            catch(const std::exception& e) { L(ERROR, "catch exception: " << e.what()); }
            continue;
            
          case h("-h"):
          case h("--help"):
            show_help();
            exit(0);
          case h("-v"):
          case h("--version"):
            std::cout << etupirka_t::version_string();
            exit(0);
        }
      }
      
      return conf;
    }
    
    void commandline_helper_t::show_help()
    {
      L(INFO, "NOT IMPLEMENTED");
    }
  }
}
