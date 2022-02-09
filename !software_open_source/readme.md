# Open source software for this controller

## [slidershim](https://github.com/4yn/slidershim) by [4yn](https://github.com/4yn)

Software for interfacing with the controller , output keyboard presses and controls LED lights

---

## ᴄʜᴜɴɪɪᴏ.ᴅʟʟ (By [CrazyRedMachine](https://github.com/CrazyRedMachine))

| Files       | Info                                            |
| ----------- | ----------------------------------------------- |
| ᴄʜᴜɴɪɪᴏ     | ᴄʜᴜɴɪɪᴏ.ᴅʟʟ and ᴄʜᴜɴɪᴛᴇsᴛ.ᴇxᴇ source in C/CMake |
| ᴄʜᴜɴɪɪᴏ.ᴢɪᴘ | ᴄʜᴜɴɪɪᴏ.ᴅʟʟ and ᴄʜᴜɴɪᴛᴇsᴛ.ᴇxᴇ binaries          |

ᴄʜᴜɴɪᴛᴇsᴛ.ᴇxᴇ will load ᴄʜᴜɴɪɪᴏ.ᴅʟʟ to test the controller and thus can be used to test any ᴄʜᴜɴɪɪᴏ.ᴅʟʟ implementation for any other controller as well.

### Build instructions

Tested with MinGW

```
mkdir build
cd build
cmake .. -G "MinGW Makefiles"
make
```
