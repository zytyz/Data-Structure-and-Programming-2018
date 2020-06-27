/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

//#define ZIYIMgr 35
// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine const (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool
CirMgr::readCircuit(const string& fileName)
{
    ifstream myfile(fileName);
    if(!myfile.is_open())
    {
        cerr<<"Cannot open design \""<<fileName<<"\"!!"<<endl;
        return false;
    }
    #ifdef ZIYIMgr
    cout<<"file opened"<<endl;
    #endif
    string str;
    int header[5];  
    myfile>>str;
    for(int i=0;i<5;++i)
      myfile>>header[i];

    #ifdef ZIYIMgr
    for(int i=0;i<5;++i)
      cout<<header[i]<<" ";
    cout<<endl;
    #endif
    _maxVar = header[0];
    vector<unsigned> data;//temporarily saves the gate connection data
    if(!regGate(header,myfile,data))return false;
    //cout<<"data size: "<<data.size()<<endl;
    connectGate(header, data);

    #ifdef ZIYIMgr
    cout<<"gates connected"<<endl;
    #endif

    if(!setSymbols(myfile))return false;
    #ifdef ZIYIMgr
    cout<<"symbols set"<<endl;
    #endif

    setDfsList();

    #ifdef ZIYIPrintMap
    printMap();
    #endif
    myfile.close();
    return true;
   return true;
}


bool
CirMgr::regGate(int* header, ifstream& myfile,vector<unsigned>& data)
{
    //header: max var, input, latch, output, andgate
    #ifdef ZIYIMgr
    assert(header[2]==0);
    cout<<"reg gate..."<<endl;
    #endif
    //reg CONST 0
    CirGate* constGate = new CirConstGate(CONST_GATE, 0,0);
    _gateMap.insert(GatePair(0,constGate));
    //data is for connection later
    //data order: output -> aiggate(3 numbers)
    //for input
    
    for(int i=0;i<header[1];++i)
    {
        unsigned id;
        myfile>>id;
        assert(id%2==0);
        //cout<<"what is line"<<(line=="")<<endl;
        id/=2;
        CirGate* PIGate = new CirPIGate(PI_GATE,id ,2+i);
        _gateMap.insert(GatePair (id, PIGate));
        _PIList.push_back(PIGate);        
    }

    //for output
    for(int i=0;i<header[3];++i)
    {
        unsigned id;
        myfile>>id;
        CirGate* POGate = new CirPOGate(PO_GATE, unsigned(header[0]+1+i), unsigned(header[1]+2+i));
        data.push_back(id);
        _gateMap.insert(GatePair (unsigned(header[0]+1+i), POGate));
        _POList.push_back(POGate);
    }
    //for aig gate
    for(int i=0;i<header[4];++i)
    {
        unsigned literal[3];
        for(int j=0;j<3;++j)
        {
          myfile>>literal[j];
          data.push_back(literal[j]);
        }
        #ifdef ZIYIMgr
        assert(literal[0]%2 ==0);
        #endif
        unsigned id = literal[0]/2;
        CirGate* aigGate = new CirAigGate(AIG_GATE,id,unsigned(header[1]+header[3]+2+i));
        _gateMap.insert(GatePair(id ,aigGate));
        ++_aigNum;
    }
    return true;
}

