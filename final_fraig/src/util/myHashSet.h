/****************************************************************************
  FileName     [ myHashSet.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashSet ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_SET_H
#define MY_HASH_SET_H

#include <vector>
#include <cassert>
#include <iostream>

#define ZIYIHash 35

using namespace std;

//---------------------
// Define HashSet class
//---------------------
// To use HashSet ADT,
// the class "Data" should at least overload the "()" and "==" operators.
//
// "operator ()" is to generate the hash key (size_t)
// that will be % by _numBuckets to get the bucket number.
// ==> See "bucketNum()"
//
// "operator ==" is to check whether there has already been
// an equivalent "Data" object in the HashSet.
// Note that HashSet does not allow equivalent nodes to be inserted
//
template <class Data>
class HashSet
{
public:
   HashSet(size_t b = 0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashSet() { reset(); }

   // TODO: implement the HashSet<Data>::iterator
   // o An iterator should be able to go through all the valid Data
   //   in the Hash
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashSet<Data>;
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
      vector<Data>* _buckets;//the address of the first bucket in the hash
      size_t _numBuckets;
      vector<Data>* _currentBucket;
      typename vector<Data>::iterator _node;//the address of the bucket of this iterator
   };

   void init(size_t b) 
   { 
      _numBuckets = b; 
      _buckets = new vector<Data>[b+1];
      _buckets[b].clear();
      //the last vector in _buckets is for end
   }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<Data>& operator [] (size_t i) { return _buckets[i]; }
   const vector<Data>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const 
   { 
      vector<Data>* currentBucket = _buckets;
      while((size_t)(currentBucket-_buckets) < _numBuckets)
      {
         if(currentBucket->size() > 0)
            break;
         ++currentBucket;
      }
      if(currentBucket == _buckets+_numBuckets)//nothing inside
      {
         #ifdef ZIYIHash_end
         cout<<"begin is end"<<endl;
         #endif
         return end();
      }
      return iterator(_buckets,_numBuckets,currentBucket,0); 
   }
   // Pass the end
   iterator end() const 
   { 
      return iterator(_buckets,_numBuckets, _buckets+_numBuckets,0); 
   }
   // return true if no valid data
   bool empty() const 
   { 
      return begin()==end(); 
   }
   // number of valid data
   size_t size() const 
   { 
      size_t s = 0; 
      for(iterator it = begin();it!=end();++it)
      {
         ++s;
      } 
      return s;
   }

   // check if d is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const Data& d) const 
   { 
      size_t x = bucketNum(d);
      for(int i=0;i<(int)_buckets[x].size();++i)
      {
         if(d==_buckets[x].at(i))
            return true;
      }
      return false; 
   }

   // query if d is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(Data& d) const 
   { 
      size_t x = bucketNum(d);
      for(int i=0;i<(int)_buckets[x].size();++i)
      {
         if(d==_buckets[x].at(i))
         {
            d = _buckets[x].at(i);
            return true;
         }
      }
      return false; 
   }

   // update the entry in hash that is equal to d (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const Data& d) 
   { 
      
      size_t x = bucketNum(d);
      for(int i=0;i<(int)_buckets[x].size();++i)
      {
         if(d==_buckets[x].at(i))
         {
            const_cast<Data&>(_buckets[x].at(i)) = d;
            return true;
         }
      }
      insert(d);
      return false; 
   }

   // return true if inserted successfully (i.e. d is not in the hash)
   // return false is d is already in the hash ==> will not insert
   bool insert(const Data& d) 
   { 
      if(check(d))return false;
      size_t x = bucketNum(d);
      _buckets[x].push_back(d);
      cout<<"Task node inserted: ("<<d.getName()<<", "<<d.getLoad()<<")"<<endl;
      return true; 
   }

   // return true if removed successfully (i.e. d is in the hash)
   // return false otherwise (i.e. nothing is removed)
   bool remove(const Data& d) 
   {
      size_t x = bucketNum(d);
      typename vector<Data>::iterator it;
      for(it = _buckets[x].begin(); it!=_buckets[x].end(); ++it)
      {
         if(d==(*it))
            break;
      }
      if(it!=_buckets[x].end())
      {
         _buckets[x].erase(it);
         return true;
      }
      return false; 
   }

private:
   // Do not add any extra data member
   size_t            _numBuckets;
   vector<Data>*     _buckets;
   size_t bucketNum(const Data& d) const {
      return (d() % _numBuckets); }
};

#endif // MY_HASH_SET_H
