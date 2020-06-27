/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"
#include <unordered_map>

//#define ZIYIFraig 35

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed

KeyGenerator keyGen;

void
CirMgr::strash()
{
	++CirGate::globalRef;
	_hash.clear();

	for(int i=0;i<(int)_POList.size();++i)
	{
		_POList.at(i)->strashGate();
	}
	//setDfsList();
}

void
CirMgr::fraig()
{
	assert(_newFECGroups.empty());

	for(size_t i =0;i<_FECGroups.size();++i)
	{ 
		#ifdef ZIYIFraig
		cout<<"satting group "<<i<<endl;
		#endif
		satOneFecGrp(_FECGroups.at(i));
	}
	_FECGroups.clear();
	for(size_t i =0;i<_newFECGroups.size();++i)
	{
		_FECGroups.push_back(_newFECGroups.at(i));

	}
	_newFECGroups.clear();
	cout<<"Updating by UNSAT... Total #FEC Group = "<<_FECGroups.size()<<endl;
	#ifdef ZIYIFraig
	printFECPairs();
	#endif
	simulatePattern();
}

void
CirMgr::satOneFecGrp(GateList* fecGrp)
{
	
	#ifdef ZIYIFraig
	cout<<"satting one fecGrp..."<<endl;
	cout<<"fecGrp: "<<fecGrp<<endl;
	#endif
	assert(fecGrp->size()>=2);
	vector<GateList* > allPiles;//inside each pile are all equivalent gates

	ziyiSortRevFunction ziyiObject;
	sort(fecGrp->begin(),fecGrp->end(),ziyiObject);

	#ifdef ZIYIFraig
	cout<<"sorting done"<<endl;
	#endif
	GateList* pile1 = new GateList;
	pile1->push_back(fecGrp->back());//put the last one in pile one
	allPiles.push_back(pile1);
	fecGrp->pop_back();

	#ifdef ZIYIFraig
	cout<<"first pile constructed"<<endl;
	#endif


	while(!fecGrp->empty())
	{
		#ifdef ZIYIFraig
		cout<<"fecGrp size: "<<fecGrp->size()<<endl;;
		#endif


		CirGate* curGate = fecGrp->back();
		bool needNewPile = true;
		for(size_t i =0;i<allPiles.size();++i)
		{
			#ifdef ZIYIFraig
			cout<<"proving equal: "<<curGate->getID()<<" "<<allPiles.at(i)->at(0)->getID()<<endl;
			#endif
			bool result;
			if(curGate->getSimValue() == allPiles.at(i)->at(0)->getSimValue())
			 	result = proveEqual(curGate,allPiles.at(i)->at(0),false);
			else if(curGate->getSimValue() == allPiles.at(i)->at(0)->getInvSimValue())
				result = proveEqual(curGate,allPiles.at(i)->at(0),true);
			else assert(false);

			//only compare with the first gate in the pile
			if(!result)
			{
				//they are equivalent, gate should be in the pile
				#ifdef ZIYIFraig
				cout<<"one more gate goes into pile"<<endl;;
				#endif
				allPiles.at(i)->push_back(curGate);
				needNewPile = false;
				break;
			}
			else
			{
				#ifdef ZIYIFraig
				cout<<"gates are different"<<endl;
				#endif
			}
		}
		if(needNewPile)
		{
			GateList* newPile = new GateList;
			newPile->push_back(curGate);
			allPiles.push_back(newPile);
		}
		fecGrp->pop_back();
	}
	for(size_t i =0;i<allPiles.size();++i)
	{
		#ifdef ZIYIFraig
		cout<<"pile "<<i<<": ";
		for(size_t j=0;j<allPiles.at(i)->size();++j)
		{
			cout<<allPiles.at(i)->at(j)->getID()<<" ";
		}
		cout<<endl;
		#endif
		if(allPiles.at(i)->size()>1)
		{
			#ifdef ZIYIFraig
			cout<<"merging pile "<<i<<"..."<<endl;
			#endif
			mergePiles(allPiles.at(i));
		}
	}

	if(allPiles.size() >= 2)
	{
		fecGrp->clear();
		for(size_t i =0;i<allPiles.size();++i)
		{
			fecGrp->push_back(allPiles.at(i)->at(0));
		}
		_newFECGroups.push_back(fecGrp);
	}
	else
	{
		#ifdef ZIYIFraig
		cout<<"clearing this fecGrp"<<endl;
		#endif
		fecGrp->clear();
		delete fecGrp;
	}
	for(size_t i =0;i<allPiles.size();++i)
	{
		delete allPiles.at(i);
	}
	allPiles.clear();

	//delete fecGrp;
}

