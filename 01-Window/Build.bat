del *.exe
del *.obj
del *.txt

cls

cl.exe /c /EHsc vk.c

rc.exe vk.rc

link.exe vk.res user32.lib gdi32.lib /SUBSYSTEM:WINDOWS vk.obj /out:"./VK.exe"

VK.exe