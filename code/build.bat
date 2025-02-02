@echo off

mkdir ..\build
pushd ..\build
dir
:: clang++ -g -gcodeview -O2 ..\code\base.cpp ..\code\csv_parser.cpp ..\code\platform_metrics.cpp -o base.exe
clang++ -g -gcodeview -O2 -mavx2 ..\code\test.cpp ..\code\csv_parser.cpp ..\code\platform_metrics.cpp -o test.exe
:: clang++ -g -gcodeview -O2 -mavx2 ..\code\my_windows_multithread.cpp -o test.exe
popd

