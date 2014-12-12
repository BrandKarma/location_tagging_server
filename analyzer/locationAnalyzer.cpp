#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <clocale>
#include <cassert>
#include <iostream>
#include <fstream>
#include <ext/hash_map>
#include <ext/hash_set>
#include <vector>
#include <map>
#include <set>
#include <ctime>

#include "Utils.hpp"
#include "locationAnalyzer.hpp"

using namespace std;

LocationAnalyzer::~LocationAnalyzer() {
    freeNode( world );
}

void LocationAnalyzer::freeNode( Node* n ) {
    for (int i = 0; i < n->subNodes.size(); i++ )
        freeNode( n->subNodes[i] );
    delete n;
}

void LocationAnalyzer::init( const char* locPath, const char* casePath ) {
    loadCaseConversion( casePath );

    ifstream in;

    in.open( locPath );
    string line;

    world = new Node;
    set<wstring> dummy;
    world->names[L"world"] = dummy;
    world->likelihood = 1;
    world->level = 0;
    world->superNode = NULL;

    vector<Node*> activeNodes;
    activeNodes.push_back( world );

    while( in )
    {
        getline( in, line, '\n' );
        if ( line.length() == 0 || line[0] == '#' || line[0] == '!' ||
             line[0] == '\t' || line[0] == '\n' )
            continue;

//cout << line << endl;        
        vector<string> tokens = ctokenize( line, ",\t\n" );
        int level = atoi( tokens[0].c_str() );
        float likelihood = atof( tokens[1].c_str() );
        Node* cur = new Node;
        cur->level = level;
        cur->likelihood = likelihood;

        for ( int i = 2; i < tokens.size(); i++ )
        {
            wstring name;
            set<wstring> cxtNames;

            int pref = readName( tokens[i], name, cxtNames );
//cout << wtoc( name ) << endl;
            cur->names[name] = cxtNames;
            if ( pref )
                cur->prefNames.insert( name );
            str2nodes[name].push_back( cur );
            
            size_t found = name.find( L" city" );
            if ( found != string::npos )
            {
                wstring name2 = name.substr( 0, found );

                cur->names[name2] = cxtNames;
                if ( pref )
                    cur->prefNames.insert( name2 );
                str2nodes[name2].push_back( cur );
            }
        } 
        
        int last = activeNodes.size()-1;

        while ( level <= activeNodes[last]->level )
            last--;

        activeNodes[last]->subNodes.push_back( cur );
        cur->superNode = activeNodes[last];
        if ( last+1 < activeNodes.size() )
            activeNodes.resize(last+1);

        activeNodes.push_back( cur ); 
    }
//cout << "dict done" << endl << endl;
}


void LocationAnalyzer::lower( wstring& str ) {
    for ( int i = 0; i < str.length(); i++ )
    {
        if ( up2low.find(str[i]) != up2low.end() )
            str[i] = up2low[str[i]];
    }
}


void LocationAnalyzer::loadCaseConversion( const char* path ) {
    // read case conversion table
    FILE* in = fopen( path, "r" );
    if ( in == NULL )
    {
        cout << "Fail to load case conversion table: " << path << endl;fflush(stdout);
        exit(0);
    }
    unsigned int up, lo;
    while ( fscanf( in, "%x%x", &up, &lo ) >= 0 ) 
    {
        up2low.insert(pair<int,int>(up,lo));
    }

    fclose(in);
}



int LocationAnalyzer::readName( string& str, wstring& name, set<wstring>& cxtNames ) {
    vector<string> tokens = ctokenize( str, ":&" );
    name = ctow( tokens[0] );
    lower( name );
    cxtNames.clear();
    int pref = 0;

    for ( int i = 1; i < tokens.size(); i++ )
    {
        if ( tokens[i] == "pref" )
            pref = 1;
        else
        {
            wstring cxtName = ctow( tokens[i] );
            lower( cxtName );
            cxtNames.insert( cxtName );
        }
    }

    return pref;
}


float LocationAnalyzer::match( wstring& str1, wstring& str2 ) {
    Node* n1 = getNode( str1 );
    Node* n2 = getNode( str2 );
 
    if ( n1 == NULL || n2 == NULL )
        return 0;

    while ( n1->level > n2->level )        
        n1 = n1->superNode;

    while ( n1->level < n2->level )
        n2 = n2->superNode;

    while ( n1 != n2 )
    {
        n1 = n1->superNode;
        n2 = n2->superNode;
    } 

    return -2.0 * log(n1->likelihood);
}


