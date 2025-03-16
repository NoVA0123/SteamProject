@echo off

mkdir ..\build
pushd ..\build
dir
:: clang++ -g -gcodeview -O2 ..\code\base.cpp ..\code\csv_parser.cpp ..\code\platform_metrics.cpp -o base.exe
:: clang++ -g -gcodeview -O2 -mavx2 ..\code\test.cpp ..\code\csv_parser.cpp ..\code\platform_metrics.cpp ..\code\repetition_tester.cpp ..\code\combiner.cpp -o test.exe
:: clang++ -g -gcodeview -O2 -mavx2 ..\code\test_fread.cpp ..\code\csv_parser.cpp ..\code\platform_metrics.cpp ..\code\repetition_tester.cpp ..\code\combiner.cpp -o new_test.exe
:: clang++ -g -gcodeview -O2 -mavx2 ..\code\my_windows_multithread.cpp -o new_test.exe
:: clang++ -g -gcodeview -O2 -mavx2 ..\code\page_fault_display.cpp -o pointer_anatomy.exe
:: clang++ -g -gcodeview -O2 -mavx2 ..\code\asm_test.cpp ..\code\repetition_tester.cpp ..\code\asm_lib\test_asm.lib ..\code\asm_lib\nop_test.lib ..\code\asm_lib\nop_align.lib ..\code\asm_lib\mov_test.lib ..\code\asm_lib\simd_test.lib -o asm_test.exe
:: clang++ -g -gcodeview -O2 -mavx2 ..\code\windows_multithread_test.cpp ..\code\repetition_tester.cpp -o new_test.exe
clang++ -g -gcodeview -O2 -mavx2 ..\code\gpt_test.cpp ..\code\csv_parser.cpp ..\code\platform_metrics.cpp ..\code\repetition_tester.cpp -o gpt_test.exe

popd
