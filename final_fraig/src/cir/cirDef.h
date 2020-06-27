/****************************************************************************
  FileName     [ cirDef.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic data or var for cir package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_DEF_H
#define CIR_DEF_H

#include <vector>
#include "myHashMap.h"

using namespace std;

// TODO: define your own typedef or enum

class CirGate;
class CirMgr;
class SatSolver;

class Fan
{
public:
  Fan(CirGate* gt=0,bool in=0):_gate(gt),_inv(in){};
  void setFan(CirGate* gt, bool in){ _gate = gt; _inv = in;}
  CirGate* gate() const {return _gate;}
  bool inv() const {return _inv;}
  Fan& operator=(const Fan& f) { _gate = f.gate(); _inv = f.inv(); return (*this);}
  bool operator==(const Fan& f) const { return (_gate == f._gate) && (_inv==f._inv);}
private:
  CirGate* _gate;
  bool _inv;
};

typedef vector<CirGate*>           GateList;
typedef vector<Fan> FanList;
typedef vector<unsigned>           IdList;


enum GateType
{
   UNDEF_GATE = 0,
   PI_GATE    = 1,
   PO_GATE    = 2,
   AIG_GATE   = 3,
   CONST_GATE = 4,

   TOT_GATE
};

enum OptType
{
  FANIN_CONST_0 = 0,
  FANIN_CONST_1 = 1,
  IDENTICAL_FANIN = 2,
  INVERTED_FANIN = 3,

  TOT_OPT_TYPE
};






#endif // CIR_DEF_H
