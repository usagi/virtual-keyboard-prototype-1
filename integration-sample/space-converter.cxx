#include "space-converter.hxx"

namespace arisin
{
  namespace etupirka
  {
    space_converter_t::space_converter_t(const configuration_t& conf)
      : top_camera_position_(conf.space_converter.top_camera_position)
      , front_camera_position_(conf.space_converter.front_camera_position)
      , top_camera_angle_x_(conf.space_converter.top_camera_angle_x)
      , camera_fov_diagonal_(conf.space_converter.camera_fov_diagonal)
      , camera_sensor_size_(conf.space_converter.camera_sensor_size)
      , image_size_(conf.space_converter.image_size)
    {
      DLOG(INFO) << "top_camera_positon[mm]: "    << to_string(top_camera_position_);
      DLOG(INFO) << "front_camera_position[mm]: " << to_string(front_camera_position_);
      DLOG(INFO) << "top_camera_angle_x[deg]: "    << top_camera_angle_x_;
      DLOG(INFO) << "camera_fov_diagonal[deg]: "   << camera_fov_diagonal_;
      DLOG(INFO) << "camera_sensor_size[mm]: "    << to_string(camera_sensor_size_);
      DLOG(INFO) << "image_size[mm]: "            << to_string(image_size_);
      
      initialize();
    }
    
    void space_converter_t::initialize()
    {
      // std::tanなどの三角関数はラジアン単位の仕様なので弧度法単位から変換しておく
      // カメラのX軸回転角度[rad]
      auto top_camera_angle_x_rad = degrees_to_radians(top_camera_angle_x_);
      top_camera_angle_x_rad_ = top_camera_angle_x_rad;
      DLOG(INFO) << "top_camera_angle_x_rad[rad]: " << top_camera_angle_x_rad;
      
      // カメラのセンサーの対角線の長さ
      auto camera_sensor_diagonal = diagonal(x(camera_sensor_size_), y(camera_sensor_size_));
      DLOG(INFO) << "camera_sensor_diagonal[mm]: " << camera_sensor_diagonal ;
      
      // カメラの対角視野角[rad]
      auto camera_fov_diagonal_rad = degrees_to_radians(camera_fov_diagonal_);
      DLOG(INFO) << "camera_fov_diagonal_rad[rad]:" << camera_fov_diagonal_rad;
      
      // カメラの1/2対角視野角[rad]
      auto camera_fov_diagonal_div_2_rad = camera_fov_diagonal_rad / 2.l;
      DLOG(INFO) << "camera_fov_diagonal_div_2_rad[rad]: " << camera_fov_diagonal_div_2_rad;
      
      // カメラの焦点距離
      auto camera_focal_length = camera_sensor_diagonal / std::tan(camera_fov_diagonal_div_2_rad) / 2.l;
      DLOG(INFO) << "camera_focal_length[mm]: " << camera_focal_length;
      
      // カメラの1/2水平垂直視野角[rad]
      a2d_t camera_fov_div_2_rad
      {{ float_t( std::atan(x(camera_sensor_size_) / camera_focal_length / 2 ) )
       , float_t( std::atan(y(camera_sensor_size_) / camera_focal_length / 2 ) )
      }};
      camera_fov_div_2_rad_ = camera_fov_div_2_rad;
      DLOG(INFO) << "camera_fov_div_2_rad[rad]: " << to_string(camera_fov_div_2_rad);
      
#ifndef NDEBUG
      // カメラの水平垂直視野角[rad](表示用)
      a2d_t camera_fov_rad
      {{ x(camera_fov_div_2_rad) * 2
       , y(camera_fov_div_2_rad) * 2
      }};
      DLOG(INFO) << "camera_fov_rad[rad]: " << to_string(camera_fov_rad);
      
      // カメラの水平垂直視野角[deg](表示用)
      a2d_t camera_fov
      {{ radians_to_degrees(x(camera_fov_rad))
      , radians_to_degrees(y(camera_fov_rad))
      }};
      DLOG(INFO) << "camera_fov[deg]: " << to_string(camera_fov);
#endif
      
    }
    
