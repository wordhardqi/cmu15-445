/*
 * extendible_hash.h : implementation of in-memory hash table using extendible
 * hashing
 *
 * Functionality: The buffer pool manager must maintain a page table to be able
 * to quickly map a PageId to its corresponding memory location; or alternately
 * report that the PageId does not match any currently-buffered page.
 */

#pragma once

#include <cstdlib>
#include <vector>
#include <string>
#include <memory>
#include <map>
#include "hash/hash_table.h"

namespace cmudb {

template<typename K, typename V>
class ExtendibleHash : public HashTable<K, V> {

  struct Bucket {
    Bucket() = default;
    Bucket(size_t id, int depth) : bucket_id(id), bucket_depth(depth), next(nullptr) {

    }
    size_t NumElems() const {
      return bucket_items.size();
    }


    std::shared_ptr<Bucket> next;
    std::map<K, V> bucket_items;

    size_t bucket_id;
    int bucket_depth;
  };
  typedef std::shared_ptr<Bucket> BucketPtr;
 public:
  // constructor
  ExtendibleHash(size_t size);
  // helper function to generate hash addressing
  size_t HashKey(const K &key);
  // helper function to get global & local depth
  int GetGlobalDepth() const;
  int GetLocalDepth(int bucket_id) const;
  int GetNumBuckets() const;
  // lookup and modifier
  bool Find(const K &key, V &value) override;
  bool Remove(const K &key) override;
  void Insert(const K &key, const V &value) override;

 private:
  // add your own member variables here
  size_t BucketIndex(const K & Key);

  BucketPtr SplitOrOverFlow(BucketPtr bucket);
  const size_t bucket_size_;
  int depth_;
  std::vector<BucketPtr> buckets_;
  size_t num_buckets_;



};
} // namespace cmudb
