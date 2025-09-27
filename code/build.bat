@echo off

mkdir ..\build
pushd ..\build
dir


:: Compiler and linker flags ::
::-Wall -fuse-ld=lld 
set CFLAGS=-g -gcodeview -O0 -fuse-ld=lld -march=native -Wno-deprecated-declarations
set LFLAGS=-Wl,/DEBUG
set COPTM=-g -gcodeview -O1 -fuse-ld=lld -march=native -Wno-deprecated-declarations -Wno-writable-strings

:: cloc
set HEADER= \code\typedef.h \code\csv_parser.h \code\repetition_tester.h \code\combiner.h \code\data_cleaning.h \code\BPE.h \code\cosine_similarity.h 
set CPP= \code\csv_parser.cpp \code\repetition_tester.cpp \code\combiner.cpp \code\data_cleaning.cpp \code\BPE.cpp \code\cosine_similarity.cpp \code\test.cpp \code\platform_metrics.cpp 
:: clang++ %COPTM% ..\code\cloc.cpp -o cloc.exe
cloc.exe %HEADER% %CPP%

:: Source and out files
set SOURCES=..\code\test.cpp ..\code\csv_parser.cpp ..\code\platform_metrics.cpp ..\code\repetition_tester.cpp ..\code\combiner.cpp ..\code\data_cleaning.cpp ..\code\BPE.cpp ..\code\cosine_similarity.cpp 
set OUT=test.exe

:: Compile main file
echo Compiling the files...
clang++ %COPTM% %SOURCES% -o %OUT%


:: Compile debug file
:: echo Compiling Debug...
:: clang++ %CFLAGS% %SOURCES% -o %OUT%


:: Other files
:: clang++ -g -gcodeview -O2 ..\code\base.cpp ..\code\csv_parser.cpp ..\code\platform_metrics.cpp ..\code\BPE.cpp -o base.exe
:: clang++ -g -gcodeview -O2 -march=native ..\code\test.cpp ..\code\csv_parser.cpp ..\code\platform_metrics.cpp ..\code\repetition_tester.cpp ..\code\combiner.cpp ..\code\data_cleaning.cpp ..\code\BPE.cpp ..\code\cosine_similarity.cpp -o test.exe
:: clang++ -g -gcodeview -O2 -mavx2 ..\code\test_fread.cpp ..\code\csv_parser.cpp ..\code\platform_metrics.cpp ..\code\repetition_tester.cpp ..\code\combiner.cpp -o new_test.exe
:: clang++ -g -gcodeview -O2 -mavx2 ..\code\my_windows_multithread.cpp -o new_test.exe
:: clang++ -g -gcodeview -O2 -mavx2 ..\code\page_fault_display.cpp -o pointer_anatomy.exe
:: clang++ -g -gcodeview -O2 -march=native ..\code\asm_test.cpp ..\code\repetition_tester.cpp ..\code\asm_lib\test_asm.lib ..\code\asm_lib\nop_test.lib ..\code\asm_lib\nop_align.lib ..\code\asm_lib\mov_test.lib ..\code\asm_lib\simd_test2.lib -o asm_test.exe
:: clang++ -g -gcodeview -O2 -mavx2 ..\code\windows_multithread_test.cpp ..\code\repetition_tester.cpp -o new_test.exe
:: clang++ -g -gcodeview -O2 -mavx2 ..\code\gpt_test.cpp ..\code\csv_parser.cpp ..\code\platform_metrics.cpp ..\code\repetition_tester.cpp -o gpt_test.exe
:: clang++ -g -gcodeview -O2 -mavx2 ..\code\BPE.cpp ..\code\platform_metrics.cpp ..\code\repetition_tester.cpp -o simd_test.exe

popd
