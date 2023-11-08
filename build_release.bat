cl /O2 /Fe:bin\viewer src\main.c /link /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup gdi32.lib user32.lib opengl32.lib shell32.lib
