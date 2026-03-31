#ifndef ERRORVIEW_H
#define ERRORVIEW_H

#include <raylib.h>

struct ErrorView {
public:

    ErrorView(float duration):
        duration(duration), message(nullptr), pos({0,0}) {}

    void update(float dt) {
        if (this->message == nullptr)
            return;

        static float timer = this->duration;
        if (this->pos.y == 0) {
            this->pos = {10, GetScreenHeight() * 70.0f / 100.0f};
        }

        timer -= dt;
        if (timer < 0) {
            this->pos.y += 20 * dt;
            this->alpha -= 0.01;
        }

        if (this->alpha <= 0) {
            this->message = nullptr;
            timer = this->duration;
            this->pos = {0,0};
            this->alpha = 1.0f;
        }
    }

    void draw() {
        if (this->message == nullptr)
            return;

        DrawText(TextFormat("[Error]: %s", this->message), this->pos.x, this->pos.y, 13, Fade(RED, alpha));
    }

    void report(const char* message) {
        this->message = message;
    }

private:
    float duration = 3.0f;
    float alpha = 1.0f;
    Vector2 pos = {0,0};
    const char* message = nullptr;
};


#endif // ERRORVIEW_H
