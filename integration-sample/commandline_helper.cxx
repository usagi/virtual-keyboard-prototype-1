#include "commandline_helper.hxx"

namespace arisin
{
  namespace etupirka
  {
    configuration_t commandline_helper::interpret(const std::vector<std::string>& arguments)
    {
#ifndef NDEBUG
      for(size_t n = 0; n < arguments.size(); ++n)
        L(INFO, "arguments[" << n << "]: " << arguments[n]);
#endif
      
      configuration_t conf;
      
      // ToDo: @arisin が実装: arguments から conf を適切に設定する
      L(FATAL, "NOT IMPLEMENTED");
      
      return conf;
    }
  }
}
