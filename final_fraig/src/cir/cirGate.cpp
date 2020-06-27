/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.
size_t CirGate::globalRef =0;
size_t CirGate::mySIZE_T_MAX = 0xffffffffffffffff;

extern CirMgr *cirMgr;
extern KeyGenerator keyGen;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
string
CirGate::getTypeStr() const 
{   
    switch(_gateType)
    {
        case UNDEF_GATE:
            return "UNDEF";
        case PI_GATE:
            return "PI";
        case PO_GATE:
            return "PO";
        case AIG_GATE:
            return "AIG";
        case CONST_GATE:
            return "CONST";
        case TOT_GATE:
            return "No such gate";
    }
    return ""; 
}

void
CirGate::reportGate() const
{
    for(int i=0;i<80;++i)
    	cout<<"=";
    cout<<endl;
    //mstringstream ss;
    
    cout<<"= " << getTypeStr() << "(" << to_string(getID()) << ")" ;
    if(getType()==PI_GATE || getType()==PO_GATE)
    {
        if(getSymbol()!="")
          cout<< "\"" << getSymbol() << "\"";
    }
    cout<< ", line " << to_string(getLineNo())<<endl;
    cout<<"= FECs:";
    if(getType() == AIG_GATE || getType() == CONST_GATE)
    {
    	printFecList();
    }
    cout<<endl;
    cout<<"= Value: ";
    printSimValue();
    cout<<endl;
    #ifdef ZIYIGate
    cout<<hex<<getSimValue()<<dec<<endl;
    #endif
    for(int i=0;i<80;++i) cout<<"=";
    cout<<endl;
	
}

void
CirAigGate::printFecList() const
{
	if(_fecList == 0) return;
	assert(_fecList!=0);
	for(int i=0;i<(int)_fecList->size();++i)
	{
		if(_fecList->at(i) == this) continue;
		cout<<" ";
		if(_fecList->at(i)->getSimValue() == getInvSimValue())
			cout<<"!";
		cout<<_fecList->at(i)->getID();
	}
}

void
CirConstGate::printFecList() const
{
	if(_fecList == 0) return;
	assert(_fecList!=0);
	for(int i=0;i<(int)_fecList->size();++i)
	{
		if(_fecList->at(i) == this) continue;
		cout<<" ";
		if(_fecList->at(i)->getSimValue() == getInvSimValue())
			cout<<"!";
		cout<<_fecList->at(i)->getID();
	}
	
}

void
CirGate::printSimValue()const
{
	for(int i=0;i<64;++i)
	{
		if(i!=0 && i%8==0)cout<<"_";
		cout<< ((_simValue>>(63-i))&(size_t)1);
	}
	
}

void
CirGate::reportFanin(int level) const//inv default is false
{
    assert (level >= 0);
    ++CirGate::globalRef;
    cout<<"level: "<<level<<endl;
    myReportFanin(level);
}

void
CirGate::myReportFanin(int level,bool inv, int indent) const
{ 
    for(int i=0;i<indent;++i) cout<<"  ";
    printGate(inv,level);
    if(level==0)return;
    if(traversed())return;
    switch(getType())
    {
      case AIG_GATE:
      {
        ((CirAigGate*) this)->_fanInList.at(0).gate()->myReportFanin(--level,(((CirAigGate*) this)->_fanInList.at(0).inv()),++indent);
        ((CirAigGate*) this)->_fanInList.at(1).gate()->myReportFanin(level,(((CirAigGate*) this)->_fanInList.at(1).inv()),indent);
        setToGlobal();//only set to global if its fanin is reported
        break;
      } 
      case PO_GATE:
      {
        ((CirPOGate*) this)->_fanIn.gate()->myReportFanin(--level,(((CirPOGate*) this)->_fanIn.inv()),++indent);
        setToGlobal();//only set to global if its fanin is reported
        break;
      }
      default:
        break;
    }
    
}