void 
CirMgr::connectGate(int* header, vector<unsigned>& data)
{
    //header: max var, input, latch, output, andgate
    //connect POs
    #ifdef ZIYIMgr
    assert(header[3] == (int) _POList.size());
    assert(header[1] == (int) _PIList.size());
    assert(data.size() == (_POList.size() + header[4]*3));
    cout<<"connect gate..."<<endl;
    cout<<"connecting PO"<<endl;
    #endif
    //connects PO and its fanin
    //CirGate** POfanin = new CirGate* [(int)_POList.size()];
    //getPOgates(POfanin,data);

    for(int i=0;i<(int)_POList.size();++i)
    {
        unsigned id = data.at(i);
        //cout<<"PO data in file: "<<id<<endl;
        CirGate* fanIn = getGate(id/2);

        if(fanIn == NULL)
        {
            fanIn = new CirUndefGate(UNDEF_GATE,id/2,0);
            _gateMap.insert(GatePair(id/2,fanIn));
            ++_undefNum;
            //_undefList.push_back(fanIn);
        }
        _POList.at(i)->addFanIn(fanIn,(bool)(id%2));
        fanIn->addFanOut(_POList.at(i),(bool)(id%2));
    }

    #ifdef ZIYIMgr
    cout<<"connecting aig"<<endl;
    #endif
    //connects aig and its fanin
    //GateList aigGates;
    //getAigGates(aigGates,data);
    
    for(int i=0;i<header[4];i++)
    {
        unsigned aig[3];
        for(int j=0;j<3;j++)
          aig[j] = data.at( 3*i +j + (int)_POList.size());
        //getAigGates(gates,aig);
        //CirGate* aigGate = getGate(aig[0]/2);
        CirGate* aigGate = getGate(aig[0]/2);

        #ifdef ZIYIMgr
        assert(aigGate!=NULL);
        #endif
        for(int j=0;j<2;j++)
        {
          //CirGate* fanIn = getGate(aig[j+1]/2);
          CirGate* fanIn = getGate(aig[j+1]/2);
          if(fanIn==NULL)
          {
              #ifdef ZIYIMgr
              cout<<"new undef gate when connecting aig"<<endl;
              #endif
              fanIn = new CirUndefGate(UNDEF_GATE, aig[j+1]/2,0);
              _gateMap.insert(GatePair( aig[j+1]/2 ,fanIn));
              ++_undefNum;
          }
          aigGate->addFanIn(fanIn, (bool)(aig[j+1]%2) );
          fanIn->addFanOut(aigGate,(bool)(aig[j+1]%2));
        }
    }
}


bool
CirMgr::setSymbols(ifstream& myfile)
{
    
    int i=0;
    #ifdef ZIYIMgr
    cout<<"setting symbols"<<endl;
    cout<<"myfile reaches end? "<<myfile.eof()<<endl;
    #endif
    while(!myfile.eof()) 
    {
        ++i;
        string str;
        getline(myfile,str);
        if(str == "" && i!=1)break;
        if(str == "c")break;
        if(str == "")continue;
        size_t pos = str.find_first_of(' ');
        #ifdef ZIYI_unused
        cout<<i<<": str: "<<str<<endl;
        assert(pos!=string::npos);
        #endif
        string symbol[2];
        symbol[0] = str.substr(0,pos);//i1
        symbol[1] = str.substr(pos+1);//name
        #ifdef ZIYIMgr
        cout<<symbol[0]<<" "<<symbol[1]<<endl;
        //assert(symbol[0].size()==2);
        #endif
        if(symbol[0][0]=='i')
        {
          int num;
          myStr2Int(symbol[0].substr(1),num);
          ((CirPIGate*)_PIList.at(num))->setSymbol(symbol[1]);
        }
        else if(symbol[0][0]=='o')
        {
          int num;
          myStr2Int(symbol[0].substr(1),num);
          ((CirPOGate*)_POList.at(num))->setSymbol(symbol[1]);
        }
    }
    return true;
}

void
CirMgr::setDfsList()
{
  ++CirGate::globalRef;
  _dfsList.clear();
  for(int i=0;i<(int)_POList.size();++i)
  {
    searchDfs(_POList.at(i));
  }
}

void
CirMgr::searchDfs(CirGate* gate)
{
    if(gate->traversed())return;
    GateType type = gate->getType();
    switch(type)
    {
      case UNDEF_GATE:{break;}
      case CONST_GATE:
      {
        _dfsList.push_back(gate);break;
      }
      case PI_GATE:
      {
        _dfsList.push_back(gate);break;
      }
      case AIG_GATE:
      {
        FanList fanin;
        gate->getFanIn(fanin);
        for(int i=0;i<(int)fanin.size();++i)
        {
          searchDfs(fanin.at(i).gate());
        }
        _dfsList.push_back(gate);
        break;
      }
      case PO_GATE:
      {
        FanList fanin;
        gate->getFanIn(fanin);
        for(int i=0;i<(int)fanin.size();++i)
        {
          searchDfs(fanin.at(i).gate());
        }
        _dfsList.push_back(gate);
        break;
      }
      default:
      {
        #ifdef ZIYIMgr
        assert(false);
        #endif
      }
    }
    gate->setToGlobal();
}

CirGate* 
CirMgr::getGate(unsigned gid) const
{
    GateMap::const_iterator it = _gateMap.find(gid);
    if(it!=_gateMap.end())
    {
      return it->second;
    }
    else return 0;
}


