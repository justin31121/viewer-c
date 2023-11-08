mkdir bin 2> NUL
cl /W4 /Fe:bin\viewer src\main.c gdi32.lib user32.lib opengl32.lib shell32.lib
