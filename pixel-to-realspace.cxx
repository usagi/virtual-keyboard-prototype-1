#include <cmath>
#include <array>
#include <iostream>
#include <memory>
#include <typeinfo>
#include <boost/math/constants/constants.hpp>
#include <boost/program_options.hpp>

namespace
{
  using float_t = long double;
  using a3d_t = std::array<float_t, 3>;
  using a2d_t = std::array<float_t, 2>;

  template<class T>
  T pov_to_array(const boost::program_options::variable_value& pov, const T& default_value)
  {
    std::vector<typename T::value_type> v;
    
    try
    { v = pov.as<std::vector<typename T::value_type>>(); }
    catch(const boost::bad_any_cast&)
    { return default_value; }
    
    if(v.size() != sizeof(T) / sizeof(typename T::value_type))
      throw std::bad_cast();
    
    return std::move(*reinterpret_cast<T*>(v.data()));
  }

  template<class T> constexpr typename T::value_type x(const T& a) { return a.at(0); }
  template<class T> constexpr typename T::value_type y(const T& a) { return a.at(1); }
  template<class T> constexpr typename T::value_type z(const T& a) { return a.at(2); }

  constexpr auto default_top_camera_position   = a3d_t{{0.l, 207.l, 264.l}};
  constexpr auto default_front_camera_position = a3d_t{{0.l,  37.l, 350.l}};
  constexpr auto default_top_camera_angle_x    = 31.1l;
  constexpr auto default_camera_fov_diagonal   = 64.l;
  constexpr auto default_camera_sensor_size    = a2d_t{{3.60l, 2.70l}};
  constexpr auto default_image_size            = a2d_t{{640.l, 480.l}};
  constexpr auto default_top_image_target      = a2d_t{{320.l, 240.l}};
  constexpr auto default_front_image_target    = a2d_t{{320.l, 240.l}};
  constexpr auto version_info   = "vertual-keyboard-prototype-1/pixel-to-realspace\n";

  template<class T_in_out, class T_internal = float_t>
  constexpr T_in_out degrees_to_radians(T_in_out degrees)
  { return T_internal(degrees) * boost::math::constants::pi<T_internal>() / T_internal(180.l); }
  
  template<class T_in_out, class T_internal = float_t>
  constexpr T_in_out radians_to_degrees(T_in_out radians)
  { return T_internal(radians) * T_internal(180.l) / boost::math::constants::pi<T_internal>(); }

  template<class T> constexpr T pow2(const T v){ return v * v; }
  template<class T> constexpr T diagonal(const T a, const T b){ return std::sqrt( pow2(a) + pow2(b) ); }

  template<class T> constexpr T uvalue_to_snorm(const T v, const T max)
  {
    return (v - (max / 2)) / (max / 2);
  }
  
