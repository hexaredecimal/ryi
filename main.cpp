
#include <stdio.h>
#include <stdlib.h>
#include <raylib.h>
#include <functional>
#include <dirent.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <vector>

#include "tinyfiledialogs.h"
#include "license.h"
#include "build.h"

#define GRID_STEP 20

void drawBackground() {
    auto w = GetScreenWidth();
    auto h = GetScreenHeight();
    for (int y = 0; y < h; y += GRID_STEP) {
        for (int x = 0; x < w; x += GRID_STEP) {
            auto color = ((x / GRID_STEP) + (y / GRID_STEP)) % 2 == 0 ? GetColor(0x181818ff) : GetColor(0x212121ff);
            DrawRectangle(x, y, GRID_STEP, GRID_STEP, color);
        }
    }
}

typedef struct {
    char* text;
    int fontSize;
    Color bg;
    Color fg;
    int x;
    int y;
    int minWidth;
    int minHeight;
    std::function<void()> onClick;
} Button;

enum class ImageMode {
    SCALE = 1,
    CENTERED,
};

struct MenuItem {
public:
    void update(float dt);
    void draw();

    char* text;
    bool hovered;
    bool isSeparator;
    Rectangle rect;
    Color color;
    std::function<void()> onClick;
};

void MenuItem::update(float dt) {
    if (isSeparator) return;
    auto mouse_pos = GetMousePosition();
    hovered = CheckCollisionPointRec(mouse_pos, rect);
    static Color prev = color;
    color = hovered ? ORANGE : prev;
}

void MenuItem::draw() {
    if (isSeparator) {
        DrawRectangleGradientEx(rect, BLACK, BLACK, BLACK, BLACK);
        return;
    }
    DrawRectangleGradientEx(rect, color, color, color, color);
    DrawText(text, rect.x + 5, rect.y, 12, BLACK);
}

struct PopUpMenu {
public:
    void update(float dt);
    void draw();

    Rectangle rect;
    void newMenuItem(char* text, std::function<void()> func);
    void newSeparator();
    void done();
    bool visible;
private:
    std::vector<MenuItem> children;
};

void PopUpMenu::update(float dt) {

    bool leftClicked = false;
    if (IsMouseButtonPressed(MOUSE_RIGHT_BUTTON) && !visible) {
        visible = true;
        auto mouse_pos = GetMousePosition();
        rect.x = mouse_pos.x;
        rect.y = mouse_pos.y;
    }
    else if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        auto mouse_pos = GetMousePosition();
        if (CheckCollisionPointRec(mouse_pos, rect) == false) {
            visible = false;
        } else {
            leftClicked = true;
        }
    }

    if (!visible) return;
    int y = rect.y;
    for (auto& item: children) {
        item.rect.x = rect.x;
        item.rect.y = y;
        item.rect.width = rect.width;
        y += item.rect.height;
        item.update(dt);

        if (item.hovered && leftClicked && item.onClick != nullptr) {
            item.onClick();
        }

    }
}

void PopUpMenu::draw() {
    if (!visible) return;
    DrawRectangle(rect.x + 5, rect.y + 5, rect.width, rect.height + 5, BLACK);
    DrawRectangle(rect.x, rect.y, rect.width, rect.height, GetColor(0xaaaaaaff));
    for (auto child: children) {
        child.draw();
    }
}

void PopUpMenu::newMenuItem(char* text, std::function<void()> func) {
    MenuItem menuItem;
    menuItem.text = text;
    menuItem.color = GetColor(0xaaaaaaff);
    menuItem.rect.x = rect.x;
    menuItem.rect.y = rect.height;
    menuItem.rect.width = rect.width;
    menuItem.rect.height = 16;
    menuItem.onClick = func;
    menuItem.isSeparator = false;

    int text_width = strlen(text);
    for (auto child: children) {
        int current_len = strlen(child.text);
        text_width = current_len > text_width ? current_len : text_width;
    }
    children.push_back(menuItem);
    rect.height += menuItem.rect.height;
    rect.width = text_width * 10;
    done();
}

