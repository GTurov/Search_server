#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <numeric>
#include <cassert>


using namespace std;


const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

template < typename T1, typename T2>
ostream& operator<<(ostream& out, const pair<T1, T2>& element) {
    out << element.first <<": "s << element.second;
    return out;
}

template <typename Container>
void Print(ostream& out, const Container& container){
    bool first = true;
    for (const auto& element : container) {
        if(!first)
        out << ", "s ;
        out << element ;
        first = false;
    }
    return;
}

template < typename Element>
ostream& operator<<(ostream& out, const vector<Element>& container) {
    out << "["s;
    Print(out, container);
    out << "]"s;
    return out;
}

template < typename Element>
ostream& operator<<(ostream& out, const set<Element>& container) {
    out<< "{"s;
    Print(out, container);
    out<< "}"s;
    return out;
}

template < typename Key, typename Value>
ostream& operator<<(ostream& out, const map<Key, Value>& container) {
    out<< "{"s;
    Print(out, container);
    out<< "}"s;
    return out;
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const string& t_str, const string& u_str, const string& file,
                     const string& func, unsigned line, const string& hint) {
    if (t != u) {
        cout << boolalpha;
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        cout << t << " != "s << u << "."s;
        if ( !hint.empty() ) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
                const string& hint) {
    if ( !value ) {
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT("s << expr_str << ") failed."s;
        if ( !hint.empty() ) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename TestFunc>
void RunTestImpl(TestFunc& func, const string& FunctionName) {
    func();
    cerr << FunctionName << " OK"s << endl;
}

#define RUN_TEST(func)  RunTestImpl((func), #func )


enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED
};

ostream& operator<<(ostream& out, DocumentStatus status) {
    switch (status) {
    case DocumentStatus::ACTUAL: out << "DocumentStatus::ACTUAL"s; break;
    case DocumentStatus::IRRELEVANT: out << "DocumentStatus::IRRELEVANT"s; break;
    case DocumentStatus::BANNED: out << "DocumentStatus::BANNED"s; break;
    case DocumentStatus::REMOVED: out << "DocumentStatus::REMOVED"s; break;
    }
    return out;
}

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            words.push_back(word);
            word = "";
        } else {
            word += c;
        }
    }
    words.push_back(word);

    return words;
}

struct Document {
    int id;
    double relevance;
    int rating;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    int GetDocumentCount(){
        return document_count_;
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status, const vector<int>& ratings) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        const double inv_word_count = 1.0 / words.size();
        for (const string& word : words) {
            //Initialization added
            if (word_to_document_freqs_[word].count(document_id) == 0) {
                word_to_document_freqs_[word][document_id] = 0;
            }
            word_to_document_freqs_[word][document_id] += inv_word_count;
        }
        document_ratings_.emplace(document_id, ComputeAverageRating(ratings));
        document_statuses_.emplace(document_id, status);
        ++document_count_;
    }

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status = DocumentStatus::ACTUAL) const{
        return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int document_rating)
                { return document_status == status; });
    }

    template <typename KeyMapper>
    vector<Document> FindTopDocuments(const string& raw_query, const KeyMapper& key_mapper) const {
        const Query query = ParseQuery(raw_query);
        vector<Document> found_documents = FindAllDocuments(query);

        found_documents.erase(
                remove_if( found_documents.begin(), found_documents.end(),
                [this, key_mapper](const Document& document){
                    return !key_mapper(document.id, document_statuses_.at(document.id), document.rating);
                }
                ),
                found_documents.end() );

        sort(found_documents.begin(), found_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 if (abs(lhs.relevance - rhs.relevance) < EPSILON)
                   return lhs.rating > rhs.rating;
                 else
                    return lhs.relevance > rhs.relevance;
             });

        if (found_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            found_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
                }
        return found_documents;
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query, int document_id) const{
        const Query query = ParseQuery(raw_query);
        vector<string> MatchedWords;
        for (const string& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0)
                continue;
            if( find_if(word_to_document_freqs_.at(word).begin(), word_to_document_freqs_.at(word).end(),
                    [document_id](const pair<int, double>& p){
                            return (document_id == p.first);
                    }
              ) != word_to_document_freqs_.at(word).end() )
                    return make_tuple(vector<string>{},document_statuses_.at(document_id));

        }
        for (const string& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0)
                continue;

            for (const auto [id, term_freq] : word_to_document_freqs_.at(word)) {
                if ( (id == document_id) && (count(MatchedWords.begin(),MatchedWords.end(),word) == 0) ){
                    MatchedWords.push_back(word);
                }
            }
        }
        sort(MatchedWords.begin(),MatchedWords.end());
        return make_tuple(MatchedWords,document_statuses_.at(document_id));
    }

