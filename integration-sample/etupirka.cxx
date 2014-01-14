#include "etupirka.hxx"

namespace arisin
{
  namespace etupirka
  {
    enum class mode_t
    { none     // 動作しないモード
    , main     // カメラ制御〜UDP送信モード
    , reciever // UDP受信〜キーストローク発行モード
    };
    
    etupirka_t::etupirka_t(const mode_t)
    { L(FATAL, "NOT IMPLEMENTED"); }
    
    void etupirka_t::run()
    { L(FATAL, "NOT IMPLEMENTED"); }
  }
}