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
  
  typedef ushort HT;
  
  typedef struct
  {
      HT coarse[16];
      HT fine[16][16];
  } Histogram;
  
#if CV_SSE2
  #define MEDIAN_HAVE_SIMD 1

  static inline void histogram_add_simd( const HT x[16], HT y[16] )
  {
    const __m128i* rx = (const __m128i*)x;
    __m128i* ry = (__m128i*)y;
    __m128i r0 = _mm_add_epi16(_mm_load_si128(ry+0),_mm_load_si128(rx+0));
    __m128i r1 = _mm_add_epi16(_mm_load_si128(ry+1),_mm_load_si128(rx+1));
    _mm_store_si128(ry+0, r0);
    _mm_store_si128(ry+1, r1);
  }

  static inline void histogram_sub_simd( const HT x[16], HT y[16] )
  {
    const __m128i* rx = (const __m128i*)x;
    __m128i* ry = (__m128i*)y;
    __m128i r0 = _mm_sub_epi16(_mm_load_si128(ry+0),_mm_load_si128(rx+0));
    __m128i r1 = _mm_sub_epi16(_mm_load_si128(ry+1),_mm_load_si128(rx+1));
    _mm_store_si128(ry+0, r0);
    _mm_store_si128(ry+1, r1);
  }

#else
  #define MEDIAN_HAVE_SIMD 0
