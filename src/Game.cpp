#include "Game.h"
#include "Terminal.h"
#include "Directory.h"
#include <raylib.h>
#include <vector>
#include <cmath>
#include <iostream>
#include <GL/gl.h>

const int terminalFontSize = 20;

Game::Game(int screenWidth, int screenHeight)
    : screenWidth(screenWidth),
      screenHeight(screenHeight),
      showStartScreen(true),
      showBreachProtocol(false),
      breachLoading(false),
      breachLoadTimer(0.0f),
      breachAttempts(3),
      breachGame(nullptr),
      showMissionBrief(false),
      showActionScene1(false),
      showRetroBootUp(false),
      startFadeToTerminal(false),
      fadeAlpha(0.0f),
      missionTextIndex(0),
      typewriterSpeed(3),
      typewriterCounter(0),
      transitionDelay(360),
      transitionCounter(0),
      shaderLoaded(false),
      scanlineIntensityLoc(-1),
      greenTintLoc(-1),
      actionScene1({0}),
      scenesLoaded(false),
      filesystem(nullptr),
      terminal(nullptr),
      currentEnding(GameEnding::NONE),
      missionText(
          "*================================== CLASSIFIED ===================================*\n"
          "|                                      OPERATION: NULL PTR                                        |\n"
          "|                                   CLEARANCE LEVEL: OMEGA                                     | \n"
          "*====================================================================================*\n\n"
          "MISSION BRIEF:\n"
          "Intelligence has identified a high-priority target system containing\n"
          "classified access codes. Your mission is to infiltrate and extract\n"
          "the required data without detection.\n\n"
          "OBJECTIVES:\n"
          "- Gain access to the target system\n"
          "- Navigate through security measures\n"
          "- Locate and retrieve NUCLEAR LAUNCH CODES\n"
          "- Extract without leaving traces\n\n"
          "CRITICAL NOTES:\n"
          "- Security systems are active and monitoring\n"
          "- Detection will result in immediate mission failure\n"
          "- Time is of the essence\n\n"
          "EXFILTRATION REQUIREMENTS:\n"
          "Retrieve NUCLEAR launch codes from alliance network infrastructure \n\n"
          "STATUS: ACTIVE - COMMENCE OPERATION\n"
      )
{
    //Initialize();
    if (!IsAudioDeviceReady()) {
        InitAudioDevice();
    }

    for (int i = 0; i < 4; i++) typingSounds[i] = { 0 };
    for (int i = 0; i < 2; i++) {
        retroPCsounds[i] = { 0 };
        music[i] = { 0 };
    }

    breachGame = new BreachProtocol(screenWidth, screenHeight);
    InitializeEndingTexts();
}

Game::~Game() {
    UnloadSounds();
    unloadMusic();
    UnloadScenes();
    UnloadSound(floppySound);
    if (breachGame) {
        delete breachGame;
    }
    if (filesystem) {
        delete filesystem;
    }
}

void Game::DrawBreachLoading() {
    const int padding = 20;
    int windowWidth = screenWidth - 2 * padding;
    int windowHeight = screenHeight - 2 * padding;

    DrawRectangle(padding, padding, windowWidth, windowHeight, Color{0, 20, 0, 50});
    DrawRectangleLines(padding, padding, windowWidth, windowHeight, GREEN);

    const char* loadingText = "LOADING BREACH PROTOCOL";
    static float loadingAnim = 0.0f;
    loadingAnim += GetFrameTime() * 4.0f;
    std::string dots(((int)loadingAnim % 4), '.');

    DrawText(TextFormat("%s%s", loadingText, dots.c_str()),
             screenWidth/2 - MeasureText(loadingText, 30)/2,
             screenHeight/2, 30, GREEN);
}

void Game::DrawBreachProtocol() {
    if (breachGame) {
        breachGame->draw();
        DrawText(TextFormat("ATTEMPTS REMAINING: %d", breachAttempts),
                10, screenHeight - 30, 20, GREEN);
    }
}

