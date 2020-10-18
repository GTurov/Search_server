#include <iostream>

#include "search_server.h"
#include "request_queue.h"
#include "test_example_functions.h"


//void TestSearchServer() {
//    RUN_TEST(TestFindWordsFromAddedDocument);
//    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
//    RUN_TEST(TestExcludeMinusWordsDocumentContent);
//    RUN_TEST(TestMatchingDocuments);
//    RUN_TEST(TestRelevanceSorting);
//    RUN_TEST(TestRatingComputing);
//    RUN_TEST(TestPredicate);
//    RUN_TEST(TestStatusFilter);
//    RUN_TEST(TestRelevanceCheck);
//    RUN_TEST(TestConstructor);
//    RUN_TEST(TestAddDocument);
//    RUN_TEST(TestMatchAndFindDocument);
//    RUN_TEST(TestCheckIds);
//}

int main() {
    //TestSearchServer();
    SearchServer search_server("и в на"s);
    RequestQueue request_queue(search_server);

    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2, 3});
    search_server.AddDocument(3, "большой кот модный ошейник "s, DocumentStatus::ACTUAL, {1, 2, 8});
    search_server.AddDocument(4, "большой пёс скворец евгений"s, DocumentStatus::ACTUAL, {1, 3, 2});
    search_server.AddDocument(5, "большой пёс скворец василий"s, DocumentStatus::ACTUAL, {1, 1, 1});

    // 1439 запросов с нулевым результатом
    for (int i = 0; i < 1439; ++i) {
        request_queue.AddFindRequest("empty query"s);
    }
    // все еще 1439 запросов с нулевым результатом
    request_queue.AddFindRequest("пушистый пёс"s);
    // новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
    request_queue.AddFindRequest("большой ошейник"s);
    // первый запрос удален, 1437 запросов с нулевым результатом
    request_queue.AddFindRequest("скворец"s);
    std::cout << "Запросов, по которым ничего не нашлось "s << request_queue.GetNoResultRequests();
    return 0;
}
