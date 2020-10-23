#pragma once

#include "search_server.h"

#include <vector>
#include <string>
#include <queue>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server):
        search_server_(search_server)    {
    }

    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
        return ProcessQeque(raw_query,search_server_.FindTopDocuments(raw_query,document_predicate));
    }
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);
    std::vector<Document> AddFindRequest(const std::string& raw_query);
    int GetNoResultRequests() const;

private:
    struct QueryResult {
        std::string raw_query;
        std::vector<Document> result;
        bool isNoResult;
    };
    std::vector<Document> ProcessQeque(const std::string& raw_query, std::vector<Document> result);

    std::deque<QueryResult> requests_;
    const static int sec_in_day_ = 1440;
    const SearchServer& search_server_;
};
