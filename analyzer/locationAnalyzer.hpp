#ifndef LOCATIONAnalyzer_HPP
#define LOCATIONAnalyzer_HPP

#include <vector>
#include <set>
#include <map>
#include <ext/hash_map>
#include <ext/hash_set>

#include "Utils.hpp"

using namespace std;
using namespace __gnu_cxx;


struct Node {
    map<wstring,set<wstring> > names;  //the map value are the context words
    set<wstring> prefNames;
    float likelihood;
    int level;
    vector<struct Node*> subNodes;
    struct Node* superNode;
};

class LocationAnalyzer {
public:
    ~LocationAnalyzer();
    void init( const char* locPath, const char* casePath );
    float match( wstring& str1, wstring& str2 );
    vector<string> analyze( wstring& str );

private:
    void lower( wstring& str );
    void loadCaseConversion( const char* path );
    static bool cover( Node* n1, Node* n2 );
    int readName( string& str, wstring& name, set<wstring>& cxtNames );
    void freeNode( Node* n );    

    // segment name into possible sequence of location names
    vector<wstring> segment( wstring& str );  

    Node* getNode( wstring& str );
    Node* world;
    hash_map<wstring,vector<Node*>,hashWstring,eqWstring> str2nodes;
    hash_map<int,int> up2low;
};

#endif
