/*
 * Ryi Image Viewer
 *
 * Author: Gama Sibusiso
 * Date: 02-March-2026
 *
 */

#ifndef RENDERIMAGE_H
#define RENDERIMAGE_H

#include <vector>
#include <raylib.h>

/*
 * RenderImage Context struct.
 * Contains a Raylib Texture2D and the path it was loaded from.
 * Useful for displaying information about images at runtime.
 * If more info needs to be shared, this is the struct to modify
 */
struct RenderImage {
    char* path;
    Texture2D image;

    static std::vector<RenderImage> load_images_from_dir(const char*);
};

#endif // RENDERIMAGE_H
