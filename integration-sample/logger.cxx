#include "logger.hxx"

namespace arisin
{
  namespace etupirka
  {
    namespace logger
    {
      void initialize(const bool output_to_stderr)
      {
        static bool is_initialized = false;
        if(!is_initialized)
        {
          google::InitGoogleLogging("etupirka");
          if(output_to_stderr)
            google::LogToStderr();
//#ifdef NDEBUG
//          FLAGS_minloglevel = 1;
//#endif
          is_initialized = true;
        }
        DLOG(INFO) << "etupirka logger initialized";
      }
    }
  }
}