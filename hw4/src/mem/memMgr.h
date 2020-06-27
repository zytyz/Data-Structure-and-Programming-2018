/****************************************************************************
  FileName     [ memMgr.h ]
  PackageName  [ cmd ]
  Synopsis     [ Define Memory Manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MEM_MGR_H
#define MEM_MGR_H

#include <cassert>
#include <iostream>
#include <iomanip>
#include <stdlib.h>
#include <utility>

using namespace std;

// Turn this on for debugging
// #define MEM_DEBUG

//--------------------------------------------------------------------------
// Define MACROs
//--------------------------------------------------------------------------
#define MEM_MGR_INIT(T) \
MemMgr<T>* const T::_memMgr = new MemMgr<T>

#define USE_MEM_MGR(T)                                                      \
public:                                                                     \
   void* operator new(size_t t) { return (void*)(_memMgr->alloc(t)); }      \
   void* operator new[](size_t t) { return (void*)(_memMgr->allocArr(t)); } \
   void  operator delete(void* p) { _memMgr->free((T*)p); }                 \
   void  operator delete[](void* p) { _memMgr->freeArr((T*)p); }            \
   static void memReset(size_t b = 0) { _memMgr->reset(b); }                \
   static void memPrint() { _memMgr->print(); }                             \
private:                                                                    \
   static MemMgr<T>* const _memMgr
// The upper part is in the definition of class MemTestObj 
// You should use the following two MACROs whenever possible to 
// make your code 64/32-bit platform independent.
// DO NOT use 4 or 8 for sizeof(size_t) in your code
//
#define SIZE_T      sizeof(size_t)
#define SIZE_T_1    (sizeof(size_t) - 1)

// TODO: Define them by SIZE_T and/or SIZE_T_1 MACROs.
//
// To promote 't' to the nearest multiple of SIZE_T; 
// e.g. Let SIZE_T = 8;  toSizeT(7) = 8, toSizeT(12) = 16
#define toSizeT(t)      \
(((t)%SIZE_T==0)? (t) :((((t)/SIZE_T)+1)*SIZE_T))
// TODO
//
// To demote 't' to the nearest multiple of SIZE_T
// e.g. Let SIZE_T = 8;  downtoSizeT(9) = 8, downtoSizeT(100) = 96
#define downtoSizeT(t)  \
(((t)%SIZE_T==0)? (t) :((((t)/SIZE_T))*SIZE_T))
// TODO

// R_SIZE is the size of the recycle list
#define R_SIZE 256

//--------------------------------------------------------------------------
// Forward declarations
//--------------------------------------------------------------------------
template <class T> class MemMgr;


//--------------------------------------------------------------------------
// Class Definitions
//--------------------------------------------------------------------------
// T is the class that use this memory manager
//
// Make it a private class;
// Only friend to MemMgr;
//
template <class T>
class MemBlock
{
   friend class MemMgr<T>;

   // Constructor/Destructor
   MemBlock(MemBlock<T>* n, size_t b) : _nextBlock(n) {
      _begin = _ptr = new char[b]; _end = _begin + b; }
   ~MemBlock() { delete [] _begin; }

   // Member functions
   void reset() { _ptr = _begin; }//does not release the current memBlock, only clears it
   // 1. Get (at least, need to promote) 't' bytes memory from current block
   //    Promote 't' to a multiple of SIZE_T
   // 2. Update "_ptr" accordingly
   // 3. The return memory address is stored in "ret"
   // 4. Return false if not enough memory
   bool getMem(size_t t, T*& ret)
   {
      // TODO
      //just return true or false, the other part is handled by memMgr
      //any memory given out should be multiple of sizeof(size_t)
      //t: bytes
      assert(t%SIZE_T==0);
      if(getRemainSize()<t)
      {
         ret = (T*)_ptr;
         return false;
      }
      else
      {
         ret = (T*)_ptr;
         _ptr = _ptr + t;
         return true;
      }
      
   }
   size_t getRemainSize() const { return size_t(_end - _ptr); }
      
   MemBlock<T>* getNextBlock() const { return _nextBlock; }

   // Data members
   char*             _begin;
   char*             _ptr;
   char*             _end;
   MemBlock<T>*      _nextBlock;//_nextBlock is the address of the older block
};

// Make it a private class;
// Only friend to MemMgr;
//
template <class T>
class MemRecycleList
{
   //there are 256(R_SIZE) recycle lists in MemMgr
   //_arrSize is the length of one member in recycle list 
   friend class MemMgr<T>;

   // Constructor/Destructor
   MemRecycleList(size_t a = 0) : _arrSize(a), _first(0), _nextList(0) {}
   ~MemRecycleList() { reset(); }

   // Member functions
   // ----------------
   size_t getArrSize() const { return _arrSize; }
   MemRecycleList<T>* getNextList() const { return _nextList; }
   void setNextList(MemRecycleList<T>* l) { _nextList = l; }
   // pop out the first element in the recycle list
   T* popFront() {
      // TODO
      assert(_first!=0);
      T* _firstOld=_first;
      assert(numElm()!=0);//if no elements there is nothing to pop
      _first = (T*) *(size_t**)_first;
      //assert(_firstOld!=0);
      return _firstOld;
   }
   // push the element 'p' to the beginning of the recycle list
   void  pushFront(T* p) {
      // TODO
      //store "first(the address of the current first block in the new one)" in the first 8 bytes of *p
      (*(size_t**)(p)) = (size_t*)_first;
      _first = p;
      assert(_first!=0);
   }
   // Release the memory occupied by the recycle list(s)
   // DO NOT release the memory occupied by MemMgr/MemBlock
   void reset() {
      // TODO
      //direction of linked recycle list is the same as they are stored
     
      if(_nextList!=0)
      {
         //_nextList->reset();
         delete _nextList;
         _nextList=0;
      }
      _first=0;
   }

   // Helper functions
   // ----------------
   // count the number of elements in the recycle list
   size_t numElm() const {
      // TODO
      size_t num=0;
      T* ptr = _first;//for traversing recycle list
      while(ptr!=NULL)
      {
         
         ptr = (T*)*(size_t*)ptr;
         num+=1;
      }
      return num;
   }

   // Data members
   size_t              _arrSize;   // the array size of the recycled data
   T*                  _first;     // the first recycled data
   MemRecycleList<T>*  _nextList;  // next MemRecycleList
                                   //      with _arrSize + x*R_SIZE
};

template <class T>
class MemMgr
{
   #define S sizeof(T)

public:
   MemMgr(size_t b = 65536) : _blockSize(b) 
   {
      assert(b % SIZE_T == 0);
      _activeBlock = new MemBlock<T>(0, _blockSize);//0 is the address for next memBlock
      for (int i = 0; i < R_SIZE; ++i)
         _recycleList[i]._arrSize = i;
         //set the arrSize of recyclelist[i]
         //the list with arrSize=0 is used to store objects
   }
   ~MemMgr() { reset(); delete _activeBlock; }

   // 1. Remove the memory of all but the firstly allocated MemBlocks
   //    That is, the last MemBlock searched from _activeBlock.
   //    reset its _ptr = _begin (by calling MemBlock::reset())
   // 2. reset _recycleList[]
   // 3. 'b' is the new _blockSize; "b = 0" means _blockSize does not change
   //    if (b != _blockSize) reallocate the memory for the first MemBlock
   // 4. Update the _activeBlock pointer
   void reset(size_t b = 0) 
   {
      assert(b % SIZE_T == 0);
      #ifdef MEM_DEBUG
         cout << "Resetting memMgr...(" << b << ")" << endl;
      #endif // MEM_DEBUG
      // TODO
      for (int i = 0; i < R_SIZE; i++)
         _recycleList[i].reset();
      while(getNumBlocks()>1)
      {
         MemBlock<T>* tmp = _activeBlock->getNextBlock();
         delete _activeBlock;
         _activeBlock = tmp;
      }
      assert(_activeBlock->getNextBlock()==0);
      if(b==0)
      {
         _activeBlock->reset();
      }
      else
      {
         delete _activeBlock;
         _activeBlock = new MemBlock<T>(0,b);
         _activeBlock->reset();
         _blockSize = b;
      }
   }
   // Called by new
   T* alloc(size_t t) 
   {
      assert(t == S);
      #ifdef MEM_DEBUG
      cout << "Calling alloc...(" << t << ")" << endl;
      #endif // MEM_DEBUG
      return getMem(t);
   }
   // Called by new[]
   T* allocArr(size_t t) 
   {
      #ifdef MEM_DEBUG
      cout << "Calling allocArr...(" << t << ")" << endl;
      #endif // MEM_DEBUG
      // Note: no need to record the size of the array == > system will do
      return getMem(t);
   }
   // Called by delete
   void  free(T* p) 
   {
      #ifdef MEM_DEBUG
      cout << "Calling free...(" << p << ")" << endl;
      #endif // MEM_DEBUG
      //don't have to consider delete repeatedly, because it is recorded in _objList as 0
      getMemRecycleList(0)->pushFront(p);
   }
   // Called by delete[]
   void  freeArr(T* p) 
   {
      #ifdef MEM_DEBUG
      cout << "Calling freeArr...(" << p << ")" << endl;
      #endif // MEM_DEBUG
      // TODO
      
      // Get the array size 'n' stored by system,
      // which is also the _recycleList index
      // find the recylce list and store p in it
      size_t n = *(size_t*)p;
      #ifdef MEM_DEBUG
         cout << ">> Array size = " << n << endl;
         cout << "Recycling " << p << " to _recycleList[" << n << "]" << endl;
      #endif // MEM_DEBUG
      // add to recycle list...
      getMemRecycleList(n)->pushFront(p);
   }
   void print() const 
   {
      cout << "=========================================" << endl
           << "=              Memory Manager           =" << endl
           << "=========================================" << endl
           << "* Block size            : " << _blockSize << " Bytes" << endl
           << "* Number of blocks      : " << getNumBlocks() << endl
           << "* Free mem in last block: " << _activeBlock->getRemainSize()
           << endl
           << "* Recycle list          : " << endl;
    
      //cout<<"here"<<endl;
      int i = 0, count = 0;
      while (i < R_SIZE) 
      {
         //cerr<<i<<endl;
         assert(i>=0 && i<R_SIZE);
         const MemRecycleList<T>* ll = &(_recycleList[i]);
         while (ll != 0) {
            //cerr << "1" << endl;
            size_t s = ll->numElm();
            //cerr << "2" << endl;
            if (s) {
               cout << "[" << setw(3) << right << ll->_arrSize << "] = "
                    << setw(10) << left << s;
               if (++count % 4 == 0) cout << endl;
            }
            ll = ll->_nextList;
         }
         ++i;
      }
      cout << endl;
   }

private:
   size_t                     _blockSize;
   MemBlock<T>*               _activeBlock;
   MemRecycleList<T>          _recycleList[R_SIZE];

   // Private member functions
   //
   // t: #Bytes; MUST be a multiple of SIZE_T
   // return the size of the array with respect to memory size t
   // [Note] t must >= S
   // [NOTE] Use this function in (at least) getMem() to get the size of array
   //        and call getMemRecycleList() later to get the index for
   //        the _recycleList[]
   size_t getArraySize(size_t t) const 
   {
      //t is from the system when we call new[] or new
      //t is promoted toSizeT(t) before calling this function (in getMem()) 
      //if return 0 ->the user newed an object
      //if new[], t will +8 bytes at first
      assert(t % SIZE_T == 0);
      assert(t >= S);
      // TODO
      size_t size = (t-SIZE_T)/S; 
      assert(size >=0);
      return size;
   }
   // Go through _recycleList[m], its _nextList, and _nexList->_nextList, etc,
   //    to find a recycle list whose "_arrSize" == "n"
   // If not found, create a new MemRecycleList with _arrSize = n
   //    and add to the last MemRecycleList
   // So, should never return NULL
   // [Note]: This function will be called by MemMgr->getMem() to get the
   //         recycle list. Therefore, the recycle list is first created
   //         by the MTNew command, not MTDelete.
   MemRecycleList<T>* getMemRecycleList(size_t n) 
   {
      size_t m = n % R_SIZE;
      // TODO
      MemRecycleList<T>* current = &(_recycleList[m]);
      MemRecycleList<T>* target = 0;
      assert(current!=0);

      while(1)
      {
         assert(current!=0);
         if(current->getArrSize()==n)
         {
            target = current;
            break;
         }
         else if(current->getNextList()==0)
         {
            target = 0;
            break;
         }
         current = current->getNextList();
      }
      assert(current!=0);
      if(target!=0)//found the recyclelist with arrsize n
      {
         assert(target==current);
         return target;
      }
      else //target = 0-> no such list yet
      {
         target = new MemRecycleList<T>(n);
         current->setNextList(target);
         assert(target!=0);
         return target;
      }
   }
   // t is the #Bytes requested from new or new[]
   // Note: Make sure the returned memory is a multiple of SIZE_T
   T* getMem(size_t t) 
   {
      //cout<<"in getMem"<<endl;
      T* ret = 0;
      #ifdef MEM_DEBUG
         cout << "Calling MemMgr::getMem...(" << t << ")" << endl;
      #endif // MEM_DEBUG
      // 1. Make sure to promote t to a multiple of SIZE_T
      t = toSizeT(t);
      // 2. Check if the requested memory is greater than the block size.
      //    If so, throw a "bad_alloc()" exception.
      //    Print this message for exception
      //    cerr << "Requested memory (" << t << ") is greater than block size"
      //         << "(" << _blockSize << "). " << "Exception raised...\n";
      // TODO
      
      if(t>_blockSize)
      {
         cerr  << "Requested memory (" << t << ") is greater than block size"
               << "(" << _blockSize << "). " << "Exception raised...\n";
         throw bad_alloc();
      }

      
      /*catch(bad_alloc& ba)
      {
         cerr  << "Requested memory (" << t << ") is greater than block size"
               << "(" << _blockSize << "). " << "Exception raised...\n";
      }*/

      // 3. Check the _recycleList first...
      //    Print this message for memTest.debug
      //    #ifdef MEM_DEBUG
      //    cout << "Recycled from _recycleList[" << n << "]..." << ret << endl;
      //    #endif // MEM_DEBUG
      //    => 'n' is the size of array
      //    => "ret" is the return address
      size_t n = getArraySize(t);
      // TODO
      //check if target recyclelist has any data member
      MemRecycleList<T>* reList=getMemRecycleList(n);
      if(reList->numElm()!=0)
      {
         ret = reList->popFront();//ret is declared in the first line of this function
         assert(ret!=0);
         #ifdef MEM_DEBUG
            cout << "Recycled from _recycleList[" << n << "]..." << ret << endl;
         #endif // MEM_DEBUG
      //    => 'n' is the size of array
      //    => "ret" is the return address
         //return ret;
      }
      // If no match from recycle list...
      // 4. Get the memory from _activeBlock
      // 5. If not enough, recycle the remained memory and print out ---
      //    Note: recycle to the as biggest array index as possible
      //    Note: rn is the array size
      //    Print this message for memTest.debug
      //    #ifdef MEM_DEBUG
      //    cout << "Recycling " << ret << " to _recycleList[" << rn << "]\n";
      //    #endif // MEM_DEBUG
      //    ==> allocate a new memory block, and print out ---
      //    #ifdef MEM_DEBUG
      //    cout << "New MemBlock... " << _activeBlock << endl;
      //    #endif // MEM_DEBUG
      // TODO
      else if(reList->numElm()==0) // no match from recycle list
      {
         if(!(_activeBlock->getMem(t,ret)))//_activeBlock is not enough
         //if(_activeBlock->getRemainSize()<t)
         {
            //ret is the _activeBlock ptr after getMem()
            size_t remainSize = _activeBlock->getRemainSize();
            remainSize = downtoSizeT(remainSize);
            assert(remainSize%SIZE_T==0);//blocksize and the memory given out are all multiple of size_t
            size_t rn; //rn is the remained array size
            if(remainSize >= S)
            {
               if(remainSize < (S+SIZE_T))
               {
                  rn = 0;
               }
               else if(remainSize >= (S+SIZE_T))
               {
                  rn = (remainSize-SIZE_T)/S;
               }
               MemRecycleList<T>* reList_Remain =getMemRecycleList(rn);
               #ifdef MEM_DEBUG
                  cout << "Recycling " << ret << " to _recycleList[" << rn << "]\n";
               #endif // MEM_DEBUG
               reList_Remain->pushFront(ret);
            }
            _activeBlock = new MemBlock<T>(_activeBlock,_blockSize);
            #ifdef MEM_DEBUG
               cout << "New MemBlock... " << _activeBlock << endl;
            #endif // MEM_DEBUG
            assert(_activeBlock->getMem(t,ret));
         }
         /*else
         {
            assert(_activeBlock->getMem(t,ret));
         }*/
      }
      // 6. At the end, print out the acquired memory address
      #ifdef MEM_DEBUG
         cout << "Memory acquired... " << ret << endl;
      #endif // MEM_DEBUG
      assert(ret!=0);
      return ret;
   }
   // Get the currently allocated number of MemBlock's
   size_t getNumBlocks() const {
      // TODO
      size_t num=0;
      MemBlock<T>* blockPtr = _activeBlock;
      while(blockPtr!=0)
      {
         num++;
         blockPtr = blockPtr->getNextBlock();
      }
      assert(num>=1);
      return num;
   }

};

#endif // MEM_MGR_H
