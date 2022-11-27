# sdp1
A food ordering system based on GTK4
## Dependencies
* GTK-4
* GCC
## Build instructions
**Linux:**
```
cd src/
gcc $(pkg-config --cflags gtk4) -o food_order main.c $(pkg-config --libs gtk4)
```
**Windows:**
Follow [GTK docs](https://www.gtk.org/docs/installations/windows) for blinding in windows.