void Game::ProcessTerminalInput() {
    if (breachAttempts <= 0) {
        terminal.addOutput("CRITICAL SECURITY BREACH DETECTED");
        terminal.addOutput("YOU HAVE BEEN FOUND!");
        terminal.lockSystem();
        return;
    }

    if (showBreachProtocol) {
        breachGame->update();
        if (breachGame->isComplete()) {
            if (breachGame->isSuccessful()) {
                Directory* secureDir = terminal.getCurrentDir()->FindFile(".secure");
                if (secureDir) {
                    secureDir->setLocked(false);
                    terminal.addOutput("Access granted to .secure directory");
                    terminal.ProcessCommand("cd .secure");
                }
                showBreachProtocol = false;
            } else {
                breachAttempts--;
                if (breachAttempts <= 0) {
                    terminal.lockSystem();
                }
                breachGame->reset();
                showBreachProtocol = false;
            }
        }
        return;
    }

    // Check for breach protocol initiation from terminal
    if (terminal.shouldInitiateBreachProtocol() && !breachLoading && breachAttempts > 0) {
        breachLoading = true;
        breachLoadTimer = 2.0f;
        PlaySound(floppySound);
        terminal.clearBreachProtocolFlag();
        return;
    }

    // Normal terminal input processing
    int key = GetCharPressed();
    while (key > 0) {
        terminal.HandleInput(key);
        key = GetCharPressed();
    }

    if (IsKeyPressed(KEY_ENTER)) {
        terminal.ProcessCommand(terminal.GetInput());
        terminal.ClearInput();
    }

    if (IsKeyPressed(KEY_BACKSPACE)) {
        terminal.BackspaceInput();
    }

    terminal.ProcessScrollInput();
}

void Game::Initialize() {
    SetConfigFlags(FLAG_MSAA_4X_HINT | FLAG_VSYNC_HINT);
    SetExitKey(0);
    InitWindow(screenWidth, screenHeight, "Terminal Infiltrator");
    SetTargetFPS(60);

    LoadScenes();
    LoadSounds();
    loadMusic();

    filesystem = Directory::CreateFileSystem();
    terminal = Terminal(filesystem);
}

void Game::Run() {
    Initialize();

    PlayMusicStream(music[0]);
    bool hasStartedTerminalMusic = false;

    while (!WindowShouldClose()) {
        if (!hasStartedTerminalMusic) {
            UpdateMusicStream(music[0]);
        } else {
            UpdateMusicStream(music[1]);
        }

        BeginDrawing();
        ClearBackground(BLACK);

        // weird ass bug
        if (showActionScene1 && IsKeyPressed(KEY_SPACE)) {
            StopMusicStream(music[0]);
            PlaySound(retroPCsounds[1]);
            showActionScene1 = false;
            showRetroBootUp = true;
            while (GetKeyPressed() != 0) {} // Clear keyboard buffer
            terminal.ClearInput();  // Clear terminal input
            continue;
        }

        if (breachLoading) {
            breachLoadTimer -= GetFrameTime();
            DrawBreachLoading();

            if (breachLoadTimer <= 0) {
                breachLoading = false;
                showBreachProtocol = true;
                breachGame->reset();
            }
        }
        else if (showBreachProtocol) {
            DrawBreachProtocol();
        }
        else if (showStartScreen) {
            DrawStartScreen();
        }
        else if (showMissionBrief) {
            DrawMissionBrief();
        }
        else if (showActionScene1) {
            DrawActionScene1();
        }
        else if (showRetroBootUp) {
            DrawRetroBootUp();
            if (!showRetroBootUp) {
                PlayMusicStream(music[1]);
                hasStartedTerminalMusic = true;
            }
        }
        else {
            DrawTerminal();
            terminal.Draw();
        }
        ProcessTerminalInput();
        terminal.Update();

        // Check if we need to restart the game
        if (terminal.shouldRestartGame()) {
            terminal.clearRestartFlag();
            ResetGame();  // This should reset all game state including terminal
            showStartScreen = true;  // Go back to start screen
            continue;
        }

        EndDrawing();
    }
    CloseWindow();
}

void Game::DrawStartScreen() {
    const char* title = "Terminal Infiltrator";
    const char* promptText = "Press any key to start";

    int titleWidth = MeasureText(title, 40);
    int promptWidth = MeasureText(promptText, 20);

    DrawText(title, (screenWidth - titleWidth) / 2, screenHeight / 2 - 50, 40, GREEN);
    DrawText(promptText, (screenWidth - promptWidth) / 2, screenHeight / 2 + 10, 20, GREEN);

    if (GetKeyPressed() > 0) {
        showStartScreen = false;
        showMissionBrief = true;
    }
}

