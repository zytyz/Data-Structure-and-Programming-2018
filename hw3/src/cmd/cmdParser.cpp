/****************************************************************************
  FileName     [ cmdParser.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command parsing member functions for class CmdParser ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include "util.h"
#include "cmdParser.h"

#define OVERFLOW 252

using namespace std;

//----------------------------------------------------------------------
//    External funcitons
//----------------------------------------------------------------------
void mybeep();


//----------------------------------------------------------------------
//    Member Function for class cmdParser
//----------------------------------------------------------------------
// return false if file cannot be opened
// Please refer to the comments in "DofileCmd::exec", cmdCommon.cpp
bool
CmdParser::openDofile(const string& dof)
{
   // TODO...
   //cout<<"opening a new file..."<<dof<<endl;
   //cout<<_dofile<<endl;
   _dofile = new ifstream(dof.c_str());
   //cout<<_dofile<<endl;
   //cout<<_dofileStack.size()<<endl;
   if( _dofile->fail() || _dofileStack.size()>OVERFLOW )
   {
      if(_dofileStack.size()>OVERFLOW)
      {
         cerr<<"Error: dofile stack overflow ("<<OVERFLOW<<")"<<endl;
      }
      delete _dofile;
      if(!_dofileStack.empty())
      {
         _dofile=_dofileStack.top();
      }
      else _dofile=0;
      return false;
   }
   _dofileStack.push(_dofile);
   return true;
}

// Must make sure _dofile != 0
void
CmdParser::closeDofile()
{
   assert(_dofile != 0);
   // TODO...
   delete _dofile;
   _dofileStack.pop();
   if(!_dofileStack.empty())
   {
      _dofile=_dofileStack.top();
   }
   else
      _dofile=0;
   
}

// Return false if registration fails
bool
CmdParser::regCmd(const string& cmd, unsigned nCmp, CmdExec* e)
{
   // Make sure cmd hasn't been registered and won't cause ambiguity
   string str = cmd;
   unsigned s = str.size();
   if (s < nCmp) return false;
   while (true) {
      if (getCmd(str)) return false;
      if (s == nCmp) break;
      str.resize(--s);//ziyi: as str becomes shorter, check if the shortened cmd is registered
   }//ziyi: after this str will be the mandcmd

   // Change the first nCmp characters to upper case to facilitate
   //    case-insensitive comparison later.
   // The strings stored in _cmdMap are all upper case
   //
   assert(str.size() == nCmp);  // str is now mandCmd
   string& mandCmd = str;
   for (unsigned i = 0; i < nCmp; ++i)
      mandCmd[i] = toupper(mandCmd[i]);
   string optCmd = cmd.substr(nCmp);//ziyi: the second part of a cmd
   assert(e != 0);
   e->setOptCmd(optCmd);//ziyi: save it as a private member of e

   // insert (mandCmd, e) to _cmdMap; return false if insertion fails.
   return (_cmdMap.insert(CmdRegPair(mandCmd, e))).second;
   //ziyi: the above line returns true iff the Pair is successfully inserted
}

// Return false on "quit" or if excetion happens
CmdExecStatus
CmdParser::execOneCmd()
{
   bool newCmd = false;
   fileCheck:
   if (_dofile != 0)
   {
      if(!_dofile->eof())
      {
         newCmd = readCmd(*_dofile);
      }
      else//has reached the end of _dofile 
      {
         closeDofile();
         //need to check if _dofile now points to another file
         //may be an old file in _dofileStack
         //should be recursive since end of do1 may be "dof do2"
         goto fileCheck;
      }
   }
   else
      newCmd = readCmd(cin);

   // execute the command
   if (newCmd) {
      string option;
      CmdExec* e = parseCmd(option);
      if (e != 0)
         return e->exec(option);
   }
   return CMD_EXEC_NOP;
}




// For each CmdExec* in _cmdMap, call its "help()" to print out the help msg.
// Print an endl at the end.
void
CmdParser::printHelps() const
{
   // TODO...
   //cout<<"in print help"<<endl;
   for(auto& x:_cmdMap)
   {
      x.second->help();
   }
   cout<<endl;
}

void
CmdParser::printHistory(int nPrint) const
{
   assert(_tempCmdStored == false);
   if (_history.empty()) {
      cout << "Empty command history!!" << endl;
      return;
   }
   int s = _history.size();
   if ((nPrint < 0) || (nPrint > s))
      nPrint = s;
   for (int i = s - nPrint; i < s; ++i)
      cout << "   " << i << ": " << _history[i] << endl;
}


//
// Parse the command from _history.back();
// Let string str = _history.back();
//
// 1. Read the command string (may contain multiple words) from the leading
//    part of str (i.e. the first word) and retrive the corresponding
//    CmdExec* from _cmdMap
//    ==> If command not found, print to cerr the following message:
//        Illegal command!! "(string cmdName)"
//    ==> return it at the end.
// 2. Call getCmd(cmd) to retrieve command from _cmdMap.
//    "cmd" is the first word of "str".
// 3. Get the command options from the trailing part of str (i.e. second
//    words and beyond) and store them in "option"
//
CmdExec*
CmdParser::parseCmd(string& option)
{
   assert(_tempCmdStored == false);
   assert(!_history.empty());
   string str = _history.back();
   // TODO...
   assert(str[0] != 0 && str[0] != ' ');
   string cmd;
   size_t pos = myStrGetTok(str,cmd);//cmd is the first part of str
   CmdExec* e = getCmd(cmd);
   if(e!=0)//cmd is found in _cmdMap
   {
      if(pos!=string::npos)
      {
         option=str.substr(pos+1);//pos has to plus one to skip one ' ' char
      }
      else
      {
         option="";
      }
      return e;
   }
   else
   {
      cerr<<"Illegal command!! ("<<str<<")"<<endl;
      return NULL;
   }
}

// Remove this function for TODO...
//
// This function is called by pressing 'Tab'.
// It is to list the partially matched commands.
// "str" is the partial string before current cursor position. It can be 
// a null string, or begin with ' '. The beginning ' ' will be ignored.
//
// Several possibilities after pressing 'Tab'
// (Let $ be the cursor position)
// 1. LIST ALL COMMANDS
//    --- 1.1 ---
//    [Before] Null cmd
//    cmd> $
//    --- 1.2 ---
//    [Before] Cmd with ' ' only
//    cmd>     $
//    [After Tab]
//    ==> List all the commands, each command is printed out by:
//           cout << setw(12) << left << cmd;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location (including ' ')
//
// 2. LIST ALL PARTIALLY MATCHED COMMANDS
//    --- 2.1 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$                   // partially matched
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$                   // and then re-print the partial command
//    --- 2.2 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$llo                // partially matched with trailing characters
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$llo                // and then re-print the partial command
//
// 3. LIST THE SINGLY MATCHED COMMAND
//    ==> In either of the following cases, print out cmd + ' '
//    ==> and reset _tabPressCount to 0
//    --- 3.1 ---
//    [Before] partially matched (single match)
//    cmd> he$
//    [After Tab]
//    cmd> heLp $               // auto completed with a space inserted
//    --- 3.2 ---
//    [Before] partially matched with trailing characters (single match)
//    cmd> he$ahah
//    [After Tab]
//    cmd> heLp $ahaha
//    ==> Automatically complete on the same line
//    ==> The auto-expanded part follow the strings stored in cmd map and
//        cmd->_optCmd. Insert a space after "heLp"
//    --- 3.3 ---
//    [Before] fully matched (cursor right behind cmd)
//    cmd> hElP$sdf
//    [After Tab]
//    cmd> hElP $sdf            // a space character is inserted
//
// 4. NO MATCH IN FITST WORD
//    --- 4.1 ---
//    [Before] No match
//    cmd> hek$
//    [After Tab]
//    ==> Beep and stay in the same location
//
// 5. FIRST WORD ALREADY MATCHED ON FIRST TAB PRESSING
//    --- 5.1 ---
//    [Before] Already matched on first tab pressing
//    cmd> help asd$gh
//    [After] Print out the usage for the already matched command
//    Usage: HELp [(string cmd)]
//    cmd> help asd$gh
//
// 6. FIRST WORD ALREADY MATCHED ON SECOND AND LATER TAB PRESSING
//    ==> Note: command usage has been printed under first tab press
//    ==> Check the word the cursor is at; get the prefix before the cursor
//    ==> So, this is to list the file names under current directory that
//        match the prefix
//    ==> List all the matched file names alphabetically by:
//           cout << setw(16) << left << fileName;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location
//    --- 6.1 ---
//    [Before] if prefix is empty, print all the file names
//    cmd> help $sdfgh
//    [After]
//    .               ..              Homework_3.docx Homework_3.pdf  Makefile
//    MustExist.txt   MustRemove.txt  bin             dofiles         include
//    lib             mydb            ref             src             testdb
//    cmd> help $sdfgh
//    --- 6.2 ---
//    [Before] with a prefix and with mutiple matched files
//    cmd> help M$Donald
//    [After]
//    Makefile        MustExist.txt   MustRemove.txt
//    cmd> help M$Donald
//    --- 6.3 ---
//    [Before] with a prefix and with mutiple matched files,
//             and these matched files have a common prefix
//    cmd> help Mu$k
//    [After]
//    ==> auto insert the common prefix and make a beep sound
//    ==> DO NOT print the matched files
//    cmd> help Must$k
//    --- 6.4 ---
//    [Before] with a prefix and with a singly matched file
//    cmd> help MustE$aa
//    [After] insert the remaining of the matched file name followed by a ' '
//    cmd> help MustExist.txt $aa
//    --- 6.5 ---
//    [Before] with a prefix and NO matched file
//    cmd> help Ye$kk
//    [After] beep and stay in the same location
//    cmd> help Ye$kk
//
//    [Note] The counting of tab press is reset after "newline" is entered.
//
// 7. FIRST WORD NO MATCH
//    --- 7.1 ---
//    [Before] Cursor NOT on the first word and NOT matched command
//    cmd> he haha$kk
//    [After Tab]
//    ==> Beep and stay in the same location

void 
CmdParser::listCmd(const string& str)
{
   // TODO...
   //called in readCmd(written in CmdReader.o):
   //if TAB_KEY is pressed, then it will call listCmd(), but will not end the readCmd function
   //readCmd ends when NEW_LINE_KEY is pressed
   //_tabPressCount++ whenever tab is pressed
   //cout<<"tab count "<<_tabPressCount<<endl;
   //string p="";
   /*vector<string> files;
   int x = listDir(files,p);
   assert(x==0);*/
   //cout<<"files size: "<<files.size()<<endl;
   string line;//string before cursor
   if(!strBeforeCursor(line)) line="";
   if(onlySpace(line))
   {
      printCmd();
      reprintCmd();
      _tabPressCount=0;//tab press count doesn't matter, so back to 0
      return;
   }
   string cmd;
   myStrGetTok(line,cmd);
   assert(cmd!="");
   if(!spaceAfterTok(line))//has no space after first token
   {
      vector<string> matchCmds = cmdMatchPart(cmd);
      /*for(int i=0;i<(int)matchCmds.size();i++)
      {
         cout<<matchCmds.at(i)<<endl;
      }*/
      if(matchCmds.size()!=0)
      {
         if(matchCmds.size()!=1)
         {
            cout<<endl;
            for(int i=0;i<(int)matchCmds.size();i++)
            {
               cout << setw(12) << left << matchCmds.at(i);
               if(i%5==4 && i!=(int)matchCmds.size()-1)cout<<endl;
            }
            reprintCmd();
            _tabPressCount=0;
            return;
         }
         else if(matchCmds.size()==1)//only one matches
         {
            string fullCmd = matchCmds.at(0);
            assert(cmd.length() <= fullCmd.length());
            for(int i=cmd.length(); i< (int)fullCmd.length();i++)
            {
               insertChar(fullCmd[i]);
            }
            insertChar(' ');
            _tabPressCount=0;
            return;
         }
      }
      else
      {
         mybeep();
         _tabPressCount=0;
         return;
      }      
   }
   else//has space after the first token
   {
      CmdExec* e=getCmd(cmd);
      if(e!=0)//matches a cmd (5. ,6.)
      {
         if(_tabPressCount==1)//tab pressed first (5.)
         {
            cout<<endl;
            e->usage(cout);
            reprintCmd();
            return;
         }
         else//tab is pressed the second time (6.)
         {
            assert(_tabPressCount==2);
            vector<string> files;
            string specPre = findSpecPrefix(line);
            int x=listDir(files,specPre,".");
            //printFiles(files);
            //delete .DS_Store
            /*for(vector<string>::iterator it=files.begin();it!=files.end();it++)
            {
               if((*it)==".DS_Store")
               {
                  files.erase(it);
               }
            }*/
            assert(x==0);
            if(specPre=="")//(6.1)
            {
               //cout<<"specPre is nothing"<<endl;
               string filePre=findFilePrefix(files);//no specPre, find prefix of all files
               //cout<<"filePre: "<<filePre;
               assert(files.size()!=0);//file size can't be 0 bc the .exe itself is counted
               if(files.size()==1)//only one file int the directory (6.1.3)
               {
                  //cout<<"in here";
                  insertStr(files.at(0));
                  insertChar(' ');
                  _tabPressCount--;
                  return;
               }
               if(filePre=="")//(6.1.1)
               {
                  //every file should be printed
                  printFiles(files);
                  reprintCmd();
                  _tabPressCount--;
                  return;
               }
               else //there is a common file prefix (6.1.2)
               {
                  vector<string> matchFiles;
                  findMatchFiles(files, matchFiles,filePre);
                  assert(matchFiles.size()!=1);//you need to have more than one file to have a file common prefix
                  //insert prefix
                  insertStr(filePre);//there is no specPre, so just insert string
                  _tabPressCount--;
                  return;
               }
            }
            else//specPre!="", there is something before the cursor (6.2, 6.3)
            {
               vector<string> filesWithSpec;

               
               vector<string> matchFiles;//files that have a common prefix in filesWithSpec
               findMatchFiles(files,filesWithSpec,specPre);//filter the files that do not start with specPre
               string filePre=findFilePrefix(filesWithSpec,specPre);//find the common prefix for the remaining files
               if(filesWithSpec.size()==1)//only one file that matches the specPre (6.4)
               {
                  insertStr(filesWithSpec.at(0),specPre.size());
                  insertChar(' ');
                  _tabPressCount--;
                  return;
               }
               if(filesWithSpec.size()==0)//no files match the specPre (6.5)
               {
                  mybeep();
                  _tabPressCount--;
                  return;
               }
               findMatchFiles(filesWithSpec,matchFiles,filePre);
               assert(filePre.length() >= specPre.length());//filePre should be at least longer than specPre
               assert(matchFiles.size() <= filesWithSpec.size());
               
               if(filePre==specPre)//print out the possible files (6.2)
               {
                  assert(matchFiles.size() == filesWithSpec.size());
                  printFiles(matchFiles);
                  reprintCmd();
                  _tabPressCount--;
                  return;
               }
               else //print out filePre and beep (6.3)
               {
                  //assert(matchFiles.size() < filesWithSpec.size());
                  mybeep();
                  insertStr(filePre,specPre.size());
                  _tabPressCount--;
                  return;
               }
            }
            _tabPressCount--;
            return;

         }
      }
      else//7.
      {
         mybeep();
         _tabPressCount--;
         return;
      }

   }

   
}