bool
CirMgr::proveEqual(CirGate* gate1,CirGate* gate2,bool inv)
{
	assert(gate1->getType()==AIG_GATE || gate1->getType()==CONST_GATE);
	++CirGate::globalRef;
	GateList gates;
	#ifdef ZIYIFraig
	cout<<"constructing sub circuit..."<<endl;
	#endif

	subCircuit(gate1,gate2,gates);//gates is the subCircuit

	#ifdef ZIYIFraig
	cout<<"sub circuit constructed"<<endl;
	#endif

	_solver.initialize();
	genProofModel(gates);

	#ifdef ZIYIFraig
   	cout<<"model generated"<<endl;
   	#endif

	Var newV = _solver.newVar();
   	_solver.addXorCNF(newV, gate1->getVar(), false, gate2->getVar(), inv);
   	
   	#ifdef ZIYIFraig
   	cout<<"Xor CNF added"<<endl;
   	#endif

   	_solver.assumeRelease();  // Clear assumptions
   	_solver.assumeProperty(newV, true);  // k = 1
   	bool result = _solver.assumpSolve();

   	//_solver.printStats();
   	//cout<<"stats are printed"<<endl;
  
   	#ifdef ZIYIFraig
   	cout << (result? "SAT" : "UNSAT") << endl;
   	if(!result)
   	{
   		cout<<gate1->getID()<<" and ";
   		if(inv)cout<<"!";
   		cout<<gate2->getID()<<" are the same"<<endl;
   	}
   	#endif
   	
   	if (result) 
   	{
      	#ifdef ZIYIFraig
      	cout<<gate1->getID()<<" and ";
   		if(inv)cout<<"!";
   		cout<<gate2->getID()<<" are not the same"<<endl;
   		#endif

   		if(_patternTimes%64 == 0)
   		{
   			vector<size_t>* input = new vector<size_t>; 
   			for(size_t i =0; i<_PIList.size() ;++i)
   			{
   				input->push_back(0);
   			}
   			_simPattern.push_back(input);
   		}
   		assert(!_simPattern.empty());

   		vector<size_t>* curInput = _simPattern.back();
   		assert(curInput->size() == _PIList.size());
   		
      	for (size_t i = 0; i<gates.size(); ++i)
      	{
      		#ifdef ZIYIFraig
      		cout <<"gate "<<gates[i]->getID()<<": ";
         	cout<<_solver.getValue(gates[i]->getVar())<<" ";
         	#endif
         	if(gates[i]->getType() == PI_GATE)
         	{
         		size_t bit = _solver.getValue(gates[i]->getVar());
         		size_t index = getPIIndex(gates[i]);
         		#ifdef ZIYIFraig
         		assert(index != CirGate::mySIZE_T_MAX);
         		#endif
         		curInput->at(index) = ((curInput->at(index)<<1)|(bit&1));
         	}
      	}
      	#ifdef ZIYIFraig	
        cout<<endl;
        #endif
        ++_patternTimes;
   	}
	
	return result;
}

void
CirMgr::subCircuit(CirGate* gate1,CirGate* gate2,GateList& list)
{
	searchGateDfs(gate1,list);
	searchGateDfs(gate2,list);
}

void
CirMgr::genProofModel(GateList& gates)
{
	for(size_t i =0;i<gates.size();++i)
	{
		//if(gates[i]->getType() != CONST_GATE)
		//{
			Var v = _solver.newVar();
      		gates[i]->setVar(v);
		//}
		
	}
	for(size_t i =0;i<gates.size();++i)
	{
		if(gates[i]->getType() == AIG_GATE)
		{
			FanList& fanInList = ((CirAigGate* )gates[i])->getFanIn();
			
			_solver.addAigCNF(gates[i]->getVar() ,fanInList.at(0).gate()->getVar(), fanInList.at(0).inv(), fanInList.at(1).gate()->getVar(), fanInList.at(1).inv());
		}
		if(gates[i]->getType() == CONST_GATE)
		{
			if(gates.back()->getType()!=CONST_GATE)
				_solver.addAigCNF(gates[i]->getVar(), gates.back()->getVar(),false,gates.back()->getVar(),true);
			else
				_solver.addAigCNF(gates[i]->getVar(), gates.front()->getVar(),false,gates.front()->getVar(),true);
		}
	}
}

