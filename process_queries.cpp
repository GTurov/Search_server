#include "process_queries.h"

#ifdef MULTITHREAD

#include <execution>

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server,
                                                  const std::vector<std::string>& queries) {
    std::vector<std::vector<Document>> result(queries.size());
    std::transform(std::execution::par, queries.begin(), queries.end(),result.begin(),[&](std::string query)
    {return search_server.FindTopDocuments(query);});
    return result;
}

std::vector<Document> ProcessQueriesJoined(const SearchServer& search_server,
                                         const std::vector<std::string>& queries) {
    std::vector<Document> result;
    for (const auto& documents: ProcessQueries(search_server, queries) ) {
        for (const Document& document : documents) {
            result.push_back(document);
        }
    }
    return result;
}
#endif
