#include <raylib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>

#include "ryi.h"

#include "build.h"
#include "license.h"

#include <assert.h>

int Ryi::image_index = -1;
bool Ryi::grid_view = false;
int Ryi::scroll_y = 700;
bool Ryi::is_running = true;
bool Ryi::show_about = false;
float Ryi::scale_factor = 1;
float Ryi::rotation = 0;
ErrorView Ryi::debug(3.0f);
ImageMode Ryi::image_mode = ImageMode::SCALE;
Rectangle Ryi::dialog_rect = {0, 0, 0, 0};

void Ryi::init(char* path) {
    InitWindow(600, 400, "Ryi");
    SetTargetFPS(60);
    SetWindowState(FLAG_WINDOW_RESIZABLE);

    if (Ryi::is_url(path))
        Ryi::load_from_url(path);
    else
        Ryi::load_images(path);
}

void Ryi::draw_background() {
    const int GRID_STEP = 20;
    auto w = GetScreenWidth();
    auto h = GetScreenHeight();
    for (int y = 0; y < h; y += GRID_STEP) {
        for (int x = 0; x < w; x += GRID_STEP) {
            auto color = ((x / GRID_STEP) + (y / GRID_STEP)) % 2 == 0 ? GetColor(0x181818ff) : GetColor(0x212121ff);
            DrawRectangle(x, y, GRID_STEP, GRID_STEP, color);
        }
    }
}

bool Ryi::is_image_supported(char* ext) {
    if (ext == NULL) return false;

    return
        strcmp(ext, ".png") == 0 ||
        strcmp(ext, ".jpg") == 0 ||
        strcmp(ext, ".jpeg") == 0 ||
        strcmp(ext, ".bmp") == 0 ||
        strcmp(ext, ".gif") == 0;
}

Rectangle Ryi::get_dest_rect(ImageMode mode, float scaleFactor) {
    auto w = GetScreenWidth();
    auto h = GetScreenHeight();

    switch (mode) {
    case ImageMode::SCALE: return {0 - scaleFactor * 2, 0 - scaleFactor * 2, w * scaleFactor, h * scaleFactor};
    case ImageMode::CENTERED: return {(float)w/4 * scaleFactor, (float)h/4 * scaleFactor, w/2 * scaleFactor, h/2 * scaleFactor};
    default: assert(false && "unreachable"); // unreachable
    };
}

void Ryi::open_app_from_url(char* url) {
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

std::vector<RenderImage> Ryi::Ryi::_images;
void Ryi::load_images(const char* path) {
    Ryi::_images = RenderImage::load_images_from_dir(path);
    if (Ryi::_images.size() > 0)
        Ryi::image_index = 0;
    else {
        Ryi::image_index = -1;
        if (path != NULL && *path == '.')
            Ryi::debug.report("Failed to load images from current directory (`.`)");
        else
            Ryi::debug.report(TextFormat("Failed to load images from path: `%s`", path));
    }
}

int write_data(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t written = fwrite(ptr, size, nmemb, stream);
    return written;
}

void Ryi::load_from_url(const char* url) {

    char temp_path[] = "/tmp/imag.png";

    FILE *fp = fopen(temp_path, "wb");

    printf("Temporary file created at: %s\n", temp_path);
    CURL* curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        CURLcode res = curl_easy_perform(curl);

        if(res == CURLE_OK) {
            printf("Image downloaded to temp storage successfully.\n");
        } else {
            Ryi::debug.report("Failed fetching image");
            fclose(fp);
        }
        curl_easy_cleanup(curl);
    }

    fclose(fp);

    auto image = LoadTexture(temp_path);
    Ryi::_images.push_back((RenderImage){.path = strdup(url), .image = image});
    Ryi::image_index = 0;
}


bool Ryi::is_url(const char* url) {
    return url != NULL && (strncmp(url, "http://", 7) == 0 ||
           strncmp(url, "https://", 8) == 0 ||
           strncmp(url, "ftp://", 6) == 0);
}

