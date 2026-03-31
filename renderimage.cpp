#include "renderimage.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include "ryi.h"


std::vector<RenderImage> RenderImage::load_images_from_dir(const char* path){
    std::vector<RenderImage> images;
    DIR* dir = opendir(path);
    if (dir == NULL)
        return images;
    dirent* next_dir = readdir(dir);;
    while (next_dir != NULL) {
        char* file_name = next_dir->d_name;
        char* extension = strchr(file_name, '.');
        bool isValid = Ryi::is_image_supported(extension);
        if (next_dir->d_type == DT_REG && extension != NULL && isValid) {
            const char* image_path = TextFormat("%s/%s", path, file_name);
            auto image = LoadTexture(image_path);
            images.push_back((RenderImage){.path = strdup(image_path), .image = image});
        }
        next_dir = readdir(dir);;
    }


    return images;
}