void Game::DrawMissionBrief() {
    int textX = 50;
    int textY = screenHeight / 4;
    int boxPadding = 10;
    int lineHeight = 20;

    int boxWidth = screenWidth - 2 * (textX - boxPadding);
    int boxHeight = screenHeight - 2 * (textY - boxPadding);
    int reservedPromptSpace = 40;
    int textAreaHeight = boxHeight - reservedPromptSpace;
    int visibleLines = (textAreaHeight / lineHeight);

    DrawRectangle(textX - boxPadding, textY - boxPadding, boxWidth, boxHeight, Color{0, 20, 0, 50});

    BeginScissorMode(textX - boxPadding, textY - boxPadding, boxWidth, textAreaHeight);

    // Scanline effect
    static float scanlineOffset = 0;
    scanlineOffset += GetFrameTime() * 30.0f;
    if (scanlineOffset >= lineHeight) scanlineOffset = 0;

    for (int y = textY - boxPadding; y < textY + textAreaHeight; y += lineHeight) {
        int adjustedY = y + (int)scanlineOffset;
        DrawRectangle(textX - boxPadding, adjustedY, boxWidth, 2, Color{0, 255, 0, 10});
    }

    // CRT effect lines
    for (int y = textY - boxPadding; y < textY + textAreaHeight; y += 4) {
        DrawRectangle(textX - boxPadding, y, boxWidth, 1, Color{0, 0, 0, 20});
    }

    // Distortion effect
    static float distortionTime = 0;
    distortionTime += GetFrameTime();
    for (int y = textY - boxPadding; y < textY + textAreaHeight; y += lineHeight * 2) {
        float offset = sin(distortionTime * 2.0f + y * 0.1f) * 2.0f;
        DrawRectangle(textX - boxPadding + (int)offset, y, boxWidth, 1, Color{0, 255, 0, 5});
    }

    // Random glitch effect
    if (GetRandomValue(0, 100) < 2) {
        DrawRectangle(textX - boxPadding, textY - boxPadding,
                     boxWidth, textAreaHeight,
                     Color{0, 255, 0, 5});
    }

    // Typewriter effect
    if (missionTextIndex < missionText.size()) {
        typewriterCounter++;
        if (typewriterCounter >= typewriterSpeed) {
            missionTextIndex++;
            if (missionTextIndex < missionText.size() &&
                missionText[missionTextIndex - 1] != ' ' &&
                missionText[missionTextIndex - 1] != '\n') {
                int randomSoundsIndex = GetRandomValue(0, 3);
                PlaySound(typingSounds[randomSoundsIndex]);
            }
            typewriterCounter = 0;
        }
    }

    int lineCount = 0;
    int scrollOffset = 0;
    std::string visibleText = missionText.substr(0, missionTextIndex);

    for (char c : visibleText) {
        if (c == '\n') lineCount++;
    }

    if (lineCount > visibleLines) {
        scrollOffset = (lineCount - visibleLines + 1) * lineHeight;
    }

    DrawText(visibleText.c_str(), textX, textY - scrollOffset, 18, GREEN);

    EndScissorMode();

    if (missionTextIndex >= missionText.size()) {
        const char* continueText = "[ MISSION START: SPACE_BAR ]";
        int continueWidth = MeasureText(continueText, 20);

        static float flashCounter = 0;
        flashCounter += GetFrameTime() * 6.0f;
        float alpha = (sin(flashCounter) + 1.0f) * 0.5f;
        Color flashColor = Color{150, 255, 150, static_cast<unsigned char>(255 * alpha)};

        DrawText(continueText,
                 (screenWidth - continueWidth) / 2,
                 textY + textAreaHeight - 10,
                 20,
                 flashColor);

        if (IsKeyPressed(KEY_SPACE)) {
            showMissionBrief = false;
            showActionScene1 = true;
        }
    }
}

