//
// Created by zhenyus on 11/8/18.
//

#ifndef WEBCACHESIM_RANDOM_VARIANTS_H
#define WEBCACHESIM_RANDOM_VARIANTS_H

#include "cache.h"
#include "cache_object.h"
#include "pickset.h"
#include <unordered_map>
#include <list>
#include <cmath>
#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>

using namespace std;

class RandomCache : public Cache
{
public:
    PickSet<uint64_t> key_space;
    unordered_map<uint64_t, uint64_t> object_size;

    RandomCache()
        : Cache()
    {
    }
    virtual ~RandomCache()
    {
    }

    virtual bool lookup(SimpleRequest& req);
    virtual void admit(SimpleRequest& req);
    virtual void evict();
    virtual void evict(SimpleRequest& req) {
        //no need to use it
    };
};

static Factory<RandomCache> factoryRandom("Random");




#endif //WEBCACHESIM_RANDOM_VARIANTS_H
