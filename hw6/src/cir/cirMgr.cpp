/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

//#define ZIYIMgr 35
//#define ZIYIError 35
//#define ZIYITime 35
//#define ZIYIErrorSym 35

#ifdef ZIYITime
#include <ctime>
#endif

using namespace std;

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
/*   Static variables and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
//static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;
static string line;

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
              << ": Cannot redefine constant (" << errInt << ")!!" << endl;
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
    lineNo =0;
    getline(myfile,line);
    size_t tmp = line.find_first_not_of(' ');
    if(tmp!=0 && tmp!=string::npos)
    {
        colNo = 0;
        parseError(EXTRA_SPACE);
        return false;
    }
    if(tmp == string::npos)
    {
        errMsg = "aag";
        parseError(MISSING_IDENTIFIER);
        return false;
    }
    if(line.find_first_of(char(9))!=string::npos)
    {
        colNo = line.find_first_of(char(9));
        errInt = 9;
        parseError(ILLEGAL_WSPACE);
        return false;
    }
    string str;
    size_t pos = myStrGetTok(line,str);
    if(str!="aag")
    {
        cerr<<"[ERROR] Line"<<1<<": Illegal identifier \""<<str<<"\"!!"<<endl;
        return false;
    }
    if(pos==string::npos)
    {
       colNo = 3;
       errMsg = "number of variables";
       parseError(MISSING_NUM);
       return false;
    }
    line = line.substr(pos);
    int times=0;
    int header[5];
    colNo = 1;
    while(times<5)
    {
      colNo += (pos-1);
      //cout<<"colNo"<<colNo<<" times: "<<times<<endl;
      if(line[0]!=' ')
      {
        parseError(MISSING_SPACE);
        return false;
      }
      ++colNo;
      //cout<<line<<endl;
      if(line.find_first_not_of(' ',1)!=1 && line.find_first_not_of(' ',1)!=string::npos)
      {
        //cout<<line.find_first_not_of(' ',1)<<endl;
        parseError(EXTRA_SPACE);
        return false;
      }
      pos = myStrGetTok(line,str);
      if(times<4 && pos == string::npos)
      {
        errMsg = "number of variables";
        parseError(MISSING_NUM);
        return false;
      }
      if(times ==4 && pos!=string::npos)
      {
        colNo += (pos-1);
        parseError(MISSING_NEWLINE);
        return false;
      }
      int num;
      if(!myStr2Int(str,num))
      {
        errMsg = "number of "+headerIdentifer(times)+"("+str+")";
        parseError(ILLEGAL_NUM);
        return false;
      }
      header[times] = num;
      //cout<<"header[i]: "<<times<<" "<<num<<endl;
      if(times==4 && pos==string::npos)break;
      line = line.substr(pos);
      ++times;
    }
    if(header[1]+header[4] > header[0])
    {
      errMsg = "variables";
      errInt = header[0];
      parseError(NUM_TOO_SMALL);
      return false;
    }
    if(header[2]!=0)
    {
      cerr<<"[ERROR] Line 1: Illegal latches!!"<<endl;
      return false;
    }

    #ifdef ZIYIMgr
    for(int i=0;i<5;i++) cout<<header[i]<<" ";
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

    #ifdef ZIYIPrintMap
    printMap();
    #endif
    myfile.close();
    return true;
}

string
CirMgr::headerIdentifer(int times) const
{
  switch(times)
  {
    case 0: return "variables";
    case 1: return "PIs";
    case 2: return "latches";
    case 3: return "POs";
    case 4: return "AIGs";
    default:
      assert(false);
  }
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
    lineNo =1;
    for(int i=0;i<header[1];++i)
    {
        unsigned id;
        getline(myfile,line);
        #ifdef ZIYIError
        cout<<"line: "<<line<<endl;
        #endif
        colNo=0;
        //cout<<"what is line"<<(line=="")<<endl;
        if(!getPIPO(myfile,line,"PI",id))
            return false;
        #ifdef ZIYIError
        cout<<"id: "<<id<<endl;
        #endif
        ++lineNo;
        
        id/=2;
        CirGate* PIGate = new CirPIGate(PI_GATE,id ,2+i);
        _gateMap.insert(GatePair (id, PIGate));
        _PIList.push_back(PIGate); 
          
    }
    //for output
    for(int i=0;i<header[3];i++)
    {
        unsigned id;
        getline(myfile,line);
        #ifdef ZIYIError
        cout<<"line: "<<line<<endl;
        #endif
        colNo=0;
        if(!getPIPO(myfile,line,"PO",id))
            return false;
        #ifdef ZIYIError
        cout<<"id: "<<id<<endl;
        #endif
        ++lineNo;
        CirGate* POGate = new CirPOGate(PO_GATE, unsigned(header[0]+1+i), unsigned(header[1]+2+i));
        data.push_back(id);
        _gateMap.insert(GatePair (unsigned(header[0]+1+i), POGate));
        _POList.push_back(POGate);
    }
    //for aig gate
    for(int i=0;i<header[4];i++)
    {
        unsigned literal[3];
        getline(myfile,line);
        if(!getAig(myfile,line,literal))
          return false;
        ++lineNo;
        for(int j=0;j<3;++j)
        {
          data.push_back(literal[j]);
        }
        #ifdef ZIYIError
        cout<<lineNo<<" aig1: "<<literal[0]<<" aig2: "<<literal[1]<<" aig3: "<<literal[2]<<endl;
        #endif
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

bool
CirMgr::getAig(ifstream& myfile,string& line,unsigned* literal )const
{
  string str;
  if(myfile.eof() || line=="")
  {
    errMsg = "AIG";
    parseError(MISSING_DEF);
    return false;
  }
  colNo=0;
  for(int i=0;i<3;++i)
  {
      #ifdef ZIYIError
      cout<<"line: "<<line<<" colNo: "<<colNo<<endl;
      #endif
      if(line.find_first_not_of(' ')!=0)
      {
        parseError(EXTRA_SPACE);
        return false;
      }
      size_t pos = myStrGetTok(line,str);
      colNo+=pos;
      //if(i==0)--colNo;
      int num =0;
      if(!myStr2Int(str,num))
      {
        errMsg = str;
        parseError(ILLEGAL_NUM);
        return false;
      }
      literal[i] = (unsigned)num;
      if(i==0)
      {
        if(literal[0]==1 || literal[0]==0)
        {
          colNo = 0;
          errInt = literal[0];
          parseError(REDEF_CONST);
          return false;
        }
        if((errGate = getGate(literal[0]/2))!=NULL)
        {
          errInt = literal[0];
          errMsg = "AIG";
          #ifdef ZIYIMgr
          errGate->reportGate();
          printMap();
          #endif
          parseError(REDEF_GATE);
          return false;
        }
      }
      if((literal[i]/2)>_maxVar)
      { 
        errInt = literal[i];
        colNo-=pos;
        parseError(MAX_LIT_ID);
        return false;
      }
      if(pos!=string::npos && i==2)
      {
        //colNo = pos;
        parseError(MISSING_NEWLINE);
        return false;
      }
      if(pos==string::npos && i!=2)
      {
        errMsg = "AIG";
        parseError(MISSING_DEF);
        return false;
      }
      ++colNo;
      if(i!=2)
        line = line.substr(pos+1);
  }
  return true;
}

bool 
CirMgr::getPIPO(ifstream& myfile,string& line,const string& gt,unsigned& id)const
{
  string str;
  if(myfile.eof() || line=="")
  {
    errMsg = gt;
    parseError(MISSING_DEF);
    return false;
  }
  if(line.find_first_not_of(' ')!=0)
  {
    parseError(EXTRA_SPACE);
    return false;
  }
  size_t pos = myStrGetTok(line,str);
  int num =0;
  if(!myStr2Int(str,num))
  {
    errMsg = str;
    colNo += (pos-1);
    parseError(ILLEGAL_NUM);
    return false;
  }
  if(pos!=string::npos)
  {
    colNo = pos;
    parseError(MISSING_NEWLINE);
    return false;
  }
  id = (unsigned)num;
  if(gt =="PI")
  {
    if(id==1 || id==0)
    {
      errInt = id;
      parseError(REDEF_CONST);
      return false;
    }
    if(id%2!=0)
    {
      errMsg = "PI";
      errInt = id;
      parseError(CANNOT_INVERTED);
      return false;
    }
  }
  if((id/2)>_maxVar)
  { 
    errInt = id;
    parseError(MAX_LIT_ID);
    return false;
  }
  if(gt=="PI")//PO's id in file is not PO's id in CirMgr
  {
    if((errGate = getGate(id/2))!=NULL)
    {
      errInt = id;
      errMsg = gt;
      #ifdef ZIYIMgr
      errGate->reportGate();
      printMap();
      #endif
      parseError(REDEF_GATE);
      return false;
    }
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
    #ifdef ZIYITime
    clock_t before,after;
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
        fanIn->addFanOut(_POList.at(i));
    }

    #ifdef ZIYIMgr
    cout<<"connecting aig"<<endl;
    #endif

    #ifdef ZIYITime
    int num=0;
    #endif
    //connects aig and its fanin
    //GateList aigGates;
    //getAigGates(aigGates,data);
    
    for(int i=0;i<header[4];i++)
    {
        #ifdef ZIYITime
        cout<<"connecting one aig"<<(num)<<endl;
        #endif

        unsigned aig[3];
        for(int j=0;j<3;j++)
          aig[j] = data.at( 3*i +j + (int)_POList.size());

        #ifdef ZIYITime
        ++num;
        if(num%1000 ==0)
          before = clock();
        #endif
        //getAigGates(gates,aig);
        //CirGate* aigGate = getGate(aig[0]/2);
        CirGate* aigGate = getGate(aig[0]/2);

        #ifdef ZIYITime
        if(num%1000 ==0)
        {
          after = clock();
          cout<<"time for getting gate: "<<(after-before)<<endl;
        }
        #endif

        #ifdef ZIYIMgr
        assert(aigGate!=NULL);
        #endif

        for(int j=0;j<2;j++)
        {
          #ifdef ZIYITime
          if(num%1000 ==0)
            before = clock();
          #endif

          //CirGate* fanIn = getGate(aig[j+1]/2);
          CirGate* fanIn = getGate(aig[j+1]/2);

          #ifdef ZIYITime
          if(num%1000 ==0)
          {
            after = clock();
            cout<<"time for getting gate for fanin: "<<(after-before)<<endl;
          }
          #endif

          if(fanIn==NULL)
          {
              #ifdef ZIYIMgr
              cout<<"new undef gate when connecting aig"<<endl;
              #endif
              fanIn = new CirUndefGate(UNDEF_GATE, aig[j+1]/2,0);
              _gateMap.insert(GatePair( aig[j+1]/2 ,fanIn));
              ++_undefNum;
              //_undefList.push_back(fanIn);
          }

          #ifdef ZIYITime
          if(num%1000 ==0)
            before = clock();
          #endif

          aigGate->addFanIn(fanIn, (bool)(aig[j+1]%2) );
          fanIn->addFanOut(aigGate);

          #ifdef ZIYITime
          if(num%1000 ==0)
          {
            after = clock();
            cout<<"time for setting address for fanin: "<<(after-before)<<endl;
          }
          #endif

        }
    }
}

bool
CirMgr::setSymbols(ifstream& myfile)
{
    
    int i=0;
    bool oneNewLine = false;
    #ifdef ZIYIMgr
    cout<<"setting symbols"<<endl;
    cout<<"myfile reaches end? "<<myfile.eof()<<endl;
    #endif
    while(!myfile.eof()) 
    {
        ++i;
        string str;
        getline(myfile,str);
        colNo=0;
        if(str == "" && oneNewLine ==false)
        {
          oneNewLine = true;
          continue;
        }
        if(str == "" && oneNewLine == true)
        {
          parseError(ILLEGAL_SYMBOL_TYPE);
          return false;
        }
        if(str == "c")break;
        //if(str == "")continue;
        if(str.find_first_not_of(' ')!=0)
        {
          parseError(EXTRA_SPACE);
          return false;
        }
        if(str.find_first_of(char(9))!=string::npos)
        {
            colNo = str.find_first_of(char(9));
            errInt = 9;
            parseError(ILLEGAL_WSPACE);
            return false;
        }
        size_t pos = str.find_first_of(' ');
        if(pos == string::npos)
        {
          parseError(MISSING_SPACE);
          return false;
        }
        #ifdef ZIYIErrorSym
        cout<<i<<": str: "<<str<<endl;
        assert(pos!=string::npos);
        #endif
        string symbol[2];
        symbol[0] = str.substr(0,pos);//i1
        symbol[1] = str.substr(pos+1);//name
        #ifdef ZIYIErrorSym
        cout<<"symbols: "<<symbol[0]<<" and "<<symbol[1]<<endl;
        assert(symbol[0]!="");
        #endif
        if(symbol[1]=="")
        {
          errMsg = "symbolic name";
          parseError(MISSING_IDENTIFIER);
          return false;
        }
        if(symbol[0][0]!= 'i' && symbol[0][0]!='o')
        {
          errMsg = symbol[0][0];
          parseError(ILLEGAL_SYMBOL_TYPE);
          return false;
        }
        if(symbol[0].substr(1)=="")//maybe cannot call substr
        {
          errMsg = "symbol index()";
          parseError(ILLEGAL_NUM);
          return false;
        }
        int num;
        if(!myStr2Int(symbol[0].substr(1),num))
        {
          errMsg = "symbol index("+(symbol[0].substr(1))+")";
          parseError(ILLEGAL_NUM);
          return false;
        }
        if(symbol[0][0]=='i')
        {
          if(num>=(int)_PIList.size())
          {
            errMsg = "PI index";
            errInt = num;
            parseError(NUM_TOO_BIG);
            return false;
          }
          if(((CirPIGate*)_PIList.at(num))->getSymbol() != "")
          {
            errMsg = symbol[0][0];
            errInt = num;
            parseError(REDEF_SYMBOLIC_NAME);
            return false;
          }
          ((CirPIGate*)_PIList.at(num))->setSymbol(symbol[1]);
        }
        else if(symbol[0][0]=='o')
        {
          if(num>=(int)_POList.size())
          {
            errMsg = "PO index";
            errInt = num;
            parseError(NUM_TOO_BIG);
            return false;
          }
          if(((CirPOGate*)_POList.at(num))->getSymbol() != "")
          {
            errMsg = symbol[0][0];
            errInt = num;
            parseError(REDEF_SYMBOLIC_NAME);
            return false;
          }
          ((CirPOGate*)_POList.at(num))->setSymbol(symbol[1]);
        }
        ++lineNo;
    }
    return true;
}

void
CirMgr::printMap()const
{
  cout<<"printing Map"<<endl;
  index = 0;
  for(auto x: _gateMap)
  {
      cout<<x.first<<" "<<x.second<<" "
          <<(x.second->getTypeStr())<<" "
          <<(x.second->getLineNo())<<endl;
      //(printGateNet(x.second));
  }
}

// return '0' if "gid" corresponds to an undefined gate.
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
CirMgr::printNetlist() const
{
    cout<<endl;
    index = 0;
    ++CirGate::globalRef;
    for(int i=0;i<(int)_POList.size();++i)
    {
        dfsNet(_POList.at(i));
    }
}

void
CirMgr::dfsNet(CirGate* gate) const
{
    if( gate->traversed() == true)return;
    GateType type = gate->getType();
    switch(type)
    {
        case PI_GATE:
        {
            printGateNet(gate);
            break;
        }
        case PO_GATE:
        {
            pair<CirGate*,bool> fanIn = ((CirPOGate*)gate)->getFanIn();
            #ifdef ZIYIMgr
            assert(fanIn.first!=NULL);
            #endif
            dfsNet(fanIn.first);
            printGateNet(gate);
            break;
        }
        case AIG_GATE:
        {
            CirGate** fanInList;
            bool* fanInInvList;
            ((CirAigGate*)gate)->getFanIn(fanInList,fanInInvList);
            for(int i=0;i<2;i++)
            {
                dfsNet(fanInList[i]);
            }
            printGateNet(gate);
            _aigList.push_back(gate);
            break;
        }
        case CONST_GATE:
        {
            printGateNet(gate);
            break;
        }
        case UNDEF_GATE:
        {
            break;
        }
        default:
            #ifdef ZIYIMgr
            assert(false);
            #endif
            break;
    }
    gate->setToGlobal();
}

void
CirMgr::printGateNet(CirGate* gate) const
{
    GateType type = gate->getType();
    if(type == UNDEF_GATE) return;
    cout<<"["<<index<<"] "
        <<setw(4)<<left
        <<(gate->getTypeStr())
        <<(gate->getID());
    if(type == PI_GATE)
    {
        string sym = ((CirPIGate*)gate)->getSymbol();
        if(sym!="")
        {
          cout<<" ("<<sym<<")";
        }
    }
    else if (type == PO_GATE)
    {
        pair<CirGate*,bool> fanIn = ((CirPOGate*)gate)->getFanIn();
        #ifdef ZIYIMgr
        assert(fanIn.first!=NULL);
        #endif
        cout<<" ";
        if( (fanIn.first)->getType() == UNDEF_GATE )cout<<"*";
        if(fanIn.second == true) cout<<"!";
        cout<<(fanIn.first->getID());
        string sym = ((CirPOGate*)gate)->getSymbol();
        if(sym!="")
        {
          cout<<" ("<<sym<<")";
        }
    }
    else if(type == AIG_GATE)
    {
        CirGate** fanInList;
        bool* fanInInvList;
        ((CirAigGate*)gate)->getFanIn(fanInList,fanInInvList);
        
        for(int i=0;i<2;++i)
        {
            cout<<" ";
            if(fanInList[i]->getType() == UNDEF_GATE)cout<<"*";
            if(fanInInvList[i] == true )cout<<"!";
            cout<<(fanInList[i]->getID());
        }
    }
    cout<<endl;
    ++index;
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
                const GateList& fanOutList = ((CirUndefGate*)x.second)->getFanOut();
                for(int i=0;i<(int)fanOutList.size();++i)
                {
                    if(fanOutList.at(i)->traversed())continue;
                    cout <<" "<< fanOutList.at(i)->getID();
                    fanOutList.at(i)->setToGlobal();
                }
            }
        }
        cout<<endl;
    }
    bool existUnused = false;
    for(auto x:_gateMap)//defined but not used
    {
        if(x.second->getType() == AIG_GATE)
        {
            const GateList fanOutList = ((CirAigGate*)x.second)->getFanOut();
            if(fanOutList.size()==0) 
            {
              if(!existUnused)cout<<"Gates defined but not used  :";
              cout <<" "<< (x.second->getID());
              existUnused = true;
            }
        }
        if(x.second->getType() == PI_GATE) 
        {
            const GateList fanOutList = ((CirPIGate*)x.second)->getFanOut();
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
CirMgr::writeAag(ostream& outfile) const
{
    if(_aigList.empty()) aigListDfs();
    outfile <<"aag "<<_maxVar<<" "<<_PIList.size()<<" "<<0<<" "
            <<_POList.size()<<" "<<_aigList.size()<<endl;
    //input
    for(int i=0;i<(int)_PIList.size();++i)
    {
      outfile<< (_PIList.at(i)->getID())*2 <<endl;
    }
    //output
    for(int i=0;i<(int)_POList.size();++i)
    {
      pair<CirGate*,bool> fanin = ((CirPOGate*)_POList.at(i))->getFanIn();
      if(fanin.second == 1)
        outfile<< (fanin.first->getID())*2 +1 <<endl;
      else
        outfile<< (fanin.first->getID())*2<<endl;
    }
    //aigGates
    for(int i=0;i<(int)_aigList.size();++i)
    {
      CirGate** fanInList;
      bool* fanInInvList;
      outfile << (_aigList.at(i)->getID())*2;
      ((CirAigGate*)_aigList.at(i))->getFanIn(fanInList,fanInInvList);
      for(int j=0;j<2;j++)
      {
          if(fanInInvList[j]==1)
            outfile<<" "<<(fanInList[j]->getID())*2 +1;
          else
            outfile<<" "<<(fanInList[j]->getID())*2 ;
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
CirMgr::aigListDfs()const
{
    ++CirGate::globalRef;
    //cout<<"this is not written yet"<<endl;
    for(int i=0;i<(int)_POList.size();++i)
    {
        dfsForAigGate(_POList.at(i));
    }
}

void
CirMgr::dfsForAigGate(CirGate* gate)const
{ 
    if( gate->traversed() == true)return;
    GateType type = gate->getType();
    switch(type)
    {
        case PO_GATE:
        {
            pair<CirGate*,bool> fanIn = ((CirPOGate*)gate)->getFanIn();
            #ifdef ZIYIMgr
            assert(fanIn.first!=NULL);
            #endif
            dfsForAigGate(fanIn.first);
            break;
        }
        case AIG_GATE:
        {
            CirGate** fanInList;
            bool* fanInInvList;
            ((CirAigGate*)gate)->getFanIn(fanInList,fanInInvList);
            for(int i=0;i<2;i++)
            {
                dfsForAigGate(fanInList[i]);
            }
            _aigList.push_back(gate);
            break;
        }
        default:  
          break;
    }
    gate->setToGlobal();
}

