//
// Created by zhenyus on 11/16/19.
//


#include "relaxed_belady.h"
#include "utils.h"

using namespace std;

bool RelaxedBeladyCache::lookup(const SimpleRequest &_req) {
    auto &req = dynamic_cast<const AnnotatedRequest &>(_req);
    current_t = req.seq;
    auto it = key_map.find(req.id);
    if (it != key_map.end()) {
        //update past timestamps
        auto list_idx = it->second.first;
        if (within_boundary == list_idx) {
            if (req.next_seq - current_t >= belady_boundary) {
                //TODO: should modify instead of insert
                it->second = {beyond_boundary, beyond_boundary_meta.size()};
                beyond_boundary_meta.emplace_back(req);
            } else {
                within_boundary_meta.emplace(req.next_seq, pair(req.id, req.size));
            }
            within_boundary_meta.erase(req.seq);
        } else {
            auto pos = it->second.second;
            auto &meta = beyond_boundary_meta[pos];
            assert(meta._key == req.id);
            meta.update(req);
            if (meta._future_timestamp - current_t < belady_boundary) {
                beyond_meta_remove_and_append(pos);
            }
        }
        return true;
    }
    return false;
}

void RelaxedBeladyCache::admit(const SimpleRequest &_req) {
    auto &req = static_cast<const AnnotatedRequest &>(_req);
    const uint64_t &size = req.size;
    // object feasible to store?
    if (size > _cacheSize) {
        LOG("L", _cacheSize, req.id, size);
        return;
    }

    auto it = key_map.find(req.id);
    if (it == key_map.end()) {
        if (req.next_seq - req.seq >= belady_boundary) {
            key_map.insert({req.id, {beyond_boundary, beyond_boundary_meta.size()}});
            beyond_boundary_meta.emplace_back(req);
        } else {
            key_map.insert({req.id, {within_boundary, 0}});
            within_boundary_meta.emplace(req.next_seq, pair(req.id, req.size));
        }
        _currentSize += size;
    }

    // check more eviction needed?
    while (_currentSize > _cacheSize) {
        evict();
    }
}


pair<RelaxedBeladyCache::MetaT, uint32_t> RelaxedBeladyCache::rank() {
    auto rand_idx = _distribution(_generator);
    while (!beyond_boundary_meta.empty()) {
        auto pos = rand_idx % beyond_boundary_meta.size();
        auto &meta = beyond_boundary_meta[pos];
        if (meta._future_timestamp - current_t < belady_boundary) {
            beyond_meta_remove_and_append(pos);
        } else {
            return {beyond_boundary, pos};
        }
    }
    return {within_boundary, 0};
}

void RelaxedBeladyCache::evict() {
    auto epair = rank();
    auto &meta_type = epair.first;
    auto &old_pos = epair.second;
//
//    //record meta's future interval
//
//#ifdef EVICTION_LOGGING
//    {
//        auto &meta = meta_holder[0][old_pos];
//        //record eviction decision quality
//        unsigned int decision_qulity =
//                static_cast<double>(meta._future_timestamp - current_t) / (_cacheSize * 1e6 / byte_million_req);
//        decision_qulity = min((unsigned int) 255, decision_qulity);
//        eviction_distances.emplace_back(decision_qulity);
//    }
//#endif

    if (within_boundary == meta_type) {
        auto it = within_boundary_meta.begin();
        key_map.erase(it->second.first);
        _currentSize -= it->second.second;
        within_boundary_meta.erase(it);
    } else {
        beyond_meta_remove(old_pos);
    }
}

