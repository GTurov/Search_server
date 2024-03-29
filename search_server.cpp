#include "search_server.h"
#include "string_processing.h"

#include <iostream>
#include <algorithm>
#include <numeric>
#include <cmath>


using namespace std;

const map<string, double> SearchServer::empty_word_freqs_ = map<string, double>();

int SecureSum(int sum, int x) {
    if ( (sum < 0) && (x < 0) && (numeric_limits<int>::min() - x > sum)) {
        return numeric_limits<int>::min();
    }
    else if ( (sum > 0) && (x > 0) && (numeric_limits<int>::max() - x < sum)) {
        return numeric_limits<int>::max();
    }
    else {
        return sum+x;
    }
}

SearchServer::SearchServer(const string& stop_words_text)
    : SearchServer(SplitIntoWords(stop_words_text)) {
}

int SearchServer::GetDocumentCount() const{
    return documents_.size();
}

const map<string, double>& SearchServer::GetWordFrequencies(int document_id) const {
    return (documents_.count(document_id)!=0 ?documents_.at(document_id).word_to_freqs:empty_word_freqs_);
}

void SearchServer::AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
    if (document_id < 0) {
        throw invalid_argument("document_id < 0"s);
    }
    if (documents_.count(document_id) > 0) {
        throw invalid_argument("document already exists"s);
    }
    const vector<string> words = SplitIntoWordsNoStop(document);
    const double inv_word_count = 1.0 / words.size();
    map<string,double> word_to_freq;
    for (const string& word : words) {
        if (word_to_document_freqs_[word].count(document_id) == 0) {
            word_to_document_freqs_[word][document_id] = 0;
        }
        word_to_document_freqs_[word][document_id] += inv_word_count;
        if (word_to_freq.count(word) == 0) {
            word_to_freq[word] = 0;
        }
        word_to_freq[word] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status, word_to_freq});
    document_ids_.push_back(document_id);
}

void SearchServer::RemoveDocument(int document_id) {
    auto it = find(document_ids_.begin(),document_ids_.end(),document_id);
    if (it==document_ids_.end()) {
        return;
    }
    document_ids_.erase(it);
    vector<string> words_for_remove;
    for (auto& [word, document_freqs] :  word_to_document_freqs_) {
        document_freqs.erase(document_id);
        if (document_freqs.size()==0) {
            words_for_remove.push_back(word);
        }
    }
    for (const string& word: words_for_remove) {
        word_to_document_freqs_.erase(word);
    }
    documents_.erase(document_id);
}

vector<Document> SearchServer::FindTopDocuments(const string& raw_query, DocumentStatus status) const{
#ifdef SHOW_OPERATION_TIME
    LOG_DURATION_STREAM("Operation time", cout);
#endif
    return FindTopDocuments(raw_query, [status]([[maybe_unused]] int document_id, DocumentStatus document_status, [[maybe_unused]] int document_rating)
            { return document_status == status; });
}

tuple<vector<string>, DocumentStatus> SearchServer::MatchDocument(const string& raw_query, int document_id) const{
    const Query query = ParseQuery(raw_query);
    vector<string> MatchedWords;
    for (const string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        if ( find_if(word_to_document_freqs_.at(word).begin(), word_to_document_freqs_.at(word).end(),
                [document_id](const pair<int, double>& p){
                        return (document_id == p.first);
                }
          ) != word_to_document_freqs_.at(word).end() ) {
                return make_tuple(vector<string>{},documents_.at(document_id).status);
        }

    }
    for (const string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto [id, term_freq] : word_to_document_freqs_.at(word)) {
            if ( (id == document_id) && (count(MatchedWords.begin(),MatchedWords.end(),word) == 0) ){
                MatchedWords.push_back(word);
            }
        }
    }
    sort(MatchedWords.begin(),MatchedWords.end());
    return make_tuple(MatchedWords,documents_.at(document_id).status);
}


bool SearchServer::IsStopWord(const string& word) const {
    return stop_words_.count(word) > 0;
}

bool SearchServer::IsMinusWord(const string &word) {
        if (word[0] == '-') {
            if ( (word.size() == 1) || ((word.size()>1)&&(word[1]=='-')) ) {
                throw invalid_argument("Invaid minus word"s);
            }
            return true;
        }
        return false;
    }

