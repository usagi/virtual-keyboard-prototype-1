#include "key-invoker.hxx"

namespace arisin
{
  namespace etupirka
  {
    key_invoker_t::key_invoker_t(const configuration_t& conf)
    {
    }
    
    key_invoker_t::~key_invoker_t()
    {
      for(auto key : pressing_keys)
        operator()(key, WonderRabbitProject::key::writer_t::state_t::up);
    }
    
    void key_invoker_t::operator()(int key_usb_hid_usage_id, WonderRabbitProject::key::writer_t::state_t key_state)
    {
      DLOG(INFO) << "invoke: key_usb_hid_usage_id(" << key_usb_hid_usage_id << ") key_state(" << int(key_state) << ")";
      switch(key_state)
      {
        // down イベントの場合は押下中のキーを pressing_keys に保持する
        case WonderRabbitProject::key::writer_t::state_t::down:
          pressing_keys.emplace(key_usb_hid_usage_id);
          break;
        // down_and_up, up, その他（念の為）の場合
        case WonderRabbitProject::key::writer_t::state_t::down_and_up:
        case WonderRabbitProject::key::writer_t::state_t::up:
        default:
          pressing_keys.erase(key_usb_hid_usage_id);
      }
      
      // ToDo: libWRP-keyの <USB-HID Usage ID> --> <key_name> 変換機能を用いてkey_usb_hid_usage_idを文字列のキー名に変換
      const auto& key_helper = WonderRabbitProject::key::key_helper_t::instance();
      const auto key_name =  key_helper.name_from_usb_hid_usage_id(key_usb_hid_usage_id);
      
      // ToDo: libWRP-keyのwriter_tを用いて文字列のキー名とkey_stateからキーイベントを発行
      const auto& key_writer = WonderRabbitProject::key::writer_t::instance();
      key_writer(key_name, key_state);
    }
  }
}