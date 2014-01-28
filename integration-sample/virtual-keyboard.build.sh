#!/bin/sh

echo "  [build: virtual-keyboard.sqlite3] begin"
echo "    * source-dir: $1"
echo "    * build-dir : $2"
echo "    * target-dir: $3"

if [ `uname -s` = 'Darwin' ]; then
  echo "    * OS        : Darwin"
  sed_option_ex='-E';
else
  echo "    * OS        : GNU/Linux or other"
  sed_option_ex='-r';
fi

echo "    * sed option: ${sed_option_ex}"

if test -f virtual-keyboard.sqlite3
then
  rm -v virtual-keyboard.sqlite3
fi

echo "    generate sqlite3 importable data: $2/virtual-keyboard-data.csv"

tail -n +2 $1/virtual-keyboard.csv | sed $sed_option_ex "s/^(([^,]+,){5}[^,]+),.*/\\1/g" > $2/virtual-keyboard-data.csv

echo "    create table: $3/virtual-keyboard.sqlite3"
sqlite3 $3/virtual-keyboard.sqlite3 "create table test(x real,y real,w real,h real,s real,id integer)" || exit 1
echo "    import data : $3/virtual-keyboard.sqlite3"
sqlite3 -separator , $3/virtual-keyboard.sqlite3 ".import $2/virtual-keyboard-data.csv test" || exit 2

echo "  build succeeded"
