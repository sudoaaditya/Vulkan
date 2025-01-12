del *.exe
del *.obj
del *.txt

cls

cl.exe /c /EHsc MyWindow.c

rc.exe resource.rc

link.exe resource.res user32.lib gdi32.lib /SUBSYSTEM:WINDOWS MyWindow.obj /out:"./OGL.exe"

OGL.exe