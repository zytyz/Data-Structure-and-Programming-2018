/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include <cstring>
#include "memCmd.h"
#include "memTest.h"
#include "cmdParser.h"
#include "util.h"
#include <cassert>
#include <algorithm>

using namespace std;

extern MemTest mtest;  // defined in memTest.cpp
void getTokens(const string& option,vector<string>& tokens);


bool
initMemCmd()
{
   if (!(cmdMgr->regCmd("MTReset", 3, new MTResetCmd) &&
         cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
         cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
         cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)
      )) {
      cerr << "Registering \"mem\" commands fails... exiting" << endl;
      return false;
   }
   return true;
}


//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
CmdExecStatus
MTResetCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token))
      return CMD_EXEC_ERROR;
   if (token.size()) {
      int b;
      if (!myStr2Int(token, b) || b < int(toSizeT(sizeof(MemTestObj)))) {
         cerr << "Illegal block size (" << token << ")!!" << endl;
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
      }
      #ifdef MEM_MGR_H
      mtest.reset(toSizeT(b));
      #else
      mtest.reset();
      #endif // MEM_MGR_H
   }
   else
      mtest.reset();//no option, doesn't change the block size
   return CMD_EXEC_DONE;
}

void
MTResetCmd::usage(ostream& os) const
{  
   os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void
MTResetCmd::help() const
{  
   cout << setw(15) << left << "MTReset: " 
        << "(memory test) reset memory manager" << endl;
}


//----------------------------------------------------------------------
//    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
//----------------------------------------------------------------------
CmdExecStatus
MTNewCmd::exec(const string& option)
{
    // TODO
    vector<string> rawTokens;
    getTokens(option,rawTokens);//tokens can have any size
    bool forArray=false;
    for(int i=0;i<(int)rawTokens.size()-1;i++)//check if there is "-Array" in tokens
    {
        if(myStrNCmp("-Array",rawTokens.at(i),2)==0)forArray=true;
        //if "-Array" is the last token, forArray is false
    }
    if(!forArray)
    {
        vector<string> tokens;
        if(!CmdExec::lexOptions(option,tokens,1)) 
           return CMD_EXEC_ERROR;

        //token is the one only token in option
        assert(tokens== rawTokens);
        assert(rawTokens.size()==1);
        int num;
        if((!myStr2Int(tokens.at(0),num)) || (num<=0))
        {
            //the only token is not a legal integer
            return CmdExec::errorOption(CMD_OPT_ILLEGAL,tokens.at(0));
            //return CMD_EXEC_ERROR;
        }
        try
        {
          mtest.newObjs((size_t)num); 
        }
        catch(pair<size_t,size_t> p)
        {
          size_t t = p.first;
          size_t _blockSize = p.second;
          cerr  << "Requested memory (" << t << ") is greater than block size"
               << "(" << _blockSize << "). " << "Exception raised...\n";
        }
        //cout<<"num: "<<num<<endl;    
    }
    else//for Array
    {
        vector<string> tokens;
        if(!CmdExec::lexOptions(option,tokens,3))
          return CMD_EXEC_ERROR;
        assert(tokens == rawTokens);
        assert(tokens.size()==3);
        //const vector<string>& tok = tokens;
        int i=0;
        for(i=0;i<(int)tokens.size();i++)
        {
          if(myStrNCmp("-Array",tokens.at(i),2)==0)
            break;
        }
        assert(i<(int)(tokens.size()-1));

        int arraySize, numObjects;
        if(!myStr2Int(tokens.at(i+1),arraySize) || (arraySize<=0))
        {
            //the only token is not a legal integer
            CmdExec::errorOption(CMD_OPT_ILLEGAL,tokens.at(i+1));
            //cout<<"Illegal option!! ("<<(*(it+1))<<")"<<endl;
            return CMD_EXEC_ERROR;
        }
        if(!myStr2Int(tokens.at((i+2)%3),numObjects) || (numObjects<=0))
        {
            CmdExec::errorOption(CMD_OPT_ILLEGAL,tokens.at((i+2)%3));
            return CMD_EXEC_ERROR;
        }
        try
        {
          mtest.newArrs((size_t)numObjects, (size_t) arraySize);
        }
        catch(bad_alloc& ba)
        {
          
          return CMD_EXEC_ERROR;
        }
        
        //cout<<"numObj: "<<numObjects<<endl;
        //cout<<"arraySize: "<<arraySize<<endl;
    }


    // Use try-catch to catch the bad_alloc exception
    return CMD_EXEC_DONE;
}

void
MTNewCmd::usage(ostream& os) const
{  
   os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t arraySize)]\n";
}

