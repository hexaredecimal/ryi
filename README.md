# RYI - Render Yet another Image
> A small image viewer inspired by feh

<div align="center">


  [![Build Ryi with nob](https://github.com/hexaredecimal/ryi/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/hexaredecimal/ryi/actions/workflows/c-cpp.yml)



</div>

### Features
- [x] Grid gallery
- [x] Image view
- [x] Open file dialog
- [x] Directory traversal using `opeddir`
- [x] Loading image from url
- [x] Popup menu
- [ ] Configuration
- [x] Zoom (in/out and reset)
- [x] Image Rotation


### Build
#### Requirements
- gcc/clang
- nob.h (included)
- libcurl
- raylib

#### Build Process
- Clone the repo:
```sh
git clone https://github.com/hexaredecimal/ryi.git
```

- Build nob
```sh
cd ryi
cc nob.c -o nob
```

- Run nob to build the final executable
```sh
./nob && ./ryi
```

### Contributing
All contibutions are welcome, just don't modify files to put your name etc. Your PRs should actually do work. 


