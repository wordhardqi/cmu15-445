#include <list>
#include <cassert>
#include "hash/extendible_hash.h"
#include "page/page.h"

namespace cmudb {

/*
 * constructor
 * array_size: fixed array size for each bucket
 */
template<typename K, typename V>
ExtendibleHash<K, V>::ExtendibleHash(size_t size):bucket_size_(size), depth_(0) {
  buckets_.emplace_back(Bucket(0, 0));
  num_buckets_ = 1;
}

/*
 * helper function to calculate the hashing address of input key
 */
template<typename K, typename V>
size_t ExtendibleHash<K, V>::HashKey(const K &key) {
  return std::hash<K>()(key);
}

/*
 * helper function to return global depth of hash table
 * NOTE: you must implement this function in order to pass test
 */
template<typename K, typename V>
int ExtendibleHash<K, V>::GetGlobalDepth() const {
  return depth_;
}

/*
 * helper function to return local depth of one specific bucket
 * NOTE: you must implement this function in order to pass test
 */
template<typename K, typename V>
int ExtendibleHash<K, V>::GetLocalDepth(int bucket_id) const {
  assert(0 <= bucket_id && static_cast<size_t >(bucket_id) < buckets_.size());
  if (buckets_[bucket_id]!=nullptr) {
    return buckets_[bucket_id]->bucket_depth;
  }
  return -1;

}

/*
 * helper function to return current number of bucket in hash table
 */
template<typename K, typename V>
int ExtendibleHash<K, V>::GetNumBuckets() const {
  return num_buckets_;
}

/*
 * lookup function to find value associate with input key
 */
template<typename K, typename V>
bool ExtendibleHash<K, V>::Find(const K &key, V &value) {
  return false;
}

/*
 * delete <key,value> entry in hash table
 * Shrink & Combination is not required for this project
 */
template<typename K, typename V>
bool ExtendibleHash<K, V>::Remove(const K &key) {
  return false;
}

template<typename K, typename V>
size_t ExtendibleHash<K, V>::BucketIndex(const K &Key) {
  return HashKey(Key) && ((1 << GetGlobalDepth()) - 1);
}




template<typename K, typename V>
typename ExtendibleHash<K, V>::BucketPtr ExtendibleHash<K, V>::SplitOrOverFlow(BucketPtr bucket) {

  assert(bucket->NumElems() > bucket_size_);
  const size_t global_depth = GetGlobalDepth();
  const size_t current_bucket_depth = GetLocalDepth(bucket->bucket_id);
  assert(global_depth >= current_bucket_depth);

  const size_t possible_new_depth = current_bucket_depth + 1;
  std::map<K, V> new_map;
  //ToDO:: Scan overflow buckets.
  for (auto it = bucket->bucket_items.begin(); it!=bucket->bucket_items.end(); ++it) {
    const size_t elem_hask_key = HashKey(it->first) & ((1 << possible_new_depth) -1);
    if (elem_hask_key!=bucket->bucket_id) {
      new_map[it->first] = std::move(bucket->bucket_items.at(it->first));
    }
  }

  auto lambda_split = [](BucketPtr bucket ){


  };

  if (new_map.size()==0) {
    //over flow

    auto cur_bucket = bucket;
    while (bucket->NumElems() > bucket_size_) {
      cur_bucket->next = BucketPtr(new Bucket(0, current_bucket_depth));
      cur_bucket = cur_bucket->next;

      while (cur_bucket->NumElems() <= bucket_size_) {
        auto it = bucket->bucket_items.begin();
        cur_bucket->bucket_items[it->first] = std::move(it->second);
        bucket->bucket_items.erase(it);
      }

    }
  } else {
    //split



  }

}

/*
 * insert <key,value> entry in hash table
 * Split & Redistribute bucket when there is overflow and if necessary increase
 * global depth
 */
template<typename K, typename V>
void ExtendibleHash<K, V>::Insert(const K &key, const V &value) {
  const size_t hash_key = BucketIndex(key);
  if (buckets_[hash_key]==nullptr) {
    buckets_[hash_key] = std::make_shared<Bucket>(new Bucket(hash_key, depth_));
    ++num_buckets_;
  }
  auto bucket = buckets_[hash_key];

  if (bucket->bucket_items.find(key)!=bucket->bucket_items.end()) {
    bucket->bucket_items[key] = value;
    return;
  }
  const int prev_depth = depth_;
  // we first insert new <K,V> then we decide whether to split or overflow or anything.
  bucket->bucket_items[key] = value;

  if (bucket->NumElems() > bucket_size_) {
    auto new_bucket = SplitOrOverFlow(bucket);
    if (new_bucket) {
      if (new_bucket->bucket_depth > prev_depth) {
        depth_ = new_bucket->bucket_depth;
        while (new_bucket->next!=nullptr) {
          new_bucket = new_bucket->next;
        }
        const int resize_factor = 1 << (new_bucket->bucket_depth - prev_depth);
        const size_t old_size = buckets_.size();
        buckets_.resize(old_size*resize_factor);
        const size_t rehashed_key = BucketIndex(key);
        new_bucket->bucket_id = rehashed_key;
        new_bucket->bucket_items[key] = value;
        assert(buckets_[hash_key]!=nullptr);
        buckets_[rehashed_key] = new_bucket;

        for (size_t old_ind = 0; old_ind < old_size; ++old_ind) {
          if (old_ind!=hash_key) {
            for (size_t new_bucket_ind = old_size; new_bucket_ind < buckets_.size(); new_bucket_ind += old_size) {
              buckets_[new_bucket_ind] = buckets_[old_ind];
            }
          }

        }

      } else {
        while (new_bucket->next!=nullptr) {
          new_bucket = new_bucket->next;
        }
        if (new_bucket->bucket_id!=hash_key) {
          // bucket splitting
          new_bucket->bucket_items[key] = value;
          buckets_[new_bucket->bucket_id] = new_bucket;
        } else {
          // overflow
          new_bucket->bucket_items[key] = value;
        }

      }
    }
  } else {
    bucket->bucket_items.insert({key, value});

  }

}

template
class ExtendibleHash<page_id_t, Page *>;
template
class ExtendibleHash<Page *, std::list<Page *>::iterator>;
// test purpose
template
class ExtendibleHash<int, std::string>;
template
class ExtendibleHash<int, std::list<int>::iterator>;
template
class ExtendibleHash<int, int>;
} // namespace cmudb
