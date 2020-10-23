#include "string_processing.h"

#include <algorithm>
#include <stdexcept>

using namespace std;

bool IsValidWord(const string& word) {
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

bool CheckWord(const string& word) {
    return (IsValidWord(word)?1: throw invalid_argument("Special symbol detected"s));
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (CheckWord(word)) {
                words.push_back(word);
            }
            word = "";
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        if(CheckWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}
