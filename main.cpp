#include "search_server.h"
#include "test_example_functions.h"

#include <iostream>
#include <execution>

using namespace std;

int main() {
    TestSearchServer();
    SearchServer search_server("and with"s);

    search_server.AddDocument(0, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, {1, 2});
    search_server.AddDocument(1, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2});
    search_server.AddDocument(2, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, {1, 2});
    search_server.AddDocument(3, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, {1, 2});
    search_server.AddDocument(4, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, {1, 2});


    const vector<string> queries = {
        "nasty rat -not"s,
        "not very funny nasty pet"s,
        "curly hair"s
    };
//    int id = 0;
//    for (
//        const auto& documents : ProcessQueries(search_server, queries)
//    ) {
//        cout << documents.size() << " documents for query ["s << queries[id++] << "]"s << endl;
//    }

//    for (const Document& document : ProcessQueriesJoined(search_server, queries)) {
//           cout << "Document "s << document.id << " matched with relevance "s << document.relevance << endl;
//       }


//    const string query = "curly and funny"s;

//    auto report = [&search_server, &query] {
//        cout << search_server.GetDocumentCount() << " documents total, "s
//             << search_server.FindTopDocuments(query).size() << " documents for query ["s << query << "]"s << endl;
//    };

//    report();
//    // однопоточная версия
//    search_server.RemoveDocument(5);
//    report();
//    // однопоточная версия
//    search_server.RemoveDocument(execution::seq, 1);
//    report();
//    // многопоточная версия
//    search_server.RemoveDocument(execution::par, 2);
//    report();

    const string query = "curly and funny -not"s;

    {
        const auto [words, status] = search_server.MatchDocument(query, 1);
                cout << words.size() << " words for document 1"s << endl;
                // 1 words for document 1
    }

        {
            const auto [words, status] = search_server.MatchDocument(execution::seq, query, 2);
                    cout << words.size() << " words for document 2"s << endl;
                    // 2 words for document 2
        }

            {
                const auto [words, status] = search_server.MatchDocument(execution::par, query, 3);
                        cout << words.size() << " words for document 3"s << endl;
                        // 0 words for document 3
            }


    return 0;

}



