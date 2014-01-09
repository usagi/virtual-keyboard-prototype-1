# virtual-keyboard-prototype-1

仮想キーボード試作1型

# memo

## build方法

### 依存ライブラリー(git submodule)の入手

リポジトリー入手時の初回は `submodule init`を行って下さい。

    git submodule init

依存ライブラリーの更新

    git submodule update

### 本サンプルのビルド

    mkdir build
    cd build
    cmake -G Ninja ..
    ninja

などして下さい。

なお、clang++の場合には3.3以上、
g++の場合には4.7以上のバージョンを必要とします。

### 依存性

- cmake
    - http://www.cmake.org/
- Ninja, GNU make など cmake の対応するビルドツール
    - http://martine.github.io/ninja/
    - http://www.gnu.org/software/make/
- g++-4.7, clang++-3.3 などの C++11 に対応したコンパイラー
    - http://gcc.gnu.org/
    - http://clang.llvm.org/

## raspbianにおける必要パッケージ等の導入方法

    apt-get install \
      cmake \
      g++-4.7 \
      libboost-all-dev \
      libopencv-dev \
      sqlite3 \
      libsqlite3-dev \

ほかにraspbianにパッケージが無いため手作業でインストールが必要なものとして、

- Ninja

以上があります。

また、 raspbian では clang++-3.3 を執筆現在まだリポジトリーから入手できないので
g++-4.7を用意し、cmake時に-DCMAKE_CXX_COMPILER=g++-4.7、
またはalternativesを設定してあれば単に-DCMAKE_CXX_COMPILER=g++を指示する必要があります。

## サンプルアプリの共通事項

- 実行時オプション
    - -h : ヘルプを表示
    - -v : バージョン情報を表示
    - オプション無しで実行 : -h で確認できるデフォルトの値で実行

## virtual-keyboard.sqlite3 データベースについて

- virtual-keyboard.xlsx
    - データ編集用マスター
    - このファイルからの自動ビルドは行われない
    - 変更を加えた場合は.csvを出力する
- virtual-keyboard.csv
    - ビルドシステムによりvirtual-keyboard.sqlite3を生成するソースとなる
- virtual-keyboard-data.csv
    - ビルド途中でsqlite3にロードさせる為に元の.csvから必要なデータ部分だけを残して削ぎ落とした中間ファイル
    - build/CMakeFiles/virtual-keyboard-tester.dir/virtual-keyboard-data.csvとして生成される
- virtual-keyboard.sqlite3
    - アプリケーションで使用する為のsqlite3データベース
    - ビルドシステムによりvirtual-keyboard.csvから自動的に生成される
    - testテーブルにキーボードデータが収められる
    - 将来的には複数のキーボードデータを収めて使う

### tips: Key Usage ID

- USB HID Usage Tables
    - http://www.usb.org/developers/devclass_docs/Hut1_12v2.pdf
- USBキーボードのキーコード
    - http://www2d.biglobe.ne.jp/~msyk/keyboard/layout/usbkeycode.html

## virtual-keyboard-tester

仮想キーボードに対して指位置の入力（X座標、Y座標、S座標:ストローク深さ座標）を与える事で
仮想キーボードデータベースにあるtestキーボードテーブルから、
キーのヒット状態（＋人間が理解可能な文字列によるヒットしたキーの名称）を表示するサンプル。

## virtual-keyboard-viewer

仮想キーボードデータベースからキー情報を読み込み可視化するサンプル。

## keyboard-writer

システム（OS）に対して仮想キーボードデバイスを生成し、任意のキー入力をソフトウェアエミュレーションにより
システムへ送出するサンプル。

Linux（UNIX、ネイティブPOSIX系）と、その他のシステム（Windows、OSXも？）では
異なる方法でシステムにキー入力を与える必要がある。
現在はとりあえずLinux（Linux Mint 15 KDEで動作確認）のみに対応。

## sender/reciever

UDP/IPによりデータを送信するサンプル、受信するサンプル。

## camera-preview

システムに接続された任意台数のカメラを制御するサンプル。撮影機能付き。

## pixel-to-realspace

X軸回転を許容するtop-camと軸回転を許容しないfront-camの2台の映像における
ターゲットピクセルから実空間における3次元座標を推定するサンプル。

## finger-position

画像から手指と思しきピクセル座標を検出するサンプル。
