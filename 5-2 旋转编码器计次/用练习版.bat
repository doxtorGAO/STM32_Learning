@echo off
chcp 936 >nul
cd /d "%~dp0"
echo.
echo   正在切换到练习版 - 空白骨架 ...
echo.
if not exist "_答案备份\Encoder.c" (
  echo   [错误] 找不到 _答案备份 文件夹，已中止。
  echo   请确认参考答案还在，再重试。
  echo.
  pause
  exit /b 1
)
copy /Y "练习骨架\Encoder.c" "Hardware\Encoder.c" >nul
copy /Y "练习骨架\Encoder.h" "Hardware\Encoder.h" >nul
copy /Y "练习骨架\main.c"        "User\main.c"            >nul
echo   完成。现在打开 Keil 工程，就能看到需要填空的 TODO 了。
echo.
echo   参考答案安全保存在 _答案备份 文件夹里，
echo   随时可以双击 用答案版.bat 恢复。
echo.
pause