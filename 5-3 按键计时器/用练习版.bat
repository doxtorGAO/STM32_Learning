@echo off
chcp 936 >nul
cd /d "%~dp0"
echo.
echo   正在切换到【练习版】—— 空白骨架 ...
echo.
if not exist "_答案备份\Timer.c" (
  echo   [错误] 找不到 _答案备份 文件夹，已中止。
  echo   先确认参考代码还在，再试一次。
  echo.
  pause
  exit /b 1
)
if not exist "_我的答案存档" mkdir "_我的答案存档"
echo   先把你现在工程里的代码存档 ...
copy /Y "Hardware\Timer.c" "_我的答案存档\Timer.c" >nul
copy /Y "Hardware\Key.c"   "_我的答案存档\Key.c"   >nul
copy /Y "User\main.c"       "_我的答案存档\main.c"   >nul
echo.
copy /Y "练习骨架\Timer.c" "Hardware\Timer.c" >nul
copy /Y "练习骨架\Key.c"   "Hardware\Key.c"   >nul
copy /Y "练习骨架\main.c"  "User\main.c"      >nul
echo   完成。现在打开 Keil 编译，会看到 8 个 TODO。
echo.
echo   按 练习说明.md 里的顺序，一次只填一块，
echo   每填一块编译一次、烧录一次。
echo.
echo   你原来写的代码存在 _我的答案存档 里，没丢。
echo   卡住了随时双击【用答案版.bat】恢复成能跑的版本。
echo.
pause
