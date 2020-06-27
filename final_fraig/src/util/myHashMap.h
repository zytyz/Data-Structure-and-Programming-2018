/****************************************************************************
  FileName     [ myHashMap.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashMap and Cache ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2009-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_MAP_H
#define MY_HASH_MAP_H

#include <vector>

using namespace std;

// TODO: (Optionally) Implement your own HashMap and Cache classes.

//-----------------------
// Define HashMap classes
//-----------------------
// To use HashMap ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//


//

/*
template <class HashKey, class HashData>
class HashMap
{
typedef pair<HashKey, HashData> HashNode;

public:
   HashMap(size_t b=0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashMap() { reset(); }

   // [Optional] TODO: implement the HashMap<HashKey, HashData>::iterator
   // o An iterator should be able to go through all the valid HashNodes
   //   in the HashMap
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashMap<HashKey,HashData>;
   public:
      iterator(vector<Data>* b =0 ,size_t n=0 ,vector<Data>* cb = 0 ,size_t index = 0)
         :_buckets(b),_numBuckets(n),_currentBucket(cb)//these two are constants->cannot assign iterator to Data in another map
      {
        if(_buckets!=0)
        {
            #ifdef ZIYIHash
            assert((size_t)(_currentBucket - _buckets) <= _numBuckets);
            assert(_currentBucket->size() >= index);//=index is for end
            #endif
            _node = _currentBucket->begin() +index;
        } 
      }
      ~iterator(){}
      iterator& operator = (const iterator&i)
      {
         #ifdef ZIYIHash
         i.valid();
         #endif
         _buckets = i._buckets;
         _numBuckets = i._numBuckets;
         _currentBucket = i._currentBucket;
         _node = i._node;
         return (*this);
      }
      bool operator == (const iterator& i)const
      {
         return (_node == i._node);
      }
      bool operator != (const iterator& i)const
      {
         return (_node != i._node);
      }
      const Data& operator * () const 
      { 
         #ifdef ZIYIHash
         valid();
         #endif
         return (*_node); 
      }
      iterator& operator ++ () 
      { 
         #ifdef ZIYIHash
         valid();//end() of the hash map means the next vector after _buckets+n
         #endif
         if( ++_node == _currentBucket->end())//change to the next bucket
         {
            ++_currentBucket;
            while((size_t)(_currentBucket - _buckets) < _numBuckets)
            {
               if(_currentBucket->size()>0)
                  break;
               ++_currentBucket;
            }
            if(_currentBucket->size()!=0)//has not reached the end of the hash
            {
               _node = _currentBucket->begin();
            }
            else
            {
               #ifdef ZIYIHash_end
               cout<<"reached the end of the hash"<<endl;
               #endif
               _node = _currentBucket->begin();//end() of the map is (_buckets+n)->begin()
            }
         }
         return (*this); 
      }
      iterator operator ++ (int i)
      { 
         iterator tmp = (*this);
         ++(*this);
         return tmp;
      }
      iterator& operator -- ()
      {
         #ifdef ZIYIHash
         valid();
         #endif
         if( _node == _currentBucket->begin())
         {
            vector<Data>* tmp = _currentBucket;
            --_currentBucket;
            while(_currentBucket > _buckets)
            {
               if(_currentBucket->size() >0)
                  break;
               --_currentBucket;
            }
            if((_currentBucket==_buckets) && (_currentBucket->size()==0))
            {
               #ifdef ZIYIHash
               cout<<"reached the begin of the hash"<<endl;
               #endif
               //do not change _node otherwise it would be invalid
               _currentBucket = tmp;
               //change currentBucket back to tmp so that the iterator is still valid
            }
            else
            {
               _node = --(_currentBucket->end());
            }    
         }
         else --_node;
         return (*this);
      }
      iterator operator--(int i)
      {
         iterator tmp = (*this);
         --(*this);
         return tmp;
      }
      
   private:
      void valid() const
      { 
         assert(_buckets!=0);
         assert(_numBuckets!=0);
         bool notEnd = ((size_t)(_currentBucket - _buckets) < _numBuckets) &&
                        (_currentBucket->size()!=0) &&
                        _node!= _currentBucket->end();
         bool isEnd = (_currentBucket == _buckets + _numBuckets) &&
                        (_currentBucket->size()==0);
         assert( isEnd || notEnd);
      }
      vector<HashNode>* _buckets;//the address of the first bucket in the hash
      size_t _numBuckets;
      vector<HashNode>* _currentBucket;
      typename vector<HashNode>::iterator _node;//the address of the bucket of this iterator
   };

   void init(size_t b) 
   {
      reset(); _numBuckets = b; _buckets = new vector<HashNode>[b]; 
   }

   void reset() 
   {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }

   void clear() 
   {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }

   size_t numBuckets() const { return _numBuckets; }

   vector<HashNode>& operator [] (size_t i) { return _buckets[i]; }
   const vector<HashNode>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const { return iterator(); }
   // Pass the end
   iterator end() const { return iterator(); }
   // return true if no valid data
   bool empty() const { return true; }
   // number of valid data
   size_t size() const { size_t s = 0; return s; }

   // check if k is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const HashKey& k) const { return false; }

   // query if k is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(const HashKey& k, HashData& d) const { return false; }

   // update the entry in hash that is equal to k (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const HashKey& k, HashData& d) { return false; }

   // return true if inserted d successfully (i.e. k is not in the hash)
   // return false is k is already in the hash ==> will not insert
   bool insert(const HashKey& k, const HashData& d) { return true; }

   // return true if removed successfully (i.e. k is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const HashKey& k) { return false; }

private:
   // Do not add any extra data member
   size_t                   _numBuckets;
   vector<HashNode>*        _buckets;

   size_t bucketNum(const HashKey& k) const {
      return (k() % _numBuckets); }

};


//---------------------
// Define Cache classes
//---------------------
// To use Cache ADT, you should define your own HashKey class.
// It should at least overload the "()" and "==" operators.
//
// class CacheKey
// {
// public:
//    CacheKey() {}
//    
//    size_t operator() () const { return 0; }
//   
//    bool operator == (const CacheKey&) const { return true; }
//       
// private:
// }; 
// 
template <class CacheKey, class CacheData>
class Cache
{
typedef pair<CacheKey, CacheData> CacheNode;

public:
   Cache() : _size(0), _cache(0) {}
   Cache(size_t s) : _size(0), _cache(0) { init(s); }
   ~Cache() { reset(); }

   // NO NEED to implement Cache::iterator class

   // TODO: implement these functions
   //
   // Initialize _cache with size s
   void init(size_t s) { reset(); _size = s; _cache = new CacheNode[s]; }
   void reset() {  _size = 0; if (_cache) { delete [] _cache; _cache = 0; } }

   size_t size() const { return _size; }

   CacheNode& operator [] (size_t i) { return _cache[i]; }
   const CacheNode& operator [](size_t i) const { return _cache[i]; }

   // return false if cache miss
   bool read(const CacheKey& k, CacheData& d) const {
      size_t i = k() % _size;
      if (k == _cache[i].first) {
         d = _cache[i].second;
         return true;
      }
      return false;
   }
   // If k is already in the Cache, overwrite the CacheData
   void write(const CacheKey& k, const CacheData& d) {
      size_t i = k() % _size;
      _cache[i].first = k;
      _cache[i].second = d;
   }

private:
   // Do not add any extra data member
   size_t         _size;
   CacheNode*     _cache;
};*/


#endif // MY_HASH_H
