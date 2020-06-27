/****************************************************************************
  FileName     [ dbJson.cpp ]
  PackageName  [ db ]
  Synopsis     [ Define database Json member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <climits>
#include <limits>
#include <cmath>
#include <cstring>
#include <string>
#include <algorithm>
#include <sstream>
#include "dbJson.h"
#include "util.h"

using namespace std;

/*****************************************/
/*          Global Functions             */
/*****************************************/
ostream&
operator << (ostream& os, const DBJsonElem& j)
{
   os << "\"" << j._key << "\" : " << j._value;
   return os;
}

istream& operator >> (istream& is, DBJson& j)//ziyi: cin>>j, where j is a DBJson
{
    // TODO: to read in data from Json file and store them in a DB 
    // - You can assume the input file is with correct JSON file format
    // - NO NEED to handle error file format
    assert(j._obj.empty());
    j._jsonReadIn=true;
    istream &myfile=is;
    string line;
    vector<string> block;
    
    while (1)
    {
        getline (myfile,line);
        if(line!="{" && line != "}" &&line!="\0")
        {
            block.push_back(line);
        }
        if(line.find("}")!=string::npos)//stopped when } is read so that eofbit is not set 
            break;
    }
    //cout<<myfile.eof()<<endl;
    for(vector<string>::iterator it=block.begin();it!=block.end();it++)
    {
        char *lineptr=new char [(*it).length()+1];
        strcpy(lineptr,(*it).c_str());//convert the line string into a char array
        char *tokenptr;
        tokenptr=strtok(lineptr," :,\t\"\b\r");//get the token of the line
        if(*tokenptr=='{' || *tokenptr=='}' || *tokenptr=='\n')continue;
        string key(tokenptr);//convert the char array into string
        //cout<<"key: "<<key<<endl;
        tokenptr=strtok(NULL," :,\t\"\b\r");
        string elestr(tokenptr);
        stringstream ss(elestr);//convert the char array into int
        int ele;
        ss>>ele;
        //cout<<"ele: "<<ele<<endl;
        DBJsonElem tmp(key,ele);
        j._obj.push_back(tmp);
        delete [] lineptr;
    }
    block.clear();

    return is;
}

ostream& operator << (ostream& os, const DBJson& j)
{
    // TODO
    cout<<'{'<<endl;
    if(!j._obj.empty())
    {
        int i=0;
        for( i=0;i<(int)j._obj.size()-1;i++)
        {
          cout<<"  "<<j._obj[i]<<','<<endl;
        }
        cout<<"  "<<j._obj[i]<<endl;
    }
    cout<<'}'<<endl;
    return os;
}

/**********************************************/
/*   Member Functions for class DBJsonElem    */
/**********************************************/
/*****************************************/
/*   Member Functions for class DBJson   */
/*****************************************/
void
DBJson::reset()
{
    // TODO
    _jsonReadIn=false;
    _obj.clear();

}

// return false if key is repeated
bool
DBJson::add(const DBJsonElem& elm)
{
    // TODO
    for(int i=0;i<(int)_obj.size();i++)
    {
      if(elm.key()==_obj.at(i).key())
      {
        cout<<"Error: Element with key "<<"\""<<elm.key()<<"\""<<" already exists!!"<<endl;
        return false;
      }
    }
    _obj.push_back(elm);
    return true;
}

// return NAN if DBJson is empty
float
DBJson::ave() const
{
    // TODO
    if(_obj.empty())
    {
        //cout<<"Error: The average of the DB is nan."<<endl;
        return numeric_limits<double>::quiet_NaN();
    }
    int sum=0;
    for(int i=0;i<(int)_obj.size();i++)
    {
        sum+=_obj.at(i).value();
    }
    float ave=(double)sum/(double)_obj.size();
    //cout<<"The average of the DB is ";
    //cout<<fixed<<setprecision(2)<<ave<<"."<<endl;
    return ave;
}

// If DBJson is empty, set idx to size() and return INT_MIN
int
DBJson::max(size_t& idx) const
{
    // TODO
    if(_obj.empty())
    {
        idx=_obj.size();
        return INT_MIN;
    }
    int max = _obj.at(0).value();
    idx=0;
    for(int i=0;i<(int)_obj.size();i++)
    {
        if(max < _obj.at(i).value()) 
        {
            max=_obj.at(i).value();
            idx=i;
        }
    }
    return max;
}

// If DBJson is empty, set idx to size() and return INT_MIN
int
DBJson::min(size_t& idx) const
{
    // TODO  
    if(_obj.empty())
    {
        idx=_obj.size();
        return INT_MAX;
    }
    int min = _obj.at(0).value();
    idx=0;
    for(int i=0;i<(int)_obj.size();i++)
    {
        if(min > _obj.at(i).value()) 
        {
            min=_obj.at(i).value();
            idx=i;
        }
    }
    return min;
}

void
DBJson::sort(const DBSortKey& s)
{
   // Sort the data according to the order of columns in 's'
   ::sort(_obj.begin(), _obj.end(), s);
}

void
DBJson::sort(const DBSortValue& s)
{
   // Sort the data according to the order of columns in 's'
   ::sort(_obj.begin(), _obj.end(), s);
}

// return 0 if empty
int
DBJson::sum() const
{
    // TODO
    int s = 0;
    if(!_obj.empty())
    {
        for(int i=0;i<(int)_obj.size();i++)
        {
            s+=_obj.at(i).value();
        }
    }
    return s;
}
