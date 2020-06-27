/****************************************************************************
  FileName     [ myMinHeap.h ]
  PackageName  [ util ]
  Synopsis     [ Define MinHeap ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_MIN_HEAP_H
#define MY_MIN_HEAP_H

#include <algorithm>
#include <vector>
#include <cassert>
#include <iostream>

#define ZIYIHeap 35

template <class Data>
class MinHeap
{
public:
    MinHeap(size_t s = 0) { if (s != 0) _data.reserve(s); }
    ~MinHeap() {}

    void clear() { _data.clear(); }

    // For the following member functions,
    // We don't respond for the case vector "_data" is empty!
    const Data& operator [] (size_t i) const { return _data[i]; }   
    Data& operator [] (size_t i) { return _data[i]; }

    size_t size() const { return _data.size(); }

    // TODO
    const Data& min() const 
    { 
      assert(_data.size()>0);
      return _data.at(0); 
    }
    void insert(const Data& d) 
    { 
      if(_data.size()==0)
      {
        _data.push_back(d);
        return;
      }
      _data.push_back(d);//d is _data.at(n)
      size_t n = _data.size();//_data.at(0) has index 1, so that the parent is child/2
      #ifdef ZIYIHeap
      assert(n!=0);
      #endif
      size_t current = n;
      while(current>1)
      {
        //swap current and parent
        size_t parent = current/2;
        if( _data.at(parent-1) <= d)
          break;
        _data.at(current-1) = _data.at(parent-1);
        current = parent;
      }
      _data.at(current-1) = d;
    }
    void delMin() 
    { 
        if(_data.size() == 0)
        {
          #ifdef ZIYIHeap
          cout<<"deleting the min Node but there is no node"<<endl;
          #endif
          return;
        }
        //Data tmp = _data.at(0);
        delData(0);
        //return tmp;
    }
    void delData(size_t i)//i is between 0 and s-1 
    { 
        #ifdef ZIYIHeap
        assert(i>=0 && i<= _data.size()-1);
        #endif
        size_t current = i+1;
        size_t child = 2*current;
        size_t n = _data.size();
        //_data.at(0) = _data.at(n);//copy the last element to the first
        while(child <=n)//while current node has at least one children(left)
        {
          if(child<n)//the current node has right child
          {
            if(_data.at(child)<_data.at(child-1))
              ++child;//wish to find the smaller child
          }
          if(_data.at(n-1) <= _data.at(child-1))
            break;
          _data.at(current-1) = _data.at(child-1);
          current = child;
          child = 2*current;
        }
        _data.at(current-1) = _data.at(n-1);
        _data.pop_back();
    }

//private:
   // DO NOT add or change data members
   vector<Data>   _data;//use a vector(array) to implement minheap
};

#endif // MY_MIN_HEAP_H
