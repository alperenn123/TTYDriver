#!/bin/sh
color_i="printf $(tput setaf 2)"
color_e="printf $(tput setaf 1)"
color_w="printf $(tput setaf 3)"
current_dir=$(pwd)
CMAKE_FILE=CMakeLists.txt
MAKE_FILE=Makefile
cd $current_dir/test_app

if test -f "$CMAKE_FILE"; then
    $color_i
    cmake .
    make
else
    $color_e
    echo "$CMAKE_FILE does not exist."
fi
cd ../tty_driver
if test -f "$MAKE_FILE"; then
    $color_w
    make
else
    $color_e
    echo "$MAKE_FILE does not exist."
fi

