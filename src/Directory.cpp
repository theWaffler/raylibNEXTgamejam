#include "Directory.h"
#include <iostream>

// Static member initializations
bool Directory::gotNukeCodes = false;
bool Directory::foundClue1 = false;
bool Directory::foundClue2 = false;
bool Directory::foundClue3 = false;
std::string Directory::playerThought = "";
float Directory::thoughtTimer = 0.0f;

Directory::Directory(const std::string& name, Directory* parent, bool isHidden, bool isDir,
                     const std::string& owner, const std::string& group)
    : name(name), parent(parent), leftChild(nullptr), rightSibling(nullptr),
      isHidden(isHidden), owner(owner), group(group), size(4096), isDirectory(isDir),
      isBinary(false), hiddenOffset(0), isNetworkConfig(false), isLocked(false),
      analysisAttempts(MAX_ANALYSIS_ATTEMPTS) {
    lastModified = std::time(nullptr);
    InitializePermissions(isDir);
}

Directory::~Directory() {
    delete leftChild;
    delete rightSibling;
}

void Directory::InitializePermissions(bool isDir) {
    permissions = isDir ? "drwxr-xr-x" : "-rw-r--r--";
}

void Directory::initializeNetworkConfig() {
    isNetworkConfig = true;
    analysisAttempts = MAX_ANALYSIS_ATTEMPTS;

    std::string visibleContent =
        "Server Information:\n"
        "=============================\n"
        "Primary Server\n"
        "Address: ***.***.***.***\n"
        "Username: user\n"
        "Port: 8080\n"
        "Status: Active\n\n"
        "Backup Server\n"
        "Address: 10.0.1.42\n"
        "Username: backup_admin\n"
        "Port: 22\n"
        "Status: Standby\n\n"
        "Network Map:\n"
        "0A:00:01:2A → Primary\n"
        "C0:A8:01:64 → Unknown\n"
        "0A:00:01:42 → Backup\n\n"
        "[NOTE: Run analysis for full system map]";

    setContent(visibleContent);
}

std::string Directory::generateHexDump(const std::string& input) {
    std::stringstream hexDump;
    hexDump << std::hex << std::setfill('0');

    for (size_t i = 0; i < input.length(); i += 16) {
        // Address
        hexDump << std::setw(8) << i << "  ";

        // Hex values
        for (size_t j = 0; j < 16; j++) {
            if (i + j < input.length()) {
                hexDump << std::setw(2) << static_cast<int>(static_cast<unsigned char>(input[i + j])) << " ";
            } else {
                hexDump << "   ";
            }
            if (j == 7) hexDump << " ";
        }

        hexDump << " |";

        // ASCII representation
        for (size_t j = 0; j < 16 && i + j < input.length(); j++) {
            char c = input[i + j];
            hexDump << (isprint(c) ? c : '.');
        }

        hexDump << "|\n";
    }

    return hexDump.str();
}

std::string Directory::analyzeFile() {
    if (!isNetworkConfig || analysisAttempts <= 0) {
        return "Analysis failed or no attempts remaining.";
    }

    analysisAttempts--;

    // Each analysis attempt reveals different information
    switch(analysisAttempts) {
        case 2:
            return
                "=== Network Analysis Tool v2.1 ===\n"
                "Scanning visible network nodes...\n\n"
                "Known Nodes:\n"
                "- Primary (0A:00:01:2A)\n"
                "- Backup (0A:00:01:42)\n"
                "\nAnomaly Detected: Additional MAC address found\n"
                "C0:A8:01:64 - No registered hostname\n"
                "\nNote: Use 'hexdump' command for raw data analysis";

        case 1:
            return
                "=== Network Analysis Tool v2.1 ===\n"
                "Performing deep scan...\n\n"
                "MAC Address Translation:\n"
                "C0:A8:01:64 → 192.168.1.100\n"
                "\nPort Scan Results:\n"
                "192.168.1.100:444 [FILTERED]\n"
                "\nWarning: Connection attempts being logged";

        case 0:
            return generateHexDump(content) +
                "\nAnalysis complete. No further attempts allowed.\n"
                "Tip: Some servers require non-standard ports for SSH connections.";

        default:
            return "No analysis attempts remaining.";
    }
}

std::string Directory::GetDetailedInfo() const {
    char timeStr[80];
    struct tm* timeinfo = localtime(&lastModified);
    strftime(timeStr, sizeof(timeStr), "%b %d %H:%M", timeinfo);

    char buffer[256];
    snprintf(buffer, sizeof(buffer), "%-10s %-3d %-8s %-8s %-5zu %s %s",
             permissions.c_str(), 2, owner.c_str(), group.c_str(),
             size, timeStr, name.c_str());

    return std::string(buffer) + (isDirectory ? "/" : "");
}

Directory* Directory::CreateDirectory(const std::string& name, Directory* parent, bool isHidden, bool isDir) {
    return new Directory(name, parent, isHidden, isDir);
}

Directory* Directory::AddChild(Directory* parent, const std::string& name, bool isHidden, bool isDir) {
    if (!parent) return nullptr;

    Directory* newDir = CreateDirectory(name, parent, isHidden, isDir);
    if (!parent->leftChild) {
        parent->leftChild = newDir;
    } else {
        Directory* current = parent->leftChild;
        while (current->rightSibling) {
            current = current->rightSibling;
        }
        current->rightSibling = newDir;
    }
    return newDir;
}

Directory* Directory::AddSibling(Directory* node, const std::string& name, bool isHidden, bool isDir) {
    if (!node) return nullptr;

    Directory* newDir = CreateDirectory(name, node->parent, isHidden, isDir);
    Directory* current = node;
    while (current->rightSibling) {
        current = current->rightSibling;
    }
    current->rightSibling = newDir;
    return newDir;
}

