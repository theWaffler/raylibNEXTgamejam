#include "Terminal.h"
#include <raylib.h>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <thread>

/*
Terminal::Terminal(Directory* root)
    : currentDir(root), rootDir(root), currentInput(""),
      scrollOffset(0), maxScrollback(1000), currentScrollPosition(0),
      linesPerPage(10), waitingForDecision(false) {
    output.push_back("Terminal initialized. Type '--help' for commands.");
    messageDialog = new PopupDialog();
    storyDialog = new PopupDialog();
    UpdatePrompt();
}
*/

Terminal::Terminal(Directory* root)
    : currentDir(root), rootDir(root), currentInput(""),
      scrollOffset(0), maxScrollback(1000), currentScrollPosition(0),
      linesPerPage(10), waitingForDecision(false) {
    output.push_back("Terminal initialized. Type '--help' for commands.");

    // Initialize dialog pointers with debug output
    //output.push_back("DEBUG: Initializing message dialog");
    messageDialog = new PopupDialog();
    if (!messageDialog) {
        output.push_back("ERROR: Failed to create message dialog");
    }

    //output.push_back("DEBUG: Initializing story dialog");
    storyDialog = new PopupDialog();
    if (!storyDialog) {
        output.push_back("ERROR: Failed to create story dialog");
    }

    UpdatePrompt();
}

Terminal::~Terminal() {}

void Terminal::ProcessScrollInput() {
    if (IsKeyPressed(KEY_PAGE_UP) || IsKeyPressed(KEY_UP)) {
        ScrollUp();
    }
    if (IsKeyPressed(KEY_PAGE_DOWN) || IsKeyPressed(KEY_DOWN)) {
        ScrollDown();
    }
}

void Terminal::ScrollUp() {
    if (scrollOffset < output.size() - 1) {
        scrollOffset++;
    }
}

void Terminal::ScrollDown() {
    if (scrollOffset > 0) {
        scrollOffset--;
    }
}

void Terminal::displayContent() {
    BeginDrawing();
    ClearBackground(BLACK);

    int visibleLines = std::min(linesPerPage, static_cast<int>(output.size()) - scrollOffset);
    int yPos = 20;

    for (int i = 0; i < visibleLines; ++i) {
        DrawText(output[output.size() - 1 - scrollOffset - i].c_str(), 10, yPos, 20, GREEN);
        yPos += 20;
    }
    EndDrawing();
}

/* old
void Terminal::ExecuteCAT(const std::string& filename) {
    Directory* fileNode = currentDir->FindFile(filename);
    if (!fileNode) {
        output.push_back("cat: " + filename + ": No such file");
        return;
    }

    std::string content = fileNode->getVisibleContent();
    std::istringstream stream(content);
    std::string line;
    catContent.clear();

    // Clear previous output and reset scroll position for fresh display
    output.push_back(""); // Add a blank line for spacing
    while (std::getline(stream, line)) {
        output.push_back(line);
    }
    scrollOffset = 0; // Reset scroll offset for new content
    displayContent(); // Initial display of content
}
*/

void Terminal::ExecuteCAT(const std::string& filename) {
    Directory* fileNode = currentDir->FindFile(filename);
    if (!fileNode) {
        output.push_back("cat: " + filename + ": No such file");
        return;
    }

    // Special handling for launch codes
    if (filename == "codes.txt" && m_isRemoteServer) {
        output.push_back("DEBUG: Found codes.txt in remote server");
        if (!messageDialog) {
            messageDialog = new PopupDialog();
        }
        std::string choiceText =
            "NUCLEAR LAUNCH CODES\n"
            "===================\n"
            "Authorization: ALPHA-ZULU-9\n"
            "Confirmation: OMEGA-DELTA-4\n"
            "Target Coordinates: [CLASSIFIED]\n"
            "Launch Window: IMMEDIATE\n\n"
            "What would you like to do?\n\n"
            "1: Submit codes to Regime Command\n"
            "2: Continue exploring the system";

        if (Directory::hasFoundAllClues()) {
            choiceText += "\n3: Send modified launch codes";
        }
        choiceText += "\n\nPress number key to choose";

        output.push_back("DEBUG: Attempting to show message dialog");
        messageDialog->Show(choiceText);
        output.push_back("DEBUG: Message dialog shown");
        waitingForDecision = true;
        return;
    }

    // Regular file display
    std::string content = fileNode->getVisibleContent();
    std::istringstream stream(content);
    std::string line;

    output.push_back("");
    while (std::getline(stream, line)) {
        output.push_back(line);
    }
    scrollOffset = 0;
    displayContent();
}