void
CirGate::printGate(bool inv,int level) const
{
    if(inv)cout<<"!";
    cout<<getTypeStr()<<" "<<getID();
    if(traversed() && level!=0 )cout<<" (*)";
    //cout<<"level: "<<level;
    cout<<endl;
}

void
CirGate::reportFanout(int level) const
{
    assert (level >= 0);
    ++CirGate::globalRef;
    myReportFanout(level);
}

void
CirGate::myReportFanout(int level, bool inv ,int indent) const
{
    for(int i=0;i<indent;++i) cout<<"  ";
    printGate(inv,level);
    if(level==0)return;
    if(traversed())return;
    FanList fanOutList;
    switch(getType())
    {
      case AIG_GATE:      
      case PI_GATE:      
      case CONST_GATE:
      case UNDEF_GATE:
      	getFanOut(fanOutList);
      	--level;
      	++indent;
      	for(int i=0;i<(int)fanOutList.size();++i)               
		{                                                       
		  fanOutList.at(i).gate()->myReportFanout(level,fanOutList.at(i).inv(),indent);  
		}                                                       
		setToGlobal();  
      default:
        break;
    }
} 

void
CirAigGate::deleteGate()
{
  for(int i=0;i<(int)_fanInList.size();++i)
  {
    #ifdef ZIYIGate
    cout<<"fanin gate id "<<_fanInList.at(i).gate()->getID()<<endl;
    #endif
    _fanInList.at(i).gate()->deleteFanOut(this);
  }
  for(int i=0;i<(int)_fanOutList.size();++i)
  {
    #ifdef ZIYIGate
    cout<<"fanout gate id "<<_fanOutList.at(i).gate()->getID()<<endl;
    #endif
    _fanOutList.at(i).gate()->deleteFanIn(this);
  }
  cout<<"Sweeping: "<<getTypeStr()<<"("<<getID()<<")"<<" removed..."<<endl;
  delete this;
}

void
CirUndefGate::deleteGate()
{
  for(int i=0;i<(int)_fanOutList.size();++i)
  {
    #ifdef ZIYIGate
    cout<<"fanout gate id "<<_fanOutList.at(i).gate()->getID()<<endl;
    #endif
    _fanOutList.at(i).gate()->deleteFanIn(this);
  }
  cout<<"Sweeping: "<<getTypeStr()<<"("<<getID()<<")"<<" removed..."<<endl;
  delete this;
}


#define deleteFanOutMacro(T)\
void 																		\
T::deleteFanOut(CirGate* targate)											\
{ 																			\
  for(FanList::iterator it= _fanOutList.begin();it!=_fanOutList.end();++it)	\
  {																			\
    if(it->gate() == targate)												\
    {																		\
      _fanOutList.erase(it);												\
      break;																\
    }																		\
  }																			\
}

deleteFanOutMacro(CirAigGate);
deleteFanOutMacro(CirPIGate);
deleteFanOutMacro(CirUndefGate);
deleteFanOutMacro(CirConstGate);


#define replaceFanOutMacro(T)											\
void																	\
T::replaceFanOut(CirGate* oldFanOut,Fan& newFanOut)						\
{																		\
	for(int i=0;i<(int)_fanOutList.size();++i)							\
	{																	\
		if(_fanOutList.at(i).gate() == oldFanOut)						\
		{																\
			bool inv = (newFanOut.inv() + _fanOutList.at(i).inv())%2;	\
			_fanOutList.at(i).setFan(newFanOut.gate(),inv);				\
		}																\
	}																	\
}

replaceFanOutMacro(CirAigGate);
replaceFanOutMacro(CirPIGate);
replaceFanOutMacro(CirConstGate);


void
CirAigGate::deleteFanIn(CirGate* targate)
{
  for(FanList::iterator it= _fanInList.begin();it!=_fanInList.end();++it)
  {
    if(it->gate() == targate)
    {
      _fanInList.erase(it);
      break;
    }
  }
}

