#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <ctime>

class Directory {
private:
    std::string name;
    std::string content;
    std::string permissions;
    Directory* parent;
    Directory* leftChild;
    Directory* rightSibling;
    bool isHidden;
    bool isDirectory;
    std::string owner;
    std::string group;
    size_t size;
    time_t lastModified;
    bool isBinary;
    size_t hiddenOffset;

    // Network config members
    bool isNetworkConfig;
    int analysisAttempts;
    static const int MAX_ANALYSIS_ATTEMPTS = 3;

    // Private helper methods
    void InitializePermissions(bool isDir);
    void initializeNetworkConfig();
    std::string generateHexDump(const std::string& input);

    bool isLocked;
    static bool gotNukeCodes;
    static bool foundClue1;
    static bool foundClue2;
    static bool foundClue3;
    static std::string playerThought;
    static float thoughtTimer;

public:
    Directory(const std::string& name, Directory* parent = nullptr, bool isHidden = false,
             bool isDir = false, const std::string& owner = "root",
             const std::string& group = "root");
    ~Directory();

    // Accessors
    std::string getName() const { return name; }
    Directory* getParent() const { return parent; }
    Directory* getLeftChild() const { return leftChild; }
    Directory* getRightSibling() const { return rightSibling; }
    bool getIsHidden() const { return isHidden; }
    bool getIsDirectory() const { return isDirectory; }
    std::string getFullContent() const { return content; }

    // Network config methods
    bool isConfigFile() const { return isNetworkConfig; }
    bool hasRemainingAttempts() const { return analysisAttempts > 0; }
    std::string analyzeFile();

    // Filesystem methods
    static Directory* CreateDirectory(const std::string& name, Directory* parent,
                                    bool isHidden, bool isDir);
    static Directory* AddChild(Directory* parent, const std::string& name,
                             bool isHidden, bool isDir);
    static Directory* AddSibling(Directory* node, const std::string& name,
                               bool isHidden, bool isDir);
    static Directory* CreateFileSystem();

    Directory* addSubdirectory(const std::string& name, bool isHidden);
    Directory* addFile(const std::string& name, const std::string& content, bool isBinary);
    Directory* FindFile(const std::string& filename);

    std::string DisplayTree(int level = 0, bool isLast = true) const;
    std::string GetDetailedInfo() const;
    std::string getVisibleContent() const;

    void setContent(const std::string& visibleContent);

    bool isDirectoryLocked() const { return isLocked; }
    void setLocked(bool locked) { isLocked = locked; }

    static bool hasNukeCodes() { return gotNukeCodes; }
    static bool hasFoundClue1() { return foundClue1; }
    static bool hasFoundClue2() { return foundClue2; }
    static bool hasFoundClue3() { return foundClue3; }
    static bool hasFoundAllClues() { return foundClue1 && foundClue2 && foundClue3; }

    static void setNukeCodes(bool value) { gotNukeCodes = value; }
    static void setClue1(bool value) { foundClue1 = value; }
    static void setClue2(bool value) { foundClue2 = value; }
    static void setClue3(bool value) { foundClue3 = value; }

    static void setPlayerThought(const std::string& thought, float duration = 5.0f) {
        playerThought = thought;
        thoughtTimer = duration;
    }

    static bool hasActiveThought() { return thoughtTimer > 0.0f; }
    static const std::string& getCurrentThought() { return playerThought; }
    static void updateThoughtTimer(float deltaTime) {
        if (thoughtTimer > 0.0f) {
            thoughtTimer -= deltaTime;
            if (thoughtTimer <= 0.0f) {
                playerThought.clear();
            }
        }
    }
};