void Terminal::HandleInput(int key) {

    if (key == KEY_ESCAPE) {
        m_shouldRestart = true;
    }

    if (key >= 32 && key <= 126) {  // Printable ASCII characters
        currentInput += static_cast<char>(key);
    }
}

void Terminal::BackspaceInput() {
    if (!currentInput.empty()) {
        currentInput.pop_back();
    }
}

void Terminal::ClearInput() {
    currentInput.clear();
}

void Terminal::ExecuteLS(const std::string& args) {
    bool showHidden = args.find("-a") != std::string::npos;
    bool showDetail = args.find("-l") != std::string::npos;

    Directory* node = currentDir->getLeftChild();

    if (!node) {
        output.push_back("Directory is empty.");
        return;
    }

    int totalFiles = CountFiles(currentDir, showHidden);
    if (showDetail) {
        output.push_back("total " + std::to_string(totalFiles));
    }

    while (node) {
        if (showHidden || !node->getIsHidden()) {
            if (showDetail) {
                output.push_back(node->GetDetailedInfo());
            } else {
                output.push_back(node->getName() + (node->getIsDirectory() ? "/" : ""));
            }
        }
        node = node->getRightSibling();
    }
}

void Terminal::ExecuteCD(const std::string& path) {
    if (path.empty() || path == "~") {
        currentDir = rootDir;
        UpdatePrompt();
        return;
    }

    if (path == "..") {
        if (currentDir->getParent()) {
            currentDir = currentDir->getParent();
            UpdatePrompt();
        }
        return;
    }

    Directory* node = currentDir->getLeftChild();
    while (node) {
        if (node->getName() == path && node->getIsDirectory()) {
            // Check if directory is locked before allowing access
            if (node->isDirectoryLocked()) {
                output.push_back("Access denied: Directory is locked.");
                //output.push_back("Use 'breach " + path + "' to attempt access.");
                return;
            }
            currentDir = node;
            UpdatePrompt();
            return;
        }
        node = node->getRightSibling();  // Use getter method instead
    }

    output.push_back("cd: " + path + ": No such directory");
}

void Terminal::ExecutePWD() {
    std::string path;
    Directory* temp = currentDir;
    while (temp) {
        path = "/" + temp->getName() + path;
        temp = temp->getParent();
    }
    output.push_back(path.empty() ? "/" : path);
}

void Terminal::ExecuteXXD(const std::string& filename) {
    Directory* fileNode = currentDir->FindFile(filename);
    if (!fileNode) {
        output.push_back("xxd: " + filename + ": No such file");
        return;
    }

    std::string content = fileNode->getFullContent();
    std::stringstream hexDump;
    hexDump << std::hex << std::setfill('0');

    const int maxBytes = 512;  // Limit to the first 128 bytes
    const int bytesPerLine = 14;  // Display 8 bytes per line for balanced view

    // Limit content to `maxBytes`
    int length = std::min(static_cast<int>(content.length()), maxBytes);

    for (int i = 0; i < length; i += bytesPerLine) {
        // Address with a wider field
        hexDump << std::setw(6) << i << ": ";

        // Hex values with extra spacing between bytes
        for (int j = 0; j < bytesPerLine; j++) {
            if (i + j < length) {
                hexDump << std::setw(1) << static_cast<int>(static_cast<unsigned char>(content[i + j])) << " ";
            } else {
                hexDump << "|";  // Padding for alignment if line is short
            }
        }

        // ASCII representation with extra space between hex and ASCII sections
        hexDump << "|";
        for (int j = 0; j < bytesPerLine && i + j < length; j++) {
            char c = content[i + j];
            hexDump << (isprint(c) ? c : '.');
        }
        hexDump << "|";

        output.push_back(hexDump.str());
        hexDump.str("");  // Clear the stringstream
        hexDump.clear();  // Reset any error flags
    }

    if (length < content.length()) {
        output.push_back("... [truncated] ...");  // Indicate truncation if content is larger
    }
}

