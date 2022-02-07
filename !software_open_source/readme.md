# Open source software for this controller

---

| Files                 | Info                                             |
| --------------------- | ------------------------------------------------ |
| chuniio               | chuniio.dll and chunitest.exe source in C/CMake  |
| chuniio.zip           | chuniio.dll and chunitest.exe binaries           |

chunitest.exe will load chuniio.dll to test the controller and thus can be used to test any chuniio.dll implementation for any other controller as well.

## Build instructions

Tested with MinGW

```
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
make
```

