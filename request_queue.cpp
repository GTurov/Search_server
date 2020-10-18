#include "request_queue.h"

#include <algorithm>

using namespace std;

vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {
    return ProcessQeque(raw_query,search_server_.FindTopDocuments(raw_query, status));
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
    return ProcessQeque(raw_query,search_server_.FindTopDocuments(raw_query));
}

int RequestQueue::GetNoResultRequests() const {
    return count_if(requests_.begin(),requests_.end(),[](const QueryResult& r){return r.isNoResult;});
}

vector<Document> RequestQueue::ProcessQeque(const string& raw_query, vector<Document> result) {
    QueryResult r;
    r.raw_query = raw_query;
    r.result = result;
    r.isNoResult = r.result.empty();
    requests_.push_back(r);
    if (requests_.size() > sec_in_day_) {
        requests_.pop_front();
    }
    return r.result;
}