void
MTNewCmd::help() const
{  
   cout << setw(15) << left << "MTNew: " 
        << "(memory test) new objects" << endl;
}


//----------------------------------------------------------------------
//    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
//----------------------------------------------------------------------
CmdExecStatus
MTDeleteCmd::exec(const string& option)
{
    // TODO
    
    vector<string> rawTokens;
    getTokens(option,rawTokens);
    bool forArray=false;
    for(int i=0;i<(int)rawTokens.size();i++)//check if there is "-Array" in tokens
    {
        if(myStrNCmp("-Array",rawTokens.at(i),2)==0)forArray=true;
        //"-Array" can be the last token
    }
    if(!forArray)
    {
        vector<string>& tokens=rawTokens;
        
        if (tokens.size() < 2) 
        {
            if(tokens.size()>=1)
            {
                errorOption(CMD_OPT_MISSING, tokens.at((int)(tokens.size()-1)));
            }
            else
            {
                errorOption(CMD_OPT_MISSING, "");
            }
            
            return CMD_EXEC_ERROR;
        }
        else if (tokens.size() > 2) 
        {
            errorOption(CMD_OPT_EXTRA, tokens.at(2));
            return CMD_EXEC_ERROR;
        }
        
        assert(tokens==rawTokens);
        assert(tokens.size()==2);
        if((myStrNCmp("-Index",tokens.at(0),2)!=0) && (myStrNCmp("-Random",tokens.at(0),2)!=0) )
        {
            CmdExec::errorOption(CMD_OPT_ILLEGAL,tokens.at(0));
            return CMD_EXEC_ERROR;
        }
        if(myStrNCmp("-Index",tokens.at(0),2)==0)//for index
        {
            int num;
            if(!myStr2Int(tokens.at(1),num) || (num<0) )
            {
                //the token is not a legal integer
                CmdExec::errorOption(CMD_OPT_ILLEGAL,tokens.at(1));
                return CMD_EXEC_ERROR;
            }
            if(num>=(int)mtest.getObjListSize())
            {
                cerr<<"Size of object list ("<<(int)mtest.getObjListSize()<<") is <= "<<num<<"!!"<<endl;
                CmdExec::errorOption(CMD_OPT_ILLEGAL,tokens.at(1));
                return CMD_EXEC_ERROR;
            }
            //cout<<"Index: "<<num<<endl;
            mtest.deleteObj((size_t)num);

        }
        else if(myStrNCmp("-Random",tokens.at(0),2)==0)//for random
        {
            int num;
            if(!myStr2Int(tokens.at(1),num) || (num<=0) )
            {
                //the token is not a legal integer
                CmdExec::errorOption(CMD_OPT_ILLEGAL,tokens.at(0));
                return CMD_EXEC_ERROR;
            }
            if(mtest.getObjListSize()==0)
            {
                cerr<<"Size of object list is 0!!"<<endl;
                CmdExec::errorOption(CMD_OPT_ILLEGAL,tokens.at(0));
                return CMD_EXEC_ERROR;
            }
            //cout<<"Random: "<<num<<endl;
            //generate num random numbers, and delete that index
            //if the memory is already deleted, ignore and do nothing
            
            for(int i=0;i<num;i++)
            {
               mtest.deleteObj((size_t)(rnGen(mtest.getObjListSize())));
            }
        }
    }
    else //forArray
    {
        vector<string> tokens;
        //cout<<"in ForArray"<<endl;
        if(!lexOptions(option,tokens,3))
            return CMD_EXEC_ERROR;
        assert(tokens == rawTokens);
        int i=0;
        for(i=0;i<(int)tokens.size();i++)
        {
            if(myStrNCmp("-Array",tokens.at(i),2)==0)
                break;
        }
        assert(i!=(int)tokens.size());
        if(myStrNCmp("-Index",tokens.at((i+1)%3),2)!=0 && myStrNCmp("-Random",tokens.at((i+1)%3),2)!=0)
        {
            CmdExec::errorOption(CMD_OPT_ILLEGAL,tokens.at(0));
            return CMD_EXEC_ERROR;
        }
        if(myStrNCmp("-Index",tokens.at((i+1)%3),2)==0)
        {
            int num;
            if(!myStr2Int(tokens.at((i+2)%3),num) || (num<0) )
            {
                //the token is not a legal integer
                CmdExec::errorOption(CMD_OPT_ILLEGAL,tokens.at((i+2)%3));
                return CMD_EXEC_ERROR;
            }
            if(num>=(int)mtest.getArrListSize())
            {
                cerr<<"Size of array list ("<<(int)mtest.getArrListSize()<<") is <= "<<num<<"!!"<<endl;
                CmdExec::errorOption(CMD_OPT_ILLEGAL,tokens.at((i+2)%3));
                return CMD_EXEC_ERROR;
            }
            //cout<<"Index: Array: "<<num<<endl;
            mtest.deleteArr((size_t)num);
        }
        else if(myStrNCmp("-Random",tokens.at((i+1)%3),2)==0)//for random
        {
            int num;
            if(!myStr2Int(tokens.at((i+2)%3),num) || (num<=0) )
            {
                //the token is not a legal integer
                CmdExec::errorOption(CMD_OPT_ILLEGAL,tokens.at((i+2)%3));
                return CMD_EXEC_ERROR;
            }
            if(mtest.getArrListSize()==0)
            {
                cerr<<"Size of array list is 0!!"<<endl;
                CmdExec::errorOption(CMD_OPT_ILLEGAL,tokens.at((i+1)%3));
                return CMD_EXEC_ERROR;
            }
            //cout<<"Random: Array: "<<num<<endl;
            //generate num random numbers, and delete that index
            //if the memory is already deleted, ignore and do nothing
            for(int i=0;i<num;i++)
            {
               mtest.deleteArr((size_t)(rnGen(mtest.getArrListSize())));
            }
        }
    }
    return CMD_EXEC_DONE;
}

void
MTDeleteCmd::usage(ostream& os) const
{  
   os << "Usage: MTDelete <-Index (size_t objId) | "
      << "-Random (size_t numRandId)> [-Array]" << endl;
}

void
MTDeleteCmd::help() const
{  
   cout << setw(15) << left << "MTDelete: " 
        << "(memory test) delete objects" << endl;
}


//----------------------------------------------------------------------
//    MTPrint
//----------------------------------------------------------------------
CmdExecStatus
MTPrintCmd::exec(const string& option)
{
   // check option
   if (option.size())
      return CmdExec::errorOption(CMD_OPT_EXTRA, option);
   mtest.print();

   return CMD_EXEC_DONE;
}

void
MTPrintCmd::usage(ostream& os) const
{  
   os << "Usage: MTPrint" << endl;
}

void
MTPrintCmd::help() const
{  
   cout << setw(15) << left << "MTPrint: " 
        << "(memory test) print memory manager info" << endl;
}

void getTokens(const string& option,vector<string>& tokens)
{
  string token;
  size_t n = myStrGetTok(option, token);
  while (token.size()) 
  {
    tokens.push_back(token);
    n = myStrGetTok(option, token, n);
  }
}


