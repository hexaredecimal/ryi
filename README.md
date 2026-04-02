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

### Screenshots
- A few screenshots
<table>
  <tr>
    <th>Description</th>
    <th>Screenshot</th>
  </tr>
    
  <tr>
    <td>Basic Image view</td>
    <td><img width="613" height="421" alt="image" src="https://github.com/user-attachments/assets/a5806830-c390-4f65-88cd-a8bca93e1e48" /></td>
  </tr>
  <tr>
    <td>Gallery view</td>
    <td><img width="601" height="419" alt="image" src="https://github.com/user-attachments/assets/fb7b4e87-4101-40c8-9739-edd143431119" /></td>
  </tr>
    <tr>
    <td>Open Directory</td>
    <td><img width="798" height="522" alt="image" src="https://github.com/user-attachments/assets/02182956-0c28-48ea-9eb6-324f53fee2b4" /></td>
  </tr>

 


</table>


### Contributing
All contibutions are welcome, just don't modify files to put your name etc. Your PRs should actually do work. 