std::string Directory::DisplayTree(int level, bool isLast) const {
    std::string result;
    std::string indent;

    for (int i = 0; i < level - 1; i++) {
        indent += (i == level - 2) ? "|   " : "    ";
    }

    if (level > 0) {
        indent += isLast ? "+---" : "+---";
    }

    result += indent + name + (isDirectory ? "/" : "") + "\n";

    if (leftChild) {
        Directory* child = leftChild;
        std::vector<Directory*> children;
        while (child) {
            children.push_back(child);
            child = child->rightSibling;
        }

        for (size_t i = 0; i < children.size(); i++) {
            result += children[i]->DisplayTree(level + 1, i == children.size() - 1);
        }
    }
    return result;
}

Directory* Directory::addSubdirectory(const std::string& name, bool isHidden) {
    return AddChild(this, name, isHidden, true);
}

Directory* Directory::addFile(const std::string& name, const std::string& content, bool isBinary) {
    Directory* newFile = new Directory(name, this, false, false);
    newFile->content = content;
    newFile->isBinary = isBinary;
    if (!leftChild) {
        leftChild = newFile;
    } else {
        Directory* current = leftChild;
        while (current->rightSibling) {
            current = current->rightSibling;
        }
        current->rightSibling = newFile;
    }
    return newFile;
}

Directory* Directory::FindFile(const std::string& filename) {
    Directory* node = leftChild;
    while (node) {
        if (node->name == filename) {
            return node;
        }
        node = node->rightSibling;
    }
    return nullptr;
}

std::string Directory::getVisibleContent() const {
    return content.substr(0, hiddenOffset);
}

void Directory::setContent(const std::string& visibleContent) {
    content = visibleContent;
    hiddenOffset = visibleContent.length();
}

Directory* Directory::CreateFileSystem() {
    try {
        Directory* root = CreateDirectory("root", nullptr, false, true);

        // Add standard directories
        Directory* etc = AddChild(root, "etc", false, true);
        Directory* home = AddSibling(etc, "home", false, true);
        Directory* var = AddSibling(home, "var", false, true);
        Directory* opt = AddSibling(var, "opt", false, true);

        // Add .secure directory in user's home
        Directory* homeUser = AddChild(home, "user", false, true);
        Directory* secureDir = AddChild(homeUser, ".secure", true, true);
        secureDir->setLocked(true);

        Directory* serverInfo = AddChild(secureDir, "remoteServer.txt", false, false);
        serverInfo->setContent(
            "ALLIANCE COMMAND SERVER\n"
            "=====================\n"
            "IP: 192.168.1.100\n"
            "PORT: 444\n"
            "USER: admin\n\n"
            "WARNING: ICE Defense System Active\n"
            "Multiple breach attempts will trigger lockdown\n"
            "Nuclear launch codes stored on secure server.\n"
        );

        // Add clue 1 in /etc/.evidence/
        Directory* evidenceDir = AddChild(etc, ".evidence", true, true);
        Directory* evidenceFile = AddChild(evidenceDir, "regime_activities.txt", false, false);
        evidenceFile->setContent(
            "ALLIANCE INTELLIGENCE REPORT\n"
            "===========================\n"
            "SUBJECT: Regime Civilian Operations\n\n"
            "Our operatives have confirmed the Regime's true intentions.\n"
            "The 'humanitarian aid' missions are fronts for systematic\n"
            "civilian elimination operations.\n\n"
            "Confirmed civilian casualties: 10,000+\n"
            "Destroyed infrastructure: 75% of urban centers\n"
            "Chemical weapons deployed: Multiple instances confirmed\n\n"
            "UN inspectors are being actively blocked from sites.\n"
            "Regime maintains public story of humanitarian aid.\n\n"
            "-- Alliance Intelligence Division\n"
        );

        // Add clue 2 in /var/log/.archived_logs/
        Directory* logDir = AddChild(var, "log", false, true);
        Directory* hiddenLogs = AddChild(logDir, ".archived_logs", true, true);
        Directory* clue2File = AddChild(hiddenLogs, "intercepted_comms.log", false, false);
        clue2File->setContent(
            "INTERCEPTED REGIME COMMUNICATION\n"
            "==============================\n"
            "FROM: High Command\n"
            "TO: Field Operations\n\n"
            "Aid centers are successfully masking our operations.\n"
            "Continue using humanitarian convoys for weapons transport.\n"
            "Civilian registration provides excellent targeting data.\n\n"
            "Maintain deniability. Mark all dissidents as 'insurgents'.\n"
            "Prepare population centers for 'final solution'.\n"
        );

        // Add clue 3 in /opt/.internal/
        Directory* hiddenData = AddChild(opt, ".internal", true, true);
        Directory* clue3File = AddChild(hiddenData, "operation_truth.enc", false, false);
        clue3File->setContent(
            "REGIME INTERNAL MEMO - TOP SECRET\n"
            "===============================\n"
            "The Alliance must be eliminated completely.\n"
            "Nuclear deployment will target:\n"
            "- Major population centers\n"
            "- Civilian shelters\n"
            "- Humanitarian aid camps\n\n"
            "Estimated civilian casualties: 50 million+\n"
            "Acceptable losses for regime victory.\n\n"
            "Maintain humanitarian aid narrative until launch.\n"
        );

        return root;

    } catch (const std::exception& e) {
        std::cerr << "Error creating filesystem: " << e.what() << "\n";
        return nullptr;
    }
}
