#pragma once
#include <raylib.h>
#include <string>
#include <map>
#include "BreachProtocol.h"
#include "Terminal.h"
#include "Directory.h"
#include "PopupDialog.h"

class Game {
private:
    int screenWidth;
    int screenHeight;
    bool showStartScreen;
    bool showMissionBrief;
    bool showActionScene1;
    bool showRetroBootUp;
    bool startFadeToTerminal;
    float fadeAlpha;
    size_t missionTextIndex;
    int typewriterSpeed;
    int typewriterCounter;
    int transitionDelay;
    int transitionCounter;
    std::string missionText;

    bool shaderLoaded;
    int scanlineIntensityLoc;
    int greenTintLoc;
    Texture2D actionScene1;
    bool scenesLoaded;
    Shader greenTintShader;

    Sound typingSounds[4];
    Sound retroPCsounds[2];
    Music music[2];

    Directory* filesystem;
    Terminal terminal;

    // Initialization methods
    void Initialize();
    void LoadScenes();
    void UnloadScenes();
    void LoadSounds();
    void UnloadSounds();
    void loadMusic();
    void unloadMusic();
    void SetupFilesystem();
    void ProcessTerminalInput();

    // Drawing methods
    void DrawStartScreen();
    void DrawMissionBrief();
    void DrawActionScene1();
    void DrawRetroBootUp();
    void DrawTerminal();

    //breach protocol members
    BreachProtocol* breachGame;
    bool showBreachProtocol;
    bool breachLoading;
    float breachLoadTimer;
    int breachAttempts;
    Sound floppySound;

    // breach methods
    void DrawBreachLoading();
    void DrawBreachProtocol();
    void ProcessBreachInput();

    // Ending handling - keeping your existing enum class
    enum class GameEnding {
        NONE,
        BAD_ENDING,
        GOOD_ENDING
    };
    GameEnding currentEnding;
    PopupDialog endingDialog;
    std::map<GameEnding, std::string> endingTexts;

    // Ending methods
    void DrawEndingDialog();
    void ShowEnding(const std::string& message);
    void InitializeEndingTexts();
    void HandleEndingState();
    void ResetGame();

    // Add ending texts mapped to your enum
    const std::string GOOD_ENDING_TEXT =
        "With full knowledge of the Regime's true nature...\n"
        "You decide to sabotage the launch codes before submission.\n"
        "The nuclear strike fails.\n"
        "Your conscience is clear, but your days are numbered...\n"
        "\n"
        "OPERATION OUTCOME: ETHICAL INTERVENTION\n"
        "\n"
        "Having discovered the truth about the Regime's atrocities, you chose\n"
        "to act against their plans.\n"
        "\n"
        "CONSEQUENCES:\n"
        "- Launch codes were successfully corrupted before transmission\n"
        "- Nuclear strike failed across all targeted locations\n"
        "- Millions of civilian lives saved from immediate destruction\n"
        "- Evidence of the Regime's true nature has been preserved\n"
        "\n"
        "Your conscious decision to sabotage the operation prevented what would\n"
        "have been one of the largest civilian massacres in history. The evidence\n"
        "you discovered may help bring the truth to light.\n"
        "\n"
        "However, your actions have not gone unnoticed. The Regime's retribution\n"
        "will be swift and merciless. You've sacrificed your safety for the lives\n"
        "of millions.\n"
        "\n"
        "GOOD ENDING: Conscious Objector\n"
        "\n"
        "You chose to stand against tyranny, whatever the personal cost.\n";

    const std::string BAD_ENDING_TEXT =
        "Submitting launch codes to Regime Command...\n"
        "Authorization codes verified.\n"
        "Launch sequence initiated.\n"
        "\n"
        "OPERATION OUTCOME: CATASTROPHIC FAILURE\n"
        "\n"
        "You submitted the launch codes to Regime Command without discovering the truth.\n"
        "\n"
        "CONSEQUENCES:\n"
        "- Nuclear weapons strike Alliance territory with devastating accuracy\n"
        "- Millions of civilian casualties reported across multiple population centers\n"
        "- Humanitarian aid camps and civilian shelters were primary targets\n"
        "- The Regime's true nature remains hidden from the world\n"
        "\n"
        "Your unwitting cooperation helped facilitate one of the largest-scale\n"
        "civilian massacres in history. The Regime's propaganda machine ensures\n"
        "that history remembers this as a 'necessary peacekeeping operation.'\n"
        "\n"
        "BAD ENDING: Unwitting Accomplice\n"
        "\n"
        "Perhaps if you had explored more of the system, you might have\n"
        "discovered the truth about the Regime's true intentions...\n";

public:
    Game(int screenWidth, int screenHeight);
    ~Game();
    void Run();
};