#endif

  static inline void histogram_add( const HT x[16], HT y[16] )
  {
    int i;
    for( i = 0; i < 16; ++i )
      y[i] = (HT)(y[i] + x[i]);
  }

  static inline void histogram_sub( const HT x[16], HT y[16] )
  {
    int i;
    for( i = 0; i < 16; ++i )
      y[i] = (HT)(y[i] - x[i]);
  }

  static inline void histogram_muladd( int a, const HT x[16], HT y[16] )
  {
    for( int i = 0; i < 16; ++i )
      y[i] = (HT)(y[i] + a * x[i]);
  }

  static void medianBlur_8u_O1( const cv::Mat& _src, cv::Mat& _dst, int ksize )
  {
  /**
  * HOP is short for Histogram OPeration. This macro makes an operation \a op on
  * histogram \a h for pixel value \a x. It takes care of handling both levels.
  */
  #define HOP(h,x,op) \
      h.coarse[x>>4] op, \
      *((HT*)h.fine + x) op

  #define COP(c,j,x,op) \
      h_coarse[ 16*(n*c+j) + (x>>4) ] op, \
      h_fine[ 16 * (n*(16*c+(x>>4)) + j) + (x & 0xF) ] op

      int cn = _dst.channels(), m = _dst.rows, r = (ksize-1)/2;
      size_t sstep = _src.step, dstep = _dst.step;
      Histogram CV_DECL_ALIGNED(16) H[4];
      HT CV_DECL_ALIGNED(16) luc[4][16];

      int STRIPE_SIZE = std::min( _dst.cols, 512/cn );

      std::vector<HT> _h_coarse(1 * 16 * (STRIPE_SIZE + 2*r) * cn + 16);
      std::vector<HT> _h_fine(16 * 16 * (STRIPE_SIZE + 2*r) * cn + 16);
      HT* h_coarse = cv::alignPtr(&_h_coarse[0], 16);
      HT* h_fine = cv::alignPtr(&_h_fine[0], 16);
  #if MEDIAN_HAVE_SIMD
      volatile bool useSIMD = cv::checkHardwareSupport(CV_CPU_SSE2);
  #endif

      for( int x = 0; x < _dst.cols; x += STRIPE_SIZE )
      {
          int i, j, k, c, n = std::min(_dst.cols - x, STRIPE_SIZE) + r*2;
          const uchar* src = _src.data + x*cn;
          uchar* dst = _dst.data + (x - r)*cn;

          memset( h_coarse, 0, 16*n*cn*sizeof(h_coarse[0]) );
          memset( h_fine, 0, 16*16*n*cn*sizeof(h_fine[0]) );

          // First row initialization
          for( c = 0; c < cn; c++ )
          {
              for( j = 0; j < n; j++ )
                  COP( c, j, src[cn*j+c], += (HT)(r+2) );

              for( i = 1; i < r; i++ )
              {
                  const uchar* p = src + sstep*std::min(i, m-1);
                  for ( j = 0; j < n; j++ )
                      COP( c, j, p[cn*j+c], ++ );
              }
          }

          for( i = 0; i < m; i++ )
          {
              const uchar* p0 = src + sstep * std::max( 0, i-r-1 );
              const uchar* p1 = src + sstep * std::min( m-1, i+r );

              memset( H, 0, cn*sizeof(H[0]) );
              memset( luc, 0, cn*sizeof(luc[0]) );
              for( c = 0; c < cn; c++ )
              {
                  // Update column histograms for the entire row.
                  for( j = 0; j < n; j++ )
                  {
                      COP( c, j, p0[j*cn + c], -- );
                      COP( c, j, p1[j*cn + c], ++ );
                  }

                  // First column initialization
                  for( k = 0; k < 16; ++k )
                      histogram_muladd( 2*r+1, &h_fine[16*n*(16*c+k)], &H[c].fine[k][0] );

              #if MEDIAN_HAVE_SIMD
                  if( useSIMD )
                  {
                      for( j = 0; j < 2*r; ++j )
                          histogram_add_simd( &h_coarse[16*(n*c+j)], H[c].coarse );

                      for( j = r; j < n-r; j++ )
                      {
                          int t = 2*r*r + 2*r, b, sum = 0;
                          HT* segment;

                          histogram_add_simd( &h_coarse[16*(n*c + std::min(j+r,n-1))], H[c].coarse );

                          // Find median at coarse level
                          for ( k = 0; k < 16 ; ++k )
                          {
                              sum += H[c].coarse[k];
                              if ( sum > t )
                              {
                                  sum -= H[c].coarse[k];
                                  break;
                              }
                          }
                          assert( k < 16 );

                          /* Update corresponding histogram segment */
                          if ( luc[c][k] <= j-r )
                          {
                              memset( &H[c].fine[k], 0, 16 * sizeof(HT) );
                              for ( luc[c][k] = HT(j-r); luc[c][k] < MIN(j+r+1,n); ++luc[c][k] )
                                  histogram_add_simd( &h_fine[16*(n*(16*c+k)+luc[c][k])], H[c].fine[k] );

                              if ( luc[c][k] < j+r+1 )
                              {
                                  histogram_muladd( j+r+1 - n, &h_fine[16*(n*(16*c+k)+(n-1))], &H[c].fine[k][0] );
                                  luc[c][k] = (HT)(j+r+1);
                              }
                          }
                          else
                          {
                              for ( ; luc[c][k] < j+r+1; ++luc[c][k] )
                              {
                                  histogram_sub_simd( &h_fine[16*(n*(16*c+k)+MAX(luc[c][k]-2*r-1,0))], H[c].fine[k] );
                                  histogram_add_simd( &h_fine[16*(n*(16*c+k)+MIN(luc[c][k],n-1))], H[c].fine[k] );
                              }
                          }

                          histogram_sub_simd( &h_coarse[16*(n*c+MAX(j-r,0))], H[c].coarse );

                          /* Find median in segment */
                          segment = H[c].fine[k];
                          for ( b = 0; b < 16 ; b++ )
                          {
                              sum += segment[b];
                              if ( sum > t )
                              {
                                  dst[dstep*i+cn*j+c] = (uchar)(16*k + b);
                                  break;
                              }
                          }
                          assert( b < 16 );
                      }
                  }
                  else
              #endif
                  {
                      for( j = 0; j < 2*r; ++j )
                          histogram_add( &h_coarse[16*(n*c+j)], H[c].coarse );

                      for( j = r; j < n-r; j++ )
                      {
                          int t = 2*r*r + 2*r, b, sum = 0;
                          HT* segment;

                          histogram_add( &h_coarse[16*(n*c + std::min(j+r,n-1))], H[c].coarse );

                          // Find median at coarse level
                          for ( k = 0; k < 16 ; ++k )
                          {
                              sum += H[c].coarse[k];
                              if ( sum > t )
                              {
                                  sum -= H[c].coarse[k];
                                  break;
                              }
                          }
                          assert( k < 16 );

                          /* Update corresponding histogram segment */
                          if ( luc[c][k] <= j-r )
                          {
                              memset( &H[c].fine[k], 0, 16 * sizeof(HT) );
                              for ( luc[c][k] = HT(j-r); luc[c][k] < MIN(j+r+1,n); ++luc[c][k] )
                                  histogram_add( &h_fine[16*(n*(16*c+k)+luc[c][k])], H[c].fine[k] );

                              if ( luc[c][k] < j+r+1 )
                              {
                                  histogram_muladd( j+r+1 - n, &h_fine[16*(n*(16*c+k)+(n-1))], &H[c].fine[k][0] );
                                  luc[c][k] = (HT)(j+r+1);
                              }
                          }
                          else
                          {
                              for ( ; luc[c][k] < j+r+1; ++luc[c][k] )
                              {
                                  histogram_sub( &h_fine[16*(n*(16*c+k)+MAX(luc[c][k]-2*r-1,0))], H[c].fine[k] );
                                  histogram_add( &h_fine[16*(n*(16*c+k)+MIN(luc[c][k],n-1))], H[c].fine[k] );
                              }
                          }

                          histogram_sub( &h_coarse[16*(n*c+MAX(j-r,0))], H[c].coarse );

                          /* Find median in segment */
                          segment = H[c].fine[k];
                          for ( b = 0; b < 16 ; b++ )
                          {
                              sum += segment[b];
                              if ( sum > t )
                              {
                                  dst[dstep*i+cn*j+c] = (uchar)(16*k + b);
                                  break;
                              }
                          }
                          assert( b < 16 );
                      }
                  }
              }
          }
      }

  #undef HOP
  #undef COP
  }

  static void medianBlur_8u_Om( const cv::Mat& _src, cv::Mat& _dst, int m )
  {
    #define N  16
    int           zone0[4][N];
    int           zone1[4][N*N];
    int           x, y;
    int           n2 = m*m/2;
    cv::Size      size = _dst.size();
    const uchar*  src = _src.data;
    uchar*        dst = _dst.data;
    int           src_step = (int)_src.step, dst_step = (int)_dst.step;
    int           cn = _src.channels();
    const uchar*  src_max = src + size.height*src_step;

    #define UPDATE_ACC01( pix, cn, op ) \
    {                                   \
        int p = (pix);                  \
        zone1[cn][p] op;                \
        zone0[cn][p >> 4] op;           \
    }

    //CV_Assert( size.height >= nx && size.width >= nx );
    for( x = 0; x < size.width; x++, src += cn, dst += cn )
    {
      uchar* dst_cur = dst;
      const uchar* src_top = src;
      const uchar* src_bottom = src;
      int k, c;
      int src_step1 = src_step, dst_step1 = dst_step;

      if( x % 2 != 0 )
      {
        src_bottom = src_top += src_step*(size.height-1);
        dst_cur += dst_step*(size.height-1);
        src_step1 = -src_step1;
        dst_step1 = -dst_step1;
      }

      // init accumulator
      memset( zone0, 0, sizeof(zone0[0])*cn );
      memset( zone1, 0, sizeof(zone1[0])*cn );

      for( y = 0; y <= m/2; y++ )
      {
        for( c = 0; c < cn; c++ )
        {
          if( y > 0 )
          {
            for( k = 0; k < m*cn; k += cn )
              UPDATE_ACC01( src_bottom[k+c], c, ++ );
          }
          else
          {
            for( k = 0; k < m*cn; k += cn )
              UPDATE_ACC01( src_bottom[k+c], c, += m/2+1 );
          }
        }

        if( (src_step1 > 0 && y < size.height-1) ||
            (src_step1 < 0 && size.height-y-1 > 0) )
          src_bottom += src_step1;
      }

      for( y = 0; y < size.height; y++, dst_cur += dst_step1 )
      {
        // find median
        for( c = 0; c < cn; c++ )
        {
          int s = 0;
          for( k = 0; ; k++ )
          {
            int t = s + zone0[c][k];
            if( t > n2 ) break;
            s = t;
          }

          for( k *= N; ;k++ )
          {
            s += zone1[c][k];
            if( s > n2 ) break;
          }

          dst_cur[c] = (uchar)k;
        }

        if( y+1 == size.height )
          break;

        if( cn == 1 )
        {
          for( k = 0; k < m; k++ )
          {
            int p = src_top[k];
            int q = src_bottom[k];
            zone1[0][p]--;
            zone0[0][p>>4]--;
            zone1[0][q]++;
            zone0[0][q>>4]++;
          }
        }
        else if( cn == 3 )
        {
          for( k = 0; k < m*3; k += 3 )
          {
            UPDATE_ACC01( src_top[k], 0, -- );
            UPDATE_ACC01( src_top[k+1], 1, -- );
            UPDATE_ACC01( src_top[k+2], 2, -- );

            UPDATE_ACC01( src_bottom[k], 0, ++ );
            UPDATE_ACC01( src_bottom[k+1], 1, ++ );
            UPDATE_ACC01( src_bottom[k+2], 2, ++ );
          }
        }
        else
        {
          assert( cn == 4 );
          for( k = 0; k < m*4; k += 4 )
          {
            UPDATE_ACC01( src_top[k], 0, -- );
            UPDATE_ACC01( src_top[k+1], 1, -- );
            UPDATE_ACC01( src_top[k+2], 2, -- );
            UPDATE_ACC01( src_top[k+3], 3, -- );

            UPDATE_ACC01( src_bottom[k], 0, ++ );
            UPDATE_ACC01( src_bottom[k+1], 1, ++ );
            UPDATE_ACC01( src_bottom[k+2], 2, ++ );
            UPDATE_ACC01( src_bottom[k+3], 3, ++ );
          }
        }

        if( (src_step1 > 0 && src_bottom + src_step1 < src_max) ||
            (src_step1 < 0 && src_bottom + src_step1 >= src) )
          src_bottom += src_step1;

        if( y >= m/2 )
          src_top += src_step1;
      }
    }
    #undef N
    #undef UPDATE_ACC
  }
  
  namespace table
  {
    const uchar icvSaturate8u_cv[] =
      {
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
          0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
          0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,  15,
        16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,
        32,  33,  34,  35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,
        48,  49,  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,
        64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
        80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,  95,
        96,  97,  98,  99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111,
        112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125, 126, 127,
        128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
        144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159,
        160, 161, 162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175,
        176, 177, 178, 179, 180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191,
        192, 193, 194, 195, 196, 197, 198, 199, 200, 201, 202, 203, 204, 205, 206, 207,
        208, 209, 210, 211, 212, 213, 214, 215, 216, 217, 218, 219, 220, 221, 222, 223,
        224, 225, 226, 227, 228, 229, 230, 231, 232, 233, 234, 235, 236, 237, 238, 239,
        240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
        255
      };
  }
  
  template<class T>
  constexpr uchar CV_FAST_CAST_8U(const T t) { return table::icvSaturate8u_cv[t + 256]; }
  //#define CV_FAST_CAST_8U(t)  (assert(-256 <= (t) && (t) <= 512), icvSaturate8u_cv[(t)+256])
  
  struct MinMax8u
  {
    typedef uchar value_type;
    typedef int arg_type;
    enum { SIZE = 1 };
    arg_type load(const uchar* ptr) { return *ptr; }
    void store(uchar* ptr, arg_type val) { *ptr = (uchar)val; }
    void operator()(arg_type& a, arg_type& b) const
    {
      int t = CV_FAST_CAST_8U(a - b);
      b += t; a -= t;
    }
  };

  struct MinMax16u
  {
    typedef ushort value_type;
    typedef int arg_type;
    enum { SIZE = 1 };
    arg_type load(const ushort* ptr) { return *ptr; }
    void store(ushort* ptr, arg_type val) { *ptr = (ushort)val; }
    void operator()(arg_type& a, arg_type& b) const
    {
      arg_type t = a;
      a = std::min(a, b);
      b = std::max(b, t);
    }
  };

  struct MinMax16s
  {
    typedef short value_type;
    typedef int arg_type;
    enum { SIZE = 1 };
    arg_type load(const short* ptr) { return *ptr; }
    void store(short* ptr, arg_type val) { *ptr = (short)val; }
    void operator()(arg_type& a, arg_type& b) const
    {
      arg_type t = a;
      a = std::min(a, b);
      b = std::max(b, t);
    }
  };

  struct MinMax32f
  {
    typedef float value_type;
    typedef float arg_type;
    enum { SIZE = 1 };
    arg_type load(const float* ptr) { return *ptr; }
    void store(float* ptr, arg_type val) { *ptr = val; }
    void operator()(arg_type& a, arg_type& b) const
    {
      arg_type t = a;
      a = std::min(a, b);
      b = std::max(b, t);
    }
  };

  #if CV_SSE2

  struct MinMaxVec8u
  {
    typedef uchar value_type;
    typedef __m128i arg_type;
    enum { SIZE = 16 };
    arg_type load(const uchar* ptr) { return _mm_loadu_si128((const __m128i*)ptr); }
    void store(uchar* ptr, arg_type val) { _mm_storeu_si128((__m128i*)ptr, val); }
    void operator()(arg_type& a, arg_type& b) const
    {
      arg_type t = a;
      a = _mm_min_epu8(a, b);
      b = _mm_max_epu8(b, t);
    }
  };


  struct MinMaxVec16u
  {
    typedef ushort value_type;
    typedef __m128i arg_type;
    enum { SIZE = 8 };
    arg_type load(const ushort* ptr) { return _mm_loadu_si128((const __m128i*)ptr); }
    void store(ushort* ptr, arg_type val) { _mm_storeu_si128((__m128i*)ptr, val); }
    void operator()(arg_type& a, arg_type& b) const
    {
      arg_type t = _mm_subs_epu16(a, b);
      a = _mm_subs_epu16(a, t);
      b = _mm_adds_epu16(b, t);
    }
  };


  struct MinMaxVec16s
  {
    typedef short value_type;
    typedef __m128i arg_type;
    enum { SIZE = 8 };
    arg_type load(const short* ptr) { return _mm_loadu_si128((const __m128i*)ptr); }
    void store(short* ptr, arg_type val) { _mm_storeu_si128((__m128i*)ptr, val); }
    void operator()(arg_type& a, arg_type& b) const
    {
      arg_type t = a;
      a = _mm_min_epi16(a, b);
      b = _mm_max_epi16(b, t);
    }
  };


  struct MinMaxVec32f
  {
    typedef float value_type;
    typedef __m128 arg_type;
    enum { SIZE = 4 };
    arg_type load(const float* ptr) { return _mm_loadu_ps(ptr); }
    void store(float* ptr, arg_type val) { _mm_storeu_ps(ptr, val); }
    void operator()(arg_type& a, arg_type& b) const
    {
      arg_type t = a;
      a = _mm_min_ps(a, b);
      b = _mm_max_ps(b, t);
    }
  };

  #else

  typedef MinMax8u MinMaxVec8u;
  typedef MinMax16u MinMaxVec16u;
  typedef MinMax16s MinMaxVec16s;
  typedef MinMax32f MinMaxVec32f;

  #endif

  template<class Op, class VecOp>
  static void
  medianBlur_SortNet( const cv::Mat& _src, cv::Mat& _dst, int m )
  {
      typedef typename Op::value_type T;
      typedef typename Op::arg_type WT;
      typedef typename VecOp::arg_type VT;

      const T* src = (const T*)_src.data;
      T* dst = (T*)_dst.data;
      int sstep = (int)(_src.step/sizeof(T));
      int dstep = (int)(_dst.step/sizeof(T));
      cv::Size size = _dst.size();
      int i, j, k, cn = _src.channels();
      Op op;
      VecOp vop;
      volatile bool useSIMD = cv::checkHardwareSupport(CV_CPU_SSE2);

      if( m == 3 )
      {
          if( size.width == 1 || size.height == 1 )
          {
              int len = size.width + size.height - 1;
              int sdelta = size.height == 1 ? cn : sstep;
              int sdelta0 = size.height == 1 ? 0 : sstep - cn;
              int ddelta = size.height == 1 ? cn : dstep;

              for( i = 0; i < len; i++, src += sdelta0, dst += ddelta )
                  for( j = 0; j < cn; j++, src++ )
                  {
                      WT p0 = src[i > 0 ? -sdelta : 0];
                      WT p1 = src[0];
                      WT p2 = src[i < len - 1 ? sdelta : 0];

                      op(p0, p1); op(p1, p2); op(p0, p1);
                      dst[j] = (T)p1;
                  }
              return;
          }

          size.width *= cn;
          for( i = 0; i < size.height; i++, dst += dstep )
          {
              const T* row0 = src + std::max(i - 1, 0)*sstep;
              const T* row1 = src + i*sstep;
              const T* row2 = src + std::min(i + 1, size.height-1)*sstep;
              int limit = useSIMD ? cn : size.width;

              for(j = 0;; )
              {
                  for( ; j < limit; j++ )
                  {
                      int j0 = j >= cn ? j - cn : j;
                      int j2 = j < size.width - cn ? j + cn : j;
                      WT p0 = row0[j0], p1 = row0[j], p2 = row0[j2];
                      WT p3 = row1[j0], p4 = row1[j], p5 = row1[j2];
                      WT p6 = row2[j0], p7 = row2[j], p8 = row2[j2];

                      op(p1, p2); op(p4, p5); op(p7, p8); op(p0, p1);
                      op(p3, p4); op(p6, p7); op(p1, p2); op(p4, p5);
                      op(p7, p8); op(p0, p3); op(p5, p8); op(p4, p7);
                      op(p3, p6); op(p1, p4); op(p2, p5); op(p4, p7);
                      op(p4, p2); op(p6, p4); op(p4, p2);
                      dst[j] = (T)p4;
                  }

                  if( limit == size.width )
                      break;

                  for( ; j <= size.width - VecOp::SIZE - cn; j += VecOp::SIZE )
                  {
                      VT p0 = vop.load(row0+j-cn), p1 = vop.load(row0+j), p2 = vop.load(row0+j+cn);
                      VT p3 = vop.load(row1+j-cn), p4 = vop.load(row1+j), p5 = vop.load(row1+j+cn);
                      VT p6 = vop.load(row2+j-cn), p7 = vop.load(row2+j), p8 = vop.load(row2+j+cn);

                      vop(p1, p2); vop(p4, p5); vop(p7, p8); vop(p0, p1);
                      vop(p3, p4); vop(p6, p7); vop(p1, p2); vop(p4, p5);
                      vop(p7, p8); vop(p0, p3); vop(p5, p8); vop(p4, p7);
                      vop(p3, p6); vop(p1, p4); vop(p2, p5); vop(p4, p7);
                      vop(p4, p2); vop(p6, p4); vop(p4, p2);
                      vop.store(dst+j, p4);
                  }

                  limit = size.width;
              }
          }
      }
      else if( m == 5 )
      {
          if( size.width == 1 || size.height == 1 )
          {
              int len = size.width + size.height - 1;
              int sdelta = size.height == 1 ? cn : sstep;
              int sdelta0 = size.height == 1 ? 0 : sstep - cn;
              int ddelta = size.height == 1 ? cn : dstep;

              for( i = 0; i < len; i++, src += sdelta0, dst += ddelta )
                  for( j = 0; j < cn; j++, src++ )
                  {
                      int i1 = i > 0 ? -sdelta : 0;
                      int i0 = i > 1 ? -sdelta*2 : i1;
                      int i3 = i < len-1 ? sdelta : 0;
                      int i4 = i < len-2 ? sdelta*2 : i3;
                      WT p0 = src[i0], p1 = src[i1], p2 = src[0], p3 = src[i3], p4 = src[i4];

                      op(p0, p1); op(p3, p4); op(p2, p3); op(p3, p4); op(p0, p2);
                      op(p2, p4); op(p1, p3); op(p1, p2);
                      dst[j] = (T)p2;
                  }
              return;
          }

          size.width *= cn;
          for( i = 0; i < size.height; i++, dst += dstep )
          {
              const T* row[5];
              row[0] = src + std::max(i - 2, 0)*sstep;
              row[1] = src + std::max(i - 1, 0)*sstep;
              row[2] = src + i*sstep;
              row[3] = src + std::min(i + 1, size.height-1)*sstep;
              row[4] = src + std::min(i + 2, size.height-1)*sstep;
              int limit = useSIMD ? cn*2 : size.width;

              for(j = 0;; )
              {
                  for( ; j < limit; j++ )
                  {
                      WT p[25];
                      int j1 = j >= cn ? j - cn : j;
                      int j0 = j >= cn*2 ? j - cn*2 : j1;
                      int j3 = j < size.width - cn ? j + cn : j;
                      int j4 = j < size.width - cn*2 ? j + cn*2 : j3;
                      for( k = 0; k < 5; k++ )
                      {
                          const T* rowk = row[k];
                          p[k*5] = rowk[j0]; p[k*5+1] = rowk[j1];
                          p[k*5+2] = rowk[j]; p[k*5+3] = rowk[j3];
                          p[k*5+4] = rowk[j4];
                      }

                      op(p[1], p[2]); op(p[0], p[1]); op(p[1], p[2]); op(p[4], p[5]); op(p[3], p[4]);
                      op(p[4], p[5]); op(p[0], p[3]); op(p[2], p[5]); op(p[2], p[3]); op(p[1], p[4]);
                      op(p[1], p[2]); op(p[3], p[4]); op(p[7], p[8]); op(p[6], p[7]); op(p[7], p[8]);
                      op(p[10], p[11]); op(p[9], p[10]); op(p[10], p[11]); op(p[6], p[9]); op(p[8], p[11]);
                      op(p[8], p[9]); op(p[7], p[10]); op(p[7], p[8]); op(p[9], p[10]); op(p[0], p[6]);
                      op(p[4], p[10]); op(p[4], p[6]); op(p[2], p[8]); op(p[2], p[4]); op(p[6], p[8]);
                      op(p[1], p[7]); op(p[5], p[11]); op(p[5], p[7]); op(p[3], p[9]); op(p[3], p[5]);
                      op(p[7], p[9]); op(p[1], p[2]); op(p[3], p[4]); op(p[5], p[6]); op(p[7], p[8]);
                      op(p[9], p[10]); op(p[13], p[14]); op(p[12], p[13]); op(p[13], p[14]); op(p[16], p[17]);
                      op(p[15], p[16]); op(p[16], p[17]); op(p[12], p[15]); op(p[14], p[17]); op(p[14], p[15]);
                      op(p[13], p[16]); op(p[13], p[14]); op(p[15], p[16]); op(p[19], p[20]); op(p[18], p[19]);
                      op(p[19], p[20]); op(p[21], p[22]); op(p[23], p[24]); op(p[21], p[23]); op(p[22], p[24]);
                      op(p[22], p[23]); op(p[18], p[21]); op(p[20], p[23]); op(p[20], p[21]); op(p[19], p[22]);
                      op(p[22], p[24]); op(p[19], p[20]); op(p[21], p[22]); op(p[23], p[24]); op(p[12], p[18]);
                      op(p[16], p[22]); op(p[16], p[18]); op(p[14], p[20]); op(p[20], p[24]); op(p[14], p[16]);
                      op(p[18], p[20]); op(p[22], p[24]); op(p[13], p[19]); op(p[17], p[23]); op(p[17], p[19]);
                      op(p[15], p[21]); op(p[15], p[17]); op(p[19], p[21]); op(p[13], p[14]); op(p[15], p[16]);
                      op(p[17], p[18]); op(p[19], p[20]); op(p[21], p[22]); op(p[23], p[24]); op(p[0], p[12]);
                      op(p[8], p[20]); op(p[8], p[12]); op(p[4], p[16]); op(p[16], p[24]); op(p[12], p[16]);
                      op(p[2], p[14]); op(p[10], p[22]); op(p[10], p[14]); op(p[6], p[18]); op(p[6], p[10]);
                      op(p[10], p[12]); op(p[1], p[13]); op(p[9], p[21]); op(p[9], p[13]); op(p[5], p[17]);
                      op(p[13], p[17]); op(p[3], p[15]); op(p[11], p[23]); op(p[11], p[15]); op(p[7], p[19]);
                      op(p[7], p[11]); op(p[11], p[13]); op(p[11], p[12]);
                      dst[j] = (T)p[12];
                  }

                  if( limit == size.width )
                      break;

                  for( ; j <= size.width - VecOp::SIZE - cn*2; j += VecOp::SIZE )
                  {
                      VT p[25];
                      for( k = 0; k < 5; k++ )
                      {
                          const T* rowk = row[k];
                          p[k*5] = vop.load(rowk+j-cn*2); p[k*5+1] = vop.load(rowk+j-cn);
                          p[k*5+2] = vop.load(rowk+j); p[k*5+3] = vop.load(rowk+j+cn);
                          p[k*5+4] = vop.load(rowk+j+cn*2);
                      }

                      vop(p[1], p[2]); vop(p[0], p[1]); vop(p[1], p[2]); vop(p[4], p[5]); vop(p[3], p[4]);
                      vop(p[4], p[5]); vop(p[0], p[3]); vop(p[2], p[5]); vop(p[2], p[3]); vop(p[1], p[4]);
                      vop(p[1], p[2]); vop(p[3], p[4]); vop(p[7], p[8]); vop(p[6], p[7]); vop(p[7], p[8]);
                      vop(p[10], p[11]); vop(p[9], p[10]); vop(p[10], p[11]); vop(p[6], p[9]); vop(p[8], p[11]);
                      vop(p[8], p[9]); vop(p[7], p[10]); vop(p[7], p[8]); vop(p[9], p[10]); vop(p[0], p[6]);
                      vop(p[4], p[10]); vop(p[4], p[6]); vop(p[2], p[8]); vop(p[2], p[4]); vop(p[6], p[8]);
                      vop(p[1], p[7]); vop(p[5], p[11]); vop(p[5], p[7]); vop(p[3], p[9]); vop(p[3], p[5]);
                      vop(p[7], p[9]); vop(p[1], p[2]); vop(p[3], p[4]); vop(p[5], p[6]); vop(p[7], p[8]);
                      vop(p[9], p[10]); vop(p[13], p[14]); vop(p[12], p[13]); vop(p[13], p[14]); vop(p[16], p[17]);
                      vop(p[15], p[16]); vop(p[16], p[17]); vop(p[12], p[15]); vop(p[14], p[17]); vop(p[14], p[15]);
                      vop(p[13], p[16]); vop(p[13], p[14]); vop(p[15], p[16]); vop(p[19], p[20]); vop(p[18], p[19]);
                      vop(p[19], p[20]); vop(p[21], p[22]); vop(p[23], p[24]); vop(p[21], p[23]); vop(p[22], p[24]);
                      vop(p[22], p[23]); vop(p[18], p[21]); vop(p[20], p[23]); vop(p[20], p[21]); vop(p[19], p[22]);
                      vop(p[22], p[24]); vop(p[19], p[20]); vop(p[21], p[22]); vop(p[23], p[24]); vop(p[12], p[18]);
                      vop(p[16], p[22]); vop(p[16], p[18]); vop(p[14], p[20]); vop(p[20], p[24]); vop(p[14], p[16]);
                      vop(p[18], p[20]); vop(p[22], p[24]); vop(p[13], p[19]); vop(p[17], p[23]); vop(p[17], p[19]);
                      vop(p[15], p[21]); vop(p[15], p[17]); vop(p[19], p[21]); vop(p[13], p[14]); vop(p[15], p[16]);
                      vop(p[17], p[18]); vop(p[19], p[20]); vop(p[21], p[22]); vop(p[23], p[24]); vop(p[0], p[12]);
                      vop(p[8], p[20]); vop(p[8], p[12]); vop(p[4], p[16]); vop(p[16], p[24]); vop(p[12], p[16]);
                      vop(p[2], p[14]); vop(p[10], p[22]); vop(p[10], p[14]); vop(p[6], p[18]); vop(p[6], p[10]);
                      vop(p[10], p[12]); vop(p[1], p[13]); vop(p[9], p[21]); vop(p[9], p[13]); vop(p[5], p[17]);
                      vop(p[13], p[17]); vop(p[3], p[15]); vop(p[11], p[23]); vop(p[11], p[15]); vop(p[7], p[19]);
                      vop(p[7], p[11]); vop(p[11], p[13]); vop(p[11], p[12]);
                      vop.store(dst+j, p[12]);
                  }

                  limit = size.width;
              }
          }
      }
  }
  
  void medianBlur(const cv::Mat& src0, cv::Mat& dst, int ksize)
  {

    if( ksize <= 1 )
    {
      dst = src0.clone();
      return;
    }
    
    cv::Mat src;
    dst.create( src0.size(), src0.type() );
    
    
    bool useSortNet = ksize == 3 || (ksize == 5
#if !CV_SSE2
            && src0.depth() > CV_8U
#endif
        );

    if( useSortNet )
    {
        if( dst.data != src0.data )
            src = src0;
        else
            src0.copyTo(src);

        if( src.depth() == CV_8U )
            medianBlur_SortNet<MinMax8u, MinMaxVec8u>( src, dst, ksize );
        else if( src.depth() == CV_16U )
            medianBlur_SortNet<MinMax16u, MinMaxVec16u>( src, dst, ksize );
        else if( src.depth() == CV_16S )
            medianBlur_SortNet<MinMax16s, MinMaxVec16s>( src, dst, ksize );
        else if( src.depth() == CV_32F )
            medianBlur_SortNet<MinMax32f, MinMaxVec32f>( src, dst, ksize );
        else
            CV_Error(CV_StsUnsupportedFormat, "");

        return;
    }
    else
    {
      cv::copyMakeBorder( src0, src, 0, 0, ksize/2, ksize/2, cv::BORDER_REPLICATE );

      int cn = src0.channels();
      CV_Assert( src.depth() == CV_8U && (cn == 1 || cn == 3 || cn == 4) );

      double img_size_mp = (double)(src0.total())/(1 << 20);
      if( ksize <= 3 + (img_size_mp < 1 ? 12 : img_size_mp < 4 ? 6 : 2)*(MEDIAN_HAVE_SIMD && cv::checkHardwareSupport(CV_CPU_SSE2) ? 1 : 3))
        medianBlur_8u_Om( src, dst, ksize );
      else
        medianBlur_8u_O1( src, dst, ksize );
    }
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
