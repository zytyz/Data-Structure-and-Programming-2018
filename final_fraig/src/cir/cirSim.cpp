/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

#include <cmath>

//#define ZIYISim 35
//#define ZIYISimConst
//#define ZIYISimBabe
//#define ZIYITime 35

#ifdef ZIYITime
#include<ctime>
#endif

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{

	++CirGate::globalRef;
	setDfsList();

	size_t thresHold = (log(_PIList.size()))+5;
	size_t failTimes = thresHold;

	size_t groupsNum;
	size_t N=0;
	while(failTimes)
	{
		//simulate once
		//randomly generate patterns for all PIs
		++N;
		groupsNum = _FECGroups.size();

		vector<size_t> input;
		for(int i=0;i<(int)_PIList.size();++i)
		{
			size_t x = 0;
			for(int n =0;n<32;++n)
			{
				int a = (rnGen(10)%4);
				//cout<<"a"<<a;
				x = ((x<<2)|a);
			}
			//cout<<endl<<hex<<x<<dec<<endl;
			input.push_back(x);
			//_PIList.at(i)->setSimValue(x);
			//cout<<"PI "<<_PIList.at(i)->getID()<<": "<<x<<endl;
		}
		simulateOnce(input);
		if(groupsNum == _FECGroups.size())
		{
			--failTimes;
		}
		else
		{
			failTimes = thresHold;
		}
	}
	cout<<(N*64)<<" patterns simulated."<<endl;
	
}

void
CirMgr::fileSim(ifstream& patternFile)
{
	//vector<vector<size_t>> simInputs;
	setDfsList();
	vector<string> lines;
	while(!patternFile.eof())
	{
		string tmp;
		patternFile>>tmp;
		//cout<<tmp<<endl;
		//cannot check valid lines right now because the "fine" part should be simulated
		if(tmp!="") lines.push_back(tmp);
	}
	patternFile.close();

	int totalSimTimes;
	if(lines.size()%64 == 0) totalSimTimes = lines.size()/64;
	else totalSimTimes = lines.size()/64 +1;

	int patternNum = 0;//the number of already simulated patterns
	
	for(int i=0;i<totalSimTimes;++i)
	{
		vector<string> subLines;
		for(int k=0;k<64;++k)
		{
			if((i*64+k)>=(int)lines.size()) break;
			subLines.push_back(lines.at(i*64+k));
		}

		vector<size_t> simInput;
		if(!getSimInput(subLines,simInput))
		{
			cerr<<patternNum<<" patterns simulated."<<endl;
			return;
		}
		#ifdef ZIYISim
		for(int k=0;k<(int)simInput.size();++k)
		{
			cout<<hex<<simInput.at(k)<<dec<<endl;
		}
		#endif
		simulateOnce(simInput);//including collect FECGroups
		patternNum+=64;
		if(_simLog!=0)writeSimLog(subLines);
		//simInputs.push_back(simInput);
	}
	cout<<lines.size()<<" patterns simulated."<<endl;
}

/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/

bool
CirMgr::getSimInput(vector<string>& lines,vector<size_t>& simInput)
{
	#ifdef ZIYISim
	assert(lines.size()<=64);
	//assert(lines.at(0).size()==_PIList.size());
	#endif

	for(int i=0;i<(int)_PIList.size();++i)
	{
		size_t tmp =0;
		simInput.push_back(tmp);
	}
	//assert(_PIList.size()<=64);

	for(int i=0;i<(int)lines.size();++i)
	{
		#ifdef ZIYISim_old
		cout<<"lines["<<i<<"]: "<<lines[i]<<endl;
		#endif

		for(int k=0;k<(int)lines[i].size();++k)
		{
			if(lines[i].size() != _PIList.size())
			{
				cerr<<"Error: Pattern("<<(lines[i])<<") length("<<(lines[i].size())<<") does not match the number of inputs("<<(_PIList.size())<<") in a circuit!!"<<endl;
				return false;
			}
			char chr = lines[i][k];
			if(chr!='0' && chr!='1')
			{
				cerr<<"Error: Pattern("<<lines[i]<<") contains a non-0/1 character('"<<chr<<"')."<<endl;
				return false;
			}
			size_t bit = chr-'0'+0;
			simInput.at(k) = simInput.at(k)|(bit<<i);
			#ifdef ZIYISim_old
			cout<<k<<" input: "<<hex<<simInput.at(k)<<dec<<endl;
			#endif
		}
		#ifdef ZIYISim_old
		cout<<endl;
		#endif
	}
	return true;
}

