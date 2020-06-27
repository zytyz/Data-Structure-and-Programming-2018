/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <map>
#include <unordered_map>
#include <algorithm>

#include "sat.h"

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"
#include "cirGate.h"

extern CirMgr *cirMgr;
extern KeyGenerator keyGen;

typedef pair<unsigned, CirGate*> GatePair;
typedef map<unsigned, CirGate*> GateMap;



class CirMgr
{
public:
    CirMgr():_aigNum(0),_undefNum(0),_maxVar(0),_patternTimes(0) {}
    ~CirMgr() 
    {
        #ifdef ZIYIMgrh
        cout<<"deleting gates"<<endl;
        #endif
        for(auto x:_gateMap)
        {
            delete x.second;
        }
        if(_FECGroups.size()!=0)
        {
          for(int i=0;i<(int)_FECGroups.size();++i)
          {
            delete _FECGroups.at(i);
          }
          _FECGroups.clear();
        }
        if(_simPattern.size()!=0)
        {
          /*for(int i=0; i<(int)_simPattern.size();++i)
          {
            delete _simPattern.at(i);
          }*/
          _simPattern.clear();
        }
    }

    // Access functions
    // return '0' if "gid" corresponds to an undefined gate.
    CirGate* getGate(unsigned gid) const;
    // Member functions about circuit construction
    bool readCircuit(const string&);

    // Member functions about circuit optimization
    void sweep();
    void optimize();

    // Member functions about simulation
    void randomSim();
    void fileSim(ifstream&);
    void setSimLog(ofstream *logFile) { _simLog = logFile; }

    // Member functions about fraig
    void strash();
    void printFEC() const;
    void fraig();

    // Member functions about circuit reporting
    void printSummary() const;
    void printNetlist();
    void printPIs() const;
    void printPOs() const;
    void printFloatGates() const;
    void printFECPairs() const;
    void writeAag(ostream&);
    void writeGate(ostream&, CirGate*) const;
    void eraseMap(unsigned);
    void decreUndefNum(){ --_undefNum;}
    void decreAigNum(){ --_aigNum;}

    HashIterator hashFind(size_t s){ return _hash.find(s);}
    void hashInsert(HashNode n){ _hash.insert(n); }
    HashIterator hashEnd(){return _hash.end(); }
private:
    
    bool regGate(int* header, ifstream& myfile, vector<unsigned>& data);
    void connectGate(int* header, vector<unsigned>& data);
    bool setSymbols(ifstream& myfile);
    void setDfsList();
    void searchDfs(CirGate* gate);
    void collectFECGroups();
    void rearrangeFECGrp(GateList* fecGrp);
    void sortFECGroups();
    void writeSimLog(vector<string>& lines);
    bool getSimInput(vector<string>& lines,vector<size_t>& simInput);
    void simulateOnce(vector<size_t>& input);
    void printMap()const;
    void searchGateDfs(CirGate* gate,GateList& list) const;
    void satOneFecGrp(GateList* fecGrp);
    bool proveEqual(CirGate* gate1,CirGate* gate2,bool inv);
    void subCircuit(CirGate* gate1,CirGate* gate2,GateList& list);
    void genProofModel(GateList& gates);
    void mergePiles(GateList* pile);
    void merge2Gates(CirGate* gate1,CirGate* gate2);
    void simulatePattern();
    size_t getPIIndex(CirGate* gate) const;
    GateMap _gateMap;
    GateList _POList;
    GateList _PIList;
    GateList _dfsList;
    //GateList _unusedList;//from dfs, should be formed and cleared after sweep()
    //mutable GateList _aigList;//for dfs, is empty before going any dfs traversal
    //GateList _undefList;
    size_t _aigNum;//number of aig gates in the dfs list
    size_t _undefNum;
    size_t _maxVar;

    HashMap _hash;
    vector<GateList*> _FECGroups;
    vector<GateList*> _newFECGroups;

    ofstream  *_simLog;

    SatSolver _solver;
    vector<vector<size_t>* > _simPattern;//patterns to break fec pairs
    size_t _patternTimes;//times that i found a pattern, when %64==0 i need a new size_t in _simPattern

};

struct ziyiSortFunction
{
    bool operator() (CirGate* gate1, CirGate* gate2) 
    { 
        return (gate1->getID() < gate2->getID());
    }
};

struct ziyiSortRevFunction
{
    bool operator() (CirGate* gate1, CirGate* gate2) 
    { 
        return (gate1->getID() > gate2->getID());
    }
};

struct ziyiSortFunction2
{
    bool operator() (GateList* list1, GateList* list2) 
    { 
        return (list1->at(0)->getID() < list2->at(0)->getID());
    }
};

struct ziyiSortDFSFunction
{
    bool operator() (CirGate* gate1, CirGate* gate2) 
    { 
        if(gate1->getType() == CONST_GATE || gate2->getType() == CONST_GATE)
        {
          if(gate1->getType() == CONST_GATE) return true;
          else if(gate2->getType() == CONST_GATE)return false;
        }
        for(int i =0;i<(int)gates.size();++i)
        {
          if(gates[i] == gate1 || gates[i] == gate2)
          {
            if(gates[i] == gate1) return true;
            else return false;
          }
        }
        cerr<<"sorting goes wrong"<<endl;
        return false;
    }
    ziyiSortDFSFunction(GateList& gt){ gates = gt;}
    GateList gates;
};

#endif // CIR_MGR_H
