/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

//#define ZIYIOpt 35

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed

void
CirMgr::sweep()
{
	setDfsList();
	IdList unusedGates;
	for(auto x: _gateMap)
	{
	  if(!x.second->traversed())
	  {
	    if(x.second->getType()==AIG_GATE)
	    {
			x.second->deleteGate();
			--_aigNum;
			unusedGates.push_back(x.first);
	    }
	    else if(x.second->getType()==UNDEF_GATE)
	    {
			x.second->deleteGate();
			--_undefNum;
			unusedGates.push_back(x.first);
	    }
	  }
	}
	for(int i=0;i<(int)unusedGates.size();++i)
	{
		_gateMap.erase(unusedGates.at(i));
	}
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...

void
CirMgr::optimize()
{
	++CirGate::globalRef;

	for(int i=0;i<(int)_POList.size();++i)
	{
		_POList.at(i)->optGate();
	}
	//setDfsList();
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/

void
CirMgr::eraseMap(unsigned id)
{
	_gateMap.erase(id);
}