    space_converter_t::a3d_t space_converter_t::operator()
    ( const a2d_t& top_image_target
    , const a2d_t& front_image_target
    ) const
    {
      DLOG(INFO) << "top-image-target: "   << to_string(top_image_target);
      DLOG(INFO) << "front-image-target: " << to_string(front_image_target);
      
      // top-camのターゲットのピクセル値をsnorm値に
      //   ※snorm値: "signed normalized value"、日本語にすると符号付き正規化値
      //            : 具体的には [-1.0〜+1.0] の範囲に、ものの値を正規化（尺度を整える）した値
      //   ※unorm値: "unsigned normalized value"、日本語では符号無し正規化値
      //            : 具体的には [ 0.0〜+1.0] に範囲に、ものの値を正規化した値
      //            : 基本的にはsnormもunormも対象を最大値に対する比率で表す方法です。
      a2d_t top_image_target_snorm
      {{ uvalue_to_snorm(x(top_image_target), x(image_size_))
       , uvalue_to_snorm(y(top_image_target), y(image_size_))
      }};
      DLOG(INFO) << "top_image_target_snorm[-]: " << to_string(top_image_target_snorm);
      
      // top-camのターゲットピクセルへの偏差角度
      //   ※top-camの視線中心からターゲットピクセルへは(左右,上下)に何度ずれた射線となるか
      //   ※要注意として、ここで、カメラのスクリーンに映ったイメージは
      //     カメラがZ軸-を向いて撮影したものだから、
      //     カメラスクリーンのX軸+方向は現実世界のX軸+方向と逆転しています。
      //     なので、このカメラのスクリーン座標系から現実世界への座標系への変換に伴い、
      //     X軸の符号反転が発生します。
      a2d_t top_image_target_deviation_angle_rad
      {{ -x(top_image_target_snorm) * x(camera_fov_div_2_rad_)
       ,  y(top_image_target_snorm) * y(camera_fov_div_2_rad_)
      }};
      DLOG(INFO) << "top_image_target_deviation_angle_rad[rad]: " << to_string(top_image_target_deviation_angle_rad);
      
      // top-camとそのターゲットピクセルを通る直線の傾き角度
      //   ※top-camはX軸回転(=「Z値に対する"Y値"の変動」)を持っている
      a2d_t top_image_target_line_angle
      {{ x(top_image_target_deviation_angle_rad)
       , y(top_image_target_deviation_angle_rad) + top_camera_angle_x_rad_
      }};
      DLOG(INFO) << "top_image_target_line_angle[rad]: " << to_string(top_image_target_line_angle);
      
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
      const auto top_yz_b = y(top_camera_position_) - top_yz_a * z(top_camera_position_);
      DLOG(INFO) << "top YZ-plane line function [mm]: y = f(z) = " << top_yz_a << " z + " << top_yz_b;
      
      // front-camのターゲットのピクセル値をsnorm値に
      a2d_t front_image_target_snorm
      {{ uvalue_to_snorm(x(front_image_target), x(image_size_))
       , uvalue_to_snorm(y(front_image_target), y(image_size_))
      }};
      DLOG(INFO) << "front_image_target_snorm [-]: " << to_string(front_image_target_snorm);
      
      // front-camのターゲットピクセルへの偏差角度
      //   ※front-camの視線中心からターゲットピクセルへは(左右,上下)に何度ずれた射線となるか
      a2d_t front_image_target_deviation_angle_rad
      {{ x(front_image_target_snorm) * x(camera_fov_div_2_rad_)
       , y(front_image_target_snorm) * y(camera_fov_div_2_rad_)
      }};
      DLOG(INFO) << "front_image_target [rad]: " << to_string(front_image_target_deviation_angle_rad);
      
      // top-camとそのターゲットピクセルを通る直線の傾き角度
      //   ※front-camはXYZの全ての軸回転量は0で真正面を向いている
      //     という前提なので偏差角度と等しい。
      a2d_t front_image_target_line_angle
      {{ x(front_image_target_deviation_angle_rad)
       , y(front_image_target_deviation_angle_rad)
      }};
      DLOG(INFO) << "front_image_target_line_angle [rad]: " << to_string(top_image_target_line_angle);
      
      // YZ平面(真横から見た平面図)における
      // front-camとそのスクリーンの中心を通る直線の式 y = f(z)
      //   ※top-camと同様に傾き a と 切片 b を求める
      const auto front_yz_a = std::tan(y(front_image_target_line_angle));
      const auto front_yz_b = y(front_camera_position_) - front_yz_a * z(front_camera_position_);
      DLOG(INFO) << "front YZ-plane line function [mm]: y = f(z) = " << front_yz_a << " z + " << front_yz_b;
      
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
      DLOG(INFO) << "cross point y [mm]: " << cross_point_y;
      
      // top-cam について x = f(y) と
      //                  z = f(y) の関数式を定義する
      // z = f(y)
      auto z_from_y = [&](const float_t y_value)
      {
        // y = a * z + b を z について変形して
        // z = (y - b) / a となるので、
        return (y_value - top_yz_b) / top_yz_a;
      };
      DLOG(INFO) << "z_from_y prepared";
      
      // x = f(y)
      // XZ平面についてはまだ考えていないので、
      // YZ平面の時と同様にして新たに算出する。
      auto x_from_z = [&](const float_t z_value)
      {
        // 一次方程式の傾きaと切片bの標準形より
        // x = a * z + b
        //   a = dx/dy = tan(φ) ※φ（読み: ふぁい、θはさっきもう使ったから次の文字、というだけ）
        //   φ = x(top_image_target_line_angle)
        const auto top_xz_a = std::tan(x(top_image_target_line_angle));
        //   b = x - a * z
        //     top-cam の位置座標(x,y,z)より x と z を代入して
        //   b = x(top_camera_position) - top_xy_a * z(top_camera_position)
        const auto top_xz_b = x(top_camera_position_) - top_xz_a * z(top_camera_position_);
        // 以上、 x = f(z) における傾きaと切片bを求めたので x について解ける
        DLOG(INFO) << "top XZ-plane line function   [mm]: x = f(y) = " << top_xz_a << " * z + " << top_xz_b;
        return top_xz_a * z_value + top_xz_b;
      };
      DLOG(INFO) << "x_from_z prepared";
      
      // 実空間の座標値(x,y,z)における y が定まり、
      // x = f(y)
      // z = f(y)
      // が与えられているので、
      // top-camとfront-camのターゲットピクセルの示す現実空間の完全な座標値が明らかとなる。
      const a3d_t cross_point
      {{ x_from_z(z_from_y(cross_point_y))
      , cross_point_y
      , z_from_y(cross_point_y)
      }};
      DLOG(INFO) << "cross_point [mm]: " << to_string(cross_point);
      
      return std::move(cross_point);
    }
    
  }
}