void Terminal::ExecuteHelp() {
    output.push_back("Available commands:");
    output.push_back("  ls [-a]        : List HIDDEN files & directories");
    output.push_back("  cd <dir>       : Change directory");
    output.push_back("  pwd            : Print working directory");
    output.push_back("  cat <file>     : Display file contents");
    output.push_back("  xxd <file>     : Display file contents in hex");
    //output.push_back("  analyze <file> : Analyze network configuration");
    output.push_back("  breach <dir>   : Initiate ICE breach protocol");
    output.push_back("  ssh <user@ip> -p <port> : Connect to remote server");
    output.push_back("  clear          : Clear the terminal");
    output.push_back("  help           : Display this help message");
}

int Terminal::CountFiles(Directory* dir, bool includeHidden) const {
    int count = 0;
    Directory* node = dir->getLeftChild();
    while (node) {
        if (includeHidden || !node->getIsHidden()) {
            count++;
        }
        node = node->getRightSibling();
    }
    return count;
}

void Terminal::UpdatePrompt() {
    std::string path;
    Directory* temp = currentDir;
    while (temp && temp->getName() != "root") {
        path = "/" + temp->getName() + path;
        temp = temp->getParent();
    }
    prompt = "root@alliance1:" + (path.empty() ? "/" : path) + "$ ";
}

/* old
void Terminal::ProcessCommand(const std::string& command) {
    if (command.empty()) {
        return;
    }

    // Check if system is locked
    if (m_isLocked) {
        output.push_back("ERROR: System locked - Security breach detected");
        return;
    }

    // Add command to output history
    output.push_back(prompt + command);

    // Parse command and arguments
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;

    std::string args;
    std::getline(iss, args);
    if (!args.empty()) args = args.substr(1);

    // Process commands
    if (cmd == "help" || cmd == "--help") {
        ExecuteHelp();
    }
    else if (cmd == "ls") {
        ExecuteLS(args);
    }
    else if (cmd == "cd") {
        ExecuteCD(args);
    }
    else if (cmd == "pwd") {
        ExecutePWD();
    }
    else if (cmd == "cat") {
        // Special handling for codes.txt in remote server
        if (args == "codes.txt" && m_isRemoteServer) {
            ExecuteCAT(args);
            m_showEndingDialog = true;
            return;
        }
        // Check for clue files
        else if (args == "regime_activities.txt") {
            ExecuteCAT(args);
            if (currentDir->FindFile(args)) {
                Directory::setClue1(true);
                Directory::setPlayerThought("...These are our humanitarian missions?", 5.0f);
            }
        }
        else if (args == "intercepted_comms.log") {
            ExecuteCAT(args);
            if (currentDir->FindFile(args)) {
                Directory::setClue2(true);
                Directory::setPlayerThought("...We're using aid centers for targeting?", 5.0f);
            }
        }
        else if (args == "operation_truth.enc") {
            ExecuteCAT(args);
            if (currentDir->FindFile(args)) {
                Directory::setClue3(true);
                Directory::setPlayerThought("...50 million civilians... This can't be right...", 5.0f);
            }
        }
        else {
            ExecuteCAT(args);
        }
    }
    else if (cmd == "xxd" || cmd == "hexdump") {
        ExecuteXXD(args);
    }
    else if (cmd == "clear") {
        output.clear();
    }
    else if (cmd == "analyze") {
        ProcessAnalyzeCommand(args);
    }
    else if (cmd == "ssh") {
        ProcessSSHCommand(command);
    }

    else if (cmd == "breach") {
        // First try to find it as a directory by searching through children
        Directory* node = currentDir->getLeftChild();
        bool found = false;
        while (node) {
            if (node->getName() == args) {
                found = true;
                if (node->isDirectoryLocked()) {
                    output.push_back("Initiating ICE breach protocol...");
                    m_initiateBreachProtocol = true;
                } else {
                    output.push_back("breach: target is not locked: " + args);
                }
                return;
            }
            node = node->getRightSibling();  // Use getter method instead
        }

        if (!found) {
            output.push_back("breach: target not found: " + args);
        }
    }

    else if (cmd == "exit" && m_isRemoteServer) {
        output.push_back("Disconnecting from remote server...");
        m_isRemoteServer = false;
        // Reset to original directory
        while (currentDir->getParent() != nullptr) {
            currentDir = currentDir->getParent();
        }
        UpdatePrompt();
    }
    else {
        output.push_back(cmd + ": command not found");
    }

    // Post-command processing
    if (m_gameOver) {
        output.push_back("\nCRITICAL ERROR: Terminal locked");
        output.push_back("Security breach detected - System shutdown imminent");
    }
}
*/