void
CmdParser::printFiles(const vector<string>& files)
{
   cout<<endl;
   for(int i=0;i<(int)files.size();i++)
   {
      cout << setw(16) << left << files.at(i);
      if( i%5==4 ) cout<<endl;
   }
}

void
CmdParser::insertStr(string& str,int len)
{
   for(int i=len;i<(int)str.size();i++)
   {
      insertChar(str[i]);
   }
}

void
CmdParser::findMatchFiles(vector<string>& files,vector<string>& matchFiles,string specPre)
{
   assert(specPre!="");
   //choose the files with the specPre and the pre
   for(int i=0;i<(int)files.size();i++)
   {
      size_t pos = files.at(i).find(specPre);//need to be modified bc specPre may be in the middle of the files
      if(pos==0) matchFiles.push_back(files.at(i));
   }
}

string 
CmdParser::findFilePrefix(vector<string> files ,string specPre)//pass by value, bc shouldn't modify files
{
   //specPre: specified prefix
   //const vector<string> filesMod=files;//copy files to filesMod
   if(files.size()<2)return "";
   if(files.size()==2)
     return findCommonPrefix(files.at(0),files.at(1),specPre);
   string tmpPre = findCommonPrefix(files.at(0),files.at(1),specPre);
   for(int i=3;i<(int)files.size();i++)
   {
      tmpPre = findCommonPrefix(files.at(i),tmpPre,specPre);
   }
   return tmpPre;
}