void
CirMgr::simulateOnce(vector<size_t>& input)
{
	++CirGate::globalRef;

	#ifdef ZIYISimBug
	static int t =0;
	for(int i=0;i<100;++i)
		cout<<endl;
	cout<<t<<endl;
	++t;
	#endif

	cout << "Total #FEC Group = " << _FECGroups.size() << flush;
	cout << char(13) << setw(30) << ' ' << char(13);

	assert(input.size()==_PIList.size());	
	for(int i=0;i<(int)_PIList.size();++i)
	{
		_PIList.at(i)->setSimValue(input.at(i));
		#ifdef ZIYISim
		cout<<"PI "<<_PIList.at(i)->getID()<<": "<<input.at(i)<<endl;
		#endif
	}
	//simulate the gates
	for(int i=0;i<(int)_POList.size();++i)
	{
		_POList.at(i)->simulateGate();
	}
	#ifdef ZIYISim
	cout<<endl;
	cout<<"start collecting groups..."<<endl;
	#endif
	//collect the FEC Groups(IFEC?)
	collectFECGroups();
	#ifdef ZIYISim
	cout<<"groups collected"<<endl;
	for(int i=0;i<(int)_FECGroups.size();++i)
	{
		for(int j=0;j<(int)_FECGroups[i]->size();++j)
		{
			cout<<_FECGroups[i]->at(j)->getID()<<" ";
		}
		cout<<endl;
	}
	#endif

	#ifdef ZIYISim
	cout<<"sorting fec groups..."<<endl;
	#endif
	sortFECGroups();
}

void
CirMgr::sortFECGroups()
{
	ziyiSortFunction ziyiobject;
	ziyiSortFunction2 ziyiobject2;
	for(size_t i =0;i<_FECGroups.size();++i)
	{
		GateList::iterator beg = _FECGroups.at(i)->begin();
		GateList::iterator en = _FECGroups.at(i)->end();
		sort(beg,en,ziyiobject);
	}
	sort(_FECGroups.begin(),_FECGroups.end(),ziyiobject2);
}

void
CirMgr::collectFECGroups()
{
	assert(_newFECGroups.size()==0);
	if(_FECGroups.size()==0)
	{
		#ifdef ZIYISim
		cout<<"constructing the first group"<<endl;
		#endif
		GateList* fecGrp = new GateList;

		//setDfsList();
		for(int i=0;i<(int)_dfsList.size();++i)
		{
			if(_dfsList.at(i)->getType()==AIG_GATE )
			{
				fecGrp->push_back(_dfsList.at(i));
				//x.second->clearFecGate();
			}
		}
		//if(fecGrp->find())
		fecGrp->push_back(getGate(0));

		#ifdef ZIYISim
		cout<<"printing out the first group"<<endl;
		for(int i=0;i<(int)fecGrp->size();++i)
		{
			cout<<fecGrp->at(i)->getID()<<" ";
		}
		cout<<endl;
		#endif

		_FECGroups.push_back(fecGrp);
	}

	#ifdef ZIYITime
	clock_t before,after;
	before = clock();
	after = before;
	#endif

	assert(_newFECGroups.empty());
	for(size_t i=0;i<_FECGroups.size();++i)
	{
		rearrangeFECGrp(_FECGroups.at(i));
	}

	#ifdef ZIYITime
	after = clock();
	cout<<"rearranging time: "<<(after - before)<<endl;
	#endif


	#ifdef ZIYITime
	before = clock();
	after  = before;
	#endif

	_FECGroups.clear();
	for(size_t i=0;i<_newFECGroups.size();++i)
	{
		_FECGroups.push_back(_newFECGroups.at(i));
	}
	_newFECGroups.clear();
	#ifdef ZIYITime
	after = clock();
	cout<<"copying time: "<<(after - before)<<endl;
	#endif

}

