@echo off
chcp 936 >nul
cd /d "%~dp0"
echo.
echo   正在恢复【答案版】—— 能跑的完整实现 ...
echo.
if not exist "_答案备份\Timer.c" (
  echo   [错误] 找不到 _答案备份 文件夹。
  echo.
  pause
  exit /b 1
)
if not exist "_我的答案存档" mkdir "_我的答案存档"
copy /Y "Hardware\Timer.c" "_我的答案存档\Timer.c" >nul
copy /Y "Hardware\Key.c"   "_我的答案存档\Key.c"   >nul
copy /Y "User\main.c"       "_我的答案存档\main.c"   >nul
echo   你当前写的代码已自动存档到 _我的答案存档，没覆盖掉。
echo.
copy /Y "_答案备份\Timer.c" "Hardware\Timer.c" >nul
copy /Y "_答案备份\Key.c"   "Hardware\Key.c"   >nul
copy /Y "_答案备份\main.c"  "User\main.c"      >nul
echo   已恢复参考答案，回 Keil 重新编译下载就行。
echo.
pause
