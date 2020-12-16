//#include "search_server.h"
//#include "test_example_functions.h"

//#ifdef MULTITHREAD
//#include <execution>
//#endif
//#include <iostream>

//using namespace std;

//#include "log_duration.h"
//#include "process_queries.h"

//#include <random>
//#include <string>
//#include <vector>

//using namespace std;

//string GenerateWord(mt19937& generator, int max_length) {
//    const int length = uniform_int_distribution(1, max_length)(generator);
//    string word;
//    word.reserve(length);
//    for (int i = 0; i < length; ++i) {
//        word.push_back(uniform_int_distribution((short)'a', (short)'z')(generator));
//    }
//    return word;
//}

//vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length) {
//    vector<string> words;
//    words.reserve(word_count);
//    for (int i = 0; i < word_count; ++i) {
//        words.push_back(GenerateWord(generator, max_length));
//    }
//    sort(words.begin(), words.end());
//    words.erase(unique(words.begin(), words.end()), words.end());
//    return words;
//}

//string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int word_count, double minus_prob = 0) {
//    string query;
//    for (int i = 0; i < word_count; ++i) {
//        if (!query.empty()) {
//            query.push_back(' ');
//        }
//        if (uniform_real_distribution<>(0, 1)(generator) < minus_prob) {
//            query.push_back('-');
//        }
//        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
//    }
//    return query;
//}

//vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count) {
//    vector<string> queries;
//    queries.reserve(query_count);
//    for (int i = 0; i < query_count; ++i) {
//        queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
//    }
//    return queries;
//}

//template <typename ExecutionPolicy>
//void Test(string_view mark, SearchServer search_server, const string& query, ExecutionPolicy&& policy) {
//    LOG_DURATION(mark);
//    const int document_count = search_server.GetDocumentCount();
//    int word_count = 0;
//    for (int id = 0; id < document_count; ++id) {
//        const auto [words, status] = search_server.MatchDocument(policy, query, id);
//        word_count += words.size();
//    }
//    cout << word_count << endl;
//}

//#define TEST(policy) Test(#policy, search_server, query, execution::policy)

//int main() {
////{
////    mt19937 generator;

////    const auto dictionary = GenerateDictionary(generator, 1000, 10);
////    const auto documents = GenerateQueries(generator, dictionary, 10'000, 70);

////    const string query = GenerateQuery(generator, dictionary, 500, 0.1);

////    SearchServer search_server(dictionary[0]);
////    for (size_t i = 0; i < documents.size(); ++i) {
////        search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
////    }

////    TEST(seq);
////    TEST(par);
////            }

//    TestSearchServer();
//    SearchServer server("in on the"s);
//    server.AddDocument(1, "cat in the city"s, DocumentStatus::ACTUAL, {1, 2, 3});
//    server.AddDocument(2, "cat in the village"s, DocumentStatus::ACTUAL, {1, 2, 3});
//    server.AddDocument(3, "tiger in the city"s,  DocumentStatus::ACTUAL, {7, 8, 9});
//    server.AddDocument(4, "bird in the village"s,  DocumentStatus::ACTUAL, {10, 11, 12});
//    server.AddDocument(5, "dog in the space"s,  DocumentStatus::ACTUAL, {10, 11, 12});
//    cout<<server.GetWordFrequencies(2)<<endl;
//    const auto [words, status] = server.MatchDocument("city cat dog"s, 1);
//            for (auto w: words) {
//        cout << w <<" "s;
//    }
//            cout<<words<<endl;

//    const tuple<vector<string_view>, DocumentStatus> item = server.MatchDocument("city cat dog"s, 1);
//    cout << get<vector<string_view>>(item) << endl;

//    cout<<server.FindTopDocuments("city cat");


////    return 0;
//#ifdef MULTITHREAD
//   { SearchServer search_server("and with"s);