void Game::DrawActionScene1() {
    const int padding = 20;
    float scale = fmin((float)(screenWidth - 2 * padding) / actionScene1.width,
                       (float)(screenHeight - 100 - 2 * padding) / actionScene1.height);

    int drawWidth = (int)(actionScene1.width * scale);
    int drawHeight = (int)(actionScene1.height * scale);
    int drawX = (screenWidth - drawWidth) / 2;
    int drawY = (screenHeight - drawHeight) / 2 - 20;

    DrawRectangle(0, 0, screenWidth, screenHeight, Color{0, 20, 0, 50});

    static float maskY = drawHeight;
    float maskWidth = drawWidth * 1.2f;
    float maskHeight = drawHeight / 2.0f;
    float scrollSpeed = 20;

    if (maskY > 0) {
        maskY -= GetFrameTime() * scrollSpeed;
    } else {
        DrawRectangle(0, 0, screenWidth, screenHeight, Color{0, 20, 0, 50});

        const char* completeText = "[ DESCENT COMPLETE: PRESS SPACE TO CONTINUE ]";
        int completeWidth = MeasureText(completeText, 20);

        DrawText(completeText,
                 (screenWidth - completeWidth) / 2,
                 screenHeight - 40,
                 20,
                 Color{0, 255, 0, 255});

        if (IsKeyPressed(KEY_SPACE)) {
            //StopMusicStream(music[0]);
            //PlaySound(retroPCsounds[1]);
            showActionScene1 = false;
            showRetroBootUp = true;
        }
        return;
    }

    BeginScissorMode(drawX - (int)((maskWidth - drawWidth) / 2),
                     drawY + (int)maskY - maskHeight, (int)maskWidth, (int)maskHeight);

    DrawTexturePro(actionScene1,
                   Rectangle{0, 0, (float)actionScene1.width, (float)actionScene1.height},
                   Rectangle{(float)drawX, (float)drawY, (float)drawWidth, (float)drawHeight},
                   Vector2{0, 0}, 0.0f, Color{0, 255, 0, 255});

    EndScissorMode();

    static float scanlineOffset = 0;
    scanlineOffset += GetFrameTime() * 30.0f;
    if (scanlineOffset >= 20) scanlineOffset = 0;

    for (int y = drawY; y < drawY + drawHeight; y += 20) {
        int adjustedY = y + (int)scanlineOffset;
        DrawRectangle(drawX, adjustedY, drawWidth, 2, Color{0, 255, 0, 10});
    }

    static float distortionTime = 0;
    distortionTime += GetFrameTime();
    for (int y = drawY; y < drawY + drawHeight; y += 4) {
        float offset = sin(distortionTime * 2.0f + y * 0.1f) * 2.0f;
        DrawRectangle(drawX + (int)offset, y, drawWidth, 1, Color{0, 255, 0, 5});
    }

    if (GetRandomValue(0, 100) < 2) {
        DrawRectangle(drawX, drawY, drawWidth, drawHeight, Color{0, 255, 0, 5});
    }
}

