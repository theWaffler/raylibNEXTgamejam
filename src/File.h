#ifndef FILE_H
#define FILE_H

#include <string>

class File {
public:
    enum ContentType { Text, Binary };

    File(const std::string& name, const std::string& initialContent = "", ContentType type = Text);

    std::string getName() const;
    std::string getContent() const;
    void setContent(const std::string& content);
    ContentType getContentType() const;

private:
    std::string name;
    std::string content;
    ContentType type;
};

#endif // FILE_H
