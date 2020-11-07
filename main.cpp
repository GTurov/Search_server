#include "search_server.h"
#include "request_queue.h"
#include "test_example_functions.h"
#include "paginator.h"

#include <iostream>

using namespace std;

//int main() {
//    TestSearchServer();
//    SearchServer search_server("и в на"s);
//    RequestQueue request_queue(search_server);

//    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
//    search_server.AddDocument(2, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2, 3});
//    search_server.AddDocument(3, "большой кот модный ошейник "s, DocumentStatus::ACTUAL, {1, 2, 8});
//    search_server.AddDocument(4, "большой пёс скворец евгений"s, DocumentStatus::ACTUAL, {1, 3, 2});
//    search_server.AddDocument(5, "большой пёс скворец василий"s, DocumentStatus::ACTUAL, {1, 1, 1});

//    // 1439 запросов с нулевым результатом
//    for (int i = 0; i < 1439; ++i) {
//        request_queue.AddFindRequest("empty query"s);
//    }
//    // все еще 1439 запросов с нулевым результатом
//    request_queue.AddFindRequest("пушистый пёс"s);
//    // новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
//    request_queue.AddFindRequest("большой ошейник"s);
//    // первый запрос удален, 1437 запросов с нулевым результатом
//    request_queue.AddFindRequest("скворец"s);
//    std::cout << "Запросов, по которым ничего не нашлось "s << request_queue.GetNoResultRequests();
//    return 0;
//}

//int main() {
//    SearchServer search_server("and with"s);

//    AddDocument(search_server, 1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, {7, 2, 7});
//    AddDocument(search_server, 2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2});

//    // дубликат документа 2, будет удалён
//    AddDocument(search_server, 3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2});

//    // отличие только в стоп-словах, считаем дубликатом
//    AddDocument(search_server, 4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, {1, 2});

//    // множество слов такое же, считаем дубликатом документа 1
//    AddDocument(search_server, 5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, {1, 2});

//    // добавились новые слова, дубликатом не является
//    AddDocument(search_server, 6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, {1, 2});

//    // множество слов такое же, как в id 6, несмотря на другой порядок, считаем дубликатом
//    AddDocument(search_server, 7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, {1, 2});

//    // есть не все слова, не является дубликатом
//    AddDocument(search_server, 8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, {1, 2});

//    // слова из разных документов, не является дубликатом
//    AddDocument(search_server, 9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, {1, 2});

//    std::cout << "Before duplicates removed: "s << search_server.GetDocumentCount() << std::endl;
//    RemoveDuplicates(search_server);
//    std::cout << "After duplicates removed: "s << search_server.GetDocumentCount() << std::endl;
//}


int main() {
    TestSearchServer();
    SearchServer search_server("and with"s);

    search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2, 3});
    search_server.AddDocument(3, "big cat nasty hair"s, DocumentStatus::ACTUAL, {1, 2, 8});
    search_server.AddDocument(4, "big dog cat Vladislav"s, DocumentStatus::ACTUAL, {1, 3, 2});
    search_server.AddDocument(5, "big dog hamster Borya"s, DocumentStatus::ACTUAL, {1, 1, 1});

    const auto search_results = search_server.FindTopDocuments("curly dog"s);
    int page_size = 2;
    const auto pages = Paginate(search_results, page_size);

    // Выводим найденные документы по страницам
    for (auto page = pages.begin(); page != pages.end(); ++page) {
        cout << *page << endl;
        cout << "Page break"s << endl;
    }
}
