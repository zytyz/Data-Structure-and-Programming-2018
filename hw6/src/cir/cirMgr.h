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

using namespace std;

#include "cirDef.h"
#include "cirGate.h"

//#define ZIYIMgrh 35

extern CirMgr *cirMgr;
typedef pair<unsigned, CirGate*> GatePair;
typedef map<unsigned, CirGate*> GateMap;
// TODO: Define your own data members and member functions
class CirMgr
{
public:
    CirMgr():_aigNum(0),_undefNum(0),_maxVar(0),index(0) {}
    ~CirMgr() 
    {
        #ifdef ZIYIMgrh
        cout<<"deleting gates"<<endl;
        #endif
        for(auto x:_gateMap)
        {
            delete x.second;
        }
    }
    // Access functions
    // return '0' if "gid" corresponds to an undefined gate.
    CirGate* getGate(unsigned gid) const;

    // Member functions about circuit construction
    bool readCircuit(const string&);

    // Member functions about circuit reporting
    void printSummary() const;
    void printNetlist() const;
    void printPIs() const;
    void printPOs() const;
    void printFloatGates() const;
    void writeAag(ostream&) const;

private:  
    bool regGate(int* header, ifstream& myfile, vector<unsigned>& data);
    void connectGate(int* header, vector<unsigned>& data);
    bool setSymbols(ifstream& myfile);
    void printMap()const;
    void dfsNet(CirGate* gate) const;
    void printGateNet(CirGate* gate) const;
    void aigListDfs()const;
    void dfsForAigGate(CirGate* gate)const;
    bool getPIPO(ifstream&,string&,const string&,unsigned& )const;
    bool getAig(ifstream&,string&,unsigned* )const;
    string headerIdentifer(int times) const;
    GateMap _gateMap;
    GateList _POList;
    GateList _PIList;
    mutable GateList _aigList;//for dfs, is empty before going any dfs traversal
    //GateList _undefList;
    size_t _aigNum;//number of aig gates
    size_t _undefNum;
    size_t _maxVar;
    mutable size_t index;//for printing netlist
};

#endif // CIR_MGR_H