void Terminal::ProcessCommand(const std::string& command) {
    if (command.empty()) {
        return;
    }
    // Check if system is locked
    if (m_isLocked) {
        output.push_back("ERROR: System locked - Security breach detected");
        return;
    }
    // Add command to output history
    output.push_back(prompt + command);
    // Parse command and arguments
    std::istringstream iss(command);
    std::string cmd;
    iss >> cmd;
    std::string args;
    std::getline(iss, args);
    if (!args.empty()) args = args.substr(1);
    // Process commands
    if (cmd == "help" || cmd == "--help") {
        ExecuteHelp();
    }
    else if (cmd == "ls") {
        ExecuteLS(args);
    }
    else if (cmd == "cd") {
        ExecuteCD(args);
    }
    else if (cmd == "pwd") {
        ExecutePWD();
    }
    else if (cmd == "cat") {
        // Check for clue files
        if (args == "regime_activities.txt") {
            ExecuteCAT(args);
            if (currentDir->FindFile(args)) {
                Directory::setClue1(true);
                Directory::setPlayerThought("...These are our humanitarian missions?", 5.0f);
            }
        }
        else if (args == "intercepted_comms.log") {
            ExecuteCAT(args);
            if (currentDir->FindFile(args)) {
                Directory::setClue2(true);
                Directory::setPlayerThought("...We're using aid centers for targeting?", 5.0f);
            }
        }
        else if (args == "operation_truth.enc") {
            ExecuteCAT(args);
            if (currentDir->FindFile(args)) {
                Directory::setClue3(true);
                Directory::setPlayerThought("...50 million civilians... This can't be right...", 5.0f);
            }
        }
        else {
            ExecuteCAT(args);
        }
    }
    else if (cmd == "xxd" || cmd == "hexdump") {
        ExecuteXXD(args);
    }
    else if (cmd == "clear") {
        output.clear();
    }
    else if (cmd == "analyze") {
        ProcessAnalyzeCommand(args);
    }
    else if (cmd == "ssh") {
        ProcessSSHCommand(command);
    }
    else if (cmd == "breach") {
        // First try to find it as a directory by searching through children
        Directory* node = currentDir->getLeftChild();
        bool found = false;
        while (node) {
            if (node->getName() == args) {
                found = true;
                if (node->isDirectoryLocked()) {
                    output.push_back("Initiating ICE breach protocol...");
                    m_initiateBreachProtocol = true;
                } else {
                    output.push_back("breach: target is not locked: " + args);
                }
                return;
            }
            node = node->getRightSibling();
        }
        if (!found) {
            output.push_back("breach: target not found: " + args);
        }
    }
    else if (cmd == "exit" && m_isRemoteServer) {
        output.push_back("Disconnecting from remote server...");
        m_isRemoteServer = false;
        // Reset to original directory
        while (currentDir->getParent() != nullptr) {
            currentDir = currentDir->getParent();
        }
        UpdatePrompt();
    }
    else {
        output.push_back(cmd + ": command not found");
    }
}

void Terminal::ProcessAnalyzeCommand(const std::string& filename) {
    Directory* file = currentDir->FindFile(filename);
    if (!file || !file->isConfigFile()) {
        output.push_back("Error: Cannot analyze this file type.");
    } else if (!file->hasRemainingAttempts()) {
        output.push_back("No analysis attempts remaining.");
    } else {
        std::string analysis = file->analyzeFile();
        std::istringstream iss(analysis);
        std::string line;

        while (std::getline(iss, line)) {
            output.push_back(line);
            std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Optional delay
        }
    }
}

