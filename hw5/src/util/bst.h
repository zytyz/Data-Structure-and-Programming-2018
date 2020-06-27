/****************************************************************************
  FileName     [ bst.h ]
  PackageName  [ util ]
  Synopsis     [ Define binary search tree package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BST_H
#define BST_H

#include <cassert>

using namespace std;

template <class T> class BSTree;

//#define ZIYI 35
//#define TIME 35
// BSTreeNode is supposed to be a private class. User don't need to see it.
// Only BSTree and BSTree::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class BSTreeNode
{
	// TODO: design your own class!!
public:
	friend class BSTree<T>;
	friend class BSTree<T>::iterator;
	BSTreeNode(const T& x, BSTreeNode<T>* l=0, BSTreeNode<T>* r=0,BSTreeNode<T>* p=0)
	: _data(x), _left(l),_right(r),_parent(p){}
	~BSTreeNode(){}//cannot release memory because the memory can be pointed by more than one

	T _data;
	BSTreeNode<T>* _left;
	BSTreeNode<T>* _right;
	BSTreeNode<T>* _parent;
};


template <class T>
class BSTree
{
	// TODO: design your own class!!
public:
	BSTree(): _root(0),_size(0)
	{
		_tail = new BSTreeNode<T>(T(""));
	}
	~BSTree(){}
	class iterator 
	{ 
		friend class BSTree;
	public:
		iterator(const iterator& i): _node(i._node){}
		iterator(BSTreeNode<T>* n = 0): _node(n){}
		~iterator(){}
		const T& operator * () const { return _node->_data; }
		T& operator * () { return _node->_data; }
		iterator& operator ++ () //iterator goes from the first element to _tail
		{ 
			#ifdef ZIYI
			assert(_node->_data !=T(""));//_tail cannot ++_tail
			#endif
			if(_node->_right == NULL)
			{
				T x = _node->_data;
				do
				{
					_node = _node->_parent;
				}while((_node->_data) < x);
			}
			else//find min of right subtree
			{
				_node = _node->_right;
				while( (_node->_left) != NULL)
				{
					_node = _node->_left;
				}
			}
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
		 	//if(_node->_data )
		 	//haven't asserted that --_root cannot happen
		 	if(_node->_parent == NULL)
		 	{
		 		//node is _root
		 		#ifdef ZIYI_it
		 		cout<<"calling --_root"<<endl;
		 		//assert(false);
		 		#endif
		 		if(_node->_left == NULL)
		 		{
		 			//root is the first node
		 			return (*this);
		 		}
		 		else
		 		{
		 			//find root's predecessor
		 			_node = _node->_left;
		 			while(_node->_right!=NULL)
		 			{
		 				_node = _node->_right;
		 			}
		 		}
		 	}
		 	else if(_node->_data == T(""))//the iterator is the tail (end())
		 	{
		 		_node = _node->_parent;
		 	}
		 	else if(_node->_left == NULL)//find the ancestor smaller than itself
		 	{
		 		T x = _node->_data;
		 		do
		 		{
		 			if(_node->_parent == NULL)break;
		 			_node = _node->_parent;
		 		}while((_node->_data) >= x);
		 	}
		 	else//find max in the left subtree
		 	{
		 		_node = _node->_left;
		 		while(_node->_right != NULL)
		 		{
		 			_node = _node->_right;
		 		}
		 	}
		 	return (*this);
		}//--it
		iterator operator -- (int) 
		{ 
	 		#ifdef ZIYI_it
	 		if(_node->_parent ==NULL)
	 			cout<<"calling _root--"<<endl;
	 		#endif
		 	iterator tmp = (*this);
		 	--(*this);
		 	return tmp;
		}//it--

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
    	BSTreeNode<T>* _node;
	};

	
	size_t size() const{ return _size; }
	iterator begin() const
	{
		if(empty())return end();
		BSTreeNode<T>* tmp = _root;
		//assert(_root!=NULL);
		while(tmp->_left != NULL)
		{
			tmp = tmp->_left;
		}
		//assert(tmp!=NULL);
		return iterator(tmp);
	}
	iterator end() const//"past the end" iterator
	{
		#ifdef ZIYI
		if(_root == NULL)
		{
			assert(empty());
			assert(_tail->_parent == NULL);
			//_tail->_parent = NULL;
		}
		assert(_tail->_parent != NULL);
		#endif
		return iterator(_tail);
	}
	bool empty() const
	{
		return (_size == 0);
	}
	void insert(const T& x)//need to maintain _tail
	{
		//_tail knows its parents, the last element doesn't know tail
		//the last element should know its right child is _tail
		//++li needs to reach end()
		//cout<<"insert data: "<<x._str<<endl;
		if(empty())
		{
			//cout<<"here"<<endl;
			_root = new BSTreeNode<T>(x,0,_tail,0);//_root->_right should be _tail when inserting the first element
			_tail->_parent = _root;
		}
		else
		{
			//need to check _tail
			//cout<<"here2"<<endl;
			BSTreeNode<T>* tmp = _root;
			bool changeTail = false;
			while(1)
			{
				//cout<<"current tmp: "<<tmp->_data<<endl;
				if( x <= tmp->_data )
				{
					if(tmp->_left == NULL) break;
					//cout<<"in finding left:"<<endl;
					tmp = tmp->_left;
					continue;
				}
				else if(x > tmp->_data)
				{
					//cout<<"bigger"<<endl;
					if(tmp->_right == NULL)
					{
						//cout<<"should break here"<<endl;
						break;
					}
					if((tmp->_right) == _tail)//tmp is the last element
					{
						changeTail = true;
						break;
					}
					//cout<<"in finding right"<<endl;
					//cout<<"tmp->_right: "<<tmp->_right<<endl;
					tmp = tmp->_right;
					continue;
				}
			}
			//cout<<"changeTail: "<<changeTail<<endl;
			if(!changeTail)
			{
				if(x > (tmp->_data))
				{
					//cout<<"new right node"<<endl;
					tmp->_right = new BSTreeNode<T>(x,0,0,tmp);
				}
				else if(x <= (tmp->_data))
				{
					//cout<<"new left node"<<endl;
					tmp->_left = new BSTreeNode<T>(x,NULL,NULL,tmp);
				}
			}
			else
			{
				//cout<<"new right node with tail"<<endl;
				tmp->_right = new BSTreeNode<T>(x,0,_tail,tmp);
				_tail->_parent = tmp->_right;
			}
		}
		++_size;
	}
	void pop_back()
	{
		if(empty())return;
		BSTreeNode<T>* pop = _tail->_parent;
		//cout<< (pop->_data)._str<<endl;
		//pop may be _root
		//pop may have a left child
		if(pop == _root)
		{
			if(pop->_left == NULL)
			{
				//the tree only has one node, which is _root
				delete _root;
				_root = NULL;
				_tail->_parent =NULL;
			}
			else
			{
				//has a left subtree
				//need to update _tail
				BSTreeNode<T>* tmp = pop->_left;
				while(tmp->_right != NULL)
				{
					tmp = tmp->_right;
				}
				_root = _root->_left;
				_root->_parent = NULL;
				tmp->_right = _tail;
				_tail->_parent = tmp;
				delete pop;
				pop = NULL;
			}
		}
		else 
		{
			if(pop->_left == NULL)
			{
				_tail->_parent = pop->_parent;
				(_tail->_parent)->_right = _tail;
				delete pop;
			}
			else
			{
				BSTreeNode<T>* tmp = pop->_left;
				while(tmp->_right != NULL)
				{
					tmp = tmp->_right;
				}
				_tail->_parent = tmp;
				tmp->_right = _tail;
				pop->_left->_parent = pop->_parent;
				pop->_parent->_right = pop->_left;
				delete pop;
			}
		}
		--_size;
	}
	void pop_front()
	{
		if(empty())return;
		#ifdef ZIYI 
		assert(_root != NULL);
		#endif
		BSTreeNode<T>* pop = begin()._node;
		//begin might be _root
		//begin might have a right child
		//begin has no left child
		if(pop == _root)
		{
			_root = pop->_right;
			_root->_parent = NULL;
			delete pop;
			pop = NULL;
		}
		else
		{
			if(pop->_right ==NULL)
			{
				(pop->_parent)->_left = NULL;
				delete pop;
			}
			else
			{
				//pop has a right child
				pop->_parent->_left = pop->_right;
				pop->_right->_parent = pop->_parent;
				delete pop;
			}
		}
		--_size;
	}
	void clear()
	{
		#ifdef ZIYI
		cout<<"clearing..."<<endl;
		#endif
		while(!empty())
		{
			pop_back();
		}
	}
	bool erase(const T& x)
	{
		if(empty())return false;
		iterator it = find(x);
		if(it == end()) 
		{
			#ifdef ZIYI
			cout<<"did not find element"<<endl;
			#endif
			return false;
		}
		#ifdef ZIYI
		cout<< *it <<endl;
		cout<<"found index: "<<findIndex(it)<<endl;
		#endif
		erase(it);
		return true;
	}
	bool erase(iterator it)//it is pos
	{
		if(empty())return false;

		#ifdef ZIYI
		static int erase_times = 0;
		++erase_times;
		cout<<"erase random times: "<<erase_times<<endl;
		if(it._node == _tail)
			cout<<"trying to erase tail"<<endl;
		assert(it._node != _tail);
		#endif

		BSTreeNode<T>*& node = it._node;
		bool rightChild =(node->_right != NULL);
		bool leftChild = (node->_left != NULL);
		if(node == _root)
		{
			#ifdef ZIYI
			cout<<"deleting root..."<<endl;
			assert(rightChild);
			#endif

			if(rightChild && !leftChild)
			{
				//_root must have at least a right child _tail
				#ifdef ZIYI
				cout<<"root does not have a left child"<<endl;
				#endif
				if( node->_right == _tail)
				{
					//the only node is _root
					#ifdef ZIYI
					cout<<"root is the only node"<<endl;
					assert(_tail->_parent == _root);
					#endif
					_tail->_parent = NULL;
					delete _root;
					_root = NULL;
				}
				else
				{
					//_root has a right subtree but no left subtree
					#ifdef ZIYI
					cout<<"root has a right subtree"<<endl;
					#endif
					_root = _root->_right;
					_root->_parent = NULL;
					delete node;
				}
				
			}
			else if(rightChild && leftChild)
			{
				#ifdef ZIYI
				cout<<"root has left child"<<endl;
				#endif
				if(node->_right == _tail)
				{
					//root has a left subtree
					//root's right child is tail
					#ifdef ZIYI
					cout<<"root only has a left subtree"<<endl;
					#endif
					//assert(node->_right != NULL);
					iterator pred = predecessor(it);//it._node is _root
					//cout<<"pred index: "<<findIndex(pred)<<endl;
					pred._node->_right = _tail;
					_tail->_parent = pred._node;
					_root = _root->_left;
					delete _root->_parent;
					_root->_parent = NULL;
					//delete node;
				}
				else
				{
					//root really has two children
					#ifdef ZIYI
					cout<<"root has two subtrees"<<endl;
					#endif
					
					deleteNode2Sub(it);
				}
			}
		}
		else//node is not _root
		{
			#ifdef ZIYI
			cout<<"deleting node(node is not root)..."<<endl;
			#endif
			if(!rightChild && !leftChild)
			{
				//it == end() is already returned
				#ifdef ZIYI
				cout<<"deleting node has no children"<<endl;
				#endif
				#ifdef TIME
				static int noChild =0;
				cout<<"deleting no child node: "<<(++noChild)<<endl; 
				#endif
				if(isRightChild(it))
					(node->_parent)->_right = NULL;
				else
					(node->_parent)->_left = NULL;		
				delete node;	
			}
			else if(rightChild && !leftChild)
			{
				//same for lastELe
				#ifdef ZIYI
				cout<<"deleting node has no left child"<<endl;
				if(node->_right == _tail)
					cout<<"deleting node's right child is tail"<<endl;
				else
					cout<<"deleting node has right child"<<endl;
				#endif

				#ifdef TIME
				static int rightChild =0;
				cout<<"deleting only right child node: "<<(++rightChild)<<endl; 
				#endif

				if(isRightChild(it))
				{
					#ifdef ZIYI
					cout<<"deleting node is right child"<<endl;
					#endif
					(node->_parent)->_right = node->_right;
					(node->_right)->_parent = node->_parent;
				}
				else
				{
					#ifdef ZIYI
					cout<<"deleting node is left child"<<endl;
					#endif
					(node->_parent)->_left = node->_right;
					(node->_right)->_parent = node->_parent;
				}
				delete node;
			}
			else if(!rightChild && leftChild)
			{
				#ifdef ZIYI
				cout<<"deleting node has no right child"<<endl;
				cout<<"deleting node has left child"<<endl;
				#endif

				#ifdef TIME
				static int leftChild =0;
				cout<<"deleting only left child node: "<<(++leftChild)<<endl; 
				#endif

				if(isRightChild(it))
				{
					#ifdef ZIYI
					cout<<"deleting node is right child"<<endl;
					#endif
					(node->_parent)->_right = node->_left;
					(node->_left)->_parent = node->_parent;
				}
				else
				{
					#ifdef ZIYI
					cout<<"deleting node is left child"<<endl;
					#endif
					(node->_parent)->_left = node->_left;
					(node->_left)->_parent = node->_parent;
				}
				
				delete node;
			}
			else//two children
			{
				if( node->_right == _tail )//last Ele has a left child and a tail right child
				{
					#ifdef ZIYI
					cout<<"deleting node is the last Ele"<<endl;
					assert(isRightChild(it));
					#endif
					(node->_parent)->_right = node->_left;
					(node->_left)->_parent = node->_parent;
					iterator pred = predecessor(it);
					(pred._node)->_right = _tail;
					_tail->_parent = pred._node;
					delete node;
				}
				else//really has two children
				{
					#ifdef ZIYI
					cout<<"deleting node has 2 children"<<endl;
					#endif
					deleteNode2Sub(it);
				}
			}
		}
		--_size;
		return true;
	}
	iterator find(const T& x) const
	{
		if(empty()) return end();
		BSTreeNode<T>* tmp = _root;
		while(1)
		{
			if( x== tmp->_data)break; 
			if( x < tmp->_data )
			{
				if(tmp->_left == NULL) return end();
				tmp = tmp->_left;
			}
			else if(x > tmp->_data)
			{
				if(tmp->_right == NULL || ((tmp->_right) == _tail))
					return end();
				tmp = tmp->_right;
			}
		}
		return iterator(tmp);
	}
	void sort() const
	{
		return;
	}
	void print() const//verbose
	{
		if(empty()) return;
		printTree(_root);
		#ifdef ZIYI
		cout<<"size: "<<_size<<endl;
		#endif
	}
private:
	BSTreeNode<T>* _root;
	BSTreeNode<T>* _tail;
	//_tail->_data ="", _tail->_parent is the last element
	size_t _size;

	iterator successor( const iterator& it) const
	{
		#ifdef ZIYI
		assert((it._node)->_right!= NULL);
		assert( it._node->_right != _tail);
		#endif
		BSTreeNode<T>* tmp = it._node->_right;
		while(tmp->_left != NULL)
		{
			tmp = tmp->_left;
		}
		return iterator(tmp);
		/*iterator tmp = iterator((it._node)->_right);
		while((tmp._node)->_left != NULL)
		{
			(tmp._node) = (tmp._node)->_left;
		}
		return tmp;*/
	}
	iterator predecessor(const iterator& it) const
	{
		#ifdef ZIYI
		assert((it._node)->_left!= NULL);
		#endif
		//assert( it._node->_right != _tail);
		BSTreeNode<T>* tmp = it._node->_left;
		while( (tmp->_right != NULL) && (tmp->_right != _tail))
		{
			tmp = tmp->_right;
		}
		return iterator(tmp);
		/*iterator tmp = iterator((it._node)->_left);
		while((tmp._node)->_right != NULL && (tmp._node)->_right != _tail)
		{
			(tmp._node) = (tmp._node)->_right;
		}
		return tmp;*/
	}
	bool isRightChild(const iterator& it) const
	{
		#ifdef ZIYI
		assert(it._node != _root);//it can not be root
		#endif
		return ( *it > it._node->_parent->_data );
	}
	void printTree(BSTreeNode<T>* n,int x=0) const
	{
		//static int x=0;
		for(int i=0;i<x;i++)
		{
			cout<<"  ";
		}
		if(n == NULL)
		{
			cout << "[0]" <<endl;
		}
		else if (n == _tail)
		{
			cout << "[tail]" <<endl;
		}
		else
		{
			cout<< n->_data <<endl;
			printTree(n->_left,++x);
			printTree(n->_right,x);
		}
	}
	int findIndex(const iterator& it)
	{
		int index =0 ;
		iterator tmp = begin();
		iterator e = end();
		while(tmp!=e)
		{
			if(tmp == it)break;
			++tmp;
			++index;
		}
		if(tmp == e )return -1;
		return index;
	}
	void deleteNode2Sub(iterator& it)
	{
		#ifdef TIME
		static int Sub2Times = 0;
		++Sub2Times;
		cout<<"deleteNode2Sub times: "<<Sub2Times<<endl;
		#endif

		iterator succ = successor(it);
		#ifdef ZIYI
		cout<<"succ index: "<<findIndex(succ)<<endl;
		cout<< *succ <<endl;
		//succ can be the last Ele
		//need to handle tail
		assert((succ._node)->_left == NULL);//succ does not have a left child
		assert(succ._node != _tail);//succ can not be tail
		#endif
		if((succ._node)->_parent->_data == (succ._node)->_data)
		{
			#ifdef ZIYI
			cout<<"succ data is the same as its parent"<<endl;
			#endif

			#ifndef ZIYI
			iterator& succChild = succ;
			#endif

			#ifdef ZIYI
			iterator succChild = succ;
			while(((succ._node)->_parent->_data) == (succ._node)->_data)
			{
				succ._node = succ._node->_parent;
			}
			iterator succParent = iterator(succ._node);

			assert((succChild._node)->_data == succParent._node->_data);
			cout<<"succ child index: "<<findIndex(succChild)<<endl;
			cout<<"succ parent index: "<<findIndex(succParent) <<endl;
			#endif
			
			(succChild._node)->_left = (it._node)->_left;
			(it._node)->_left->_parent = succChild._node;
			
			if(it._node == _root)
			{
				#ifdef ZIYI
				cout<<"deleting root with 2 children(special case)"<<endl;
				#endif
				_root = it._node->_right;
				(it._node)->_right->_parent = NULL;
				delete it._node;
			}
			else
			{
				#ifdef ZIYI
				cout<<"deleting node(not root) with 2 children(special case)"<<endl;
				#endif
				if(isRightChild(it))
				{
					#ifdef ZIYI
					cout<<"deleting node is right child"<<endl;
					#endif
					(it._node->_parent)->_right = it._node->_right;
					(it._node)->_right->_parent = it._node->_parent;
				}
				else
				{
					#ifdef ZIYI
					cout<<"deleting node is left child"<<endl;
					#endif
					(it._node->_parent)->_left = it._node->_right;
					(it._node)->_right->_parent = it._node->_parent;
				}
				delete it._node;
			}
			return;
		}
		if((succ._node)->_right == NULL)//succ has no child
		{
			//delete succ
			#ifdef ZIYI
			cout<<"succ has no child"<<endl;
			#endif
			if(isRightChild(succ))//succ is right child
			{
				#ifdef ZIYI
				cout<<"succ is right child"<<endl;
				#endif
				((succ._node)->_parent)->_right = NULL;
			}	
			else//succ is left child
			{
				#ifdef ZIYI
				cout<<"succ is left child"<<endl;
				#endif
				((succ._node)->_parent)->_left = NULL;
			}
			*it = *succ;
			delete succ._node;
		}
		else if(((succ._node)->_right) == _tail)
		{
			//succ is the last Ele
			//succ is right child 
			//succ can not be root
			#ifdef ZIYI
			cout<<"succ is the last Ele"<<endl;
			assert(isRightChild(succ));
			#endif
			((succ._node)->_parent)->_right = _tail;
			_tail->_parent = (succ._node)->_parent;
			*it = *succ;
			delete succ._node;
		}
		else
		{
			#ifdef ZIYI
			cout<<"succ has a right subtree"<<endl;
			#endif
			//succ has a right subtree
			if(isRightChild(succ))
			{
				#ifdef ZIYI
				cout<<"succ is the right child"<<endl;
				#endif
				(succ._node)->_parent->_right = (succ._node)->_right;
				(succ._node)->_right->_parent = (succ._node)->_parent;
			}
			else
			{
				#ifdef ZIYI
				cout<<"succ is the left child"<<endl;
				#endif
				(succ._node)->_parent->_left = (succ._node)->_right;
				(succ._node)->_right->_parent = (succ._node)->_parent;
			}
			*it = *succ;
			delete succ._node;
		}
	}

};
#endif // BST_H
