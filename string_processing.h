#pragma once

#include <vector>
#include <string>

bool IsValidWord(const std::string_view &word);
bool CheckWord(const std::string_view& word);
std::vector<std::string> SplitIntoWords(const std::string_view& text);
std::vector<std::string_view> SplitIntoWordsView(const std::string_view& text);