void
CirAigGate::replaceFanIn(CirGate* oldFanIn,Fan& newFanIn)
{
	#ifdef ZIYIGate
	assert(_fanInList.size()==2);
	#endif
	for(int i=0;i<2;++i)
	{
		if(_fanInList.at(i).gate() == oldFanIn)
		{
			/*#ifdef ZIYIGate
			cout<<"gate "<<getID()<<" is replacing fanin: "<<oldFanIn->getID()<<" becomes "<<newFanIn.gate()->getID()<<endl;
			#endif*/
			bool inv = (_fanInList.at(i).inv() + newFanIn.inv())%2;
			_fanInList.at(i).setFan(newFanIn.gate(),inv);
			//replace->replaceFanOut(toBeReplace,this,_fanInList.at(i).inv());
			break;
		}
	}
}



void
CirPOGate::replaceFanIn(CirGate* oldFanIn,Fan& newFanIn)
{
	#ifdef ZIYIGate
	assert(_fanIn.gate() == oldFanIn);
	#endif
	bool inv = (_fanIn.inv() + newFanIn.inv())%2;
	_fanIn.setFan(newFanIn.gate(),inv);
}

void
CirAigGate::getOtherFanIn(Fan& otherFanIn, CirGate* knownGate) const
{
	#ifdef ZIYIGate
	assert(_fanInList.size()==2);
	#endif
	if(_fanInList.at(0).gate()==knownGate)
		otherFanIn = _fanInList.at(1);
	else if(_fanInList.at(1).gate()==knownGate)
		otherFanIn = _fanInList.at(0);
	else assert(false);
}

void
CirPOGate::optGate()
{
	_fanIn.gate()->optGate();
	#ifdef ZIYIGate
	cout<<"opting PO..."<<endl;
	#endif
}


