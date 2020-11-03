#pragma once

#include <iostream>
#include <vector>

template <typename Iterator>
class IteratorRange {
public:
    IteratorRange(Iterator first, Iterator last):
        first_(first), last_(last) {}
    Iterator begin() const {return first_;}
    Iterator end() const {return last_;}
    size_t size() const {return distance(first_,last_);}

private:
    Iterator first_;
    Iterator last_;
};

template <typename T>
std::ostream& operator<<(std::ostream& out, const IteratorRange<T>& range) {
    for (auto i = range.begin(); i != range.end(); ++i) {
        out << *i;
    }
    return out;
}

template <typename Iterator>
class Paginator {
public:
    Paginator(Iterator first, Iterator last, int page_size = 5)
    {
        Iterator PageStart = first;
        for (Iterator i = first; i != last; ++i) {
            if ( (distance(first,i) > 0) && (distance(first,i) % page_size == 0) ) {
                pages_.push_back(IteratorRange(PageStart,i));
                PageStart = i;
            }
        }
        pages_.push_back(IteratorRange(PageStart,last));
    }
    Iterator begin() const{
        return pages_.begin();
    }
    Iterator end() const{
        return pages_.end();
    }

private:
    std::vector<IteratorRange<Iterator>> pages_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}
