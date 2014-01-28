#pragma once

#include <opencv2/core/internal.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace
{

#if CV_MAJOR_VERSION < 2 || ( CV_MAJOR_VERSION == 2 && CV_MINOR_VERSION < 4)
  
  class parallel_loop_body
  {
  public:
    virtual ~parallel_loop_body(){}
    virtual void operator() (const cv::Range& range) const = 0;
  };
  
  inline void parallel_for(const cv::Range& range, const parallel_loop_body& body, double nstripes)
  { body(range); }
  
#else
  
  using parallel_loop_body = cv::ParallelLoopBody;
  inline void parallel_for(const cv::Range& range, const parallel_loop_body& body, double nstripes)
  { cv::parallel_for_(range, body, nstripes); }
  
#endif

  class BilateralFilter_8u_Invoker
    : public ::parallel_loop_body
  {
  public:
    BilateralFilter_8u_Invoker
    ( cv::Mat& _dest, const cv::Mat& _temp
    , int _radius, int _maxk
    , int* _space_ofs, float *_space_weight, float *_color_weight
    )
      : temp(&_temp), dest(&_dest), radius(_radius)
      , maxk(_maxk), space_ofs(_space_ofs), space_weight(_space_weight), color_weight(_color_weight)
    {
    }

    virtual void operator() (const cv::Range& range) const
    {
      int i, j, cn = dest->channels(), k;
      cv::Size size = dest->size();
      #if CV_SSE3
      int CV_DECL_ALIGNED(16) buf[4];
      float CV_DECL_ALIGNED(16) bufSum[4];
      static const int CV_DECL_ALIGNED(16) bufSignMask[] = { int(0x80000000), int(0x80000000), int(0x80000000), int(0x80000000) };
      bool haveSSE3 = cv::checkHardwareSupport(CV_CPU_SSE3);
      #endif

      for( i = range.start; i < range.end; i++ )
      {
        const uchar* sptr = temp->ptr(i+radius) + radius*cn;
        uchar* dptr = dest->ptr(i);

        if( cn == 1 )
        {
          for( j = 0; j < size.width; j++ )
          {
            float sum = 0, wsum = 0;
            int val0 = sptr[j];
            k = 0;
            #if CV_SSE3
            if( haveSSE3 )
            {
              __m128 _val0 = _mm_set1_ps(static_cast<float>(val0));
              const __m128 _signMask = _mm_load_ps((const float*)bufSignMask);

              for( ; k <= maxk - 4; k += 4 )
              {
                __m128 _valF = _mm_set_ps(sptr[j + space_ofs[k+3]], sptr[j + space_ofs[k+2]],
                                          sptr[j + space_ofs[k+1]], sptr[j + space_ofs[k]]);

                __m128 _val = _mm_andnot_ps(_signMask, _mm_sub_ps(_valF, _val0));
                _mm_store_si128((__m128i*)buf, _mm_cvtps_epi32(_val));

                __m128 _cw = _mm_set_ps(color_weight[buf[3]],color_weight[buf[2]],
                                        color_weight[buf[1]],color_weight[buf[0]]);
                __m128 _sw = _mm_loadu_ps(space_weight+k);
                __m128 _w = _mm_mul_ps(_cw, _sw);
                _cw = _mm_mul_ps(_w, _valF);

                _sw = _mm_hadd_ps(_w, _cw);
                _sw = _mm_hadd_ps(_sw, _sw);
                _mm_storel_pi((__m64*)bufSum, _sw);

                sum += bufSum[1];
                wsum += bufSum[0];
              }
            }
            #endif
            for( ; k < maxk; k++ )
            {
              int val = sptr[j + space_ofs[k]];
              float w = space_weight[k]*color_weight[std::abs(val - val0)];
              sum += val*w;
              wsum += w;
            }
            // overflow is not possible here => there is no need to use cv::saturate_cast
            dptr[j] = (uchar)cvRound(sum/wsum);
          }
        }
        else
        {
          assert( cn == 3 );
          for( j = 0; j < size.width*3; j += 3 )
          {
            float sum_b = 0, sum_g = 0, sum_r = 0, wsum = 0;
            int b0 = sptr[j], g0 = sptr[j+1], r0 = sptr[j+2];
            k = 0;
            #if CV_SSE3
            if( haveSSE3 )
            {
              const __m128i izero = _mm_setzero_si128();
              const __m128 _b0 = _mm_set1_ps(static_cast<float>(b0));
              const __m128 _g0 = _mm_set1_ps(static_cast<float>(g0));
              const __m128 _r0 = _mm_set1_ps(static_cast<float>(r0));
              const __m128 _signMask = _mm_load_ps((const float*)bufSignMask);

              for( ; k <= maxk - 4; k += 4 )
              {
                const int* const sptr_k0  = reinterpret_cast<const int*>(sptr + j + space_ofs[k]);
                const int* const sptr_k1  = reinterpret_cast<const int*>(sptr + j + space_ofs[k+1]);
                const int* const sptr_k2  = reinterpret_cast<const int*>(sptr + j + space_ofs[k+2]);
                const int* const sptr_k3  = reinterpret_cast<const int*>(sptr + j + space_ofs[k+3]);

                __m128 _b = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(sptr_k0[0]), izero), izero));
                __m128 _g = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(sptr_k1[0]), izero), izero));
                __m128 _r = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(sptr_k2[0]), izero), izero));
                __m128 _z = _mm_cvtepi32_ps(_mm_unpacklo_epi16(_mm_unpacklo_epi8(_mm_cvtsi32_si128(sptr_k3[0]), izero), izero));

                _MM_TRANSPOSE4_PS(_b, _g, _r, _z);

                __m128 bt = _mm_andnot_ps(_signMask, _mm_sub_ps(_b,_b0));
                __m128 gt = _mm_andnot_ps(_signMask, _mm_sub_ps(_g,_g0));
                __m128 rt = _mm_andnot_ps(_signMask, _mm_sub_ps(_r,_r0));

                bt =_mm_add_ps(rt, _mm_add_ps(bt, gt));
                _mm_store_si128((__m128i*)buf, _mm_cvtps_epi32(bt));

                __m128 _w  = _mm_set_ps(color_weight[buf[3]],color_weight[buf[2]],
                                        color_weight[buf[1]],color_weight[buf[0]]);
                __m128 _sw = _mm_loadu_ps(space_weight+k);

                _w = _mm_mul_ps(_w,_sw);
                _b = _mm_mul_ps(_b, _w);
                _g = _mm_mul_ps(_g, _w);
                _r = _mm_mul_ps(_r, _w);

                _w = _mm_hadd_ps(_w, _b);
                _g = _mm_hadd_ps(_g, _r);

                _w = _mm_hadd_ps(_w, _g);
                _mm_store_ps(bufSum, _w);

                wsum  += bufSum[0];
                sum_b += bufSum[1];
                sum_g += bufSum[2];
                sum_r += bufSum[3];
              }
            }
            #endif

            for( ; k < maxk; k++ )
            {
              const uchar* sptr_k = sptr + j + space_ofs[k];
              int b = sptr_k[0], g = sptr_k[1], r = sptr_k[2];
              float w = space_weight[k]*color_weight[std::abs(b - b0) +
                                                    std::abs(g - g0) + std::abs(r - r0)];
              sum_b += b*w; sum_g += g*w; sum_r += r*w;
              wsum += w;
            }
            wsum = 1.f/wsum;
            b0 = cvRound(sum_b*wsum);
            g0 = cvRound(sum_g*wsum);
            r0 = cvRound(sum_r*wsum);
            dptr[j] = (uchar)b0; dptr[j+1] = (uchar)g0; dptr[j+2] = (uchar)r0;
          }
        }
      }
    }

  private:
      const cv::Mat *temp;
      cv::Mat *dest;
      int radius, maxk, *space_ofs;
      float *space_weight, *color_weight;
  };
  
  void bilateralFilter_8u
  ( const cv::Mat& src, cv::Mat& dst, int d
  , double sigma_color, double sigma_space
  , int borderType = cv::BORDER_DEFAULT
  )
  {
    int cn = src.channels();
    int i, j, maxk, radius;
    cv::Size size = src.size();

    CV_Assert( (src.type() == CV_8UC1 || src.type() == CV_8UC3) && src.data != dst.data );

    if( sigma_color <= 0 )
        sigma_color = 1;
    if( sigma_space <= 0 )
        sigma_space = 1;

    double gauss_color_coeff = -0.5/(sigma_color*sigma_color);
    double gauss_space_coeff = -0.5/(sigma_space*sigma_space);

    if( d <= 0 )
        radius = cvRound(sigma_space*1.5);
    else
        radius = d/2;
    radius = MAX(radius, 1);
    d = radius*2 + 1;

    cv::Mat temp;
    copyMakeBorder( src, temp, radius, radius, radius, radius, borderType );

  #if defined HAVE_IPP && (IPP_VERSION_MAJOR >= 7)
    if( cn == 1 )
    {
      bool ok;
      IPPBilateralFilter_8u_Invoker body(temp, dst, sigma_color * sigma_color, sigma_space * sigma_space, radius, &ok );
      parallel_for(Range(0, dst.rows), body, dst.total()/(double)(1<<16));
      if( ok ) return;
    }
  #endif

    std::vector<float> _color_weight(cn*256);
    std::vector<float> _space_weight(d*d);
    std::vector<int> _space_ofs(d*d);
    float* color_weight = &_color_weight[0];
    float* space_weight = &_space_weight[0];
    int* space_ofs = &_space_ofs[0];

    // initialize color-related bilateral filter coefficients

    for( i = 0; i < 256*cn; i++ )
      color_weight[i] = (float)std::exp(i*i*gauss_color_coeff);

    // initialize space-related bilateral filter coefficients
    for( i = -radius, maxk = 0; i <= radius; i++ )
    {
      j = -radius;

      for( ; j <= radius; j++ )
      {
        double r = std::sqrt((double)i*i + (double)j*j);
        if( r > radius )
          continue;
        space_weight[maxk] = (float)std::exp(r*r*gauss_space_coeff);
        space_ofs[maxk++] = (int)(i*temp.step + j*cn);
      }
    }

    BilateralFilter_8u_Invoker body(dst, temp, radius, maxk, space_ofs, space_weight, color_weight);
    parallel_for(cv::Range(0, size.height), body, dst.total()/(double)(1<<16));
  }
  
  
  
  // in : cv::Mat<CV_8UC3(BGR24)>
  // out: cv::Mat<CV_8UC1(B1)>
  cv::Mat filter_hsv_from_BGR24_to_single_channel
  ( const cv::Mat& src
  , const float h_min, const float h_max
  , const float s_min, const float s_max
  , const float v_min, const float v_max
  )
  {
    cv::Mat hsv;
    src.convertTo(hsv, CV_32F);
    cv::cvtColor(hsv, hsv, CV_BGR2HSV);
    
    cv::Mat dst(src.rows, src.cols, CV_8UC1);
    
    using result_element_t = uint8_t;
    using hsv_pixel_t = cv::Point3f;
    using src_pixel_t = cv::Point3_<uint8_t>;
    
          auto ihsv = reinterpret_cast<hsv_pixel_t*>(hsv.data);
    const auto ehsv = ihsv + hsv.total();
          auto idst = reinterpret_cast<result_element_t*>(dst.data);
          auto isrc = reinterpret_cast<src_pixel_t*>(src.data);
    
    while(ihsv < ehsv)
    {
      const auto& p = *ihsv++;
      
      *idst++ = 
        (
          ( ( p.x >= h_min && p.x <= h_max ) || ( h_max > 360.f &&  ( p.x >= h_min || p.x <= h_max - 360.f ) ) )
          &&  p.y >= s_min && p.y <= s_max
          &&  p.z >= v_min && p.z <= v_max
        )
          ? result_element_t((uint(isrc->z) + uint(isrc->y) * 4 + uint(isrc->x) * 2) / 7)
          //? result_element_t(float(isrc->z) * 0.298912f + float(isrc->y) * 0.586611 + float(isrc->x) * 0.114478)
          : 0
          ;
      
      ++isrc;
    }
    
    return dst;
  }
}
