@echo off
echo Compiling MD5.cpp...
g++ -Wall -Wextra -Werror MD5.cpp ../io.cpp -o MD5
if %errorlevel% neq 0 (
    echo Compilation failed.
    exit /b %errorlevel%
)
echo Compilation successful.
echo Running...
MD5 --messageFile="message.txt" --outputFile="output.txt"