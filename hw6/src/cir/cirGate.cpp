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
#include <sstream>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

extern CirMgr *cirMgr;

size_t CirGate::globalRef =0;

// TODO: Implement memeber functions for class(es) in cirGate.h

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
    cout<<"=================================================="<<endl;
    //mstringstream ss;
    string str="";
    str+=("= " + getTypeStr() + "(" + to_string(getID()) + ")" );
    if(getType()==PI_GATE)
    {
        if(((CirPIGate*)this)->getSymbol()!="")
          str+= "\"" + ((CirPIGate*)this)->getSymbol() + "\"";
    }
    else if(getType()==PO_GATE)
    {
        if(((CirPOGate*)this)->getSymbol()!="")
          str+= "\"" + ((CirPOGate*)this)->getSymbol() + "\"";
    }
    str+= ", line " + to_string(getLineNo());
    //ss>>str;
    cout<<setw(49)<<left<<str;
    cout<<"="<<endl;
    cout<<"=================================================="<<endl;

}

void
CirGate::reportFanin(int level) const//inv default is false
{
    assert (level >= 0);
    ++CirGate::globalRef;
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
        ((CirAigGate*)this)->_fanInList[0]->myReportFanin(--level,(((CirAigGate*)this)->_fanInInvList[0]),++indent);
        ((CirAigGate*)this)->_fanInList[1]->myReportFanin(level,(((CirAigGate*)this)->_fanInInvList[1]),indent);
        setToGlobal();//only set to global if its fanin is reported
        break;
      } 
      case PO_GATE:
      {
        ((CirPOGate*)this)->_fanIn->myReportFanin(--level,(((CirPOGate*)this)->_fanInInv),++indent);
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
    cout<<endl;   
}

void
CirGate::reportFanout(int level) const
{
    assert (level >= 0);
    ++CirGate::globalRef;
    myReportFanout(level,this,0);
}


#define myReportFanoutPart(T)                           \
const GateList& fanOutList = ((T*)this)->getFanOut();   \
--level;                                                \
++indent;                                               \
for(int i=0;i<(int)fanOutList.size();++i)               \
{                                                       \
  fanOutList.at(i)->myReportFanout(level,this,indent);  \
}                                                       \
setToGlobal();                                          \
break;

void
CirGate::myReportFanout(int level, const CirGate* fanin,int indent) const
{
    for(int i=0;i<indent;++i) cout<<"  ";
    bool inv = false;
    switch(getType())
    {
      case AIG_GATE:
      {
          for(int i=0;i<2;++i)
          {
            if(((CirAigGate*)this)->_fanInList[i] == fanin)
            {
              inv = ((CirAigGate*)this)->_fanInInvList[i];
              break;
            }
          }
          break;
      }
      case PO_GATE:
      {
          inv = ((CirPOGate*)this)->_fanInInv;
          break;
      }
      default:
        break;
    }
    printGate(inv,level);
    if(level==0)return;
    if(traversed())return;
    switch(getType())
    {
      case AIG_GATE:
      {
        myReportFanoutPart(CirAigGate);
      }
      case PI_GATE:
      {
        myReportFanoutPart(CirPIGate);
      }
      case CONST_GATE:
      {
        myReportFanoutPart(CirConstGate);
      }
      case UNDEF_GATE:
      {
        myReportFanoutPart(CirUndefGate);
      }
      default:
        break;
    }
}


/*void
CirPOGate::dfs() const
{

}*/

