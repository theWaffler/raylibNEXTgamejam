#pragma once
#include <string>
#include <vector>
#include "Directory.h"
#include "PopupDialog.h"

class Terminal {
private:
    Directory* currentDir;
    Directory* rootDir;
    std::vector<std::string> output;
    std::string currentInput;
    std::string prompt;
    int scrollOffset;
    int maxScrollback;

    // Scroll handling variables
    int linesPerPage = 10;
    int currentScrollPosition = 0;
    std::vector<std::string> catContent;

    // Private methods
    void UpdatePrompt();
    int CountFiles(Directory* dir, bool includeHidden) const;

    // Command execution methods
    void ExecuteLS(const std::string& args);
    void ExecuteCD(const std::string& path);
    void ExecutePWD();
    void ExecuteCAT(const std::string& filename);
    void ExecuteXXD(const std::string& filename);
    void ExecuteHelp();
    void ProcessAnalyzeCommand(const std::string& filename);
    void ProcessSSHCommand(const std::string& command);

    // State flags
    bool m_isLocked = false;
    bool m_initiateBreachProtocol = false;
    bool m_isRemoteServer = false;
    Directory* previousDir = nullptr;

    // Dialog handling
    PopupDialog* messageDialog;
    PopupDialog* storyDialog;
    bool waitingForDecision;

    // restart game
    bool m_shouldRestart = false;

public:
    Terminal(Directory* root);
    void HandleInput(int key);
    void ProcessCommand(const std::string& command);
    void BackspaceInput();
    void ClearInput();

    // Destructor
    ~Terminal();

    // Accessors
    const std::vector<std::string>& GetOutput() const { return output; }
    std::string GetInput() const { return currentInput; }
    const std::string& GetPrompt() const { return prompt; }
    int GetScrollOffset() const { return scrollOffset; }

    // Scroll control
    void ScrollUp();
    void ScrollDown();
    void ProcessScrollInput();
    void displayContent();

    // System state methods
    void lockSystem() { m_isLocked = true; }
    bool isSystemLocked() const { return m_isLocked; }
    bool shouldInitiateBreachProtocol() const { return m_initiateBreachProtocol; }
    void clearBreachProtocolFlag() { m_initiateBreachProtocol = false; }
    bool isRemoteServer() const { return m_isRemoteServer; }
    void setRemoteServer(bool value) {
        m_isRemoteServer = value;
        if (!value && previousDir) {
            // Clean up SHADOW_SERVER directory
            if (currentDir && currentDir->getName() == "ALLIANCE_COMMAND_SECURE_SERVER") {
                delete currentDir;
            }
            // Return to previous directory
            currentDir = previousDir;
            previousDir = nullptr;
            UpdatePrompt();
        }
    }

    void addOutput(const std::string& message) {
        output.push_back(message);
    }

    void updateTerminalPrompt() {
        UpdatePrompt();
    }

    void updatePromptAndOutput(const std::string& outputMsg) {
        addOutput(outputMsg);
        UpdatePrompt();
    }

    Directory* getCurrentDir() const { return currentDir; }

    // Dialog handling
    void Update();
    void Draw();
    bool hasActiveDialog() const {
        return (messageDialog != nullptr && messageDialog->IsVisible()) ||
               (storyDialog != nullptr && storyDialog->IsVisible());
    }

    // restart
    bool shouldRestartGame() const { return m_shouldRestart; }
    void clearRestartFlag() { m_shouldRestart = false; }
};
