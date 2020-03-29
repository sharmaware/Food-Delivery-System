

#include "provided.h"
#include <string>
#include <vector>
#include <functional>
#include "ExpandableHashMap.h"
#include <iostream>
#include <fstream>
using namespace std;

unsigned int hasher(const GeoCoord& g)
{
    return hash<string>()(g.latitudeText + g.longitudeText);
}

class StreetMapImpl
{
public:
    StreetMapImpl();
    ~StreetMapImpl();
    bool load(string mapFile);
    bool getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const;
private:
    ExpandableHashMap<GeoCoord, vector<StreetSegment*> > map;
    vector<StreetSegment> ss;
};

StreetMapImpl::StreetMapImpl()
{
    
}

StreetMapImpl::~StreetMapImpl()
{
}

bool StreetMapImpl::load(string mapFile)
{
    ifstream infile(mapFile);
   
    if(!infile){ return false;}
    
    while(infile){
        string name;
        getline(infile, name);

        if(name == ""){
            break;
        }
     
        int count;
        infile >> count;
        infile.ignore(1000, '\n');
        
        for(int i = 0; i < count; i++){ //for each line, associate the start geocoord to its streetsegment and the end geocoord to its reverse streetsegment
            string startLat, startLong, endLat, endLong;
            infile >> startLat >> startLong >> endLat >> endLong;
            
            infile.ignore(1000, '\n');
            
            GeoCoord B (startLat, startLong);
            GeoCoord E (endLat, endLong);
            
            StreetSegment S (B, E, name);
            StreetSegment reverseS (E, B, name);
              
            ss.push_back(S);
            ss.push_back(reverseS);
        }
    }
    
    for(int i = 0; i < ss.size(); i++){
        vector<StreetSegment*>* vec = map.find(ss[i].start);
        if(vec == nullptr){
            vector<StreetSegment*> newVec;
            newVec.push_back(&(ss[i]));
            map.associate(ss[i].start, newVec);
        }
        else{
            vec->push_back(&(ss[i]));
        }
    }
    
      
    return true;
}

bool StreetMapImpl::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
    const vector<StreetSegment*> *v = map.find(gc); //?
    
    if(v == nullptr){
        return false;
    }
    
    for(auto it = v->begin(); it != v->end(); it++){
        segs.push_back(*(*it));
    }
    return true; 
}

//******************** StreetMap functions ************************************

// These functions simply delegate to StreetMapImpl's functions.
// You probably don't want to change any of this code.

StreetMap::StreetMap()
{
    m_impl = new StreetMapImpl;
}

StreetMap::~StreetMap()
{
    delete m_impl;
}

bool StreetMap::load(string mapFile)
{
    return m_impl->load(mapFile);
}

bool StreetMap::getSegmentsThatStartWith(const GeoCoord& gc, vector<StreetSegment>& segs) const
{
   return m_impl->getSegmentsThatStartWith(gc, segs);
}




