#pragma once

#include <vector>
#include <array>

#include <opencv2/core/core.hpp>

namespace arisin
{
  namespace etupirka
  {
    struct frame_packet_t final
    {
      using sequence_id_t = uint8_t;
      
      uint16_t real_data_size;
      uint8_t capture_id;
      sequence_id_t sequence_id;
      
      static constexpr size_t info_size = sizeof(real_data_size) - sizeof(capture_id) - sizeof(sequence_id);
      static constexpr size_t data_size = 65506 - info_size;
      static constexpr size_t this_size = info_size + data_size;
      
      uint8_t data[data_size];
      
      inline void set_data(const cv::Mat& m, const int jpeg_quality = 60)
      {
        std::vector<uint8_t> buffer;
        
        cv::imencode(".jpg", m, buffer, { CV_IMWRITE_JPEG_QUALITY, jpeg_quality });
        
        if(buffer.size() > data_size)
          throw std::runtime_error("encoded frame size is over.");
        
        std::copy(std::begin(buffer), std::end(buffer), &data[0]);
        real_data_size = buffer.size();
      }
      
      inline const uint8_t* data_begin() const { return &data[0]; }
      inline const uint8_t* data_end() const { return data_begin() + size_t(data_size); }
      
      using mutate_array_t = std::array<uint8_t, this_size>;
      
      inline const mutate_array_t& mutate_to_const_array() const
      { return *reinterpret_cast<const mutate_array_t*>(this); }
      
      inline mutate_array_t& mutate_to_array() const
      { return *reinterpret_cast<mutate_array_t*>(const_cast<frame_packet_t*>(this)); }
      
      using vector_t = std::vector<uint8_t>;
      
      inline vector_t to_vector() const
      { return vector_t(data_begin(), data_end());}
    };
  }
}