void Terminal::ProcessSSHCommand(const std::string& command) {
    std::istringstream iss(command);
    std::string cmd, connection, portFlag, port;
    iss >> cmd >> connection >> portFlag >> port;

    size_t atPos = connection.find('@');
    if (atPos == std::string::npos) {
        output.push_back("Invalid format. Use: ssh user@ip -p port");
        return;
    }

    std::string user = connection.substr(0, atPos);
    std::string ip = connection.substr(atPos + 1);

    if (portFlag != "-p") {
        output.push_back("Invalid format. Use: ssh user@ip -p port");
        return;
    }

    output.push_back("Attempting connection to " + ip + "...");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));

    if (user == "admin" && ip == "192.168.1.100" && port == "444") {
        output.push_back("Establishing secure connection...");
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        output.push_back("Authenticating...");
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        output.push_back("Access granted. ALLIANCE_SECURE_SERVER");
        output.push_back("----------------------------------");
        output.push_back("WARNING: This is a restricted system.");
        output.push_back("All activities are being monitored.");

        // Create SHADOW_SERVER directory
        Directory* shadowRoot = Directory::CreateDirectory("ALLIANCE_SECURE_SERVER", nullptr, false, true);
        Directory* codesFile = shadowRoot->addFile("codes.txt",
            "NUCLEAR LAUNCH CODES\n"
            "===================\n"
            "Authorization: ALPHA-ZULU-9\n"
            "Confirmation: OMEGA-DELTA-4\n",
            //"Target Coordinates: [CLASSIFIED]\n"
            //"Launch Window: IMMEDIATE\n",
            false);

        // Store current directory and switch to SHADOW_SERVER
        previousDir = currentDir;
        currentDir = shadowRoot;
        setRemoteServer(true);
        UpdatePrompt();
    } else {
        output.push_back("Connection failed: Invalid credentials");
        output.push_back("Warning: Access attempt has been logged");
    }


}

void Terminal::Update() {
    if (storyDialog && storyDialog->IsVisible()) {
        if (storyDialog && storyDialog->IsVisible()) {
            if (IsKeyPressed(KEY_ESCAPE)) {
                storyDialog->Hide();
                if (m_isLocked) {
                    m_shouldRestart = true;  // Set flag to restart game
                }
            }
            return;
        }
        return;
    }

    if (waitingForDecision && messageDialog->IsVisible()) {
        if (IsKeyPressed(KEY_ONE)) {
            messageDialog->Hide();
            waitingForDecision = false;

            if (!storyDialog) {
                storyDialog = new PopupDialog();
            }

            std::string badEndingText =
                "Submitting launch codes to Regime Command...\n\n"
                "Authorization codes verified.\n"
                "Launch sequence initiated.\n\n"
                "Hours later, nuclear weapons strike Alliance territory.\n"
                "Millions of civilians perish in the attacks.\n"
                "The Regime's 'humanitarian aid' mission is complete.\n\n"
                "BAD ENDING: Unwitting Accomplice\n\n"
                "Press ESC to restart";

            storyDialog->Show(badEndingText);
            Directory::setNukeCodes(true);
            lockSystem();
        }
        else if (IsKeyPressed(KEY_TWO)) {
            messageDialog->Hide();
            waitingForDecision = false;
            setRemoteServer(false);
            output.push_back("\nReturning to local system...");
            output.push_back("You should look around for more information...");
        }
        else if (IsKeyPressed(KEY_THREE) && Directory::hasFoundAllClues()) {
            messageDialog->Hide();
            waitingForDecision = false;

            std::string goodEndingText =
                "With full knowledge of the Regime's true nature...\n\n"
                "You carefully modify the launch codes before transmission.\n"
                "The changes are subtle but will cause complete system failure.\n\n"
                "Hours later...\n"
                "The nuclear launch systems fail across all silos.\n"
                "Millions of civilian lives are saved.\n"
                "Your conscience is clear, but your days are numbered...\n\n"
                "GOOD ENDING: Active Resistance\n\n"
                "Your discovery of the truth led to direct action.\n"
                "The evidence you found gave you the courage to act.\n\n"
                "Press ESC to restart";

            storyDialog->Show(goodEndingText);
            Directory::setNukeCodes(true);
            lockSystem();
        }
    }
}

void Terminal::Draw() {
    if (messageDialog && messageDialog->IsVisible()) {
        messageDialog->Draw();
    }
    if (storyDialog && storyDialog->IsVisible()) {
        storyDialog->Draw();
    }
}
