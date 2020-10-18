#include "document.h"

using namespace std;

ostream& operator<<(ostream& out, DocumentStatus status) {
    switch (status) {
    case DocumentStatus::ACTUAL: out << "DocumentStatus::ACTUAL"s; break;
    case DocumentStatus::IRRELEVANT: out << "DocumentStatus::IRRELEVANT"s; break;
    case DocumentStatus::BANNED: out << "DocumentStatus::BANNED"s; break;
    case DocumentStatus::REMOVED: out << "DocumentStatus::REMOVED"s; break;
    default: out << static_cast<int>(status);
    }
    return out;
}

ostream& operator<<(ostream& out, const Document& document) {
    out << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating
         << " }"s;
    return out;
}
