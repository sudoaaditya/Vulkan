cls
del MyWindow.exe
del MyWindow.obj
cl.exe /c /EHsc MyWindow.cpp
link.exe MyWindow.obj user32.lib gdi32.lib

