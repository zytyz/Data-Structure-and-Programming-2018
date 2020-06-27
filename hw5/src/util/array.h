/****************************************************************************
  FileName     [ array.h ]
  PackageName  [ util ]
  Synopsis     [ Define dynamic array package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef ARRAY_H
#define ARRAY_H

#include <cassert>
#include <algorithm>

using namespace std;

// NO need to implement class ArrayNode
//
template <class T>
class Array
{
public:
   // TODO: decide the initial value for _isSorted
   Array() : _data(0), _size(0), _capacity(0),_isSorted(false) {}
   ~Array() { delete []_data; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class Array;

   public:
      iterator(T* n= 0): _node(n) {}
      iterator(const iterator& i): _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const 
      { 
         return *(_node); 
      }
      T& operator * () 
      { 
         return *(_node); 
      }
      iterator& operator ++ () 
      { 
         ++_node;
         return (*this); 
      }
      iterator operator ++ (int) 
      { 
         iterator tmp = (*this);
         ++(*this);
         return tmp; 
      }
      iterator& operator -- () 
      { 
         --_node;
         return (*this); 
      }
      iterator operator -- (int) 
      { 
         iterator tmp = (*this);
         --(*this);
         return tmp; 
      }

      iterator operator + (int i) const 
      { 
         iterator sum(_node +i);
         return sum; 
      }
      iterator& operator += (int i) 
      { 
         _node+=i;
         return (*this); 
      }

      iterator& operator = (const iterator& i) 
      { 
         _node = i._node;
         return (*this); 
      }

      bool operator != (const iterator& i) const 
      { 
         return (_node != i._node); 
      }
      bool operator == (const iterator& i) const 
      { 
         return (_node == i._node); 
      }

   private:
      T*    _node;
   };

   // TODO: implement these functions
   iterator begin() const 
   { 
      if(_capacity==0)return 0;
      //if(empty())return end();
      return iterator(_data); 
   }
   iterator end() const 
   { 
      if(_capacity==0)return 0;
      return iterator(_data + _size);
   }
   bool empty() const 
   { 
      return _size==0; 
   }
   size_t size() const 
   { 
      return _size; 
   }

   T& operator [] (size_t i) //check valid
   { 
      assert(i<_capacity);
      return _data[i]; 
   }
   const T& operator [] (size_t i) const 
   { 
      assert(i<_capacity);
      return _data[i]; 
   }

   void push_back(const T& x) //check capacity and size
   { 
      if(_size == _capacity)expand();
      _data[_size] = x;
      ++_size;
      _isSorted = false;

   }
   void pop_front() 
   { 
      if(empty())return;
      if(_size == 1) 
      {
         _size=0;
         return;
      }
      *(_data) = *(_data+_size-1);
      --_size;
      _isSorted = false;
   }
   void pop_back() 
   { 
      if(empty())return;
      --_size;
      _isSorted = false;
   }

   bool erase(iterator pos) 
   { 
      if(empty())return false;
      *pos = *(_data+_size-1);
      --_size;
      _isSorted = false;
      return true; 
   }
   bool erase(const T& x) 
   { 
      if(empty())return false;
      int i=0;
      for(i=0;i<(int)_size;++i)
      {
         if(_data[i] == x)
            break;
      }
      if(i==(int)_size)return false;
      _data[i] = _data[_size-1];
      --_size;
      _isSorted = false;
      return true; 
   }

   iterator find(const T& x) 
   { 
      int i=0;
      for(i=0;i<(int)_size;++i)
      {
         if(_data[i] == x)
            break;
      }
      return iterator(_data+i);
   }

   void clear() 
   {
      _size=0;
   }

   // [Optional TODO] Feel free to change, but DO NOT change ::sort()
   void sort() const 
   { 
      if(_isSorted)return;
      if (!empty()) ::sort(_data, _data+_size);
      _isSorted = true; 
   }

   // Nice to have, but not required in this homework...
   // void reserve(size_t n) { ... }
   // void resize(size_t n) { ... }

private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   T*            _data;
   size_t        _size;       // number of valid elements
   size_t        _capacity;   // max number of elements
   mutable bool  _isSorted;   // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] Helper functions; called by public member functions
   void expand()//release current memory amd copy data to new resized array
   {
      if(_capacity ==0)_capacity=1;
      else _capacity*=2;
      T* newData = new T [_capacity];
      for(int i=0;i<(int)_size;++i)
      {
         newData[i] = _data[i];
      }
      delete [] _data;
      _data = newData;
   }
};

#endif // ARRAY_H
