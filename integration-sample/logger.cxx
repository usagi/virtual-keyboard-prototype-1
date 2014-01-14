#include "logger.hxx"

namespace arisin
{
  namespace etupirka
  {
    namespace logger
    {
      void initialize(const bool output_to_stderr)
      {
#ifdef ARISIN_ETUPIRIKA_LOGGER
        static bool is_initialized = false;
        if(!is_initialized)
        {
          google::InitGoogleLogging("etupirka");
          if(output_to_stderr)
            google::LogToStderr();
          is_initialized = true;
        }
#endif
        L(INFO, "etupirika logger initialized");
      }
    }
  }
}