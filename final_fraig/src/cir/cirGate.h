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
#include <iomanip>
#include "cirDef.h"
#include "sat.h"

#include <cassert>
#include <unordered_map>
#include <limits>

//#define ZIYIGate 35
//#define ZIYIGateBug 35


//#define mySIZE_T_MAX numeric_limits<size_t>::max()

using namespace std;

// TODO: Feel free to define your own classes, variables, or functions.

class CirGate;


typedef unordered_map<size_t,CirGate*> HashMap;
typedef pair<size_t, CirGate*> HashNode;
typedef unordered_map<size_t,CirGate*>::iterator HashIterator;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
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
    
    virtual void addFanIn(CirGate* gate, bool inv){cerr<<"Error: calling virtual function(addfanin) "<<getTypeStr()<<": "<<getID()<<endl;}
    virtual void addFanOut(CirGate* gate,bool inv){cerr<<"Error: calling virtual function(addfanout) "<<getTypeStr()<<": "<<getID()<<endl;}
    virtual void addFanOut(Fan& fanout){cerr<<"Error: calling virtual function(addfanout) "<<getTypeStr()<<": "<<getID()<<endl;}
    virtual void getFanIn(FanList& fanlist) const {cerr<<"Error: calling virtual function(getfanin) "<<getTypeStr()<<": "<<getID()<<endl;}
    virtual void getFanOut(FanList& fanlist) const {cerr<<"Error: calling virtual function(getfanout) GateType"<<getTypeStr()<<": "<<getID()<<endl;}
    virtual string getSymbol() const {cerr<<"Error: calling virtual function(getsymbol)"<<endl;return "";}
    
    // Printing functions
    virtual void printGateNet() const{cerr<<"Error: calling virtual function(printGateNet)"<<endl;}
    bool isAig()const {return (_gateType==AIG_GATE);}
    void reportGate() const;
    void reportFanin(int level) const;
    void myReportFanin(int level,bool inv = false,int indent =0)const;
    void printGate(bool inv,int level) const;
    void reportFanout(int level) const;
    void myReportFanout(int level,bool inv = false,int indent=0) const;
    
    virtual void deleteGate(){cerr<<"Error: calling virtual function(deleting gate) "<<getTypeStr()<<": "<<getID()<<endl;}
    virtual void deleteFanOut(CirGate* targate){cerr<<"Error: calling virtual function(delete fanout) "<<getTypeStr()<<": "<<getID()<<endl;}
    virtual void deleteFanIn(CirGate* targate){cerr<<"Error: calling virtual function(delete fanin "<<getTypeStr()<<": "<<getID()<<endl;}
    virtual void replaceFanIn(CirGate* oldFanIn,Fan& newFanIn){cerr<<"Error: calling virtual function(replace fanin) "<<getTypeStr()<<": "<<getID()<<endl;}
    virtual void replaceFanOut(CirGate* oldFanOut,Fan& newFanOut){cerr<<"Error: calling virtual function(replace fanin) "<<getTypeStr()<<": "<<getID()<<endl;}
    virtual void optGate(){return;}
    virtual void strashGate(){return;}
    virtual void simulateGate(){cerr<<"Error: calling virtual function(simulate Gate)"<<endl;}
    size_t getSimValue() const { return _simValue;}
    size_t getInvSimValue() const { return (_simValue^ mySIZE_T_MAX);}
    void setSimValue(size_t x) { _simValue = x;}
    void printSimValue()const;
    virtual void setFecList(GateList* list){cerr<<"Error: calling virtual function(addFecGate)"<<endl;}
    virtual void clearFecList(){cerr<<"Error: calling virtual function(clearFecGate)"<<endl;}
    virtual void printFecList() const{cerr<<"Error: calling virtual function(print FECList)"<<endl;}
    
    virtual void setVar(const Var& v) { _var = v; }
    virtual Var getVar() const { return _var; }

    static size_t mySIZE_T_MAX;
    static size_t globalRef;
