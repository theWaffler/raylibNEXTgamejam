#include "File.h"

File::File(const std::string& name, const std::string& initialContent, ContentType type)
    : name(name), content(initialContent), type(type) {}

std::string File::getName() const {
    return name;
}

std::string File::getContent() const {
    return content;
}

void File::setContent(const std::string& newContent) {
    content = newContent;
}

File::ContentType File::getContentType() const {
    return type;
}
