/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include <map>
#include "cirDef.h"

#define ZIYIGate

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGate
{
public:
    CirGate(GateType gt = TOT_GATE , unsigned i = 0,unsigned line = 0)
      :_gateType(gt),_iD(i),_lineNo(line) {_ref = 0;}
    virtual ~CirGate() {}

    // Basic access methods
    string getTypeStr() const;
    unsigned getLineNo() const { return _lineNo; }
    unsigned getID() const { return _iD;}
    GateType getType() const { return _gateType; }
    bool traversed() const {return (_ref==globalRef);}
    void setToGlobal() const { _ref = globalRef;}
    
    virtual void addFanIn(CirGate* gate, bool inv){cerr<<"Error: calling virtual function"<<endl;}
    virtual void addFanOut(CirGate* gate){cerr<<"Error: calling virtual function"<<endl;}
    //virtual const GateList& getFanOut() const {cerr<<"Error: calling virtual function"<<endl; return GateList();}
    // Printing functions
    void reportGate() const;
    void reportFanin(int level) const;
    void myReportFanin(int level,bool inv = false,int indent =0)const;
    void printGate(bool inv,int level) const;
    void reportFanout(int level) const;
    void myReportFanout(int level,const CirGate* fanin,int indent=0) const;
    static size_t globalRef;
protected:
    const GateType _gateType;
    const unsigned _iD;
    const unsigned _lineNo;//if undef or const, then line No is 0
    mutable size_t _ref;//to check if the gate is already visited  
private:

};

class CirAigGate: public CirGate
{
    friend CirGate;
public:
    CirAigGate(GateType gt = TOT_GATE , unsigned i = 0,unsigned line = 0)
      :CirGate(gt,i,line)
    {
        for(int i=0;i<2;i++)
        {
            _fanInList[i]=0;
            _fanInInvList[i]=0;
        }
    }
    void addFanIn(CirGate* gate, bool inv) 
    {
        #ifdef ZIYIGate
        assert(_fanInList[1]==NULL);
        #endif
        for(int i=0;i<2;i++)
        {
            if(_fanInList[i]==NULL)
            {
                #ifdef ZIYIGate
                assert(_fanInInvList[i]==0);
                #endif
                _fanInList[i] = gate;
                _fanInInvList[i] = inv;
                break;//only insert one fanin once
            }
        }
        #ifdef ZIYIGate
        assert(_fanInList[0]!=NULL);
        #endif
    }
    void addFanOut(CirGate* gate) {_fanOutList.push_back(gate); }
    void getFanIn(CirGate**& fil, bool*& invl)
    {
        fil = _fanInList;
        invl = _fanInInvList;
    }
    const GateList& getFanOut() const {return _fanOutList; }
    
private:
    CirGate* _fanInList[2]; //should only have two fanins 
    GateList _fanOutList; // typedef vector<CirGate* >  GateList
    //for inverting
    bool _fanInInvList[2];//true for inverting input
};

class CirPIGate: public CirGate
{
    friend CirGate;
public:
    CirPIGate(GateType gt = TOT_GATE , unsigned i = 0,unsigned line = 0)
      :CirGate(gt,i,line),_symbol(""){}
    void addFanOut(CirGate* gate) { _fanOutList.push_back(gate);}
    void setSymbol(string& str) { _symbol=str; }
    string getSymbol() const {return _symbol;}
    const GateList& getFanOut() const {return _fanOutList; }
private:
    GateList _fanOutList;
    string _symbol;
};

class CirPOGate: public CirGate
{
    friend CirGate;
public:
    CirPOGate(GateType gt = TOT_GATE , unsigned i = 0, unsigned line = 0)
      :CirGate(gt,i,line),_fanIn(NULL),_fanInInv(0),_symbol(""){}
    void addFanIn(CirGate* gate, bool inv) 
    {
        _fanIn = gate; 
       _fanInInv = inv;
    }
    void setSymbol(string& str) { _symbol=str; }
    string getSymbol() const {return _symbol;}
    pair<CirGate*,bool> getFanIn() const {return pair<CirGate*,bool>(_fanIn,_fanInInv);}
private:
    CirGate* _fanIn;
    bool _fanInInv;
    string _symbol;
};

class CirConstGate: public CirGate
{
    friend CirGate;
public:
    CirConstGate(GateType gt = TOT_GATE , unsigned i = 0, unsigned line =0)
      :CirGate(gt,i,line){}
    void addFanOut(CirGate* gate){_fanOutList.push_back(gate);}
    const GateList& getFanOut() const {return _fanOutList; }
private:
    GateList _fanOutList;

};

class CirUndefGate: public CirGate
{
    friend CirGate;
public:
    CirUndefGate(GateType gt = TOT_GATE , unsigned i = 0, unsigned line = 0)
      :CirGate(gt,i,line){}
    void addFanOut(CirGate* gate){ _fanOutList.push_back(gate);}
    const GateList& getFanOut() const {return _fanOutList; }
private:
    GateList _fanOutList;
};





#endif // CIR_GATE_H