string
CmdParser::findCommonPrefix(string& s1,string& s2,string specPre)
{
   //specPre: specified prefix
   string commonPre=specPre;
   int preLen=specPre.length();
   int l1=s1.length();
   int l2=s2.length();
   if(preLen > l1 || preLen > l2)
   {
      return "";
   }
   for(int i=0;i<preLen;i++)
   {
      if(!((s1[i]==s2[i])&&(s1[i]==specPre[i])))return "";
   }
   for(int i=preLen;i<l1 && i<l2 ;i++)
   {
      if(s1[i]!=s2[i])break;
      commonPre.push_back(s1[i]);
   }
   return commonPre;
}

string
CmdParser::findSpecPrefix(string& line)
{
   if(line[line.length()-1]==' ')return "";//last char in line is space
   size_t pos = line.find_last_of(' ');
   assert(pos!=string::npos);
   return line.substr(pos+1);
}

vector<string>
CmdParser::cmdMatchPart(string cmd)//pass by value so it won't modify cmd
{
   vector<string> matchCmds;
   map<string,int> fullCmds;//full cmds in lower cases, int is the length of mandcmd
   cmd = strChangeCase(cmd,"lower");//convert cmd to lower cases
   for(auto&x: _cmdMap)
   {
      string str=strChangeCase(x.first,"lower")+x.second->getOptCmd();
      fullCmds.insert(pair<string,int>(str,x.first.length()));
   }
   for(auto& x: fullCmds)
   {
      size_t pos = x.first.find(cmd);
      if(pos==0)//cmd should be a prefix of x
      {
         string upperManCmd = strChangeCase(x.first.substr(0,x.second),"upper");
         string lowerOptCmd = x.first.substr(x.second);
         matchCmds.push_back(upperManCmd + lowerOptCmd);
         //matchCmds is case sensitive
      }
   }
   return matchCmds;
}