protected:
    const GateType _gateType;
    const unsigned _iD;
    const unsigned _lineNo;//if undef or const, then line No is 0
    mutable size_t _ref;//to check if the gate is already visited
    size_t _simValue;
    Var _var;

private:
};

class CirAigGate: public CirGate
{
    friend CirGate;
public:
    CirAigGate(GateType gt = TOT_GATE , unsigned i = 0,unsigned line = 0)
      :CirGate(gt,i,line),_fecList(0){}
    void addFanIn(CirGate* gate, bool inv) 
    {
        Fan fanin(gate,inv);
        #ifdef ZIYIGate
        assert(gate!=0);
        assert(_fanInList.size()<2);
        #endif
        _fanInList.push_back(fanin);
    }
    void addFanOut(CirGate* gate,bool inv) 
    {
      Fan fanout(gate,inv);
      _fanOutList.push_back(fanout); 
    }
    void addFanOut(Fan& fanout){_fanOutList.push_back(fanout);}
    void getFanIn(FanList& fanlist) const { fanlist = _fanInList;}
    void getFanOut(FanList& fanlist) const { fanlist = _fanOutList;}
    FanList& getFanOut() {return _fanOutList;}
    FanList& getFanIn() {return _fanInList;}
    void printGateNet() const
    {
      cout<<setw(4)<<left<<getTypeStr();
      cout<<_iD;
      #ifdef ZIYIGate
      assert(_fanInList.size()==2);
      #endif
      for(int i=0;i<2;++i)
      {
        cout<<" ";
        if(_fanInList.at(i).gate()->getType() == UNDEF_GATE)
          cout<<"*";
        if(_fanInList.at(i).inv()==true)
          cout<<"!";
        cout<<_fanInList.at(i).gate()->getID();
      }
      cout<<endl;
    }
    void deleteGate();
    void deleteFanOut(CirGate* targate);
    void deleteFanIn(CirGate* targate);
    void getOtherFanIn(Fan& otherFanIn, CirGate* knownGate) const;
    void replaceFanIn(CirGate* oldFanIn,Fan& newFanIn);
    void replaceFanOut(CirGate* oldFanOut,Fan& newFanOut);

    void optGate();
    OptType needOpt() const;
    void strashGate();
    bool needStrash(HashIterator& it);

    void simulateGate();
    void setFecList(GateList* list){ _fecList = list; }
    void clearFecList(){ _fecList = 0; }
    void printFecList() const;
    //bool definedNotUsed() const { return _fanOutList.empty();}

private:
    FanList _fanInList;
    FanList _fanOutList; 
    //true for inverting input
    //FanList _fecList;//including itself
    GateList* _fecList;
};

class CirPIGate: public CirGate
{
    friend CirGate;
public:
    CirPIGate(GateType gt = TOT_GATE , unsigned i = 0,unsigned line = 0)
      :CirGate(gt,i,line),_symbol(""){}
    void addFanOut(CirGate* gate,bool inv) 
    { 
      Fan fanout(gate,inv);
      _fanOutList.push_back(fanout);
    }
    void addFanOut(Fan& fanout){_fanOutList.push_back(fanout);}
    void setSymbol(string& str) { _symbol=str; }
    string getSymbol() const {return _symbol;}
    void getFanOut(FanList& fanlist) const { fanlist = _fanOutList;}
    void printGateNet() const
    {
      cout<<setw(4)<<left<<getTypeStr();
      cout<<_iD;
      if(_symbol!="")
        cout<<" ("<<_symbol<<")";
      cout<<endl;
    }
    void deleteFanOut(CirGate* targate);
    void replaceFanOut(CirGate* oldFanOut,Fan& newFanOut);
    void simulateGate();
private:
    FanList _fanOutList;
    string _symbol;
};

