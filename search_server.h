#pragma once

#include "document.h"
#include "string_processing.h"
#include "log_duration.h"

#include <vector>
#include <set>
#include <map>
#include <string>
#include <stdexcept>

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1.0e-6;

int SecureSum(int sum, int x);

class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words)) {
    }
    explicit SearchServer(const std::string& stop_words_text);

    int GetDocumentCount() const;
    int GetDocumentId(int index) const;
    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status = DocumentStatus::ACTUAL) const;
    template <typename KeyMapper>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, const KeyMapper& key_mapper) const {
        LOG_DURATION_STREAM("Operation time", std::cout);
        const Query query = ParseQuery(raw_query);
        std::vector<Document> found_documents = FindAllDocuments(query);

        found_documents.erase(
                remove_if( found_documents.begin(), found_documents.end(),
                [this, key_mapper](const Document& document){
                    return !key_mapper(document.id, documents_.at(document.id).status, document.rating);
                }
                ),
                found_documents.end() );

        sort(found_documents.begin(), found_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 if (std::abs(lhs.relevance - rhs.relevance) < EPSILON) {
                   return lhs.rating > rhs.rating;
                 }
                 else {
                    return lhs.relevance > rhs.relevance;
                 }
             });

        if (found_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            found_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
                }
        return found_documents;
    }
    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(const std::string& raw_query, int document_id) const;
    bool IsStopWord(const std::string& word) const;
    static bool IsMinusWord(const std::string& word);
private:
    struct QueryWord {
        std::string data;
        bool is_minus;
        bool is_stop;
    };
    struct Query {
        std::set<std::string> plus_words;
        std::set<std::string> minus_words;
    };
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };
private:
    void SetStopWords(const std::string& text);
    std::vector<std::string> SplitIntoWordsNoStop(const std::string& text) const;
    template <typename StringContainer>
    std::set<std::string> MakeUniqueNonEmptyStrings(const StringContainer& strings) {
        std::set<std::string> non_empty_strings;
        for (const std::string& str : strings) {
            if (!str.empty() ) {
                if (CheckWord(str)) {
                    non_empty_strings.insert(str);
                }
            }
        }
        return non_empty_strings;
    }
    static int ComputeAverageRating(const std::vector<int>& ratings);
    QueryWord ParseQueryWord(std::string text) const;
    Query ParseQuery(const std::string& text) const ;
    double ComputeWordInverseDocumentFreq(const std::string& word) const;
    std::vector<Document> FindAllDocuments(const Query& query) const;
private:
    std::map<int, DocumentData> documents_;
    std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::vector<int> document_ids_;
};

void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status) ;
void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status,
                 const std::vector<int>& ratings) ;
void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query);
void MatchDocuments(const SearchServer& search_server, const std::string& query) ;
