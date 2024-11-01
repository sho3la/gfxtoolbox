@echo off
mkdir build
cd build
call D:\emsdk\emsdk_env.bat
call emcmake cmake ..
call emmake make