/****************************************************************************
  FileName     [ test.cpp ]
  PackageName  [ test ]
  Synopsis     [ Test program for simple database db ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <fstream>
#include <cstdlib>
#include "dbJson.h"

using namespace std;

extern DBJson dbjson;

class CmdParser;
CmdParser* cmdMgr = 0; // for linking purpose

int
main(int argc, char** argv)
{
   if (argc != 2) {  // testdb <jsonfile>
      cerr << "Error: using testdb <jsonfile>!!" << endl;
      exit(-1);
   }

   ifstream inf(argv[1]);

   if (!inf)//!inf: written in <ifstream>
   {
      cerr << "Error: cannot open file \"" << argv[1] << "\"!!\n";
      exit(-1);
   }

   if (dbjson) {
      cout << "Table is resetting..." << endl;
      dbjson.reset();
   }
   if (!(inf >> dbjson)) {
      cerr << "Error in reading JSON file!!" << endl;
      exit(-1);
   }

   cout << "========================" << endl;
   cout << " Print JSON object" << endl;
   cout << "========================" << endl;
   cout << dbjson << endl;
   dbjson.add(DBJsonElem("Abc",123));
   dbjson.add(DBJsonElem("efg",345));
   dbjson.add(DBJsonElem("Abc",233));
   dbjson.add(DBJsonElem("hij",657));
   cout << dbjson << endl;

   size_t index;
   cout<<dbjson.ave()<<endl;
   cout<<dbjson.max(index)<<" "<<index<<endl;
   cout<<dbjson.min(index)<<" "<<index<<endl;

   DBSortKey a;
   dbjson.sort(a);
   cout<<dbjson<<endl;
   DBSortValue b;
   dbjson.sort(b);
   cout<<dbjson<<endl;

   cout<<dbjson.sum()<<endl;



   // TODO
   // Insert what you want to test here by calling DBJson's member functions

   return 0;
}
