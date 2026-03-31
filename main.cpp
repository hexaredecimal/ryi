

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ryi.h"
#include "button.h"
#include "popupmenu.h"

#include "tinyfiledialogs.h"
#include "build.h"

void print_usage() {
    printf("ryi [options]\n");
    printf("options:\n");
    printf("\t<dir>       \t- The directory to load images from (Optional)\n");
    printf("\t-h          \t- Print this help infomation\n");
    printf("\n");
    printf("examples:\n");
    printf("\tryi          \t- Running the executable without any args in a folder with images will cause the it to read all images\n");
    printf("\tryi images   \t- Loads all images in dir\n");
}

int main(int argc, char *argv[]) {

    char* flag = (char*)(argc - 1 == 0 ? "." : argv[argc - 1]);

    if (strcmp(flag, "-h") == 0) {
        print_usage();
        return 1;
    }

    Ryi::init(flag);
    auto images = Ryi::images();

    auto goLeft = [&images]() {
        if (images.size() == 0) return;
        if (Ryi::image_index <= 0)
        Ryi::image_index = images.size() - 1;
        else Ryi::image_index--;
    };

    auto goRight = [&images]() {
        if (images.size() == 0) return;
        Ryi::image_index++;
        Ryi::image_index %= images.size();
    };

    auto seekLeft = (new Button)
        ->text("<<")
        ->font_size(20)
        ->bg_color(GetColor(0x181818ff))
        ->fg_color(RED)
        ->position(30, GetScreenHeight() / 2)
        ->size(30, 35)
        ->on_click(goLeft);

    auto seekRight = (new Button)
        ->text(">>")
        ->font_size(20)
        ->bg_color(GetColor(0x181818ff))
        ->fg_color(RED)
        ->position(GetScreenWidth() - seekLeft->position().x - 30 * 2, GetScreenWidth() / 2)
        ->size(30, 35)
        ->on_click(goRight);

    auto okButton = (new Button)
        ->text("ok")
        ->font_size(20)
        ->bg_color(GetColor(0x181818ff))
        ->fg_color(RED)
        ->position(GetScreenWidth() - seekLeft->position().x - 30 * 2, GetScreenHeight() / 2)
        ->size(30, 35)
        ->on_click([]() {
            Ryi::show_about = false;
        });

    auto popupMenu = (new PopUpMenu)
        ->rect({0,0,20,0});

    popupMenu->menu_item("Open Dir", [&images]() {
        char* selected_path = tinyfd_selectFolderDialog("Open Images Folder", NULL);
        if (selected_path == nullptr)
            return;

        if (images.size() > 0) {
            for (auto& image: images) {
                UnloadTexture(image.image);
                free(image.path);
            }
        }
        images.clear();
        Ryi::load_images(selected_path);
        images = Ryi::images();
    });
    popupMenu->separator();

    popupMenu->menu_item(
        "Scale Image",
        []() {
        Ryi::image_mode = ImageMode::SCALE;
        }
    );
    popupMenu->menu_item(
        "Center Image",
        []() {
        Ryi::image_mode = ImageMode::CENTERED;
        }
    );

    popupMenu->separator();
    popupMenu->menu_item("Go Left (<<)", goLeft);
    popupMenu->menu_item("Go Right (>>)", goRight);

    float rotation_factor = 1;
    popupMenu->separator();
    popupMenu->menu_item("Rotate +1deg", [rotation_factor]() {
        Ryi::rotation += rotation_factor;
        Ryi::rotation = (int)Ryi::rotation % 360;
    });
    popupMenu->menu_item("Rotate -1deg", [rotation_factor]() {
        Ryi::rotation -= rotation_factor;
        if (Ryi::rotation < 0)
            Ryi::rotation = 360 - Ryi::rotation;
    });
    popupMenu->menu_item("Reset Rotation", []() {
        Ryi::rotation = 0;
    });
    popupMenu->separator();

    popupMenu->menu_item("Configure", goLeft);

    popupMenu->separator();
    popupMenu->menu_item("Zoom In (+)", []() {
        Ryi::scale_factor += 0.1;
    });
    popupMenu->menu_item("Zoom Out (-)", []() {
        Ryi::scale_factor -= 0.1;
    });
    popupMenu->menu_item("Reset Zoom", []() {
        Ryi::scale_factor = 1;
    });

    popupMenu->separator();

    popupMenu->menu_item("Toggle Grid View", [&images]() {
        if (images.size() > 0)
            Ryi::grid_view = !Ryi::grid_view;
    });

    popupMenu->separator();

    popupMenu->menu_item("Help", []() {
        Ryi::open_app_from_url(__GIT_REPO__);
    });

    Ryi::scroll_y = GetScreenHeight();
    popupMenu->menu_item("About", []() {
        Ryi::show_about = true;
        Ryi::scroll_y = GetScreenHeight();
    });

    popupMenu->separator();
    popupMenu->menu_item("Exit", []() {
        Ryi::is_running = false;
    });

    while (Ryi::is_running) {
        auto dt = GetFrameTime();
        auto h = GetScreenHeight();

        if (!Ryi::grid_view) {
            seekLeft->update();
            seekRight->update();
            *seekRight->x() = GetScreenWidth() - seekLeft->position().x - 30 * 2,
            *seekRight->y() = h / 2;
            *seekLeft->y() = h / 2;

            if (IsKeyPressed(KEY_LEFT))
                goLeft();

            if (IsKeyPressed(KEY_RIGHT))
                goRight();

        }

        if (!Ryi::show_about) {
            if (IsKeyPressed(KEY_LEFT))
                goLeft();

            if (IsKeyPressed(KEY_RIGHT))
                goRight();
        } else {
            *okButton->x() = Ryi::dialog_rect.x + Ryi::dialog_rect.width / 2;
            *okButton->y() = Ryi::dialog_rect.y + Ryi::dialog_rect.height - 40;
            okButton = okButton->size(30, 20) ;
            okButton->update();
        }
        Ryi::debug.update(dt);
        popupMenu->update();

        auto mouse_scroll = GetMouseWheelMove();
        Ryi::scale_factor += mouse_scroll * dt;


        BeginDrawing();
        {
            Ryi::draw_background();

            if (Ryi::show_about) {
                Ryi::draw_about();
                okButton->draw();
            } else if (Ryi::grid_view) {
                Ryi::draw_grid_view();
            } else {
                Ryi::draw_image_slide();
                seekLeft->draw();
                seekRight->draw();
            }
            popupMenu->draw();

            DrawText(TextFormat("%d/%d", Ryi::image_index + 1, images.size() - 1), 5, 5, 13, BLACK);
            DrawText(TextFormat("%d/%d", Ryi::image_index + 1, images.size() - 1), 6, 6, 13, RED);
            Ryi::debug.draw();
        }
        EndDrawing();
    }

    for (auto& image: images) {
        UnloadTexture(image.image);
        free(image.path);
    }

    delete seekLeft;
    delete seekRight;
    delete okButton;
    delete popupMenu;

    return 0;
}