  void run
  ( const a3d_t& top_camera_position
  , const a3d_t& front_camera_position
  , const float_t top_camera_angle_x
  , const float_t camera_fov_diagonal
  , const a2d_t& camera_sensor_size
  , const a2d_t& image_size
  , const a2d_t& top_image_target
  , const a2d_t& front_image_target
  )
  {
    std::cout << "top camera position x     [mm]: " << x(top_camera_position) << "\n"
                 "                    y     [mm]: " << y(top_camera_position) << "\n"
                 "                    z     [mm]: " << z(top_camera_position) << "\n"
                 "front camera position x   [mm]: " << x(front_camera_position) << "\n"
                 "                      y   [mm]: " << y(front_camera_position) << "\n"
                 "                      z   [mm]: " << z(front_camera_position) << "\n"
              ;
    
    // std::tanなどの三角関数はラジアン単位の仕様なので弧度法単位から変換しておく
    // カメラのX軸回転角度[rad]
    auto top_camera_angle_x_rad = degrees_to_radians(top_camera_angle_x);
    
    std::cout << "top camera angle x       [deg]: " << top_camera_angle_x     << "\n"
                 "                         [rad]: " << top_camera_angle_x_rad << "\n"
              ;
    
    // カメラのセンサーの対角線の長さ
    auto camera_sensor_diagonal = diagonal(x(camera_sensor_size), y(camera_sensor_size));
    
    std::cout << "camera sensor width       [mm]: " << x(camera_sensor_size)  << "\n"
                 "              height      [mm]: " << y(camera_sensor_size)  << "\n"
                 "              diggonal    [mm]: " << camera_sensor_diagonal << "\n"
              ;
    
    // カメラの対角視野角[rad]
    auto camera_fov_diagonal_rad = degrees_to_radians(camera_fov_diagonal);
    // カメラの1/2対角視野角[rad]
    auto camera_fov_diagonal_div_2_rad = camera_fov_diagonal_rad / 2.l;
    
    // カメラの焦点距離
    auto camera_focal_length = camera_sensor_diagonal / std::tan(camera_fov_diagonal_div_2_rad) / 2.l;
    
    // カメラの1/2水平垂直視野角[rad]
    a2d_t camera_fov_div_2_rad
    {{ std::atan(x(camera_sensor_size)/camera_focal_length/2.l)
      ,std::atan(y(camera_sensor_size)/camera_focal_length/2.l)
    }};
    
    // カメラの水平垂直視野角[rad](表示用)
    a2d_t camera_fov_rad
    {{ x(camera_fov_div_2_rad) * 2.l
     , y(camera_fov_div_2_rad) * 2.l
    }};
    // カメラの水平垂直視野角[deg](表示用)
    a2d_t camera_fov
    {{ radians_to_degrees(x(camera_fov_rad))
     , radians_to_degrees(y(camera_fov_rad))
    }};
    
    std::cout << "camera fov diagonal         [deg]: " << camera_fov_diagonal     << "\n"
                 "                            [rad]: " << camera_fov_diagonal_rad << "\n"
                 "           horizontal       [deg]: " << x(camera_fov)       << "\n"
                 "                            [rad]: " << x(camera_fov_rad)   << "\n"
                 "           vertical         [deg]: " << y(camera_fov)       << "\n"
                 "                            [rad]: " << y(camera_fov_rad)   << "\n"
                 "       focal length          [mm]: " << camera_focal_length << "\n"
              ;
    
    // 視錐台のZ軸方向の変化量に対するX軸Y軸方向の変化量（広がり）
    a2d_t view_frustum_delta
    {{ std::tan(x(camera_fov_div_2_rad))
     , std::tan(y(camera_fov_div_2_rad))
    }};
    
    std::cout << "view frustum dx/dz           [mm]: " << x(view_frustum_delta) << "\n"
                 "             dy/dz           [mm]: " << y(view_frustum_delta) << "\n"
              ;
    
    // top-camのターゲットのピクセル値をsnorm値に
    //   ※snorm値: "signed normalized value"、日本語にすると符号付き正規化値
    //            : 具体的には [-1.0〜+1.0] の範囲に、ものの値を正規化（尺度を整える）した値
    //   ※unorm値: "unsigned normalized value"、日本語では符号無し正規化値
    //            : 具体的には [ 0.0〜+1.0] に範囲に、ものの値を正規化した値
    //            : 基本的にはsnormもunormも対象を最大値に対する比率で表す方法です。
    a2d_t top_image_target_snorm
    {{ uvalue_to_snorm(x(top_image_target), x(image_size))
     , uvalue_to_snorm(y(top_image_target), y(image_size))
    }};
    
    std::cout << "top image target x           [px]: " << x(top_image_target) << "\n"
                 "                 y           [px]: " << y(top_image_target) << "\n"
                 "                 snorm(x)     [-]: " << x(top_image_target_snorm) << "\n"
                 "                 snorm(y)     [-]: " << y(top_image_target_snorm) << "\n"
              ;
    // top-camのターゲットピクセルへの偏差角度
    //   ※top-camの視線中心からターゲットピクセルへは(左右,上下)に何度ずれた射線となるか
    a2d_t top_image_target_deviation_angle_rad
    {{ x(top_image_target_snorm) * x(camera_fov_div_2_rad)
     , y(top_image_target_snorm) * y(camera_fov_div_2_rad)
    }};
    
    std::cout << "top image target dx         [rad]: " << x(top_image_target_deviation_angle_rad) << "\n"
                 "                 dy         [rad]: " << y(top_image_target_deviation_angle_rad) << "\n"
              ;
    
    // top-camとそのターゲットピクセルを通る直線の傾き角度
    //   ※top-camはX軸回転(=「Z値に対する"Y値"の変動」)を持っている
    a2d_t top_image_target_line_angle
    {{ x(top_image_target_deviation_angle_rad)
     , y(top_image_target_deviation_angle_rad) + top_camera_angle_x_rad
    }};
    
    std::cout << "top image target line dx    [rad]: " << x(top_image_target_line_angle) << "\n"
                 "                      dy    [rad]: " << y(top_image_target_line_angle) << "\n"
              ;
    
    // YZ平面(真横から見た平面図)における
    // top-camとそのスクリーンの中心を通る直線の式 y = f(z)
    // 
    // YZ平面上の直線の方程式の傾きaと切片bによる標準形
    // y = a * z + b;
    //
    // a = dy/dz = tan(θ)
    //   θ = y(top_image_target_line_angle)
    const auto top_yz_a = std::tan(y(top_image_target_line_angle));
    // b = y - a * z
    //  この直線の確実な通過点であるtop-camの位置座標(x,y,z)を代入
    // b = y0 - a * z0
    //  y0 = y(top_camera_position)
    //  z0 = z(top_camera_position)
    const auto top_yz_b = y(top_camera_position) - top_yz_a * z(top_camera_position);
    
    std::cout << "top YZ-plane line function   [mm]: y = f(z) = " << top_yz_a << " z + " << top_yz_b << "\n";
    
    // front-camのターゲットのピクセル値をsnorm値に
    a2d_t front_image_target_snorm
    {{ uvalue_to_snorm(x(front_image_target), x(image_size))
     , uvalue_to_snorm(y(front_image_target), y(image_size))
    }};
    
    std::cout << "front image target x         [px]: " << x(front_image_target) << "\n"
                 "                   y         [px]: " << y(front_image_target) << "\n"
                 "                   snorm(x)   [-]: " << x(front_image_target_snorm) << "\n"
                 "                   snorm(y)   [-]: " << y(front_image_target_snorm) << "\n"
              ;
    // front-camのターゲットピクセルへの偏差角度
    //   ※front-camの視線中心からターゲットピクセルへは(左右,上下)に何度ずれた射線となるか
    a2d_t front_image_target_deviation_angle_rad
    {{ x(front_image_target_snorm) * x(camera_fov_div_2_rad)
     , y(front_image_target_snorm) * y(camera_fov_div_2_rad)
    }};
    
    std::cout << "front image target dx       [rad]: " << x(front_image_target_deviation_angle_rad) << "\n"
                 "                   dy       [rad]: " << y(front_image_target_deviation_angle_rad) << "\n"
              ;
    
    // top-camとそのターゲットピクセルを通る直線の傾き角度
    //   ※front-camはXYZの全ての軸回転量は0で真正面を向いている
    //     という前提なので偏差角度と等しい。
    a2d_t front_image_target_line_angle
    {{ x(front_image_target_deviation_angle_rad)
     , y(front_image_target_deviation_angle_rad)
    }};
    
    std::cout << "front image target line dx  [rad]: " << x(top_image_target_line_angle) << "\n"
                 "                        dy  [rad]: " << y(top_image_target_line_angle) << "\n"
              ;
    
    // YZ平面(真横から見た平面図)における
    // front-camとそのスクリーンの中心を通る直線の式 y = f(z)
    //   ※top-camと同様に傾き a と 切片 b を求める
    const auto front_yz_a = std::tan(y(front_image_target_line_angle));
    const auto front_yz_b = y(front_camera_position) - front_yz_a * z(front_camera_position);
    
    std::cout << "front YZ-plane line function [mm]: y = f(z) = " << front_yz_a << " z + " << front_yz_b << "\n";
    
    // YZ平面におけるtop-camのターゲット直線とfront-camのターゲット直線の交点を求める
    //   2つの直線の式からなる連立方程式をyについて解く
    //     y = top_yz_a   * z + top_yz_b    ... (1)
    //     y = front_yz_a * z + front_yz_b  ... (2)
    //  代入: (1):y <-- (2):y
    //     top_yz_a * z + top_yz_b = front_yz_a * z + front_yz_b
    //     top_yz_a * z - front_yz_a * z = front_yz_b - top_yz_b
    //     z (top_yz_a - front_yz_a) = front_yz_b - top_yz_b
    //     z = (front_yz_b - top_yz_b) / (top_yz_a - front_yz_a)  ... (3)
    //  代入: (1):z <-- (3):z
    //     y = top_yz_a * {(front_yz_b - top_yz_b) / (top_yz_a - front_yz_a)} + top_yz_b
    const auto cross_point_y = top_yz_a * ( (front_yz_b - top_yz_b) / (top_yz_a - front_yz_a) ) + top_yz_b;
    
    std::cout << "cross point y                [mm]: " << cross_point_y << "\n";
    
    // top-cam について x = f(y) と
    //                  z = f(y) の関数式を定義する
    // z = f(y)
    auto z_from_y = [&](const float_t y_value)
    {
      // y = a * z + b を z について変形して
      // z = (y - b) / a となるので、
      return (y_value - top_yz_b) / top_yz_a;
    };
    
    // x = f(y)
    // XY平面についてはまだ考えていないので、
    // YZ平面の時と同様にして新たに算出する。
    auto x_from_y = [&](const float_t y_value)
    {
      // 一次方程式の傾きaと切片bの標準形より
      // x = a * y + b
      //   a = dx/dy = tan(φ) ※φ（読み: ふぁい、θはさっきもう使ったから次の文字、というだけ）
      //   φ = x(top_image_target_line_angle)
      const auto top_xy_a = std::tan(x(top_image_target_line_angle));
      //   b = x - a * y
      //     top-cam の位置座標(x,y,z)より x と y を代入して
      //   b = x(top_camera_position) - top_xy_a * y(top_camera_position)
      const auto top_xy_b = x(top_camera_position) - top_xy_a * y(top_camera_position);
      // 以上、 x = f(y) における傾きaと切片bを求めたので x について解ける
      return top_xy_a * y_value + top_xy_b;
    };
    
    // 実空間の座標値(x,y,z)における y が定まり、
    // x = f(y)
    // z = f(y)
    // が与えられているので、
    // top-camとfront-camのターゲットピクセルの示す現実空間の完全な座標値が明らかとなる。
    const a3d_t cross_point
    {{ x_from_y(cross_point_y)
     , std::move(cross_point_y)
     , z_from_y(cross_point_y)
    }};
    
    std::cout << "cross point                  [mm]: (x,y,z) = ("
              << x(cross_point) << ", "
              << y(cross_point) << ", "
              << z(cross_point) << ")"
              << "\n";
  }

