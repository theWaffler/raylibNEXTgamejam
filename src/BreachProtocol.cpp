#include "BreachProtocol.h"

BreachProtocol::BreachProtocol(int width, int height)
    : screenWidth(width),
      screenHeight(height),
      isVerticalMove(false),
      currentRow(0),
      currentCol(0),
      gameWon(false),
      gameLost(false),
      timer(START_TIME) {
    initializeGame();
}

void BreachProtocol::initializeGame() {
    // Randomly select one of the predefined matrices and sequences
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, predefinedMatrices.size() - 1);
    int selectedMatrix = dis(gen);

    matrix = predefinedMatrices[selectedMatrix];
    requiredSequence = predefinedSequences[selectedMatrix];
    buffer.clear();
    selectedPositions.clear();
    timer = START_TIME;
    gameWon = false;
    gameLost = false;
    isVerticalMove = false;
    currentRow = 0;
    currentCol = 0;
}

void BreachProtocol::update() {
    if (gameWon || gameLost) return;

    timer -= GetFrameTime();
    if (timer <= 0) {
        timer = 0;
        gameLost = true;
        return;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        Vector2 mousePos = GetMousePosition();
        const int cellWidth = 60;
        const int cellHeight = 30;
        const int startX = (screenWidth - (MATRIX_SIZE * (cellWidth + CELL_PADDING))) / 2;
        const int startY = 50;

        int col = (mousePos.x - startX) / (cellWidth + CELL_PADDING);
        int row = (mousePos.y - startY) / (cellHeight + CELL_PADDING);

        if (row >= 0 && row < MATRIX_SIZE && col >= 0 && col < MATRIX_SIZE) {
            if (buffer.size() < BUFFER_SIZE) {
                bool validMove = false;

                if (selectedPositions.empty()) {
                    if (row == 0) {
                        validMove = true;
                        currentCol = col;
                        isVerticalMove = true;
                    }
                } else {
                    if (isVerticalMove && col == currentCol && row != selectedPositions.back().row) {
                        validMove = true;
                        currentRow = row;
                        isVerticalMove = false;
                    } else if (!isVerticalMove && row == currentRow && col != selectedPositions.back().col) {
                        validMove = true;
                        currentCol = col;
                        isVerticalMove = true;
                    }
                }

                if (validMove) {
                    buffer.push_back(matrix[row][col]);
                    selectedPositions.push_back({row, col});

                    if (buffer == requiredSequence) {
                        gameWon = true;
                    }
                }
            }
        }
    }
}

void BreachProtocol::draw() {
    // Define basic layout constants
    const int cellWidth = 60;
    const int cellHeight = 30;
    const int verticalSpacing = 50;  // Space between major sections

    // Calculate matrix position to center it horizontally
    const int startX = (screenWidth - (MATRIX_SIZE * (cellWidth + CELL_PADDING))) / 2;
    const int startY = 70;  // Initial vertical position

    // Draw title and timer
    DrawText("CODE MATRIX", startX, startY - 30, 20, GREEN);
    // Timer changes color to red when time is running low (less than 3 seconds)
    DrawText(TextFormat("TIME: %.1f", timer),
            startX + (MATRIX_SIZE * (cellWidth + CELL_PADDING)) - 100,  // Right-aligned
            startY - 30, 20, timer > 3 ? GREEN : RED);

    // Draw the main code matrix grid
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            // Calculate position for each cell
            Rectangle cell = {
                static_cast<float>(startX + j * (cellWidth + CELL_PADDING)),
                static_cast<float>(startY + i * (cellHeight + CELL_PADDING)),
                static_cast<float>(cellWidth),
                static_cast<float>(cellHeight)
            };

            // Determine cell background color based on state
            Color cellColor = BLACK;  // Default background

            // Highlight previously selected positions
            for (const auto& pos : selectedPositions) {
                if (pos.row == i && pos.col == j) {
                    cellColor = DARKGRAY;
                }
            }

            // Highlight current valid move options
            if ((!isVerticalMove && i == currentRow) || (isVerticalMove && j == currentCol)) {
                cellColor = DARKBLUE;
            }

            // Draw cell background and border
            DrawRectangleRec(cell, cellColor);
            DrawRectangleLinesEx(cell, 1, GREEN);

            // Draw cell content (the code)
            const char* text = matrix[i][j].c_str();
            // Center the text within the cell
            Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 20, 1);
            Vector2 textPos = {
                cell.x + (cell.width - textSize.x) / 2,
                cell.y + (cell.height - textSize.y) / 2
            };
            DrawText(text, textPos.x, textPos.y, 20, GREEN);
        }
    }

    // Draw buffer section
    const int bufferStartX = startX;
    const int bufferStartY = startY + (MATRIX_SIZE * (cellHeight + CELL_PADDING)) + verticalSpacing;

    DrawText("BUFFER", bufferStartX, bufferStartY - 25, 20, GREEN);

    // Draw buffer cells
    for (int i = 0; i < BUFFER_SIZE; i++) {
        Rectangle cell = {
            static_cast<float>(bufferStartX + i * (cellWidth + CELL_PADDING)),
            static_cast<float>(bufferStartY),
            static_cast<float>(cellWidth),
            static_cast<float>(cellHeight)
        };

        // Draw empty buffer cell
        DrawRectangleRec(cell, BLACK);
        DrawRectangleLinesEx(cell, 1, GREEN);

        // If this buffer position has been filled, draw the code
        if (i < buffer.size()) {
            const char* text = buffer[i].c_str();
            Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 20, 1);
            Vector2 textPos = {
                cell.x + (cell.width - textSize.x) / 2,
                cell.y + (cell.height - textSize.y) / 2
            };
            DrawText(text, textPos.x, textPos.y, 20, GREEN);
        }
    }

    // Draw required sequence section
    const int seqStartX = startX;
    const int seqStartY = bufferStartY + cellHeight + verticalSpacing;

    DrawText("REQUIRED SEQUENCE", seqStartX, seqStartY - 25, 20, GREEN);

    // Draw sequence cells
    for (size_t i = 0; i < requiredSequence.size(); i++) {
        Rectangle cell = {
            static_cast<float>(seqStartX + i * (cellWidth + CELL_PADDING)),
            static_cast<float>(seqStartY),
            static_cast<float>(cellWidth),
            static_cast<float>(cellHeight)
        };
        // Draw cell background and border
        DrawRectangleRec(cell, BLACK);
        DrawRectangleLinesEx(cell, 1, GREEN);
        // Draw the required code (in yellow to distinguish from buffer)
        const char* text = requiredSequence[i].c_str();
        Vector2 textSize = MeasureTextEx(GetFontDefault(), text, 20, 1);
        Vector2 textPos = {
            cell.x + (cell.width - textSize.x) / 2,
            cell.y + (cell.height - textSize.y) / 2
        };
        DrawText(text, textPos.x, textPos.y, 20, YELLOW);
    }
    // Draw game state messages
    if (gameWon) {
        const char* winText = "BREACH SUCCESSFUL!";
        int textWidth = MeasureText(winText, 30);
        // Center the success message
        DrawText(winText, (screenWidth - textWidth) / 2, screenHeight - 70, 30, GREEN);
    } else if (gameLost) {
        const char* loseText = "BREACH FAILED!";
        int textWidth = MeasureText(loseText, 30);
        // Center the failure message
        DrawText(loseText, (screenWidth - textWidth) / 2, screenHeight - 70, 30, RED);
    }
}

void BreachProtocol::reset() {
    initializeGame();
}