private:
    set<string> stop_words_;
    map<string, map<int, double>> word_to_document_freqs_;
    map<int, int> document_ratings_;
    map<int, DocumentStatus> document_statuses_;
    int document_count_ = 0;

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        return (!ratings.size()?0:
        accumulate(ratings.begin(), ratings.end(),0)/static_cast<int>(ratings.size()));
    }

    struct QueryWord {
        string data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(string text) const {
        bool is_minus = false;
        if (text[0] == '-') {
            is_minus = true;
            text = text.substr(1);
        }
        return {
            text,
            is_minus,
            IsStopWord(text)
        };
    }

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    Query ParseQuery(const string& text) const {
        Query query;
        for (const string& word : SplitIntoWords(text)) {
            const QueryWord query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    query.minus_words.insert(query_word.data);
                } else {
                    query.plus_words.insert(query_word.data);
                }
            }
        }
        return query;
    }

    // Existence required
    double ComputeWordInverseDocumentFreq(const string& word) const {
        return log(document_ratings_.size() * 1.0 / word_to_document_freqs_.at(word).size());
    }

    vector<Document> FindAllDocuments(const Query& query) const {
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
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
                matched_documents.push_back({
                    document_id,
                    relevance,
                    document_ratings_.at(document_id)
                });
        }
        return matched_documents;
    }
};

void PrintDocument(const Document& document) {
    cout << "{ "s
         << "document_id = "s << document.id << ", "s
         << "relevance = "s << document.relevance << ", "s
         << "rating = "s << document.rating
         << " }"s << endl;
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

// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что документ находится по поисковому запросу, который содержит слова из документа

void TestFindWordsFromAddedDocument() {
    //----------------------------- LONDON IS THE CAPITAL OF GREAT BRITAN ---------------------------------------
    //---------------------------------- SORRY FOR MY ENGLISH :) ------------------------------------------------

    SearchServer server;
    ASSERT_HINT(server.FindTopDocuments(""s).empty(),"Returned document for empty query and empty server"s);
    ASSERT_HINT(server.FindTopDocuments("rabbit"s).empty(),"Returned document empty server"s);

    server.AddDocument(1, "cat in the city"s, DocumentStatus::ACTUAL, {1, 2, 3});
    server.AddDocument(2, "cat in the village"s, DocumentStatus::ACTUAL, {1, 2, 3});
    server.AddDocument(3, "tiger in the city"s,  DocumentStatus::ACTUAL, {7, 8, 9});
    server.AddDocument(4, "bird in the village"s,  DocumentStatus::ACTUAL, {10, 11, 12});
    server.AddDocument(5, "dog in the space"s,  DocumentStatus::ACTUAL, {10, 11, 12});

    ASSERT_EQUAL_HINT(server.GetDocumentCount(),5,"Added documents count error"s);

    ASSERT_HINT(server.FindTopDocuments(""s).empty(),"Empty query returned documents"s);
    ASSERT_HINT(server.FindTopDocuments("rabbit"s).empty(),"Absend word int the query returned documents"s);

    {
        const auto found_docs = server.FindTopDocuments("cat"s);
        ASSERT_EQUAL_HINT(found_docs.size(),2u,"Wrong found documents count returned"s);
        ASSERT_HINT(count_if(found_docs.begin(),found_docs.end(),[](const Document& d){return d.id==1;}),"Document 1 missed"s);
        ASSERT_HINT(count_if(found_docs.begin(),found_docs.end(),[](const Document& d){return d.id==2;}),"Document 2 missed"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("the"s);
        ASSERT_HINT(found_docs.size() == 5u, "Word in the middle of query missed"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("city"s);
        ASSERT_HINT(found_docs.size() == 2u, "Word in the end of query missed"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("cat tiger bird"s);
        ASSERT_EQUAL_HINT(found_docs.size(), 4u, "Some words of query missed"s);
    }
}

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};

    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(),"Stop word detected in the result"s);
    }
}