void Game::DrawRetroBootUp() {
    const int padding = 20;
    int terminalWidth = screenWidth - 2 * padding;
    int terminalHeight = screenHeight - 2 * padding;
    static float bootTimer = 0.0f;
    static bool hasDrawnPowerOn = false;
    static bool hasSoundPlayed = false;
    static float powerOnEffect = 0.0f;
    static int scrollOffset = 0;
    static int memoryProgress = 0;
    static std::vector<std::string> bootMessages{
        "SCO UNIX System V/386 Release 3.2",
        "",
        "CPU Type: i486DX",
        "Clock Speed: 66 MHz",
        "",
        "Performing memory test...",
        "Memory Test:     ",
    };

    static std::vector<std::string> postMemoryMessages{
        "",
        "Memory Configuration:",
        "Base Memory:    640K",
        "Extended:      7168K",
        "Cache:          256K",
        "Total Memory:   8064K",
        "",
        "Hardware Detection:",
        "Serial Port (tty0).........Detected",
        "Serial Port (tty1).........Detected",
        "Parallel Port (lp0)........Detected",
        "",
        "Initializing File Systems...",
        "Checking root filesystem...",
        "/ ...........................OK",
        "/usr ........................OK",
        "/var ........................OK",
        "",
        "Starting UNIX services:",
        "syslogd ................. Running",
        "inetd ................... Running",
        "cron .................... Running",
        "",
        "INIT: Entering runlevel 3",
        "",
        "Unix System V Release 3.2",
        "login: _"
    };

    if (!hasSoundPlayed) {
        PlaySound(retroPCsounds[1]);
        hasSoundPlayed = true;
    }

    bootTimer += GetFrameTime();

    DrawRectangle(0, 0, screenWidth, screenHeight, Color{0, 20, 0, 50});

    if (!hasDrawnPowerOn) {
        powerOnEffect += GetFrameTime() * 2.0f;
        if (powerOnEffect >= 1.0f) {
            hasDrawnPowerOn = true;
        }
        int linePos = screenHeight * (1.0f - powerOnEffect);
        DrawRectangle(0, linePos, screenWidth, 4, Color{0, 255, 0, 180});
        return;
    }

    DrawRectangleRounded(
        Rectangle{(float)padding, (float)padding,
                  (float)terminalWidth, (float)terminalHeight},
        0.1f,
        20,
        Color{0, 40, 0, 100}
    );

    BeginScissorMode(padding, padding, terminalWidth, terminalHeight);

    static float scanlineOffset = 0;
    scanlineOffset += GetFrameTime() * 30.0f;
    if (scanlineOffset >= 20) scanlineOffset = 0;

    for (int y = padding; y < screenHeight - padding; y += 20) {
        int adjustedY = y + (int)scanlineOffset;
        DrawRectangle(padding, adjustedY, terminalWidth, 2, Color{0, 255, 0, 10});
    }

    for (int y = padding; y < screenHeight - padding; y += 4) {
        DrawRectangle(padding, y, terminalWidth, 1, Color{0, 0, 0, 20});
    }

    static float distortionTime = 0;
    distortionTime += GetFrameTime();
    for (int y = padding; y < screenHeight - padding; y += 40) {
        float offset = sin(distortionTime * 2.0f + y * 0.1f) * 2.0f;
        DrawRectangle(padding + (int)offset, y, terminalWidth, 1, Color{0, 255, 0, 5});
    }

    if (GetRandomValue(0, 100) < 3) {
        DrawRectangle(padding, padding, terminalWidth, terminalHeight, Color{0, 255, 0, 5});
    }

    if (bootTimer > 2.0f && bootTimer < 6.0f) {
        float progress = (bootTimer - 2.0f) / 4.0f;
        memoryProgress = (int)(progress * 100.0f);
        if (memoryProgress > 100) memoryProgress = 100;
        bootMessages[6] = "Memory Test: " + std::to_string(memoryProgress) + "%";
    } else if (bootTimer >= 5.5f) {
        bootMessages[6] = "Memory Test: 100%";

        int postMemIndex = (bootTimer - 4.0f) / 0.45f;
        for(int i = 0; i <= postMemIndex && i < postMemoryMessages.size(); i++) {
            bootMessages.push_back(postMemoryMessages[i]);
        }
    }

    int lineHeight = 25;
    int visibleLines = (terminalHeight - padding) / lineHeight;

    int totalMessages = bootMessages.size();
    if (totalMessages > visibleLines) {
        scrollOffset = (totalMessages - visibleLines + 1) * lineHeight;
    }

    for (int i = 0; i < bootMessages.size(); i++) {
        int yPos = 40 + i * lineHeight - scrollOffset;
        if (yPos >= padding && yPos < terminalHeight) {
            DrawText(bootMessages[i].c_str(), 40, yPos, 20, GREEN);
        }
    }

    EndScissorMode();

    if (bootTimer > 16.0f) {
        showRetroBootUp = false;
        hasDrawnPowerOn = false;
        hasSoundPlayed = false;
        powerOnEffect = 0.0f;
        bootTimer = 0.0f;
        memoryProgress = 0;
        scrollOffset = 0;
        bootMessages = std::vector<std::string>{
            "SCO UNIX System V/386 Release 3.2",
            "",
            "CPU Type: i486DX",
            "Clock Speed: 66 MHz",
            "",
            "Performing memory test...",
            "Memory Test:     "
        };
    }
}