void
CirMgr::rearrangeFECGrp(GateList* fecGrp)
{
	#ifdef ZIYISim
	cout<<"rearranging fecGrp"<<endl;
	assert(fecGrp!=0);
	//assert(_newFECGroups.empty());
	#endif

	HashMap hash;//simValue, CirGate*
	unordered_map<size_t,GateList* > hash4NewGrps;//size_t is the simValue, GateList* is the pointer to the new grp
	
	for(size_t i=0;i<(fecGrp->size());++i)
	{
		#ifdef ZIYISim
		cout<<"going through gate "<<fecGrp->at(i)->getID()<<" in one fecGrp..."<<endl;
		#endif

		#ifdef ZIYISimConst
		if(fecGrp->at(i)->getID() == 0)
			cout<<"going through gate "<<fecGrp->at(i)->getID()<<" in one fecGrp..."<<endl;
		#endif

		size_t simValue = fecGrp->at(i)->getSimValue();
		size_t simInvValue = fecGrp->at(i)->getInvSimValue();

		HashIterator it = hash.find(simValue);
		HashIterator itInv = hash.find(simInvValue);
		assert( it==hash.end() || itInv == hash.end());
		//at most one of them can be in the hash map

		if(it == hash.end() && itInv == hash.end())
		{
			#ifdef ZIYISim
			cout<<"no same (inv) value in the map, inserting "<<fecGrp->at(i)->getID()<<" in hash"<<endl;
			#endif

			#ifdef ZIYISimConst
			if(fecGrp->at(i)->getID() == 0)
				cout<<"no same (inv) value in the map, inserting "<<fecGrp->at(i)->getID()<<" in hash"<<endl;
			#endif

			hash.insert(HashNode(simValue,fecGrp->at(i)));
			fecGrp->at(i)->clearFecList();
			//there is only one value of the conjugates in the hash map
		}
		else if(it != hash.end() && itInv == hash.end())
		{
			//has FEC Groups
			#ifdef ZIYISim
			cout<<"has a same value in the map, the gates are "<<it->second->getID()<<" in hash "
				<<" and "<<fecGrp->at(i)->getID()<<" now inserting"<<endl;
			#endif

			#ifdef ZIYISimConst
			if(fecGrp->at(i)->getID() == 0)
				cout<<"has a same value in the map, the gates are "<<it->second->getID()<<" in hash "
				<<" and "<<fecGrp->at(i)->getID()<<" now inserting"<<endl;
			#endif

			//check if there is a new fecGroup already
			if(hash4NewGrps.find(simValue) != hash4NewGrps.end())
			{
				//there is a new group already
				unordered_map<size_t,GateList* >::iterator tmp = hash4NewGrps.find(simValue);
				tmp->second->push_back(fecGrp->at(i));
				fecGrp->at(i)->setFecList(tmp->second);
			}
			else
			{
				//need to construct a new group
				GateList* newFecGrp = new GateList;
				_newFECGroups.push_back(newFecGrp);

				assert(hash4NewGrps.find(simValue) == hash4NewGrps.end());
				hash4NewGrps[simValue] = newFecGrp;

				newFecGrp->push_back(fecGrp->at(i));
				newFecGrp->push_back(it->second);

				it->second->setFecList(newFecGrp);
				fecGrp->at(i)->setFecList(newFecGrp);
			}
		}
		else if( it == hash.end() && itInv != hash.end())
		{
			//has inv fec groups
			#ifdef ZIYISim
			cout<<"has a same (inv) value in the map, the gates are "<<itInv->second->getID()<<" in hash "
				<<" and "<<fecGrp->at(i)->getID()<<" now inserting"<<endl;
			#endif

			#ifdef ZIYISimConst
			if(fecGrp->at(i)->getID() == 0)
				cout<<"has a same (inv) value in the map, the gates are "<<itInv->second->getID()<<" in hash "
				<<" and "<<fecGrp->at(i)->getID()<<" now inserting"<<endl;
			#endif


			if(hash4NewGrps.find(simInvValue) != hash4NewGrps.end())
			{
				//there is already a new ifec group for this simValue;
				unordered_map<size_t,GateList* >::iterator tmp = hash4NewGrps.find(simInvValue);
				tmp->second->push_back(fecGrp->at(i));
				fecGrp->at(i)->setFecList(tmp->second);
			}
			else
			{
				//need to construct one new group
				GateList* newFecGrp = new GateList;
				_newFECGroups.push_back(newFecGrp);

				assert(hash4NewGrps.find(simInvValue) == hash4NewGrps.end());
				hash4NewGrps[simInvValue] = newFecGrp;

				newFecGrp->push_back(fecGrp->at(i));
				newFecGrp->push_back(itInv->second);
				itInv->second->setFecList(newFecGrp);
				fecGrp->at(i)->setFecList(newFecGrp);
			}
		}
	}
	delete fecGrp;
	//delete fecGrp (all the gates are in newFECGroups now)
}

void
CirMgr::writeSimLog(vector<string>& lines)
{
	assert(_simLog!=0);
	vector<size_t> POsimValues;
	for(int i=0;i<(int)_POList.size();++i)
	{
		size_t tmp = _POList.at(i)->getSimValue();
		POsimValues.push_back(tmp);
	}
	vector<string> POlines;
	for(int j=0;j<(int)_POList.size();++j)
	{
		for(int i=0;i<(int)lines.size();++i)
		{
			if(j==0) POlines.push_back("0");
			else POlines.at(i)+="0";
		}
	}
	for(int i=0;i<(int)lines.size();++i)
	{
		for(int j=0;j<(int)_POList.size();++j)
		{
			POlines[i][j] = ((POsimValues.at(j)>>i)&1)+'0';
		}
	}	
	for(int i=0;i<(int)lines.size();++i)
	{
		(*_simLog)<<lines[i]<<" ";
		(*_simLog)<<POlines.at(i)<<endl;
		#ifdef ZIYISim
		cout<<lines[i]<<" ";
		cout<<POlines.at(i)<<endl;
		#endif
	}
	
}
