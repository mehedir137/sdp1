# sdp1

A food ordering system based on GTK4

## Dependencies

* GTK4
* GCC

### Dependencies Package Name

| Distribution  | GTK4           | GCC |
|---------------|----------------|-----|
| Arch          | gtk4           | gcc
| Debian/Ubuntu | libgtk\-4\-1   |build-essential


## Build instructions

**Linux:**
```bash
cd src/
gcc $(pkg-config --cflags gtk4) -o food_order main.c $(pkg-config --libs gtk4)
```

**Windows:**
Please follow [GTK docs](https://www.gtk.org/docs/installations/windows) for bilding in windows.