std::vector<RenderImage> Ryi::images() {
    return Ryi::_images;
}

void Ryi::draw_about() {
    auto rect = Ryi::get_dest_rect(ImageMode::CENTERED, 1.0f);
    auto h = GetScreenHeight();
    auto w = GetScreenWidth();

    rect.width *= 1.3;
    rect.height *= h <= 600 ? 1.03 : 1.2;

    rect.x = (w - rect.width) / 2;
    rect.y = (h - rect.height) / 2;

    DrawText(__LICENSE__, w <= 600 ? rect.x - rect.width/12: rect.x + rect.width / 4, Ryi::scroll_y, 12, GetColor(0x66aaccff));
    Ryi::scroll_y--;

    if (Ryi::scroll_y >= 10000) Ryi::scroll_y = h;
    // if (rect.width > 750) rect.width = 750;
    // if (rect.height > 350) rect.height = 350;
    DrawRectanglePro({rect.x + 5, rect.y + 5, rect.width, rect.height}, {0, 0}, 0, BLACK);
    DrawRectanglePro(rect, {0, 0}, 0, GetColor(0x262626ff));

    int font_size = 20;
    char* about_header = "About Ryi";
    int text_height = MeasureText(about_header, font_size);
    int text_len = strlen(about_header);
    int y = rect.y + text_height * 0.1;
    DrawText(about_header, rect.x + rect.width / 2 - text_len * 6, y, font_size, WHITE);

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

#if defined(__linux__) || defined(__unix__)
    DrawText(TextFormat("Build By      : %s", __BUILD_BY__), x, y, 12, WHITE);
    y += 20;
#endif

    DrawText(TextFormat("Build Platform: %s", __BUILD_ON__), x, y, 12, WHITE);
    y += 20;
    {

        bool is_first = true;
        char *command_copy = strdup(__BUILD_COMMAND__);
        char *line = strtok(command_copy, "\n");
        while (line != NULL) {
            if (CheckCollisionPointRec({(float)x, (float)y}, rect) == false) break;

            if (is_first) {
                DrawText(TextFormat("Build Command: %s", line), x, y, 12, GREEN);
                is_first = false;
            } else {
                DrawText("+", x, y, 12, BLUE);
                DrawText(TextFormat("\t%s", line), x + 20, y, 12, GREEN);
            }
            y+= 20;
            line = strtok(NULL, "\n");
        }

    }

    Ryi::dialog_rect = rect;
    if (CheckCollisionPointRec(GetMousePosition(), rect)) {
        DrawRectangleLinesEx(rect, 1, ORANGE);
    }
}

void Ryi::draw_grid_view() {
    auto w = GetScreenWidth();
    auto h = GetScreenHeight();
    int i = 0;

    Rectangle hovered_rect = {0,0,0,0};
    int hovered_index = -1;
    int grid_w = 60 + w / Ryi::_images.size();
    int grid_h = 60 + h / Ryi::_images.size();
    if (Ryi::_images.size() > 0) {
        for (int y = 10; y < h; y += grid_h) {
            for (int x = 10; x < w; x += grid_w) {
                if ((size_t)i == Ryi::_images.size() - 1)
                    goto _out;

                int index = i++;
                auto image = Ryi::_images[index].image;
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

            auto image = Ryi::_images[hovered_index].image;
            auto image_path = Ryi::_images[hovered_index].path;
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
            DrawText(TextFormat("path: %s   [%d/%d]", image_path, hovered_index + 1, Ryi::_images.size() - 1), 20, h - 60, 14, RED);
            if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_MIDDLE_BUTTON)) {
                image_index = hovered_index;
                grid_view = !grid_view;
            }

            if (rotation == 0) {
                DrawRectangleLinesEx(hovered_rect, 1, ORANGE);
            }
        }
    }
}

void Ryi::draw_image_slide() {
    if (Ryi::_images.size() > 0) {
        auto image = Ryi::_images[image_index].image;
        auto rect = Ryi::get_dest_rect(image_mode, scale_factor);

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
}