void
CirMgr::mergePiles(GateList* pile)
{
	ziyiSortDFSFunction ziyiObject(_dfsList);

	sort(pile->begin(),pile->end(),ziyiObject);
	assert(pile->size()>1);

	#ifdef ZIYIFraig
	cout<<"after sorting by dfs..."<<endl;
	for(size_t i =0;i<pile->size();++i)
	{
		cout<<pile->at(i)->getID()<<" ";
	}
	cout<<endl;
	#endif

	for(size_t i =1;i<pile->size();++i)
	{
		cout<<"Fraig: "<<pile->at(0)->getID()<<" merging ";
		if(pile->at(0)->getSimValue() == pile->at(i)->getInvSimValue())
			cout<<"!";
		cout<<pile->at(i)->getID()<<"..."<<endl;
		if(pile->at(i)->getType() == CONST_GATE)
			merge2Gates(pile->at(i),pile->at(0));
		else
			merge2Gates(pile->at(0),pile->at(i));
	}
}

void
CirMgr::merge2Gates(CirGate* gate1,CirGate* gate2)
{
	bool inv;
	if(gate1->getSimValue() == gate2->getInvSimValue()) inv = true;
	else inv = false;
	assert(gate2->getType() == AIG_GATE);

	if(gate1->getType() == CONST_GATE)
	{
		FanList& fanOutList = ((CirAigGate*)gate2)->getFanOut();
		for(int i=0;i<(int)fanOutList.size();++i)
		{
			Fan tmp(gate1,inv);
			bool linkInv = (inv + fanOutList.at(i).inv())%2;
			fanOutList.at(i).gate()->replaceFanIn(gate2, tmp);
			gate1->addFanOut(fanOutList.at(i).gate(),linkInv);
		}
		FanList& fanInList = ((CirAigGate*)gate2)->getFanIn();
		for(int i=0;i<2;++i)
		{
			fanInList.at(i).gate()->deleteFanOut(gate2);
		}
	}
	else
	{
		
		FanList& fanOutList = ((CirAigGate*)gate2)->getFanOut();
		for(int i=0;i<(int)fanOutList.size();++i)
		{
			Fan tmp(gate1,inv);
			bool linkInv = (inv + fanOutList.at(i).inv())%2;
			fanOutList.at(i).gate()->replaceFanIn(gate2, tmp);
			gate1->addFanOut(fanOutList.at(i).gate(),linkInv);
		}
		FanList& fanInList = ((CirAigGate*)gate2)->getFanIn();
		for(int i=0;i<2;++i)
		{
			Fan tmp(gate1,0);
			fanInList.at(i).gate()->deleteFanOut(gate2);
			//gate1->addFanIn(fanInList.at(i).gate(),fanInList.at(i).inv());
		}

	}
	--_aigNum;
	eraseMap(gate2->getID());
	delete gate2;
}

void
CirMgr::simulatePattern()
{

	#ifdef ZIYIFraig
	cout<<"simulating the patterns get from proving"<<endl;
	cout<<_simPattern.size()<<" patterns"<<endl;
	cout<<"pattern Times: "<<_patternTimes<<endl;
	#endif
	for(size_t i = 0;i<_simPattern.size();++i)
	{
		simulateOnce((*_simPattern.at(i)));
	}

	cout<<"Updating by SAT... Total #FEC Group = "<<_FECGroups.size()<<endl;
	for(size_t i=0;i<_simPattern.size();++i)
	{
		_simPattern.at(i)->clear();
		delete _simPattern.at(i);
		_simPattern.clear();
	}
	_patternTimes = 0;
}


size_t
CirMgr::getPIIndex(CirGate* gate) const
{
	#ifdef ZIYIFraig
	assert(gate->getType() == PI_GATE);
	#endif
	for(size_t i=0;i<_PIList.size();++i)
	{
		if(_PIList.at(i) == gate) return i;
	}
	return CirGate::mySIZE_T_MAX;

}
/********************************************/
/*   Private member functions about fraig   */
/********************************************/
