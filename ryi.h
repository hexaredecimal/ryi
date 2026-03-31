/*
 * Ryi Image Viewer
 *
 * Author: Gama Sibusiso
 * Date: 02-March-2026
 *
 */


#ifndef RYI_H
#define RYI_H

#include <raylib.h>
#include "imagemode.h"
#include "renderimage.h"
#include "errorview.h"
/*
 * Ryi struct
 * Holds all important app routines, including the render logic for different screens.
 */
struct Ryi {
public:
    static void init(char *flag);
    static void draw_background();
    static bool is_image_supported(char*);
    static Rectangle get_dest_rect(ImageMode, float);
    static void open_app_from_url(char*);
    static void load_images(char*);
    static std::vector<RenderImage> images();

    static void draw_about();
    static void draw_grid_view();
    static void draw_image_slide();

    static int image_index;
    static bool grid_view;
    static bool is_running;
    static bool show_about;
    static float scale_factor;
    static float rotation;
    static Rectangle dialog_rect;
    static ImageMode image_mode;
    static int scroll_y;
    static ErrorView debug;
private:
    static std::vector<RenderImage> _images;
};
#endif // RYI_H
