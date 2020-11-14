#include "test_example_functions.h"
#include "paginator.h"
#include "request_queue.h"

#include <algorithm>
#include <cassert>
#include <cmath>

using namespace std;

void AssertImpl(bool value, const string& expr_str, const string& file, const string& func, unsigned line,
                const string& hint) {
    if (!value) {
        cout << file << "("s << line << "): "s << func << ": "s;
        cout << "ASSERT("s << expr_str << ") failed."s;
        if ( !hint.empty() ) {
            cout << " Hint: "s << hint;
        }
        cout << endl;
        abort();
    }
}

void TestFindWordsFromAddedDocument() {
    //----------------------------- LONDON IS THE CAPITAL OF GREAT BRITAN ---------------------------------------
    //---------------------------------- SORRY FOR MY ENGLISH :) ------------------------------------------------

    SearchServer server(""s);
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
        SearchServer server(""s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }
    {
        SearchServer server("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(),"Stop word detected in the result"s);
    }
}

// Тест проверяет, что поисковая система исключает документы, содержащие минус-слова
void TestExcludeMinusWordsDocumentContent() {
    SearchServer server(""s);
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
    SearchServer server(""s);
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
    SearchServer server(""s);
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
    SearchServer server(""s);
    server.AddDocument(1, "w1"s, DocumentStatus::ACTUAL, {1});
    server.AddDocument(2, "w2"s,  DocumentStatus::ACTUAL, {1,2,3});
    server.AddDocument(3, "w3"s,  DocumentStatus::ACTUAL, {-1,-2,-3});
    server.AddDocument(4, "w4"s,  DocumentStatus::ACTUAL, {-1,1});
    server.AddDocument(5, "w5"s,  DocumentStatus::ACTUAL, {numeric_limits<int>::max(),-100, 150});
    server.AddDocument(6, "w6"s,  DocumentStatus::ACTUAL, {numeric_limits<int>::min(),100, -150});
    //Максим Радышевский поведал, что авторская реализация поискового сервера не обрабатывает пустые рейтинги
    server.AddDocument(7, "w7"s,  DocumentStatus::ACTUAL, {});
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
        ASSERT_EQUAL_HINT(found_docs[0].rating, numeric_limits<int>::max()/3, "Large rating computing error"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("w6"s);
        ASSERT_EQUAL_HINT(found_docs[0].rating, numeric_limits<int>::min()/3, "Large rating computing error"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("w7"s);
        ASSERT_EQUAL_HINT(found_docs[0].rating, 0, "Empty rating computing error"s);
    }

}

void TestPredicate() {
    SearchServer server(""s);
    server.AddDocument(11, "orange cat in the big city"s, DocumentStatus::ACTUAL, {100});
    server.AddDocument(22, "clever cat in the small village"s, DocumentStatus::ACTUAL, {1});
    server.AddDocument(33, "black cat in the dark room"s,  DocumentStatus::IRRELEVANT, {1,2,3});
    server.AddDocument(44, "yellow cat rides on the small bird"s,  DocumentStatus::BANNED, {-1,-2,-3});
    server.AddDocument(55, "pink raptor runs from orange cat"s,  DocumentStatus::REMOVED, {-1,1});

    {
        const auto documents = server.FindTopDocuments("cat"s, [](int document_id,[[maybe_unused]] DocumentStatus status,[[maybe_unused]] int rating) { return document_id % 2 == 0; });
        ASSERT_EQUAL_HINT(documents.size(), 2u, "Document_id predicate error"s);
        for (const Document& document : documents) {
            ASSERT_EQUAL_HINT(document.id % 2, 0, "Document_id predicate error"s);
        }
    }
    {
        const auto documents = server.FindTopDocuments("cat"s, []([[maybe_unused]]int document_id, [[maybe_unused]]DocumentStatus status, int rating) { return rating > 0; });
        ASSERT_EQUAL_HINT(documents.size(), 3u, "Rating predicate error"s);
        for (const Document& document : documents) {
            ASSERT_HINT(document.rating > 0, "Rating predicate error"s);
        }
    }
    {
        const auto documents = server.FindTopDocuments("cat"s, []([[maybe_unused]]int document_id, DocumentStatus status,[[maybe_unused]] int rating) { return status == DocumentStatus::ACTUAL; });
        ASSERT_EQUAL_HINT(documents.size(), 2u, "Status predicate error"s);
        for (const Document& document : documents) {
            ASSERT_HINT(((document.id == 11) ||  (document.id == 22)), "Status predicate error"s);
        }
    }
    {
    const auto documents = server.FindTopDocuments("cat"s, []([[maybe_unused]]int document_id, [[maybe_unused]]DocumentStatus status,[[maybe_unused]] int rating) { return true; });
    ASSERT_EQUAL_HINT(documents.size(), 5u, "True predicate error"s);
    }
    {
    const auto documents = server.FindTopDocuments("cat"s, []([[maybe_unused]]int document_id,[[maybe_unused]] DocumentStatus status,[[maybe_unused]] int rating) { return false; });
    ASSERT_HINT(documents.empty(), "False predicate error"s);
    }
    {
    const auto documents = server.FindTopDocuments("dog"s, []([[maybe_unused]]int document_id,[[maybe_unused]] DocumentStatus status,[[maybe_unused]] int rating) { return true; });
    ASSERT_HINT(documents.empty(), "Empty query predicate error"s);
    }
}

void TestStatusFilter() {
    SearchServer server(""s);
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
    SearchServer server("in on from with the"s);
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
        //                                         |   black         |   | raptor   |   |    cat       |
        ASSERT_HINT((std::abs(found_docs[0].relevance - (log(5.0/1) * 1.0/4 + log(5.0/1)*0 + log(5.0/5)*1.0/4)) < EPSILON), "Incorrect relevance computing"s); //id 33
        //                                         |   black     |   |  raptor      |   |    cat       |
        ASSERT_HINT((std::abs(found_docs[1].relevance - (log(5.0/1) * 0 + log(5.0/1)*1.0/7 + log(5.0/5)*1.0/7)) < EPSILON), "Incorrect relevance computing"s); //id 55
        ASSERT_EQUAL_HINT(count_if(found_docs.begin(),found_docs.end(),[](const Document& d){return d.relevance == 0;}), 3, "Toomany documents with zero relevance"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("black cat in the dark room"s); //id 33
        //                                          |   black        |   |     cat      |   |   dark         |   |    room        |
        ASSERT_HINT((std::abs(found_docs[0].relevance - (log(5.0/1) * 1.0/4 + log(5.0/5) * 1/4 + log(5.0/1) * 1.0/4 + log(5.0/1) * 1.0/4)) < EPSILON),"Incorrect relevance computing"s);
        ASSERT_EQUAL_HINT(count_if(found_docs.begin(),found_docs.end(),[](const Document& d){return d.relevance == 0;}), 4, "Toomany documents with zero relevance"s);
    }
    {
        const auto found_docs = server.FindTopDocuments("orange cat city"s);
        //                                          |   orange       |   |   cat        |   |      city      |
        ASSERT_HINT((std::abs(found_docs[0].relevance - (log(5.0/2) * 1.0/4 + log(5/5) * 1.0/4 + log(5.0/1) * 1.0/4)) < EPSILON),"Incorrect relevance computing"s); //id 11
        //                                          |   orange       |   |   cat        |   |      city    |
        ASSERT_HINT((std::abs(found_docs[1].relevance - (log(5.0/2) * 1.0/7 + log(5.0/5) * 1.0/7 + log(5.0/1) * 0)) < EPSILON),"Incorrect relevance computing"s); //id 55
        ASSERT_EQUAL_HINT(count_if(found_docs.begin(),found_docs.end(),[](const Document& d){return d.relevance == 0;}), 3, "Too many documents with zero relevance"s);
    }
}

void TestConstructor() {
    try {
        SearchServer server("and or n\x12t from"s);
        ASSERT_HINT(false, "Constructor with string fail");
    } catch (const exception& e) {

    }
    try {
        SearchServer server(set<string>{"and"s,"n\x12t"s, "from"s});
        ASSERT_HINT(false, "Constructor with container fail");
    } catch (const exception& e) {

    }
}

void TestAddDocument() {
    SearchServer server("and or from"s);
    server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
    server.AddDocument(4, "большой пёс скворец евгений"s, DocumentStatus::ACTUAL, {1, 1, 1});
    try {
        server.AddDocument(1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2});
        ASSERT_HINT(false, "Adding duplicate document fail");
    } catch (const exception& e) {}
    try {
        server.AddDocument(-1, "пушистый пёс и модный ошейник"s, DocumentStatus::ACTUAL, {1, 2});
        ASSERT_HINT(false, "Adding document with Id <0 fail");
    } catch (const exception& e) {}
    try {
        server.AddDocument(3, "большой пёс скво\x12рец евгений"s, DocumentStatus::ACTUAL, {1, 3, 2});
        ASSERT_HINT(false, "Adding document with special symbol fail");
    } catch (const exception& e) {}
}

void TestMatchAndFindDocument() {
    SearchServer server("and or from"s);
    server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, {7, 2, 7});
    server.AddDocument(4, "большой пёс скворец евгений"s, DocumentStatus::ACTUAL, {1, 1, 1});
    try {
        server.FindTopDocuments("пушистый --кот"s);
        ASSERT_HINT(false, "Double - search fail");
    } catch (const exception& e) {}
    try {
        server.FindTopDocuments("пушистый -"s);
        ASSERT_HINT(false, "Single - search fail");
    } catch (const exception& e) {}
    try {
        server.FindTopDocuments("пушистый скво\x12рец"s);
        ASSERT_HINT(false, "Special symbol search fail");
    } catch (const exception& e) {}
    try {
        server.MatchDocument("модный --пёс"s,0);
        ASSERT_HINT(false, "Double - match fail");
    } catch (const exception& e) {}
    try {
        server.MatchDocument("пушистый - хвост"s,0);
        ASSERT_HINT(false, "Single - match fail");
    } catch (const exception& e) {}
    try {
        server.MatchDocument("пушистый скво\x12рец"s,0);
        ASSERT_HINT(false, "Special symbol match fail");
    } catch (const exception& e) {}
}

