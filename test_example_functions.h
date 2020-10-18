#pragma once

#include <ostream>
#include <string>

#include "search_server.h"

//using std::literals::operator""s;

template < typename T1, typename T2>
std::ostream& operator<<(std::ostream& out, const std::pair<T1, T2>& element) {
    out << element.first <<std::string(": ") << element.second;
    return out;
}

template <typename Container>
void Print(std::ostream& out, const Container& container){
    bool first = true;
    for (const auto& element : container) {
        if (!first)
        out << ", "s ;
        out << element ;
        first = false;
    }
    return;
}

template < typename Element>
std::ostream& operator<<(std::ostream& out, const std::vector<Element>& container) {
    out << "["s;
    Print(out, container);
    out << "]"s;
    return out;
}

template < typename Element>
std::ostream& operator<<(std::ostream& out, const std::set<Element>& container) {
    out << "{"s;
    Print(out, container);
    out << "}"s;
    return out;
}

template < typename Key, typename Value>
std::ostream& operator<<(std::ostream& out, const std::map<Key, Value>& container) {
    out << "{"s;
    Print(out, container);
    out << "}"s;
    return out;
}

template <typename T, typename U>
void AssertEqualImpl(const T& t, const U& u, const std::string& t_str, const std::string& u_str, const std::string& file,
                     const std::string& func, unsigned line, const std::string& hint) {
    if (t != u) {
        std::cout << std::boolalpha;
        std::cout << file << "("s << line << "): "s << func << ": "s;
        std::cout << "ASSERT_EQUAL("s << t_str << ", "s << u_str << ") failed: "s;
        std::cout << t << " != "s << u << "."s;
        if ( !hint.empty() ) {
            std::cout << " Hint: "s << hint;
        }
        std::cout << std::endl;
        abort();
    }
}

#define ASSERT_EQUAL(a, b) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_EQUAL_HINT(a, b, hint) AssertEqualImpl((a), (b), #a, #b, __FILE__, __FUNCTION__, __LINE__, (hint))

void AssertImpl(bool value, const std::string& expr_str, const std::string& file, const std::string& func, unsigned line,
                const std::string& hint);

#define ASSERT(expr) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, ""s)

#define ASSERT_HINT(expr, hint) AssertImpl(!!(expr), #expr, __FILE__, __FUNCTION__, __LINE__, (hint))

template <typename TestFunc>
void RunTestImpl(TestFunc& func, const std::string& FunctionName) {
    func();
    std::cerr << FunctionName << " OK"s << std::endl;
}

#define RUN_TEST(func)  RunTestImpl((func), #func )

void TestFindWordsFromAddedDocument();

void TestExcludeStopWordsFromAddedDocumentContent();

void TestExcludeMinusWordsDocumentContent();

void TestMatchingDocuments();

void TestRelevanceSorting();

void TestRatingComputing();

void TestPredicate();

void TestStatusFilter();

void TestRelevanceCheck();

void TestConstructor();

void TestAddDocument();

void TestMatchAndFindDocument();

void TestCheckIds();

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