/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
    cout<<endl;
    cout<<"Circuit Statistics"<<endl;
    for(int i=0;i<18;i++)cout<<"=";
    cout<<endl;
    cout<<"  PI        "<< setw(4) << right <<_PIList.size()<<endl;
    cout<<"  PO        "<< setw(4) << right <<_POList.size()<<endl;
    cout<<"  AIG      "<< setw(5) << right <<_aigNum<<endl;
    for(int i=0;i<18;i++)cout<<"-";
    cout<<endl;
    cout<<"  Total    " << setw(5) << right
        << _PIList.size()+_POList.size()+_aigNum <<endl;
}

void
CirMgr::printNetlist() 
{
    #ifdef ZIYIMgr
    assert(_dfsList.size()!=0);
    #endif
    setDfsList();

    #ifdef ZIYIMgr
    cout<<"dfsList is set"<<endl;
    #endif

    cout << endl;
    for (unsigned i = 0, n = _dfsList.size(); i < n; ++i) {
      cout << "[" << i << "] ";
      _dfsList[i]->printGateNet();
    }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for(int i=0;i<(int)_PIList.size();++i)
   {
      cout<<" "<<(_PIList.at(i)->getID());
   }
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for(int i=0;i<(int)_POList.size();++i)
   {
      cout<<" "<<(_POList.at(i)->getID());
   }
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
  if(_undefNum!=0)//there are floating fanins
    {
        cout<<"Gates with floating fanin(s):";
        ++CirGate::globalRef;
        for(auto x: _gateMap)
        {
            if(x.second->getType() == UNDEF_GATE )
            {
                FanList fanOutList;
                x.second->getFanOut(fanOutList);
                for(int i=0;i<(int)fanOutList.size();++i)
                {
                    if(fanOutList.at(i).gate()->traversed())continue;
                    cout <<" "<< fanOutList.at(i).gate()->getID();
                    fanOutList.at(i).gate()->setToGlobal();
                }
            }
        }
        cout<<endl;
    }
    bool existUnused = false;
    for(auto x:_gateMap)//defined but not used
    {
        if((x.second->getType() == AIG_GATE) || (x.second->getType() == PI_GATE))
        {
            FanList fanOutList;
            x.second->getFanOut(fanOutList);
            if(fanOutList.size()==0) 
            {
              if(!existUnused)cout<<"Gates defined but not used  :";
              cout <<" "<< (x.second->getID());
              existUnused = true;
            }
        }
    }
    if(existUnused)cout<<endl;
}

void
CirMgr::printFECPairs() const
{
    #ifdef ZIYIMgr
    cout<<"printing FEC pairs"<<endl;
    #endif
  
    for(int i=0;i<(int)_FECGroups.size();++i)
    {
      cout<<"["<<i<<"]";
      assert(_FECGroups.at(i)!=0);
      for(int j=0;j<(int)_FECGroups.at(i)->size();++j)
      {
        cout<<" ";
        if(_FECGroups.at(i)->at(j)->getSimValue() == _FECGroups.at(i)->at(0)->getInvSimValue())
          cout<<"!";
        cout<<_FECGroups.at(i)->at(j)->getID();
      }
      cout<<endl;
      #ifdef ZIYIMgr
      cout<<"size: "<<_FECGroups.at(i)->size()<<endl;
      #endif
    }
}

