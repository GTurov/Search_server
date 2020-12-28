#include "string_processing.h"

#include <algorithm>
#include <stdexcept>

#include <iostream>

using namespace std;

bool IsValidWord(const string_view &word) {
    return none_of(word.begin(), word.end(), [](char c) {
        return c >= '\0' && c < ' ';
    });
}

bool CheckWord(const string_view& word) {
    return (IsValidWord(word)?1: throw invalid_argument("Special symbol detected"s));
}

vector<string> SplitIntoWords(const string_view& text) {
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

vector<string_view> SplitIntoWordsView(const string_view& text) {
    vector<string_view> words;
    int word_begin = 0;
    int i = 0;
    for (i = 0; i < (int)text.size(); ++i) {
        if (text[i] == ' ') {
            if (CheckWord(text.substr(word_begin,i-word_begin))) {
                words.push_back(text.substr(word_begin,i-word_begin));
            }
            word_begin = i+1;
        }
    }
    if (word_begin != (int)text.size()) {
        if (CheckWord(text.substr(word_begin,text.size()-word_begin))) {
            words.push_back(text.substr(word_begin,text.size()-word_begin));
        }
    }
    return words;
}