bool
CmdParser::spaceAfterTok(string& line)
{
   size_t begin = line.find_first_not_of(' ');
   if(begin==string::npos)return false;//no space at all
   size_t space = line.find_first_of(' ',begin);
   if(space==string::npos)return false;
   return true;
}

bool 
CmdParser::strBeforeCursor(string& str) //return false if nothing is before cursor
{
   if(_readBuf == _readBufPtr)return false;
   for(char* ptr=_readBuf ; ptr<_readBufPtr ; ptr++)
   {
      str.push_back(*ptr);
   }
   return true;
}

bool
CmdParser::onlySpace(string& line)
{
   if(line=="")return true;
   size_t pos=line.find_first_not_of(' ');
   if(pos==string::npos)return true;
   else return false;
}

void 
CmdParser::printCmd() const
{
   cout<<endl;
   vector<string> allCmds;
   for(auto& x:_cmdMap)
   {
      string fullCmd = x.first + x.second->getOptCmd();
      allCmds.push_back(fullCmd);
   }
   for(int i=0;i<(int)allCmds.size();i++)
   {
      cout << setw(12) << left << allCmds.at(i);
      if(i%5 == 4)cout<<endl;
   }
}

// cmd is a copy of the original input
//
// return the corresponding CmdExec* if "cmd" matches any command in _cmdMap
// return 0 if not found.
//
// Please note:
// ------------
// 1. The mandatory part of the command string (stored in _cmdMap) must match
// 2. The optional part can be partially omitted.
// 3. All string comparison are "case-insensitive".
//
CmdExec*
CmdParser::getCmd(string cmd)
{
   CmdExec* e = 0;
   // TODO...
   for(auto& x:_cmdMap)//traverse _cmdMap
   {
      string fullCmd= x.first + (x.second)->getOptCmd();//ex: "HIStory"
      if(myStrNCmp(fullCmd, cmd, x.first.size())==0)//x.first: mandantory cmd
      {
         //==0 means there is a same cmd aleady stored in _cmdMap
         e=x.second;
         break;
      }
   }
   return e;
}