//    search_server.AddDocument(0, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, {1, 2});
//    search_server.AddDocument(1, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2});
//    search_server.AddDocument(2, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, {1, 2});
//    search_server.AddDocument(3, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, {1, 2});
//    search_server.AddDocument(4, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, {1, 2});


//    const vector<string> queries = {
//        "nasty rat -not"s,
//        "not very funny nasty pet"s,
//        "curly hair"s
//    };
////    int id = 0;
////    for (
////        const auto& documents : ProcessQueries(search_server, queries)
////    ) {
////        cout << documents.size() << " documents for query ["s << queries[id++] << "]"s << endl;
////    }

////    for (const Document& document : ProcessQueriesJoined(search_server, queries)) {
////           cout << "Document "s << document.id << " matched with relevance "s << document.relevance << endl;
////       }


////    const string query = "curly and funny"s;

////    auto report = [&search_server, &query] {
////        cout << search_server.GetDocumentCount() << " documents total, "s
////             << search_server.FindTopDocuments(query).size() << " documents for query ["s << query << "]"s << endl;
////    };

////    report();
////    // однопоточная версия
////    search_server.RemoveDocument(5);
////    report();
////    // однопоточная версия
////    search_server.RemoveDocument(execution::seq, 1);
////    report();
////    // многопоточная версия
////    search_server.RemoveDocument(execution::par, 2);
////    report();

//    const string query = "curly and funny -not"s;

//    {
//        const auto [words, status] = search_server.MatchDocument(query, 1);
//                cout << words.size() << " words for document 1"s << endl;
//                // 1 words for document 1
//    }

//        {
//            const auto [words, status] = search_server.MatchDocument(execution::seq, query, 2);
//                    cout << words.size() << " words for document 2"s << endl;
//                    // 2 words for document 2
//        }

//            {
//                const auto [words, status] = search_server.MatchDocument(execution::par, query, 3);
//                        cout << words.size() << " words for document 3"s << endl;
//                        // 0 words for document 3
//            }
//        #endif
//}

//    return 0;

//}

#include "search_server.h"
#include "process_queries.h"

#include "log_duration.h"

#include <iostream>
#include <random>
#include <string>
#include <vector>

using namespace std;

string GenerateWord(mt19937& generator, int max_length) {
    const int length = uniform_int_distribution(1, max_length)(generator);
    string word;
    word.reserve(length);
    for (int i = 0; i < length; ++i) {
        word.push_back(uniform_int_distribution((short)'a', (short)'z')(generator));
    }
    return word;
}

vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length) {
    vector<string> words;
    words.reserve(word_count);
    for (int i = 0; i < word_count; ++i) {
        words.push_back(GenerateWord(generator, max_length));
    }
    sort(words.begin(), words.end());
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}

string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int max_word_count) {
    const int word_count = uniform_int_distribution(1, max_word_count)(generator);
    string query;
    for (int i = 0; i < word_count; ++i) {
        if (!query.empty()) {
            query.push_back(' ');
        }
        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
    }
    return query;
}

vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count) {
    vector<string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) {
        queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
    }
    return queries;
}

template <typename QueriesProcessor>
void Test(string_view mark, QueriesProcessor processor, const SearchServer& search_server, const vector<string>& queries) {
    LOG_DURATION(mark);
    const auto documents = processor(search_server, queries);
    cout << documents.size() << endl;
}

#define TEST(processor) Test(#processor, processor, search_server, queries)

int main() {
        TestSearchServer();
    mt19937 generator;
    const auto dictionary = GenerateDictionary(generator, 10000, 25);
    const auto documents = GenerateQueries(generator, dictionary, 100'000, 10);

    SearchServer search_server(dictionary[0]);
    for (size_t i = 0; i < documents.size(); ++i) {
        search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, {1, 2, 3});
    }

    const auto queries = GenerateQueries(generator, dictionary, 10'000, 7);
    TEST(ProcessQueries);
}

