#pragma once
#include <raylib.h>
#include <string>
#include <vector>

class PopupDialog {
private:
    std::string text;
    float scrollPosition;
    float scrollSpeed;
    bool isVisible;
    int padding;
    int width;
    int height;
    int x;
    int y;
    std::vector<std::string> lines;

    void WrapText();
    void DrawScrollbar();

    bool restartRequested = false;

public:
    PopupDialog();
    void Show(const std::string& message);
    void Hide();
    void Draw();
    bool IsVisible() const { return isVisible; }
    void Update();

    bool isRestartRequested() const { return restartRequested; }
    void clearRestartRequest() { restartRequested = false; }
};
