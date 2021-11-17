all: udp.dll

udp.dll: dllmain.cpp dll.h
	i686-w64-mingw32-gcc.exe -shared -static-libgcc -static-libstdc++  -Wl,--add-stdcall-alias dllmain.cpp -o udp.dll -msse3 -lws2_32 -lgdi32

.PHONY: clean

clean:
	rm -f udp.dll