/* //this part is provided in CmdReader.o
bool
CmdParser::readCmd(istream& istr)//similar function in hw2, readcmd until enter is pressed
{
   bool newcmd=false;
   resetBufAndPrintPrompt();
   while (1) {
      ParseChar pch = getChar(istr);//ziyi: get the character we got from cin (data type is ParseChar)
      if (pch == INPUT_END_KEY) break;
      switch (pch) 
      {
         case LINE_BEGIN_KEY :
         case HOME_KEY       : moveBufPtr(_readBuf); break;
         case LINE_END_KEY   :
         case END_KEY        : moveBufPtr(_readBufEnd); break;
         case BACK_SPACE_KEY : 
         {
            if(moveBufPtr(_readBufPtr-1))
            {
               deleteChar();
            }
            break;
         }
         case DELETE_KEY     : deleteChar(); break;
         case NEWLINE_KEY    : newcmd = addHistory();
                        cout << char(NEWLINE_KEY);
                        return newcmd;
                        //resetBufAndPrintPrompt(); 
                        //break;
         case ARROW_UP_KEY   : moveToHistory(_historyIdx - 1); break;
         case ARROW_DOWN_KEY : moveToHistory(_historyIdx + 1); break;
         case ARROW_RIGHT_KEY: moveBufPtr(_readBufPtr+1); break;
         case ARROW_LEFT_KEY : moveBufPtr(_readBufPtr-1); break;
         case PG_UP_KEY      : moveToHistory(_historyIdx - PG_OFFSET); break;
         case PG_DOWN_KEY    : moveToHistory(_historyIdx + PG_OFFSET); break;
         case TAB_KEY        : 
         {
            int num=(int)((_readBufPtr- _readBuf)%TAB_POSITION);
            insertChar(' ',TAB_POSITION-num);
            break;
         }
         case INSERT_KEY     : // not yet supported; fall through to UNDEFINE
         case UNDEFINED_KEY:   mybeep(); break;
         default:  // printable character
            insertChar(char(pch)); break;
      }
   }
}
bool
CmdParser::moveBufPtr(char* const ptr)
{
   if(ptr>_readBufEnd || ptr < _readBuf )
   {
      mybeep();
      return false;
   }
   else
   {
      if(_readBufPtr > ptr)//move cursor to left
      {
         for(int i=0; i<(_readBufPtr - ptr);i++)
         {
            cout<<'\b';
         }
      }
      else if (_readBufPtr < ptr )//move cursor to right
      {
         for(char * it =_readBufPtr;it<ptr;it++)
         {
            cout<<(*it);
         }
      }
      cout.flush();
      _readBufPtr=ptr;
      return true;
   }
}

bool
CmdParser::deleteChar()
{  // TODO...
   if(_readBufPtr == _readBufEnd)
   {
      mybeep();
      return false;
   }
   else
   {
      int x=0;
      for(char * it =_readBufPtr+1; it<=_readBufEnd; it ++)
      {
         cout<<(*it);
         x++;
      }
      cout<<" ";
      for(int i=0;i<x;i++)
      {
         cout<<'\b';
      }
      for(char *it =_readBufPtr;it<=_readBufEnd-1;it++)
      {
         *it=*(it+1);
      }
      _readBufEnd-=1;
   }
   return true;
}

void
CmdParser::insertChar(char ch, int repeat)
{
   // TODO...
   assert(repeat >= 1);
   if(repeat==1)
   {
      cout<<ch;
      int x=0;
      for(char* it= _readBufPtr;it<_readBufEnd;it++)
      {
         cout<<(*it);
         x++;
      }
      for(int i=0;i<x;i++)
      {
         cout<<'\b';
      }
      for(char *it=_readBufEnd; it>= _readBufPtr;it--)
      {
         *(it+1)=(*it);
      }
      *(_readBufPtr)=ch;
      _readBufPtr+=1;
      _readBufEnd+=1;

   }
   else
   {
      for(int i=0;i<repeat;i++)insertChar(ch);
   }
}

void
CmdParser::deleteLine()
{
   // TODO...
   moveBufPtr(_readBuf);
   //cout<<(int)(_readBufEnd-_readBuf);
   int time=(int)(_readBufEnd-_readBuf);
   for(int i=0;i<time;i++)
   {
      deleteChar();
   }
}

void
CmdParser::moveToHistory(int index)
{
      // TODO...
   if( index < _historyIdx)
   {
      if(index<0)index=0;
      if(_historyIdx==0)
      {
         mybeep();
         return;
      }
      else if(_historyIdx==(int)_history.size())
      {
         
         _history.push_back(_readBuf);//store the string even if there's nothing
         //cout<<"change to true";
         _tempCmdStored=true;
      
      }
      _historyIdx=index;
      retrieveHistory();
      //cout<<"index "<<_historyIdx;
   }
   else if( index > _historyIdx)
   {
      if(_tempCmdStored==false)
      {
         //doesn't happen when retrieving history down
         //bc there is always a tmp cmd stored
         //only happens when u press down at the new line
         mybeep();
         return;
      }
      else if(_tempCmdStored==true)
      {
         if(index >= (int)_history.size()) index =(int)_history.size()-1;
         if(index <= (int)_history.size()-2)
         {
            //should consider index instead of _historyIdx in the "if" condition
            //bc we might jump more than 1
            _historyIdx =index;
            retrieveHistory();
            return; 
         }
         else if(index == (int)_history.size()-1)
         {
            if( index == _historyIdx )
            {
               //at the bottom
               mybeep();
               return;
            }
            else
            {
               //going back to the tmp cmd
               //need to delete tmp in history
               _historyIdx=index;
               retrieveHistory();
               _history.pop_back();
               _tempCmdStored=false;
               return;
            }
         }
      }
   }
}

bool
CmdParser::addHistory()
{
   // TODO...
   string add;
   bool history_added=false;
   if(_tempCmdStored)
   {
      _history.pop_back();
      _tempCmdStored=false;
      //cout<<"change to false";
   }
   if(_readBuf == _readBufEnd)
   {
      _historyIdx=(int)_history.size();
      return;
   }
   bool char_exist=false;
   for(char* it = _readBuf; it< _readBufEnd ;it++)
   {
      if((*it)!=' ')
      {
         char_exist=true;
         break;
      }
   }
   if(char_exist==false)//string only contains space characters
   {
      _historyIdx=(int)_history.size();
      return;
   }
   char * begin=_readBuf;
   char *end=_readBufEnd-1;
   for(char* it = _readBuf; it< _readBufEnd ;it++)
   {
      if((*it)!=' ' && (*it)!='\0')
      {
         begin=it;
         break;
      }
   }
   for(char* it = _readBufEnd-1 ; it>= _readBuf ; it--)
   {
      if((*it)!=' ')
      {
         end=it;
         break;
      }
   }
   for(char * it =begin ; it<=end;it++)
   {
      add.push_back(*it);
   }
   if(add!="")
   {
      _history.push_back(add);
      history_added=true;

   }
   _historyIdx=(int)_history.size();
   return history_added;
}

void
CmdParser::retrieveHistory()
{
   deleteLine();
   strcpy(_readBuf, _history[_historyIdx].c_str());
   cout << _readBuf;
   _readBufPtr = _readBufEnd = _readBuf + _history[_historyIdx].size();
}*/