void TestRemoveDuplicates() {
        SearchServer search_server("and with"s);
        AddDocument(search_server, 1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, {7, 2, 7});
        AddDocument(search_server, 2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2});
        // дубликат документа 2, будет удалён
        AddDocument(search_server, 3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2});
        // отличие только в стоп-словах, считаем дубликатом
        AddDocument(search_server, 4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, {1, 2});
        // множество слов такое же, считаем дубликатом документа 1
        AddDocument(search_server, 5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, {1, 2});
        // добавились новые слова, дубликатом не является
        AddDocument(search_server, 6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, {1, 2});
        // множество слов такое же, как в id 6, несмотря на другой порядок, считаем дубликатом
        AddDocument(search_server, 7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, {1, 2});
        // есть не все слова, не является дубликатом
        AddDocument(search_server, 8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, {1, 2});
        // слова из разных документов, не является дубликатом
        AddDocument(search_server, 9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, {1, 2});
        RemoveDuplicates(search_server);
        ASSERT_HINT(search_server.GetDocumentCount()==5, "RemoveDuplicates fail");
        search_server.RemoveDocument(6);
        search_server.RemoveDocument(9);
        ASSERT_HINT(search_server.GetDocumentCount()==3, "RemoveDocument fail");
        const auto documents = search_server.FindTopDocuments("pet"s, DocumentStatus::ACTUAL);
        ASSERT_EQUAL_HINT(documents.size(), 3, "Search result error"s);
        ASSERT_HINT(count_if(documents.begin(),documents.end(),[](const Document& d){return d.id == 1;}), "Document 1 missed"s);
        ASSERT_HINT(count_if(documents.begin(),documents.end(),[](const Document& d){return d.id == 2;}), "Document 2 missed"s);
        ASSERT_HINT(count_if(documents.begin(),documents.end(),[](const Document& d){return d.id == 8;}), "Document 8 missed"s);
}

void TestPaginate() {
    SearchServer search_server("and with"s);

    search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2, 3});
    search_server.AddDocument(3, "big cat nasty hair"s, DocumentStatus::ACTUAL, {1, 2, 8});
    search_server.AddDocument(4, "big dog cat Vladislav"s, DocumentStatus::ACTUAL, {1, 3, 2});
    search_server.AddDocument(5, "big dog hamster Borya"s, DocumentStatus::ACTUAL, {1, 1, 1});

    const auto search_results = search_server.FindTopDocuments("curly dog"s);
    int page_size = 2;
    const auto pages = Paginate(search_results, page_size);
    ASSERT_EQUAL_HINT(distance(pages.begin(),pages.end()), 2, "Pages count error"s);
    auto page = pages.begin();

    ASSERT_EQUAL_HINT(distance((*page).begin(),(*page).end()), 2, "Page 1 error"s);
    ++page;
    ASSERT_EQUAL_HINT(distance((*page).begin(),(*page).end()), 1, "Page 2 error"s);
}

void TestQueue() {
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
      ASSERT_EQUAL_HINT(request_queue.GetNoResultRequests(), 1437, "Request queue error"s);
}