void Game::DrawTerminal() {
    const int textX = 25;  // Reduced to give more padding
    const int textY = 25;  // Reduced to give more padding
    const int padding = 20;
    int terminalWidth = screenWidth - 2 * padding;
    int terminalHeight = screenHeight - 2 * padding;
    int terminalFontSize = 20;  // Reduced font size for better readability in small window

    // Background and terminal window
    DrawRectangle(0, 0, screenWidth, screenHeight, Color{0, 20, 0, 50});
    DrawRectangleRounded(
        Rectangle{(float)padding, (float)padding,
                  (float)terminalWidth, (float)terminalHeight},
        0.1f,
        20,
        Color{0, 40, 0, 100}
    );

    int lineSpacing = terminalFontSize + 3;  // Slightly increased line spacing for readability
    int maxVisibleLines = (terminalHeight - textY) / lineSpacing;

    BeginScissorMode(padding, padding, terminalWidth, terminalHeight);

    // Scanline effect
    static float scanlineOffset = 0;
    scanlineOffset += GetFrameTime() * 30.0f;
    if (scanlineOffset >= terminalFontSize) scanlineOffset = 0;
    for (int y = padding; y < screenHeight - padding; y += terminalFontSize) {
        int adjustedY = y + (int)scanlineOffset;
        DrawRectangle(padding, adjustedY, terminalWidth, 2, Color{0, 255, 0, 10});
    }

    // Distortion effect
    static float distortionTime = 0;
    distortionTime += GetFrameTime();
    for (int y = padding; y < screenHeight - padding; y += 4) {
        DrawRectangle(padding, y, terminalWidth, 1, Color{0, 0, 0, 20});
        float offset = sin(distortionTime * 2.0f + y * 0.1f) * 2.0f;
        DrawRectangle(padding + (int)offset, y, terminalWidth, 1, Color{0, 255, 0, 5});
    }

    // Random glitch effect
    if (GetRandomValue(0, 100) < 2) {
        DrawRectangle(padding, padding, terminalWidth, terminalHeight, Color{0, 255, 0, 5});
    }

    // Get terminal output and handle scrolling
    const std::vector<std::string>& output = terminal.GetOutput();
    int scrollOffset = terminal.GetScrollOffset();
    int totalLines = output.size();

    // Adjust scrollOffset to ensure it doesn’t exceed bounds
    if (scrollOffset > totalLines - maxVisibleLines) {
        scrollOffset = std::max(0, totalLines - maxVisibleLines);
    }

    // Calculate visible range considering scroll offset
    int startLine = std::max(0, totalLines - maxVisibleLines - scrollOffset);
    int endLine = std::min(totalLines, startLine + maxVisibleLines);

    // Draw terminal output with adjusted line spacing
    int yPosition = textY;
    for (int i = startLine; i < endLine; i++) {
        DrawText(output[i].c_str(), textX, yPosition, terminalFontSize, GREEN);
        yPosition += lineSpacing;
    }

    // Draw input line and cursor only if we're at the bottom
    if (scrollOffset == 0) {
        std::string inputLine = terminal.GetPrompt() + terminal.GetInput();
        DrawText(inputLine.c_str(), textX, yPosition, terminalFontSize, GREEN);

        static float cursorTime = 0;
        cursorTime += GetFrameTime() * 4.0f;
        if (sin(cursorTime) > -0.2f) {
            int cursorX = textX + MeasureText(inputLine.c_str(), terminalFontSize);
            DrawRectangle(cursorX, yPosition + 2,
                         terminalFontSize / 2, terminalFontSize - 4,
                         Color{0, 255, 0, static_cast<unsigned char>(200 + sin(cursorTime * 2) * 55)});
        }
    }

    // Only show UP indicator if we can scroll up
    if (scrollOffset < totalLines - maxVisibleLines) {
        DrawText("(UP)", screenWidth - padding - 60, padding + 10, terminalFontSize, GREEN);
    } else {
        DrawText("(UP)", screenWidth - padding - 60, padding + 10, terminalFontSize, Color{0, 255, 0, 128});
    }

    // Only show DOWN indicator if we can scroll downxxd
    if (scrollOffset > 0) {
        DrawText("(DN)", screenWidth - padding - 55, screenHeight - padding - 40, terminalFontSize, GREEN);
    } else {
        DrawText("(DN)", screenWidth - padding - 55, screenHeight - padding - 40, terminalFontSize, Color{0, 255, 0, 128});
    }

    EndScissorMode();
    terminal.Draw();
    // Ensure input and scroll handling are called each frame
    ProcessTerminalInput();
    terminal.ProcessScrollInput();  // Handle scrolling input
}