//----------------------------------------------------------------------
//    Member Function for class CmdExec
//----------------------------------------------------------------------
// return false if option contains an token
bool
CmdExec::lexNoOption(const string& option) const
{
   string err;
   myStrGetTok(option, err);
   if (err.size()) {
      errorOption(CMD_OPT_EXTRA, err);
      return false;
   }
   return true;
}

// Return false if error options found
// "optional" = true if the option is optional XD
// "optional": default = true
//
bool
CmdExec::lexSingleOption
(const string& option, string& token, bool optional) const
{
   size_t n = myStrGetTok(option, token);
   if (!optional) 
   {
      if (token.size() == 0) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
   }
   if (n != string::npos) {
      errorOption(CMD_OPT_EXTRA, option.substr(n));
      return false;
   }
   return true;
}

// if nOpts is specified (!= 0), the number of tokens must be exactly = nOpts
// Otherwise, return false.
//
bool
CmdExec::lexOptions
(const string& option, vector<string>& tokens, size_t nOpts) const
{
   string token;
   size_t n = myStrGetTok(option, token);
   while (token.size()) {
      tokens.push_back(token);
      n = myStrGetTok(option, token, n);
   }
   if (nOpts != 0) {
      if (tokens.size() < nOpts) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
      if (tokens.size() > nOpts) {
         errorOption(CMD_OPT_EXTRA, tokens[nOpts]);
         return false;
      }
   }
   return true;
}

CmdExecStatus
CmdExec::errorOption(CmdOptionError err, const string& opt) const
{
   switch (err) {
      case CMD_OPT_MISSING:
         cerr << "Error: Missing option";
         if (opt.size()) cerr << " after (" << opt << ")";
         cerr << "!!" << endl;
      break;
      case CMD_OPT_EXTRA:
         cerr << "Error: Extra option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_ILLEGAL:
         cerr << "Error: Illegal option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_FOPEN_FAIL:
         cerr << "Error: cannot open file \"" << opt << "\"!!" << endl;
      break;
      default:
         cerr << "Error: Unknown option error type!! (" << err << ")" << endl;
      exit(-1);
   }
   return CMD_EXEC_ERROR;
}


string 
CmdParser::strChangeCase(string str,string myCase)
{
    if(myCase=="lower")
    {
      for(int i =0;i<(int)str.length();i++)
      {
      if(isupper(str[i]))
      {
         str[i]=(char)tolower(str[i]);
      }
      }
    }
    else if(myCase == "upper")
    {
      for(int i =0;i<(int)str.length();i++)
      {
      if(islower(str[i]))
      {
         str[i]=(char)toupper(str[i]);
      }
      }
    }
    return str;
    
}

