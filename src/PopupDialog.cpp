#include "PopupDialog.h"

PopupDialog::PopupDialog()
    : scrollPosition(0),
      scrollSpeed(100.0f),
      isVisible(false),
      padding(20),
      width(600),
      height(400) {
    // Center the dialog
    x = (GetScreenWidth() - width) / 2;
    y = (GetScreenHeight() - height) / 2;
}

void PopupDialog::WrapText() {
    lines.clear();
    std::string currentLine;
    std::string word;

    for (char c : text) {
        if (c == ' ' || c == '\n') {
            if (MeasureText((currentLine + word).c_str(), 20) > width - (padding * 2)) {
                lines.push_back(currentLine);
                currentLine = word + " ";
            } else {
                currentLine += word + " ";
            }
            word = "";
            if (c == '\n') {
                lines.push_back(currentLine);
                currentLine = "";
            }
        } else {
            word += c;
        }
    }

    if (!word.empty()) {
        if (MeasureText((currentLine + word).c_str(), 20) > width - (padding * 2)) {
            lines.push_back(currentLine);
            lines.push_back(word);
        } else {
            lines.push_back(currentLine + word);
        }
    } else if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }
}

void PopupDialog::Show(const std::string& message) {
    text = message;
    isVisible = true;
    scrollPosition = 0;
    WrapText();
}

void PopupDialog::Hide() {
    isVisible = false;
}

void PopupDialog::DrawScrollbar() {
    float contentHeight = lines.size() * 25.0f;
    if (contentHeight <= height - (padding * 2)) return;

    float scrollbarHeight = (height - (padding * 2)) * ((height - (padding * 2)) / contentHeight);
    float maxScroll = contentHeight - (height - (padding * 2));
    float scrollPercentage = scrollPosition / maxScroll;
    float scrollbarY = y + padding + (scrollPercentage * ((height - (padding * 2)) - scrollbarHeight));

    DrawRectangle(x + width - padding, y + padding, 10, height - (padding * 2), Color{0, 100, 0, 100});
    DrawRectangle(x + width - padding, scrollbarY, 10, scrollbarHeight, GREEN);
}

void PopupDialog::Update() {
    if (!isVisible) return;

    float contentHeight = lines.size() * 25.0f;
    float maxScroll = contentHeight - (height - (padding * 2));

    if (contentHeight > height - (padding * 2)) {
        if (IsKeyDown(KEY_UP)) {
            scrollPosition = std::max(0.0f, scrollPosition - scrollSpeed * GetFrameTime());
        }
        if (IsKeyDown(KEY_DOWN)) {
            scrollPosition = std::min(maxScroll, scrollPosition + scrollSpeed * GetFrameTime());
        }

        float wheel = GetMouseWheelMove();
        if (wheel != 0) {
            scrollPosition = std::max(0.0f, std::min(maxScroll,
                scrollPosition - (wheel * 30.0f)));
        }
    }

    if (IsKeyPressed(KEY_ESCAPE)) {
        restartRequested = true;
        Hide();
    }
}

void PopupDialog::Draw() {
    if (!isVisible) return;

    // Draw semi-transparent background
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 200});

    // Draw popup window
    DrawRectangle(x, y, width, height, BLACK);
    DrawRectangleLines(x, y, width, height, GREEN);

    // Create scissor mode for text clipping
    BeginScissorMode(x + padding, y + padding, width - (padding * 2), height - (padding * 2));

    // Draw text
    for (size_t i = 0; i < lines.size(); i++) {
        float lineY = y + padding + (i * 25.0f) - scrollPosition;
        if (lineY >= y + padding - 25 && lineY <= y + height - padding) {
            DrawText(lines[i].c_str(), x + padding, lineY, 20, GREEN);
        }
    }

    EndScissorMode();

    // Draw scrollbar
    DrawScrollbar();

    // Draw instruction
    DrawText("Use UP/DOWN arrows or mouse wheel to scroll",
             x + padding, y + height - padding - 20, 15, Color{0, 255, 0, 150});
    DrawText("Press ESC to restart",
             x + width - padding - MeasureText("Press ESC to restart", 15),
             y + height - padding - 20, 15, Color{0, 255, 0, 150});
}