// Тест проверяет, что поисковая система исключает документы, содержащие минус-слова
void TestExcludeMinusWordsDocumentContent() {
    SearchServer server;
    server.AddDocument(1, "cat in the city"s, DocumentStatus::ACTUAL, {1, 2, 3});
    server.AddDocument(2, "cat in the village"s, DocumentStatus::ACTUAL, {4, 5, 6});
    server.AddDocument(3, "tiger in the city"s,  DocumentStatus::ACTUAL, {7, 8, 9});
    server.AddDocument(4, "bird in the village"s,  DocumentStatus::ACTUAL, {10, 11, 12});
    server.AddDocument(666, ""s,  DocumentStatus::ACTUAL, {666, 666, 666});

    {
        ASSERT_HINT(server.FindTopDocuments("-village"s).empty(), "Result for empty query"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("cat"s);
        ASSERT_EQUAL_HINT(found_docs.size(), 2u, "Found documents count error"s);
        ASSERT_HINT(count_if(found_docs.begin(),found_docs.end(),[](const Document& d){return d.id==1;}), "Document1 not found"s);
        ASSERT_HINT(count_if(found_docs.begin(),found_docs.end(),[](const Document& d){return d.id==2;}), "Document2 not found"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("cat -space"s);
        ASSERT_EQUAL_HINT(found_docs.size(), 2u, "Non-existet minus word crashed the server");
        ASSERT_HINT(count_if(found_docs.begin(),found_docs.end(),[](const Document& d){return d.id==1;}), "Document1 not found"s);
        ASSERT_HINT(count_if(found_docs.begin(),found_docs.end(),[](const Document& d){return d.id==2;}), "Document2 not found"s);
    }

    {
        const auto found_docs = server.FindTopDocuments("clever cat -village"s);
        ASSERT_EQUAL_HINT(found_docs.size(), 1u, "Minus word in the end of query doesn't work"s);
        ASSERT_HINT(count_if(found_docs.begin(),found_docs.end(),[](const Document& d){return d.id==1;}), "Wrong document found"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("clever -village cat"s);
        ASSERT_EQUAL_HINT(found_docs.size(), 1u, "Minus word in the middle of query doesn't work"s);
        ASSERT_HINT(count_if(found_docs.begin(),found_docs.end(),[](const Document& d){return d.id==1;}), "Wrong document found"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("-village clever cat"s);
        ASSERT_EQUAL_HINT(found_docs.size(), 1u, "Minus word in the begin of query doesn't work"s);
        ASSERT_HINT(count_if(found_docs.begin(),found_docs.end(),[](const Document& d){return d.id==1;}), "Wrong document found"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("-village cat -city"s);
        ASSERT_HINT(found_docs.empty(), "Documens found for impossible query"s);
    }
}

// Тест проверяет, что поисковая система возвращает все слова из поискового запроса, содержащиеся в документах
void TestMatchingDocuments() {
    SearchServer server;
    server.AddDocument(1, "cat in the city"s, DocumentStatus::ACTUAL, {1});
    server.AddDocument(2, "dog in the village"s, DocumentStatus::IRRELEVANT, {2});
    server.AddDocument(3, "tiger in the city"s,  DocumentStatus::BANNED, {3});
    server.AddDocument(4, "bird in the village"s,  DocumentStatus::REMOVED, {4});
    {
        const auto [words, status] = server.MatchDocument("cat city -tiger"s,1);
        ASSERT_EQUAL_HINT(status, DocumentStatus::ACTUAL, "Document status incorrect"s);
        ASSERT_EQUAL_HINT(words.size(), 2u, "Words count wrong"s);
        ASSERT_HINT(count(words.begin(),words.end(),"cat"s), "Cat is missed"s);
        ASSERT_HINT(count(words.begin(),words.end(),"city"s), "City is missed"s);
    }

    {
        const auto [words, status] = server.MatchDocument("tiger -city"s,3);
        ASSERT_EQUAL_HINT(status, DocumentStatus::BANNED, "Document status incorrect"s);
        ASSERT_EQUAL_HINT(words.size(), 0, "Document content with minus word returned"s);
    }

    {
        const auto [words, status] = server.MatchDocument("dog in the village"s,2);
        ASSERT_EQUAL_HINT(status, DocumentStatus::IRRELEVANT, "Document status incorrect"s);
        ASSERT_EQUAL_HINT(words.size(), 4u, "Word count wrong"s);
    }

    {
        const auto [words, status] = server.MatchDocument("bird in the village"s,4);
        ASSERT_EQUAL_HINT(status, DocumentStatus::REMOVED, "Document status incorrect"s);
        ASSERT_EQUAL_HINT(words.size(), 4u, "Word count wrong"s);
    }
}

void TestRelevanceSorting() {
    SearchServer server;
    server.AddDocument(11, "orange cat in the big city"s, DocumentStatus::ACTUAL, {1});
    server.AddDocument(22, "clever cat in the small village"s, DocumentStatus::ACTUAL, {2});
    server.AddDocument(33, "black tiger in the night city"s,  DocumentStatus::ACTUAL, {3});
    server.AddDocument(44, "yellow cow on the small bird"s,  DocumentStatus::ACTUAL, {4});
    server.AddDocument(55, "pink raptor runs from orange cat"s,  DocumentStatus::ACTUAL, {5});
    {
        const auto found_docs = server.FindTopDocuments("orange cat raptor"s);
        ASSERT_EQUAL_HINT(found_docs.size(), 3u, "Found documens count incorrect"s);
        for(uint32_t i = 0; i+1 < found_docs.size(); ++i)
        {
            ASSERT_HINT(found_docs.at(i).relevance >= found_docs.at(i).relevance, "Relevance sorting error"s);
        }
    }
}

void TestRatingComputing() {
    SearchServer server;
    server.AddDocument(1, "w1"s, DocumentStatus::ACTUAL, {1});
    server.AddDocument(2, "w2"s,  DocumentStatus::ACTUAL, {1,2,3});
    server.AddDocument(3, "w3"s,  DocumentStatus::ACTUAL, {-1,-2,-3});
    server.AddDocument(4, "w4"s,  DocumentStatus::ACTUAL, {-1,1});
    server.AddDocument(5, "w5"s,  DocumentStatus::ACTUAL, {-1000,100});
    {
        const auto found_docs = server.FindTopDocuments("w1"s);
        ASSERT_EQUAL_HINT(found_docs[0].rating, 1, "Single rating computing error"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("w2"s);
        ASSERT_EQUAL_HINT(found_docs[0].rating, 2, "Positive rating computing error"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("w3"s);
        ASSERT_EQUAL_HINT(found_docs[0].rating, -2, "Negative rating computing error"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("w4"s);
        ASSERT_EQUAL_HINT(found_docs[0].rating, 0, "Neutral rating computing error"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("w5"s);
        ASSERT_EQUAL_HINT(found_docs[0].rating, -450, "Large rating computing error"s);
    }
}

void TestPredicate() {
    SearchServer server;
    server.AddDocument(11, "orange cat in the big city"s, DocumentStatus::ACTUAL, {100});
    server.AddDocument(22, "clever cat in the small village"s, DocumentStatus::ACTUAL, {1});
    server.AddDocument(33, "black cat in the dark room"s,  DocumentStatus::IRRELEVANT, {1,2,3});
    server.AddDocument(44, "yellow cat rides on the small bird"s,  DocumentStatus::BANNED, {-1,-2,-3});
    server.AddDocument(55, "pink raptor runs from orange cat"s,  DocumentStatus::REMOVED, {-1,1});

    {
        const auto documents = server.FindTopDocuments("cat"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; });
        ASSERT_EQUAL_HINT(documents.size(), 2u, "Document_id predicate error"s);
        for (const Document& document : documents) {
            ASSERT_EQUAL_HINT(document.id % 2, 0, "Document_id predicate error"s);
        }
    }
    {
        const auto documents = server.FindTopDocuments("cat"s, [](int document_id, DocumentStatus status, int rating) { return rating > 0; });
        ASSERT_EQUAL_HINT(documents.size(), 3u, "Rating predicate error"s);
        for (const Document& document : documents) {
            ASSERT_HINT(document.rating > 0, "Rating predicate error"s);
        }
    }
    {
        const auto documents = server.FindTopDocuments("cat"s, [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; });
        ASSERT_EQUAL_HINT(documents.size(), 2u, "Status predicate error"s);
        for (const Document& document : documents) {
            ASSERT_HINT(((document.id == 11) ||  (document.id == 22)), "Status predicate error"s);
        }
    }
    {
    const auto documents = server.FindTopDocuments("cat"s, [](int document_id, DocumentStatus status, int rating) { return true; });
    ASSERT_EQUAL_HINT(documents.size(), 5u, "True predicate error"s);
    }
    {
    const auto documents = server.FindTopDocuments("cat"s, [](int document_id, DocumentStatus status, int rating) { return false; });
    ASSERT_HINT(documents.empty(), "False predicate error"s);
    }
    {
    const auto documents = server.FindTopDocuments("dog"s, [](int document_id, DocumentStatus status, int rating) { return true; });
    ASSERT_HINT(documents.empty(), "Empty query predicate error"s);
    }
}

void TestStatusFilter() {
    SearchServer server;
    server.AddDocument(11, "orange cat in the big city"s, DocumentStatus::ACTUAL, {100});
    server.AddDocument(22, "clever cat in the small village"s, DocumentStatus::ACTUAL, {1});
    server.AddDocument(33, "black cat in the dark room"s,  DocumentStatus::IRRELEVANT, {1,2,3});
    server.AddDocument(44, "yellow cat rides on the small bird"s,  DocumentStatus::BANNED, {-1,-2,-3});
    server.AddDocument(55, "pink raptor runs from orange cat"s,  DocumentStatus::REMOVED, {-1,1});

    {
        const auto documents = server.FindTopDocuments("cat"s);
        ASSERT_EQUAL_HINT(documents.size(), 2u, "Incorrect found document count with default status"s);
        for (const Document& document : documents) {
            ASSERT_HINT(((document.id == 11) ||  (document.id == 22)), "Documents 11 & 22 with default status missed"s);
        }
    }
    {
        const auto documents = server.FindTopDocuments("cat"s, DocumentStatus::ACTUAL);
        ASSERT_EQUAL_HINT(documents.size(), 2u, "ACTUAL status filter error"s);
        ASSERT_HINT(count_if(documents.begin(),documents.end(),[](const Document& d){return d.id == 11;}), "Document 11 missed"s);
        ASSERT_HINT(count_if(documents.begin(),documents.end(),[](const Document& d){return d.id == 22;}), "Document 22 missed"s);
    }
    {
        const auto documents = server.FindTopDocuments("cat"s, DocumentStatus::BANNED);
        ASSERT_EQUAL_HINT(documents.size(), 1u, "BANNED status filter error"s);
        ASSERT_EQUAL_HINT(documents[0].id, 44, "Incorrect document found"s);
    }
    {
        const auto documents = server.FindTopDocuments("cat"s, DocumentStatus::IRRELEVANT);
        ASSERT_EQUAL_HINT(documents.size(), 1u, "IRRELEVANT status filter error"s);
        ASSERT_EQUAL_HINT(documents[0].id, 33, "Incorrect document found"s);
    }
    {
        const auto documents = server.FindTopDocuments("cat"s, DocumentStatus::REMOVED);
        ASSERT_EQUAL_HINT(documents.size(), 1u, "REMOVED status filter error"s);
        ASSERT_EQUAL_HINT(documents[0].id, 55, "Incorrect document found"s);
    }
}

void TestRelevanceCheck() {
    const double EPSILON = 1e-6;
    SearchServer server;
    server.SetStopWords("in on from with the"s);
    server.AddDocument(11, "orange cat in the big city"s, DocumentStatus::ACTUAL, {});
    server.AddDocument(22, "big pink cat in the small box"s, DocumentStatus::ACTUAL, {1});
    server.AddDocument(33, "black cat in the dark room"s,  DocumentStatus::ACTUAL, {1,2,3});
    server.AddDocument(44, "yellow cat rides on the red cat with another small cat"s,  DocumentStatus::ACTUAL, {-1,-2,-3});
    server.AddDocument(55, "big pink raptor runs from small orange cat"s,  DocumentStatus::ACTUAL, {-1,1});
    for (const Document& document :server.FindTopDocuments("cat"s)) {
        ASSERT_EQUAL_HINT(document.relevance, 0, "Relevance of single word incorrect"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("black raptor cat"s);
        ASSERT_HINT((abs(found_docs[0].relevance - 0.402359) < EPSILON), "Incorrect relevance computing"s);
        ASSERT_HINT((abs(found_docs[1].relevance - 0.22992) < EPSILON), "Incorrect relevance computing"s);
        ASSERT_EQUAL_HINT(count_if(found_docs.begin(),found_docs.end(),[](const Document& d){return d.relevance == 0;}), 3, "Toomany documents with zero relevance"s);
    }
    {
      const auto found_docs = server.FindTopDocuments("black cat in the dark room"s);
      ASSERT_HINT((abs(found_docs[0].relevance - 1.20708) < 2*EPSILON),"Incorrect relevance computing"s);
      ASSERT_EQUAL_HINT(count_if(found_docs.begin(),found_docs.end(),[](const Document& d){return d.relevance == 0;}), 4, "Toomany documents with zero relevance"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("orange cat city"s);
        ASSERT_HINT((abs(found_docs[0].relevance - 0.631432) < EPSILON),"Incorrect relevance computing"s);
        ASSERT_HINT((abs(found_docs[1].relevance - 0.130899) < EPSILON),"Incorrect relevance computing"s);
        ASSERT_EQUAL_HINT(count_if(found_docs.begin(),found_docs.end(),[](const Document& d){return d.relevance == 0;}), 3, "Toomany documents with zero relevance"s);
    }
}

void TestSearchServer() {
    RUN_TEST(TestFindWordsFromAddedDocument);
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestExcludeMinusWordsDocumentContent);
    RUN_TEST(TestMatchingDocuments);
    RUN_TEST(TestRelevanceSorting);
    RUN_TEST(TestRatingComputing);
    RUN_TEST(TestPredicate);
    RUN_TEST(TestStatusFilter);
    RUN_TEST(TestRelevanceCheck);
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
//    SearchServer search_server;
//    search_server.SetStopWords("и в на"s);

//    search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
//    search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
//    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
//    search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});

//    cout << "ACTUAL by default:"s << endl;
//    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
//        PrintDocument(document);
//    }

//    cout << "BANNED:"s << endl;
//    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)) {
//        PrintDocument(document);
//    }

//    cout << "Even ids:"s << endl;
//    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
//        PrintDocument(document);
//    }

//    cout << "Rating > 0 :"s << endl;
//    for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return rating >0; })) {
//        PrintDocument(document);
//    }

    return 0;
}