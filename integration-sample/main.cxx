#include "etupirka.hxx"

int main(const int number_of_arguments, const char* const* const arguments)
{
  using namespace arisin::etupirka;
  logger::initialize();
  const auto conf = commandline_helper_t::interpret({arguments, arguments + number_of_arguments});
  DLOG(INFO) << "to construct etupirka";
  etupirka_t etupirka(conf);
  DLOG(INFO) << "to run";
  etupirka.run();
  DLOG(INFO) << "to exit";
}