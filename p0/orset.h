#pragma once
#include <algorithm>
#include <cstddef>
#include <limits>
#include <set>
#include <utility>

template<typename T, typename UID = std::size_t>
class orset {
    using MemberType = std::set<std::pair<T, UID>>;
private:
    MemberType elements_;
    MemberType tomb_;
public:
    orset() = default;
    orset(const orset& other) = default;
    orset(orset&& other) = default;
    orset& operator=(const orset& other) = default;
    orset& operator=(orset&& other) = default;
    ~orset() = default;


    bool contain(const T& e) const {
        auto it = elements_.lower_bound(std::make_pair(e, std::numeric_limits<UID>::min()));
        if(it == elements_.end()) return false;
        while(it != elements_.end() && it->first == e) {
            if(tomb_.find(*it) == tomb_.end()) return true;
            ++it;
        }
        return false;
    }
    void add(const T& e, UID uid) {
        elements_.emplace(e, uid);
    }
    void remove(const T& e) {
        auto it = elements_.lower_bound(std::make_pair(e, std::numeric_limits<UID>::min()));
        while (it != elements_.end() && it->first == e) {
            tomb_.emplace(*it);
            it = elements_.erase(it);
        }
    }
    void merge(const orset<T, UID>& other) {
        tomb_.insert(other.tomb_.begin(), other.tomb_.end());
        elements_.insert(other.elements_.begin(), other.elements_.end());
        auto it = elements_.begin();
        while (it != elements_.end()) {
            if (tomb_.find(*it) != tomb_.end()) {
                it = elements_.erase(it);
            } else {
                ++it;
            }
        }
    }
};