void PopUpMenu::newSeparator() {
    MenuItem menuItem;
    menuItem.text = "";
    menuItem.color = BLACK;
    menuItem.rect.x = rect.x;
    menuItem.rect.y = rect.height;
    menuItem.rect.width = rect.width;
    menuItem.rect.height = 1;
    menuItem.isSeparator = true;
    menuItem.onClick = nullptr;

    int text_width = strlen(menuItem.text);
    for (auto child: children) {
        int current_len = strlen(child.text);
        text_width = current_len > text_width ? current_len : text_width;
    }
    children.push_back(menuItem);
    rect.height += menuItem.rect.height + 4;
    rect.width = text_width * 10;
    done();
}

void PopUpMenu::done() {
    //auto last = children.end();
    rect.height -= 1.5;
}


void drawButton(Button btn) {
    auto h = MeasureText(btn.text, btn.fontSize);
    auto w = TextLength(btn.text);
    auto btnW = (float)btn.minWidth;
    auto btnH = (float)btn.minHeight;
    Rectangle rect = {(float) btn.x, (float) btn.y, btnW * w, btnH * h / btn.fontSize};
    DrawRectanglePro({rect.x + 5, rect.y + 5, rect.width, rect.height}, {0, 0}, 0, BLACK);
    DrawRectanglePro(rect, {0, 0}, 0, btn.bg);
    DrawRectangleLinesEx(rect, 1, btn.fg);
    DrawText(btn.text, rect.x + w + btnW, rect.y + h / btn.fontSize, btn.fontSize, btn.fg);
}

void updateButton(Button* btn) {
    auto h = MeasureText(btn->text, btn->fontSize);
    auto w = TextLength(btn->text);
    auto btnW = (float)btn->minWidth;
    auto btnH = (float)btn->minHeight;
    Rectangle rect = {(float) btn->x, (float) btn->y, btnW * w, btnH * h / btn->fontSize};
    auto point = GetMousePosition();

    static Color prev = btn->fg;
    auto isHovered = CheckCollisionPointRec(point, rect);
    if (isHovered) {
        btn->fg = ORANGE;
    } else {
        btn->fg = prev;
    }

    if (!IsMouseButtonPressed(MOUSE_LEFT_BUTTON))
        return;


    if (isHovered && btn->onClick) {
        btn->onClick();
    }
}

bool isImageSupported(char* ext) {
    if (ext == NULL) return false;

    return
        strcmp(ext, ".png") == 0 ||
        strcmp(ext, ".jpg") == 0 ||
        strcmp(ext, ".jpeg") == 0 ||
        strcmp(ext, ".bmp") == 0 ||
        strcmp(ext, ".gif") == 0;
}

struct RenderImage {
    char* path;
    Texture2D image;
};

std::vector<RenderImage> readImages(char* path){
    std::vector<RenderImage> images;
    DIR* dir = opendir(path);
    if (dir == NULL)
        return images;
    dirent* next_dir = readdir(dir);;
    while (next_dir != NULL) {
        char* file_name = next_dir->d_name;
        char* extension = strchr(file_name, '.');
        bool isValid = isImageSupported(extension);
        if (next_dir->d_type == DT_REG && extension != NULL && isValid) {
            const char* image_path = TextFormat("%s/%s", path, file_name);
            auto image = LoadTexture(image_path);
            images.push_back((RenderImage){.path = strdup(image_path), .image = image});
        }
        next_dir = readdir(dir);;
    }


    return images;
}

Rectangle get_dest_rect(ImageMode mode, float scaleFactor) {
    auto w = GetScreenWidth();
    auto h = GetScreenHeight();

    switch (mode) {
    case ImageMode::SCALE: return {0 - scaleFactor * 2, 0 - scaleFactor * 2, w * scaleFactor, h * scaleFactor};
    case ImageMode::CENTERED: return {(float)w/4 * scaleFactor, (float)h/4 * scaleFactor, w/2 * scaleFactor, h/2 * scaleFactor};
    default: assert(false && "unreachable"); // unreachable
    };
}