void
CirAigGate::optGate()
{
	if(traversed()) return;
	for(int i=0;i<2;++i)
	{
		_fanInList.at(i).gate()->optGate();
	}//post order

	#ifdef ZIYIGate
	cout<<"opting an aig gate..."<<endl;
	#endif

	CirAigGate* aig = ((CirAigGate*)(this));
	OptType optType = aig->needOpt();
	switch(optType)
	{
		case FANIN_CONST_1:
		{
			#ifdef ZIYIGate
			cout<<"fanin const 1"<<endl;
			#endif

			CirConstGate* constGate;
			Fan otherGate;
			if(_fanInList.at(0).gate()->getType() == CONST_GATE)
			{
				constGate = (CirConstGate* )_fanInList.at(0).gate();
				otherGate = _fanInList.at(1);
			}
			else
			{
				constGate = (CirConstGate* )_fanInList.at(1).gate();
				otherGate = _fanInList.at(0);
			}

			cout<<"Simplifying: "<<otherGate.gate()->getID()<<" merging ";
			if(otherGate.inv()==true)cout<<"!";
			cout<<aig->getID()<<"..."<<endl;

			constGate->deleteFanOut(aig);
			otherGate.gate()->deleteFanOut(aig);
			for(int i=0;i<(int)_fanOutList.size();++i)
			{
				_fanOutList.at(i).gate()->replaceFanIn(aig,otherGate);
				bool inv = (otherGate.inv() + _fanOutList.at(i).inv() )%2;
				otherGate.gate()->addFanOut(_fanOutList.at(i).gate(), inv);
			}
			break;
		}
		case FANIN_CONST_0:
		{
			#ifdef ZIYIGate
			cout<<"fanin const 0"<<endl;
			#endif
			CirConstGate* constGate;
			//bool constGateInv;
			Fan otherGate;
			if(_fanInList.at(0).gate()->getType() == CONST_GATE)
			{
				constGate = (CirConstGate* )_fanInList.at(0).gate();
				//constGateInv = _fanInList.at(0).inv();
				otherGate = _fanInList.at(1);
			}
			else
			{
				constGate = (CirConstGate* )_fanInList.at(1).gate();
				//constGateInv = _fanInList.at(1).inv();
				otherGate = _fanInList.at(0);
			}

			cout<<"Simplifying: "<<constGate->getID()<<" merging ";
			cout<<aig->getID()<<"..."<<endl;

			otherGate.gate()->deleteFanOut(aig);
			if(otherGate.gate()->getType() == UNDEF_GATE)
			{
				#ifdef ZIYIGate
				cout<<"deleting an undef Gate..."<<endl; 
				#endif
				if(((CirUndefGate*)(otherGate.gate()))->noMoreFanOut())
				{
					cirMgr->eraseMap(otherGate.gate()->getID());
					delete otherGate.gate();
					cirMgr->decreUndefNum();
				}
			}
			constGate->deleteFanOut(aig);
			for(int i=0;i<(int)_fanOutList.size();++i)
			{
				Fan constFan(constGate,0);
				_fanOutList.at(i).gate()->replaceFanIn(aig,constFan);
				constGate->addFanOut(_fanOutList.at(i));
			}
			break;
		}
		case IDENTICAL_FANIN:
		{
			
			#ifdef ZIYIGate
			cout<<"opting an aig gate: identical fanin"<<endl;
			#endif

			Fan fanin = _fanInList.at(0);
			cout<<"Simplifying: "<<fanin.gate()->getID()<<" merging ";
			if(fanin.inv()==true)cout<<"!";
			cout<<aig->getID()<<"..."<<endl;

			for(int i=0;i<2;++i)
				fanin.gate()->deleteFanOut(aig);
			
			for(int i=0;i<(int)_fanOutList.size();++i)
			{
				_fanOutList.at(i).gate()->replaceFanIn(aig,fanin);
				bool inv = (fanin.inv() + _fanOutList.at(i).inv() )%2;
				fanin.gate()->addFanOut(_fanOutList.at(i).gate(),inv);
			}
			break;
		}
		case INVERTED_FANIN:
		{
			#ifdef ZIYIGate
			cout<<"opting an aig gate: inverted fanin"<<endl;
			#endif

			Fan fanin = _fanInList.at(0);
			Fan constFan(cirMgr->getGate(0),0);
			cout<<"Simplifying: "<<constFan.gate()->getID()<<" merging ";
			cout<<aig->getID()<<"..."<<endl;

			for(int i=0;i<2;++i)
				fanin.gate()->deleteFanOut(aig);
			if(fanin.gate()->getType() == UNDEF_GATE)
			{
				if(((CirUndefGate*)(fanin.gate()))->noMoreFanOut())
				{
					cirMgr->eraseMap(fanin.gate()->getID());
					delete fanin.gate();
				}
			}

			for(int i=0;i<(int)_fanOutList.size();++i)
			{
				_fanOutList.at(i).gate()->replaceFanIn(aig,constFan);
				constFan.gate()->addFanOut(_fanOutList.at(i));
			}
			break;
		}
		default:
			break;
	}
	if(optType != TOT_OPT_TYPE)
	{
		cirMgr->eraseMap(aig->getID());
		delete aig;
		cirMgr->decreAigNum();
	}
	else
	{
		setToGlobal();
	}
}

OptType
CirAigGate::needOpt() const 
{
	assert(_fanInList.size()==2);
	for(int i=0;i<2;++i)
	{
		if(_fanInList.at(i).gate()->getType() == CONST_GATE)
		{
			if(_fanInList.at(i).inv()==true) return FANIN_CONST_1;
			else return FANIN_CONST_0;
		}
	}
	if(_fanInList.at(0).gate() == _fanInList.at(1).gate())
	{
		if(_fanInList.at(0).inv() == _fanInList.at(1).inv()) return IDENTICAL_FANIN;
		else return INVERTED_FANIN;
	}
	return TOT_OPT_TYPE;
}