void Game::LoadScenes() {
    TraceLog(LOG_INFO, "Starting to load scenes");

    const char* imagePath = "/home/jay/workspace/raylibTesting/assets/scenes/scene01.png";

    if (!FileExists(imagePath)) {
        TraceLog(LOG_ERROR, "File not found: %s", imagePath);
        scenesLoaded = false;
        return;
    }

    Image img = LoadImage(imagePath);
    if (img.data == NULL) {
        TraceLog(LOG_ERROR, "Failed to load image data");
        scenesLoaded = false;
        return;
    }

    actionScene1 = LoadTextureFromImage(img);
    UnloadImage(img);

    if (actionScene1.id == 0) {
        TraceLog(LOG_ERROR, "Failed to create texture from image");
        scenesLoaded = false;
    } else {
        TraceLog(LOG_INFO, "Texture loaded successfully: ID %d (%dx%d)",
                actionScene1.id, actionScene1.width, actionScene1.height);
        scenesLoaded = true;
    }
}

void Game::UnloadScenes() {
    if (actionScene1.id != 0) {
        UnloadTexture(actionScene1);
        actionScene1.id = 0;
    }

    if (shaderLoaded) {
        UnloadShader(greenTintShader);
        greenTintShader.id = 0;
        shaderLoaded = false;
    }

    TraceLog(LOG_INFO, "Scenes and shaders unloaded successfully");
}

void Game::LoadSounds() {
    typingSounds[0] = LoadSound("/home/jay/workspace/raylibTesting/assets/sound/keypress27.wav");
    typingSounds[1] = LoadSound("/home/jay/workspace/raylibTesting/assets/sound/keypress28.wav");
    typingSounds[2] = LoadSound("/home/jay/workspace/raylibTesting/assets/sound/keypress29.wav");
    typingSounds[3] = LoadSound("/home/jay/workspace/raylibTesting/assets/sound/keypress30.wav");
    retroPCsounds[0] = LoadSound("/home/jay/workspace/raylibTesting/assets/sound/retroPCButtonPressed.wav");
    retroPCsounds[1] = LoadSound("/home/jay/workspace/raylibTesting/assets/sound/retroPCBootUpSeq.wav");
    floppySound = LoadSound("/home/jay/workspace/raylibTesting/assets/sound/floppySound.wav");
    }

void Game::UnloadSounds() {
    for (int i = 0; i < 4; i++) {
        UnloadSound(typingSounds[i]);
    }
    for (int i = 0; i < 2; i++) {
        UnloadSound(retroPCsounds[i]);
    }
}

void Game::loadMusic() {
    music[0] = LoadMusicStream("/home/jay/workspace/raylibTesting/assets/music/Cypher.mp3");
    music[1] = LoadMusicStream("/home/jay/workspace/raylibTesting/assets/music/evasion.mp3");
    SetMusicVolume(music[0], 0.35f);
    SetMusicVolume(music[1], 0.25f);
}

void Game::unloadMusic() {
    for (int i = 0; i < 2; i++) {
        UnloadMusicStream(music[i]);
    }
}

void Game::SetupFilesystem() {
    if (filesystem) {
        delete filesystem;
    }
    filesystem = Directory::CreateFileSystem();
}

void Game::InitializeEndingTexts() {
    endingTexts[GameEnding::BAD_ENDING] = BAD_ENDING_TEXT;
    endingTexts[GameEnding::GOOD_ENDING] = GOOD_ENDING_TEXT;
}

void Game::HandleEndingState() {
    endingDialog.Update();
    if (endingDialog.IsVisible()) {
        endingDialog.Draw();

        // If dialog was closed with ESC, restart the game
        if (endingDialog.isRestartRequested()) {
            ResetGame();  // Restart the game
            endingDialog.clearRestartRequest();  // Clear the request flag
        }
    }
}

void Game::ResetGame() {
    showStartScreen = true;
    terminal = Terminal(Directory::CreateFileSystem());
    breachAttempts = 3;
    Directory::setClue1(false);
    Directory::setClue2(false);
    Directory::setClue3(false);
    Directory::setNukeCodes(false);
    currentEnding = GameEnding::NONE;
}

void Game::ShowEnding(const std::string& message) {
    currentEnding = Directory::hasFoundAllClues() ? GameEnding::GOOD_ENDING : GameEnding::BAD_ENDING;
    endingDialog.Show(endingTexts[currentEnding]);
    terminal.lockSystem();
}