void open_app_from_url(char* url) {
    char command_buffer[255];
#if defined(_WIN32) || defined(_WIN64)
    snprintf(command_buffer, sizeof(command_buffer), "cmd /c start \"\" \"%s\"", url);
#elif defined(__APPLE__)
    snprintf(command_buffer, sizeof(command_buffer), "open \"%s\"", url);
#elif defined(__linux__) || defined(__unix__) || defined(_POSIX_VERSION)
    snprintf(command_buffer, sizeof(command_buffer), "xdg-open \"%s\"", url);
#else
    #error "Unsupported Operating System"
#endif
    system(command_buffer);
}

int main(int argc, char *argv[]) {

    InitWindow(600, 400, "rayimage");
    SetTargetFPS(60);
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    char* images_path = (char*)(argc == 0 ? "." : argv[argc - 1]);
    auto images = readImages(images_path);
    size_t image_index = 0;

    auto goLeft = [&image_index, &images]() {
        if (images.size() == 0) return;
        if (image_index <= 0)
        image_index = images.size() - 1;
        else image_index--;
    };

    auto goRight = [&image_index, &images]() {
        if (images.size() == 0) return;
        image_index++;
        image_index %= images.size();
    };

    auto seekLeft = (Button){
        .text = "<<",
        .fontSize = 20,
        .bg = GetColor(0x181818ff),
        .fg = RED,
        .x = 30,
        .y = GetScreenHeight() / 2,
        .minWidth = 30,
        .minHeight = 35,
        .onClick = goLeft
    };

    auto seekRight = (Button){
        .text = ">>",
        .fontSize = 20,
        .bg = GetColor(0x181818ff),
        .fg = RED,
        .x = GetScreenWidth() - seekLeft.x - 30 * 2,
        .y = GetScreenHeight() / 2,
        .minWidth = 30,
        .minHeight = 35,
        .onClick = goRight
    };

    bool show_about = false;
    auto okButton = (Button){
        .text = "Ok",
        .fontSize = 20,
        .bg = GetColor(0x181818ff),
        .fg = RED,
        .x = GetScreenWidth() - seekLeft.x - 30 * 2,
        .y = GetScreenHeight() / 2,
        .minWidth = 30,
        .minHeight = 35,
        .onClick = [&show_about]() {
            show_about = false;
        }
    };

    PopUpMenu popupMenu;
    popupMenu.rect = {0,0, 20, 0};
    popupMenu.visible = false;

    popupMenu.newMenuItem("Open Dir", [&images, &image_index]() {
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
        images = readImages(selected_path);
        image_index = 0;
    });
    popupMenu.newSeparator();

    ImageMode image_mode = ImageMode::CENTERED;
    popupMenu.newMenuItem(
        "Scale Image",
        [&image_mode]() {
        image_mode = ImageMode::SCALE;
        }
    );
    popupMenu.newMenuItem(
        "Center Image",
        [&image_mode]() {
        image_mode = ImageMode::CENTERED;
        }
    );

    popupMenu.newSeparator();
    popupMenu.newMenuItem("Go Left (<<)", goLeft);
    popupMenu.newMenuItem("Go Right (>>)", goRight);

    float rotation = 0;
    float rotation_factor = 10;
    popupMenu.newSeparator();
    popupMenu.newMenuItem("Rotate +10deg", [&rotation, rotation_factor]() {
        rotation += rotation_factor;
        rotation = (int)rotation % 360;
    });
    popupMenu.newMenuItem("Rotate -10deg", [&rotation, rotation_factor]() {
        rotation -= rotation_factor;
        if (rotation < 0)
            rotation = 360 - rotation;
    });
    popupMenu.newMenuItem("Reset Rotation", [&rotation]() {
        rotation = 0;
    });
    popupMenu.newSeparator();

    popupMenu.newMenuItem("Configure", goLeft);

    float scaleFactor = 1.0;
    popupMenu.newSeparator();
    popupMenu.newMenuItem("Zoom In (+)", [&scaleFactor]() {
        scaleFactor += 0.1;
    });
    popupMenu.newMenuItem("Zoom Out (-)", [&scaleFactor]() {
        scaleFactor -= 0.1;
    });
    popupMenu.newMenuItem("Reset Zoom", [&scaleFactor]() {
        scaleFactor = 1.0f;
    });

    popupMenu.newSeparator();

    bool grid_view = false;
    popupMenu.newMenuItem("Toggle Grid View", [&grid_view]() {
        grid_view = !grid_view;
    });

    popupMenu.newSeparator();

    popupMenu.newMenuItem("Help", []() {
        open_app_from_url(__GIT_REPO__);
    });

    int scroll_y = GetScreenHeight();
    popupMenu.newMenuItem("About", [&show_about, &scroll_y]() {
        show_about = true;
        scroll_y = GetScreenHeight();
    });

    bool is_running = true;
    popupMenu.newSeparator();
    popupMenu.newMenuItem("Exit", [&is_running]() {
        is_running = false;
    });

    while (is_running) {
        auto dt = GetFrameTime();
        auto w = GetScreenWidth();
        auto h = GetScreenHeight();

        if (!grid_view) {
            updateButton(&seekLeft);
            updateButton(&seekRight);
            seekRight.x = GetScreenWidth() - seekLeft.x - 30 * 2,
            seekRight.y = h / 2;
            seekLeft.y = h / 2;

            if (IsKeyPressed(KEY_LEFT))
                goLeft();

            if (IsKeyPressed(KEY_RIGHT))
                goRight();
        }

        if (!show_about) {
            if (IsKeyPressed(KEY_LEFT))
                goLeft();

            if (IsKeyPressed(KEY_RIGHT))
                goRight();
        } else {
            updateButton(&okButton);
        }
        popupMenu.update(dt);

        auto mouse_scroll = GetMouseWheelMove();
        scaleFactor += mouse_scroll * dt;


        BeginDrawing();
        {
            drawBackground();

            if (show_about) {
                auto rect = get_dest_rect(ImageMode::CENTERED, 1.0f);

                rect.x += rect.width / 8;

                DrawText(__LICENSE__, rect.x - rect.width / 8, scroll_y, 12, GetColor(0x66aaccff));
                scroll_y--;

                if (scroll_y >= 10000) scroll_y = h;
                // if (rect.width > 750) rect.width = 750;
                // if (rect.height > 350) rect.height = 350;
                DrawRectanglePro({rect.x + 5, rect.y + 5, rect.width, rect.height}, {0, 0}, 0, BLACK);
                DrawRectanglePro(rect, {0, 0}, 0, GetColor(0x2626262ff));

                int font_size = 20;
                char* about_header = "About Ryi";
                int text_height = MeasureText(about_header, font_size);
                int text_len = strlen(about_header);
                int y = rect.y + text_height * 0.1;
                DrawText("About Ryi", rect.x + rect.width / 2 - text_len * 6, y, font_size, WHITE);

                int x = rect.x + 20;
                y += 20;

                DrawText("App Name      : Raylib Image Viewer", x, y, 12, WHITE);
                y += 20;
                DrawText("Short Name    : Ryi", x, y, 12, WHITE);
                y += 20;
                DrawText("Developer     : Gama Sibusiso", x, y, 12, WHITE);
                y += 20;
                DrawText(TextFormat("Git Repo      : %s", __GIT_REPO__), x, y, 12, WHITE);
                y += 20;
                DrawText("License       : GPLV3", x, y, 12, WHITE);
                y += 20;
                DrawText(TextFormat("Build Date    : %s", __BUILD_DATE__), x, y, 12, WHITE);
                y += 20;
                DrawText(TextFormat("Build By      : %s", __BUILD_BY__), x, y, 12, WHITE);
                y += 20;
                DrawText(TextFormat("Build Platform: %s", __BUILD_ON__), x, y, 12, WHITE);
                y += 20;
                DrawText(TextFormat("Build Command : %s", __BUILD_COMMAND__), x, y, 12, WHITE);

                okButton.x = rect.x + rect.width / 2;
                okButton.y = rect.y + rect.height - 40;
                okButton.minHeight = 20;

                drawButton(okButton);
                if (CheckCollisionPointRec(GetMousePosition(), rect)) {
                    DrawRectangleLinesEx(rect, 1, ORANGE);
                }
            } else if (grid_view) {
                int i = 0;
                int grid_size = w / 10;

                Rectangle hovered_rect = {0,0,0,0};
                int hovered_index = -1;
                int grid_w = 60 + w / images.size();
                int grid_h = 60 + h / images.size();
                if (images.size() > 0) {
                    for (int y = 10; y < h; y += grid_h) {
                        for (int x = 10; x < w; x += grid_w) {
                            if (i == images.size() - 1)
                                goto _out;

                            int index = i++;
                            auto image_path = images[index].path;
                            auto image = images[index].image;
                            auto rect = (Rectangle) {(float)x, (float)y, (float)grid_w, (float)grid_h};
                            if ((rect.x + rect.width) > w) {
                                i--;
                                break;
                            }

                            if (CheckCollisionPointRec(GetMousePosition(), rect)) {
                                hovered_rect = rect;
                                hovered_index = index;
                                continue;
                            } else {
                                DrawTexturePro(
                                    image,
                                    {0, 0, (float)image.width, (float)image.height},
                                    rect,
                                    {0, 0},
                                    rotation,
                                    WHITE
                                );
                            }
                        }
                    }

                    _out:

                    if (hovered_index != -1) {
                        hovered_rect.x -= 30;
                        hovered_rect.y -= 30;
                        hovered_rect.width += 30 * 2;
                        hovered_rect.height += 30 * 2;

                        if (hovered_rect.y < 0) {
                            hovered_rect.y = 0;
                            // hovered_rect.width += 30;
                            hovered_rect.height -= 30;
                        }

                        auto image = images[hovered_index].image;
                        auto image_path = images[hovered_index].path;
                        DrawTexturePro(
                            image,
                            {0, 0, (float)image.width, (float)image.height},
                            hovered_rect,
                            {0, 0},
                            rotation,
                            WHITE
                        );
                        //DrawRectanglePro(hovered_rect, {0,0}, rotation, ORANGE);
                        DrawText(TextFormat("w: %d, h: %d", image.width, image.height), w - 120, h - 60, 14, RED);
                        DrawText(TextFormat("path: %s   [%d/%d]", image_path, hovered_index + 1, images.size() - 1), 20, h - 60, 14, RED);
                        if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON)) {
                            image_index = hovered_index;
                            grid_view = !grid_view;
                        }

                        if (rotation == 0) {
                            DrawRectangleLinesEx(hovered_rect, 1, ORANGE);
                        }
                    }
                }
            } else {
                if (images.size() > 0) {
                    auto image = images[image_index].image;
                    auto rect = get_dest_rect(image_mode, scaleFactor);

                    if (image_mode == ImageMode::CENTERED) {
                        DrawRectanglePro({rect.x + 5, rect.y + 5, rect.width, rect.height}, {0, 0}, rotation, GetColor(0x000000ee));
                    }
                    DrawTexturePro(
                        image,
                        {0, 0, (float)image.width, (float)image.height},
                        rect,
                        {0, 0},
                        rotation,
                        WHITE
                    );
                }

                drawButton(seekLeft);
                drawButton(seekRight);
            }
            popupMenu.draw();

            // DrawText(TextFormat("ImageMode: %s", image_mode == ImageMode::CENTERED ? "centered" : "scale"), 5, 5, 13, ORANGE);
            DrawText(TextFormat("%d/%d", image_index + 1, images.size() - 1), 5, 5, 13, BLACK);
            DrawText(TextFormat("%d/%d", image_index + 1, images.size() - 1), 6, 6, 13, RED);
        }
        EndDrawing();
    }

    for (auto& image: images) {
        UnloadTexture(image.image);
        free(image.path);
    }

    return 0;
}
