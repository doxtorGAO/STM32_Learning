@echo off
chcp 936 >nul
cd /d "%~dp0"
echo.
echo   正在恢复参考答案版 ...
echo.
if not exist "_答案备份\Encoder.c" (
  echo   [错误] 找不到 _答案备份 文件夹。
  echo.
  pause
  exit /b 1
)
if not exist "_我的答案存档" mkdir "_我的答案存档"
copy /Y "Hardware\Encoder.c" "_我的答案存档\Encoder.c" >nul
copy /Y "Hardware\Encoder.h" "_我的答案存档\Encoder.h" >nul
copy /Y "User\main.c"            "_我的答案存档\main.c"        >nul
echo   你当前写的代码已自动存档到 _我的答案存档 文件夹，不会白写。
echo.
copy /Y "_答案备份\Encoder.c" "Hardware\Encoder.c" >nul
copy /Y "_答案备份\Encoder.h" "Hardware\Encoder.h" >nul
copy /Y "_答案备份\main.c"        "User\main.c"            >nul
echo   已恢复参考答案，可以在 Keil 里编译烧录了。
echo.
pause