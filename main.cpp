
#include <stdio.h>
#include <raylib.h>
#include <functional>
#include <dirent.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <vector>

#define GRID_STEP 20

void drawBackground() {
    auto w = GetScreenWidth();
    auto h = GetScreenHeight();
    int i = 0;
    for (int y = 0; y < h; y += GRID_STEP) {
        for (int x = 0; x < w; x += GRID_STEP) {
        auto color = i++ % 2 == 0 ? GetColor(0x181818ff) : GetColor(0x212121ff);
        DrawRectangle(x, y, GRID_STEP, GRID_STEP, color);
        }
        i++;
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
    auto text_height = MeasureText(text, 14);
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
    int padding = 10;
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
    // rect.height -= 20;
}


void drawButton(Button btn) {
    auto h = MeasureText(btn.text, btn.fontSize);
    auto w = TextLength(btn.text);
    auto btnW = (float)btn.minWidth;
    auto btnH = (float)btn.minHeight;
    Rectangle rect = {(float) btn.x, (float) btn.y, btnW * w, btnH * h / btn.fontSize};
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

std::vector<Texture> readImages(char* path){
    std::vector<Texture> images;
    DIR* dir = opendir(path);
    if (dir == NULL)
        return images;
    dirent* next_dir = readdir(dir);;
    while (next_dir != NULL) {
        char* file_name = next_dir->d_name;
        char* extension = strchr(file_name, '.');
        if (next_dir->d_type == DT_REG && extension != NULL && strcmp(extension, ".png") == 0) {
            auto image = LoadTexture(TextFormat("%s/%s", path, file_name));
            images.push_back(image);
        }
        next_dir = readdir(dir);;
    }


    return images;
}

Rectangle get_dest_rect(ImageMode mode) {
    auto w = GetScreenWidth();
    auto h = GetScreenHeight();

    switch (mode) {
    case ImageMode::SCALE: return {0, 0, (float)w, (float)h};
    case ImageMode::CENTERED: return {(float)w/4, (float)h/4, (float)w/2, (float)h/2};
    default: assert(false && "unreachable"); // unreachable
    };
}

int main(int argc, char *argv[]) {

    InitWindow(600, 400, "rayimage");
    SetTargetFPS(60);

    char* images_path = (char*)(argc == 0 ? "." : argv[argc - 1]);
    auto images = readImages(images_path);
    size_t images_count = images.size();
    size_t image_index = 0;

    auto goLeft = [&image_index, images_count]() {
        if (image_index <= 0)
        image_index = images_count - 1;
        else image_index--;
    };

    auto goRight = [&image_index, images_count]() {
        image_index++;
        image_index %= images_count;
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

    PopUpMenu popupMenu;
    popupMenu.rect = {0,0, 20, 0};
    popupMenu.visible = false;

    popupMenu.newMenuItem("Open Dir", []() {

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

    popupMenu.newSeparator();
    popupMenu.newMenuItem("Configure", goLeft);
    popupMenu.newSeparator();

    popupMenu.newMenuItem("About", goLeft);
    popupMenu.newMenuItem("License", goRight);

    popupMenu.newSeparator();
    popupMenu.newMenuItem("Exit", []() {});

    while (!WindowShouldClose()) {
        auto w = GetScreenWidth();
        auto h = GetScreenHeight();
        updateButton(&seekLeft);
        updateButton(&seekRight);
        popupMenu.update(GetFrameTime());

        BeginDrawing();
        {
            drawBackground();

            if (images_count > 0) {
                auto image = images[image_index];
                DrawTexturePro(
                    image,
                    {0, 0, (float)image.width, (float)image.height},
                    get_dest_rect(image_mode),
                    {0, 0},
                    0,
                    WHITE
                );
            }

            seekRight.x = GetScreenWidth() - seekLeft.x - 30 * 2,
            seekRight.y = h / 2;
            seekLeft.y = h / 2;
            drawButton(seekLeft);
            drawButton(seekRight);

            popupMenu.draw();

            DrawText(TextFormat("ImageMode: %s", image_mode == ImageMode::CENTERED ? "centered" : "scale"), 5, 5, 13, ORANGE);
        }
        EndDrawing();
    }

}
