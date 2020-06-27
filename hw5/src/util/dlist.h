/****************************************************************************
  FileName     [ dlist.h ]
  PackageName  [ util ]
  Synopsis     [ Define doubly linked list package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef DLIST_H
#define DLIST_H

#include <cassert>

template <class T> class DList;

// DListNode is supposed to be a private class. User don't need to see it.
// Only DList and DList::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class DListNode
{
   friend class DList<T>;
   friend class DList<T>::iterator;

   DListNode(const T& d, DListNode<T>* p = 0, DListNode<T>* n = 0):
      _data(d), _prev(p), _next(n) {}

   // [NOTE] DO NOT ADD or REMOVE any data member
   T              _data;
   DListNode<T>*  _prev;
   DListNode<T>*  _next;
};


template <class T>
class DList
{
public:
   // TODO: decide the initial value for _isSorted
   DList() :_isSorted(false)
   {
      _head = new DListNode<T>(T());
      _head->_prev = _head->_next = _head; // _head is a dummy node
   }
   ~DList() { clear(); delete _head; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class DList;

   public:
      iterator(DListNode<T>* n= 0): _node(n) {}
      iterator(const iterator& i) : _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const { return _node->_data; }
      T& operator * () { return _node->_data; }
      iterator& operator ++ () 
      { 
         _node = _node->_next;
         return (*this); 

      }//++it, no need to copy, faster
      iterator operator ++ (int) 
      { 
         iterator tmp = (*this);
         ++(*this);
         return tmp; 
      }//it++, slower
      iterator& operator -- () 
      { 
         _node = _node->_prev;
         return (*this); 
      }//--it
      iterator operator -- (int) 
      { 
         iterator tmp = (*this); 
         --(*this);
         return tmp; 
      }//it--

      iterator& operator = (const iterator& i) 
      { 
         _node = i._node;
         return *(this); 
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
      DListNode<T>* _node;
   };

   // TODO: implement these functions
   iterator begin() const 
   { 
      //if(empty())return end();
      return iterator(_head->_next); 
   }
   iterator end() const 
   { 
      return iterator(_head); 
   }
   bool empty() const 
   { 
      return (begin() == end()); 
   }
   size_t size() const 
   {  
      size_t size=0;
      DListNode<T>* tmp = _head;
      while(tmp != _head->_prev)
      {
         ++size;
         tmp = tmp->_next;
      }
      return size; 
   }

   void push_back(const T& x)
   { 
      DListNode<T>* add = new DListNode<T>(x,_head->_prev,_head);
      /*if(empty())
      {
         _head->next = add;
      }*/
      _head->_prev = ((_head->_prev)->_next = add);
      _isSorted  = false;
   }
   void pop_front() 
   { 
      if(empty())return;
      DListNode<T>* first = _head->_next;
      _head->_next = first->_next;
      (_head->_next)->_prev = _head;
      delete first;
   }
   void pop_back() 
   { 
      if(empty())return;
      DListNode<T>* last = _head->_prev;
      _head->_prev = last->_prev;
      (_head->_prev)->_next = _head;
      delete last;
   }

   // return false if nothing to erase
   bool erase(iterator pos)//pos is an iterator (node pointer)
   { 
      if(empty())return false;
      /*iterator it = begin();
      while(it!=end())//end() means _head
      {
         if(it == pos)
            break;
         ++it;
      }*/
      //assert(it!=end());
      //if(it == end()) return false;//no such element
      DListNode<T>* & node = pos._node;
      (node->_prev)->_next = node->_next;
      (node->_next)->_prev = node->_prev;
      delete node;
      return true; 
   }
   bool erase(const T& x) 
   { 
      if(empty())return false;
      iterator it = begin();
      while(it!=end())//end() means _head
      {
         if(*it == x)
            break;
         ++it;
      }
      if(it == end()) return false;//no such element
      DListNode<T>* & node = it._node;
      (node->_prev)->_next = node->_next;
      (node->_next)->_prev = node->_prev;
      delete node;
      return true; 
   }

   iterator find(const T& x) 
   { 
      iterator it = begin();
      while(it!=end())//end() means _head
      {
         if(*it == x)
            break;
         ++it;
      }
      return it; //return end() if nothing is found
   }

   void clear() 
   {
      if(empty())return;
      size_t siz = size();
      for(int i=0;i<(int)siz;++i)
      {
         pop_front();
      }
   }  // delete all nodes except for the dummy node


   /*void sort(iterator low = begin(), iterator high = end()) const
   {

   }*/
   void sort() const 
   { 
      if(_isSorted)return;
      iterator index = begin();//compare all the data after index
      size_t siz = size();
      for(int i=0;i<(int)(siz-1);++i)//find min (size-1) times
      {
         //assert(index!=(--end()));//index should not be the last element
         iterator min = index;
         for(iterator it = index; it!=end(); ++it)
         {
            if( (*it) < (*min))
            {
               min = it;
            }
         }
         swap(index,min);
         ++index;
      }
      _isSorted = true; 
   }

private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   DListNode<T>*  _head;     // = dummy node if list is empty
   mutable bool   _isSorted; // (optionally) to indicate the array is sorted
   // [OPTIONAL TODO] helper functions; called by public member functions
   void swap(iterator& it1, iterator& it2) const
   {
      T tmp = *it1;
      *it1 = *it2;
      *it2 = tmp;
   }
   
};

#endif // DLIST_H