void
CirPOGate::strashGate()
{
	_fanIn.gate()->strashGate();
}

void
CirAigGate::strashGate()
{
	if(traversed())return;
	for(int i=0;i<2;++i)
	{
		_fanInList.at(i).gate()->strashGate();
	}

	HashIterator it;
	if(!((CirAigGate*)(this))->needStrash(it))
	{
		setToGlobal();
		return;
	}
	//merge this gate to the other gate
	cout<<"Strashing: "<<it->second->getID()<<" merging "<<getID()<<"..."<<endl;
	#ifdef ZIYIGate
	assert(it->second->getType() == AIG_GATE);
	#endif
	CirAigGate* otherGate = (CirAigGate*)(it->second);
	for(int i=0;i<2;++i)
	{
		_fanInList.at(i).gate()->deleteFanOut(this);
	}

	for(int i=0;i<(int)_fanOutList.size();++i)
	{
		Fan newFan(otherGate,0);
		_fanOutList.at(i).gate()->replaceFanIn(this,newFan);
		otherGate->addFanOut(_fanOutList.at(i));
	}
	#ifdef ZIYIGate
	cout<<"deleting gate "<<getID()<<endl;
	#endif
	cirMgr->eraseMap(this->getID());
	delete this;
	cirMgr->decreAigNum();
}

bool
CirAigGate::needStrash(HashIterator& it)
{
	size_t key = keyGen(this);
	HashNode node(key,this);
	it = cirMgr->hashFind(key);
	if(it== cirMgr->hashEnd())
	{
		cirMgr->hashInsert(node);
		return false;
	}
	CirGate* possibleGate = it->second;
	assert(possibleGate->getType() == AIG_GATE);
	FanList& otherFanIn = ((CirAigGate*)possibleGate)->getFanIn();
	bool sameGates = true;
	for(int i=0;i<2;++i)
	{
		if((otherFanIn.at(i) != _fanInList.at(i)) && (otherFanIn.at(i) != _fanInList.at(1-i)))
			sameGates = false;
	}
	return sameGates;

}

void
CirPOGate::simulateGate()
{
	_fanIn.gate()->simulateGate();
	#ifdef ZIYIGate
	cout<<"simulating PO..."<<endl;
	#endif
	if(_fanIn.inv() == true )
	{
		size_t fanInInvSim = _fanIn.gate()->getInvSimValue();
		setSimValue(fanInInvSim);
	}
	else
	{
		size_t fanInSim = _fanIn.gate()->getSimValue();
		setSimValue(fanInSim);
	}
	setToGlobal();
	#ifdef ZIYIGate
	cout<<"PO sim (gate "<<getID()<<") :"; printSimValue(); cout<<endl;
	#endif
}

void
CirConstGate::simulateGate()
{
	if(traversed())return;
	setSimValue(0);
	setToGlobal();
}

void
CirUndefGate::simulateGate()
{
	if(traversed()) return;
	setSimValue(0);
	setToGlobal();
}

void
CirPIGate::simulateGate()
{
	if(traversed())return;
	setToGlobal();
}

void
CirAigGate::simulateGate()//need to consider if the pattern from last simulation does not change
{
	if(traversed()) return;

	for(int i=0;i<2;++i)
	{
		_fanInList.at(i).gate()->simulateGate();
	}
	size_t input[2];
	for(int i=0;i<2;++i)
	{
		if(_fanInList.at(i).inv() == false)
			input[i] = _fanInList.at(i).gate()->getSimValue();
		else 
			input[i] = _fanInList.at(i).gate()->getInvSimValue();
	}
	setSimValue(input[0] & input[1]);
	#ifdef ZIYIGate
	cout<<"AIG sim (gate "<<getID()<<") :"; printSimValue(); cout<<endl;
	#endif
	setToGlobal();
	

}