void
CirMgr::writeAag(ostream& outfile) 
{

    setDfsList();
    GateList aigList;
    for(int i=0;i<(int)_dfsList.size();++i)
    {
      if(_dfsList.at(i)->getType() == AIG_GATE)
        aigList.push_back(_dfsList.at(i));
    }
    outfile <<"aag "<<_maxVar<<" "<<_PIList.size()<<" "<<0<<" "
            <<_POList.size()<<" "<<aigList.size()<<endl;
    //input
    for(int i=0;i<(int)_PIList.size();++i)
    {
      outfile<< (_PIList.at(i)->getID())*2 <<endl;
    }
    //output
    for(int i=0;i<(int)_POList.size();++i)
    {
      Fan& fanin = ((CirPOGate*)_POList.at(i))->getFanIn();
      if(fanin.inv() == true)
        outfile<< (fanin.gate()->getID())*2 +1 <<endl;
      else
        outfile<< (fanin.gate()->getID())*2<<endl;
    }
    //aigGates
    for(int i=0;i<(int)aigList.size();++i)
    {
      outfile << (aigList.at(i)->getID())*2;
      FanList fanInList;
      aigList.at(i)->getFanIn(fanInList);
      for(int j=0;j<2;j++)
      {
          if(fanInList.at(j).inv()==1)
            outfile<<" "<<(fanInList.at(j).gate()->getID())*2 +1;
          else
            outfile<<" "<<(fanInList.at(j).gate()->getID())*2 ;
      }
      outfile<<endl;
    }
    //symbols
    for(int i=0;i<(int)_PIList.size();++i)
    {
        if(((CirPIGate*)_PIList.at(i))->getSymbol()!="")
        {
          outfile<<"i"<<i<<" "<<((CirPIGate*)_PIList.at(i))->getSymbol()<<endl;
        }
    }
    for(int i=0;i<(int)_POList.size();++i)
    {
        if(((CirPOGate*)_POList.at(i))->getSymbol()!="")
        {
          outfile<<"o"<<i<<" "<<((CirPOGate*)_POList.at(i))->getSymbol()<<endl;
        }
    }
    outfile<<'c'<<endl;
    outfile<<"zyt_sheep"<<endl;
}

void
CirMgr::writeGate(ostream& outfile, CirGate *g) const
{
    ++CirGate::globalRef;
    GateList list;
    searchGateDfs(g,list);
    GateList aigList,piList,poList;

    ziyiSortFunction ziyiObject;
    sort(list.begin(),list.end(),ziyiObject);

    for(int i=0;i<(int)list.size();++i)
    {
      if(list.at(i)->getType() == AIG_GATE)
        aigList.push_back(list.at(i));
      else if(list.at(i)->getType() == PO_GATE)
        poList.push_back(list.at(i));
      else if(list.at(i)->getType() == PI_GATE)
        piList.push_back(list.at(i));
    }
    outfile <<"aag "<<list.at(list.size()-1)->getID()<<" "<<piList.size()<<" "<<0<<" "
            <<1<<" "<<aigList.size()<<endl;
    //input
    for(int i=0;i<(int)piList.size();++i)
    {
      outfile<< (piList.at(i)->getID())*2 <<endl;
    }
    //output
    if(g->getType() == AIG_GATE)
    {
      outfile << g->getID()*2 <<endl;
    }
    //aigGates
    for(int i=0;i<(int)aigList.size();++i)
    {
      outfile << (aigList.at(i)->getID())*2;
      FanList fanInList;
      aigList.at(i)->getFanIn(fanInList);
      for(int j=0;j<2;j++)
      {
          if(fanInList.at(j).inv()==1)
            outfile<<" "<<(fanInList.at(j).gate()->getID())*2 +1;
          else
            outfile<<" "<<(fanInList.at(j).gate()->getID())*2 ;
      }
      outfile<<endl;
    }
    //symbols
    for(int i=0;i<(int)piList.size();++i)
    {
        if(((CirPIGate*)piList.at(i))->getSymbol()!="")
        {
          outfile<<"i"<<i<<" "<<((CirPIGate*)piList.at(i))->getSymbol()<<endl;
        }
    }
    outfile<<"o0 Gate_"<<g->getID()<<endl;
    outfile<<'c'<<endl;
    outfile<<"zyt_sheep"<<endl;


}

void
CirMgr::searchGateDfs(CirGate* gate,GateList& list) const
{
    if(gate->traversed())return;
    GateType type = gate->getType();
    switch(type)
    {
      case UNDEF_GATE:{break;}
      case CONST_GATE:
      {
        list.push_back(gate);break;
      }
      case PI_GATE:
      {
        list.push_back(gate);break;
      }
      case AIG_GATE:
      {
        FanList fanin;
        gate->getFanIn(fanin);
        for(int i=0;i<(int)fanin.size();++i)
        {
          searchGateDfs(fanin.at(i).gate(),list);
        }
        list.push_back(gate);
        break;
      }
      case PO_GATE:
      {
        FanList fanin;
        gate->getFanIn(fanin);
        for(int i=0;i<(int)fanin.size();++i)
        {
          searchGateDfs(fanin.at(i).gate(),list);
        }
        list.push_back(gate);
        break;
      }
      default:
      {
        #ifdef ZIYIMgr
        assert(false);
        #endif
      }
    }
    gate->setToGlobal();
}