vector<string> LocationAnalyzer::analyze( wstring& str ) {
    Node* n = getNode( str );
    vector<string> res;

    while( n != NULL )
    {
        string name = wtoc( n->names.begin()->first );
        res.push_back( name );        
        n = n->superNode;
    }

    int last = res.size() - 1;
    for ( int i = 0; i + i < last; i++ )
    {
        string temp = res[i];
        res[i] = res[last-i];
        res[last-i] = temp;
    }
    return res;
}


bool LocationAnalyzer::cover( Node* n1, Node* n2 ) {
    while ( n1->level < n2->level )
        n2 = n2->superNode;

    return n1 == n2;
} 




vector<wstring> LocationAnalyzer::segment( wstring& str ) {
    lower( str );
    vector<wstring> tokens = wtokenize( str, L",. \t\n" );
    vector<wstring> res;  

    wstring name;

    int i = 0, step;
    
    while ( i < tokens.size() )
    {
        step = 0;

        int j = i;
        wstring str = tokens[i];
        if ( str2nodes.find( str ) != str2nodes.end() )  // current token is already a name
        {
            name = str;
            step = 1;
        }

        j = i+1;
        while( j < i+3 && j < tokens.size() )
        {
            str += L" ";
            str += tokens[j];
            if ( str2nodes.find( str ) != str2nodes.end() )
            {
                name = str;  // select the longest
                step = j-i+1;
            }
            j++;
        }

        if ( step == 0 )  
            // no location name starts with the current word
            i++;
        else
        {
            // found a match
            res.push_back( name );
            i += step;
        }
    }

    return res;
}



Node* LocationAnalyzer::getNode( wstring& str ) {

//cout << wtoc(str) << ":";

    vector<wstring> locNames = segment( str );

    vector<Node*> locs;
    vector<Node*> locs2;  // location matched without context but priority.
    for ( int i = 0; i < locNames.size(); i++ )
    {
        wstring& name = locNames[i];

//cout << wtoc( name ) << "|";

        vector<Node*>& nodes = str2nodes[name];
        for ( int j = 0; j < nodes.size(); j++ )
        {
            assert( nodes[j]->names.find(name) != nodes[j]->names.end() );

            set<wstring>& cxtWords = nodes[j]->names[name];

            if ( !cxtWords.empty() ) 
            {
                // check context
                int cxtMatched = 0;
                for ( int k = 0; k < locNames.size(); k++ )
                    if ( cxtWords.find( locNames[k] ) != cxtWords.end() )
                    {
                        cxtMatched = 1;
                        break;
                    }

                if ( cxtMatched )
                    locs.push_back( nodes[j] );
                else if ( nodes[j]->prefNames.find(name) != nodes[j]->prefNames.end() )
                {
                    locs2.push_back( nodes[j] );
                }
            }
            else
                locs.push_back( nodes[j] );
        }
    }
//cout << endl;

    // sort nodes by levels
    for ( int i = 0; i < locs.size(); i++ )
        for ( int j = i+1; j < locs.size(); j++ )
            if ( locs[i]->level < locs[j]->level )
            {
                Node* temp = locs[i];
                locs[i] = locs[j];
                locs[j] = temp;
            }

    // find the deepest node
    if ( locs.size() > 0 )
        return locs[0];

    if ( locs2.size() > 0 )
        return locs2[0];

    return world;
}

#ifdef LOCATIONANALYZER_TEST
int main( int argc, char** argv ) {
    setlocale(LC_CTYPE, "");    

    LocationAnalyzer Analyzer;
    Analyzer.init( argv[1], argv[2] );

    string str;
    while(1)
    {
        getline( cin, str );
        wstring s = ctow(str);
        vector<string> names = Analyzer.analyze( s );
        for ( int i = 0; i < names.size(); i++ )
            cout << names[i] << "  ";
        cout << endl;
    }
    
/*
    string str1, str2;
    while(1)
    {
        getline( cin, str1 );
        getline( cin, str2 );
        wstring s1 = ctow(str1);
        wstring s2 = ctow(str2);

        cout << -2.0*log(Analyzer.match( s1, s2 )) << endl;
    }
*/
}
#endif