void SearchServer::SetStopWords(const string& text) {
    for (const string& word : SplitIntoWords(text)) {
        stop_words_.insert(word);
    }
}

vector<string> SearchServer::SplitIntoWordsNoStop(const string& text) const {
    vector<string> words;
    for (const string& word : SplitIntoWords(text)) {
        if (!IsStopWord(word)) {
            words.push_back(word);
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
    return (!ratings.size()?0:accumulate(ratings.begin(), ratings.end(),0, SecureSum)/static_cast<int>(ratings.size()));
}

SearchServer::QueryWord SearchServer::ParseQueryWord(string text) const {
    bool is_minus = IsMinusWord(text);
    return {
        is_minus?text.substr(1):text,
        is_minus,
        IsStopWord(text)
    };
}

SearchServer::Query SearchServer::ParseQuery(const string& text) const {
    Query query;
    for (const string& word : SplitIntoWords(text)) {
        const QueryWord query_word = ParseQueryWord(word);
        if (!query_word.is_stop) {
            if (query_word.is_minus) {
                query.minus_words.insert(query_word.data);
            }
            else {
                query.plus_words.insert(query_word.data);
            }
        }
    }
    return query;
}

double SearchServer::ComputeWordInverseDocumentFreq(const string& word) const {
    return log(documents_.size() * 1.0 / word_to_document_freqs_.at(word).size());
}

vector<Document> SearchServer::FindAllDocuments(const Query& query) const {
    map<int, double> document_to_relevance;
    for (const string& word : query.plus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
            document_to_relevance[document_id] += term_freq * inverse_document_freq;
        }
    }

    for (const string& word : query.minus_words) {
        if (word_to_document_freqs_.count(word) == 0) {
            continue;
        }
        for (const auto [document_id, freq] : word_to_document_freqs_.at(word)) {
            document_to_relevance.erase(document_id);
        }
    }
    vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back({
                document_id,
                relevance,
                documents_.at(document_id).rating
            });
    }
    return matched_documents;
}

void PrintMatchDocumentResult(int document_id, const vector<string>& words, DocumentStatus status) {
    cout << "{ "s
         << "document_id = "s << document_id << ", "s
         << "status = "s << static_cast<int>(status) << ", "s
         << "words ="s;
    for (const string& word : words) {
        cout << ' ' << word;
    }
    cout << "}"s << endl;
}

void AddDocument(SearchServer& search_server, int document_id, const string& document, DocumentStatus status,
                 const vector<int>& ratings) {
    try {
        search_server.AddDocument(document_id, document, status, ratings);
    } catch (const exception& e) {
        cout << "Add document error "s << document_id << ": "s << e.what() << endl;
    }
}

void FindTopDocuments(const SearchServer& search_server, const string& raw_query) {
    cout << "Search result for query: "s << raw_query << endl;
    try {
        for (const Document& document : search_server.FindTopDocuments(raw_query)) {
            cout<<document<<endl;
        }
    } catch (const exception& e) {
        cout << "Search error: "s << e.what() << endl;
    }
}

void MatchDocuments(const SearchServer& search_server, const string& query)
{
    try {
        cout << "Document matching for query: "s << query << endl;
        for (const auto document_id : search_server) {
            const auto [words, status] = search_server.MatchDocument(query, document_id);
            PrintMatchDocumentResult(document_id, words, status);
        }
    } catch (const exception& e) {
        cout << "Matching error for query "s << query << ": "s << e.what() << endl;
    }
}

void RemoveDuplicates(SearchServer& search_server) {
    set<set<string>> originals_content;
    vector<int> duplicates_ids;
    for (const int document_id : search_server) {
        set<string> content;
        for (const auto& [word, freq]: search_server.GetWordFrequencies(document_id)) {
            content.emplace(word);
        }
        if (originals_content.count(content)!=0) {
            duplicates_ids.push_back(document_id);
            cout<<"Found duplicate document id "s<<document_id<<endl;
        } else {
            originals_content.emplace(content);
        }
    }
    for (const int id : duplicates_ids) {
        search_server.RemoveDocument(id);
    }
}