  boost::program_options::variables_map option(const int& ac, const char* const * const  av)
  {
    using namespace boost::program_options;
    std::vector<std::string> a;
    options_description description("オプション");
    description.add_options()
    ("top-camera-position,t"  , value<std::vector<float_t>>()->multitoken(), "top-camera position   (eg.: -t 0 207 264)")
    ("front-camera-position,f", value<std::vector<float_t>>()->multitoken(), "front-camera position (eg.: -f 0  37 350)")
    ("top-camera-angle-x,a"   , value<float_t>()->default_value(default_top_camera_angle_x), "top-camera angle x [deg]")
    ("camera-fov-diagonal,o"  , value<float_t>()->default_value(default_camera_fov_diagonal), "camera fov diagonal [deg]")
    ("camera-sensor-size,s"   , value<std::vector<float_t>>()->multitoken(), "camera sensor size   (eg.: -s 3.60 2.70)")
    ("image-size,i"           , value<std::vector<float_t>>()->multitoken(), "image size   (eg.: -i 640 480)")
    ("top-image-target,n"     , value<std::vector<float_t>>()->multitoken(), "top image target   (eg.: -n 320 240)")
    ("front-image-target,m"   , value<std::vector<float_t>>()->multitoken(), "front image target (eg.: -m 320 240)")
    ("help,h"  , "ヘルプ")
    ("version,v", "バージョン情報")
    ;
    
    variables_map vm;
    store(parse_command_line(ac, av, description), vm);
    notify(vm);
    
    if(vm.count("help"))
      std::cout << description << std::endl;
    if(vm.count("version"))
      std::cout << version_info << std::endl;
    
    return vm;
  }
}

int main(const int ac, const char* const * const av)
try
{
  auto vm = option(ac, av);
  if(!vm.count("help") && !vm.count("version"))
    run
    ( pov_to_array<a3d_t>( vm["top-camera-position"  ], default_top_camera_position )
    , pov_to_array<a3d_t>( vm["front-camera-position"], default_front_camera_position )
    , vm["top-camera-angle-x"].as<a3d_t::value_type>()
    , vm["camera-fov-diagonal"].as<a3d_t::value_type>()
    , pov_to_array<a2d_t>( vm["camera-sensor-size"], default_camera_sensor_size )
    , pov_to_array<a2d_t>( vm["image-size"], default_image_size )
    , pov_to_array<a2d_t>( vm["top-image-target"  ], default_top_image_target )
    , pov_to_array<a2d_t>( vm["front-image-target"], default_front_image_target )
    );
}
catch(const std::exception& e)
{ std::cerr << e.what() << "\n"; }