class CirPOGate: public CirGate
{
    friend CirGate;
public:
    CirPOGate(GateType gt = TOT_GATE , unsigned i = 0, unsigned line = 0)
      :CirGate(gt,i,line),_symbol(""){}
    void addFanIn(CirGate* gate, bool inv) { _fanIn.setFan(gate,inv);}
    void setSymbol(string& str) { _symbol=str; }
    string getSymbol() const {return _symbol;}
    void getFanIn(FanList& fanlist) const {fanlist.push_back(_fanIn);}
    Fan& getFanIn(){return _fanIn;}
    void printGateNet() const
    {
      cout<<setw(4)<<left<<getTypeStr();
      cout<<_iD<<" ";
      if(_fanIn.gate()->getType() == UNDEF_GATE)
        cout<<"*";
      if(_fanIn.inv()==true)
        cout<<"!";
      cout<<_fanIn.gate()->getID();
      if(_symbol!="")
        cout<<"("<<_symbol<<")";
      cout<<endl;
    }
    void deleteFanIn(CirGate* targate){cerr<<"PO has fanin but shouldn't be deleted"<<endl;}
    void replaceFanIn(CirGate* oldFanIn,Fan& newFanIn);
    void optGate();
    void strashGate();
    void simulateGate();
private:
    Fan _fanIn;
    string _symbol;
};

class CirConstGate: public CirGate
{
    friend CirGate;
public:
    CirConstGate(GateType gt = TOT_GATE , unsigned i = 0, unsigned line =0)
      :CirGate(gt,i,line){_simValue =0;}
    void addFanOut(CirGate* gate,bool inv)
    {
      Fan fanout(gate,inv);
      _fanOutList.push_back(fanout);
    }
    void addFanOut(Fan& fanout){_fanOutList.push_back(fanout);}
    void getFanOut(FanList& fanlist) const { fanlist = _fanOutList;}
    FanList& getFanOut() { return _fanOutList;}
    void printGateNet() const
    {
      cout<<setw(4)<<getTypeStr()<<getID()<<endl;
    }
    void deleteFanOut(CirGate* targate);
    void replaceFanOut(CirGate* oldFanOut,Fan& newFanOut);
    void simulateGate();
    void setFecList(GateList* list){ _fecList = list; }
    void clearFecList(){ _fecList = 0; }
    void printFecList() const;
    //void setVar(const Var& v){_var =v; cerr<<"const 0 is setting"<<endl;}
    //Var getVar() const { cerr<<"const 0 is getting var"<<endl;   return _var; }
    //bool definedNotUsed() const { return _fanOutList.empty();}
private:
    FanList _fanOutList;
    GateList* _fecList;

};

class CirUndefGate: public CirGate
{
    friend CirGate;
public:
    CirUndefGate(GateType gt = TOT_GATE , unsigned i = 0, unsigned line = 0)
      :CirGate(gt,i,line){ _simValue =0;}
    void addFanOut(CirGate* gate,bool inv)
    { 
      Fan fanout(gate,inv);
      _fanOutList.push_back(fanout);
    }
    void addFanOut(Fan& fanout){_fanOutList.push_back(fanout);}
    void getFanOut(FanList& fanlist) const { fanlist = _fanOutList;}
    void deleteGate();
    void deleteFanOut(CirGate* targate);
    bool noMoreFanOut() const { return _fanOutList.empty(); }
    void simulateGate();
private:
    FanList _fanOutList;
};


class KeyGenerator
{
public:
    KeyGenerator(){}
    size_t operator() (const CirGate* gate) 
    { 
        assert(gate->getType() == AIG_GATE);
        FanList fanInList = ((CirAigGate*)gate)->getFanIn();
        for(int i=0;i<2;++i)
        {
          _fanIn[i] = (size_t)fanInList.at(i).gate();
        }
        if(_fanIn[0]>_fanIn[1])
        {
          return (_fanIn[0]>>3) + (_fanIn[1]>>6);
        }
        else
        {
          return (_fanIn[1]>>3) + (_fanIn[0]>>6);
        }
    }
private:
   size_t _fanIn[2];
};


#endif // CIR_GATE_H
