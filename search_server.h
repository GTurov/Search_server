#pragma once

#include "document.h"
#include "string_processing.h"
#include "log_duration.h"

#include <map>
#include <set>
#include <vector>
#include <stdexcept>
#include <string>

//#define SHOW_OPERATION_TIME

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
    const std::map<std::string, double>& GetWordFrequencies(int document_id) const;
    auto begin() const { return document_ids_.begin(); }
    auto end() const { return document_ids_.end(); }
    void AddDocument(int document_id, const std::string& document, DocumentStatus status, const std::vector<int>& ratings);
    void RemoveDocument(int document_id);
    template <typename ExecutionPolicy>
    void RemoveDocument(ExecutionPolicy&& policy, int document_id) {
        auto it = find(policy, document_ids_.begin(),document_ids_.end(),document_id);
        if (it==document_ids_.end()) {
            return;
        }
        document_ids_.erase(it);
        std::vector<std::string> words_for_remove;
        for (auto& [word, document_freqs] :  word_to_document_freqs_) {
            document_freqs.erase(document_id);
            if (document_freqs.size()==0) {
                words_for_remove.push_back(word);
            }
        }
        for (const std::string& word: words_for_remove) {
            word_to_document_freqs_.erase(word);
        }
        documents_.erase(document_id);
    }
    std::vector<Document> FindTopDocuments(const std::string& raw_query, DocumentStatus status = DocumentStatus::ACTUAL) const;
    template <typename KeyMapper>
    std::vector<Document> FindTopDocuments(const std::string& raw_query, const KeyMapper& key_mapper) const {
#ifdef SHOW_OPERATION_TIME
        LOG_DURATION_STREAM("Operation time", std::cout);
#endif
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
    template <typename ExecutionPolicy>
    std::tuple<std::vector<std::string>, DocumentStatus> MatchDocument(ExecutionPolicy&& policy, const std::string& raw_query, int document_id) const {
        const Query query = ParseQuery(raw_query);
        std::vector<std::string> MatchedWords;
        for (const std::string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            if ( find_if(policy, word_to_document_freqs_.at(word).begin(), word_to_document_freqs_.at(word).end(),
                    [document_id](const std::pair<int, double>& p){
                            return (document_id == p.first);
                    }
              ) != word_to_document_freqs_.at(word).end() ) {
                    return make_tuple(std::vector<std::string>{},documents_.at(document_id).status);
            }

        }
        for (const std::string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [id, term_freq] : word_to_document_freqs_.at(word)) {
                if ( (id == document_id) && (count(policy, MatchedWords.begin(),MatchedWords.end(),word) == 0) ){
                    MatchedWords.push_back(word);
                }
            }
        }
        sort(policy, MatchedWords.begin(),MatchedWords.end());
        return make_tuple(MatchedWords,documents_.at(document_id).status);
    }
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
        std::map<std::string, double> word_to_freqs;
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
    Query ParseQuery(const std::string &text) const ;
    double ComputeWordInverseDocumentFreq(const std::string& word) const;
    std::vector<Document> FindAllDocuments(const Query& query) const;
private:
    std::map<int, DocumentData> documents_;
    std::vector<int> document_ids_; // Первый кандидат на удаление. Может отдавать итераторы documents_, а не на этот не совсем полезный вектор?
    std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    static const std::map<std::string, double> empty_word_freqs_;
};

void PrintMatchDocumentResult(int document_id, const std::vector<std::string>& words, DocumentStatus status) ;
void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status,
                 const std::vector<int>& ratings) ;
void FindTopDocuments(const SearchServer& search_server, const std::string& raw_query);
void MatchDocuments(const SearchServer& search_server, const std::string& query);
void RemoveDuplicates(SearchServer& search_server);
std::vector<std::vector<Document>> ProcessQueries(
    const SearchServer& search_server,
    const std::vector<std::string>& queries);
std::vector<Document> ProcessQueriesJoined(const SearchServer& search_server,
        const std::vector<std::string>& queries);
