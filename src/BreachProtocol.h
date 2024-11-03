#pragma once
#include <raylib.h>
#include <vector>
#include <string>
#include <random>

class BreachProtocol {
    private:
        static const int MATRIX_SIZE = 5;
        static const int BUFFER_SIZE = 4;
        static constexpr float CELL_PADDING = 4.0f;
        static const int START_TIME = 15;

        struct Position {
            int row;
            int col;
        };

        std::vector<std::string> buffer;
        std::vector<Position> selectedPositions;
        std::vector<std::vector<std::string>> matrix;
        std::vector<std::string> requiredSequence;
        bool isVerticalMove;
        int currentRow;
        int currentCol;
        Position highlighted{0,0};
        bool gameWon;
        bool gameLost;
        float timer;
        int screenWidth;
        int screenHeight;

        // pre-defined matrices bc im too lazy to figure out why the random one doesn't work
        // rip A* pathfinding
        const std::vector<std::vector<std::vector<std::string>>> predefinedMatrices = {
            { // Matrix 1
                {"1C", "E9", "BD", "55", "1C"},
                {"BD", "55", "1C", "E9", "BD"},
                {"E9", "1C", "55", "BD", "E9"},
                {"1C", "BD", "E9", "1C", "55"},
                {"55", "E9", "BD", "1C", "E9"}
            },
            { // Matrix 2
                {"55", "1C", "E9", "BD", "1C"},
                {"E9", "55", "1C", "1C", "BD"},
                {"1C", "E9", "BD", "55", "1C"},
                {"BD", "1C", "55", "E9", "55"},
                {"1C", "BD", "E9", "1C", "E9"}
            },
            { // Matrix 3
                {"E9", "55", "1C", "BD", "E9"},
                {"1C", "E9", "BD", "55", "1C"},
                {"BD", "1C", "E9", "1C", "BD"},
                {"55", "E9", "1C", "BD", "55"},
                {"1C", "BD", "55", "E9", "1C"}
            }
        };

        const std::vector<std::vector<std::string>> predefinedSequences = {
            {"E9", "BD", "1C", "BD"},
            {"1C", "BD", "E9", "55"},
            {"E9", "1C", "BD", "55"}
        };

    public:
    BreachProtocol(int width, int height);
    void update();
    void draw();
    void reset();
    bool isComplete() const { return gameWon || gameLost; }
    bool isSuccessful() const { return gameWon; }
    void initializeGame();
};
