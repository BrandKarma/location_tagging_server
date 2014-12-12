#ifndef _UTILS_
#define _UTILS_

#include <cstring>
#include <cmath>
#include <vector>
#include <string>
#include <ext/hash_map>

using namespace std;
using namespace __gnu_cxx;


typedef unsigned int uint;


#define ABS(X) (((X) < 0 ) ? -(X) : (X))

inline wstring ctow( const string& src ) {
    int size = mbstowcs( NULL, src.c_str(), 0 );
    vector<wchar_t> store(size+1);
    mbstowcs(&store[0], src.c_str(), size+1);
    wstring dest = &store[0]; 
    return dest;
}

inline string wtoc( const wstring& src ) {
    int size = wcstombs( NULL, src.c_str(), 0 );
    vector<char> store(size+1);
    wcstombs(&store[0], src.c_str(), size+1);
    string dest = &store[0]; 
    return dest;
}

inline vector<wstring> wtokenize( const wstring& wstr,
                                  const wstring& delimiters )
{
    vector<wstring> tokens;

    // Skip delimiters at beginning.
    wstring::size_type lastPos = wstr.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    wstring::size_type pos     = wstr.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(wstr.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = wstr.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = wstr.find_first_of(delimiters, lastPos);
    }

    return tokens;
}


inline vector<string> ctokenize(const string& str,
                                const string& delimiters )
{
    vector<string> tokens;

    // Skip delimiters at beginning.
    string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (string::npos != pos || string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }

    return tokens;
}


inline wchar_t * bhm_subwstr( wchar_t* haystack, int hlen,
                       wchar_t* needle,   int nlen)
{
    int scan = 0;
    int bad_char_skip[256]; /* Officially called:
                                          * bad character shift */ 
    /* Sanity checks on the parameters */
    if (nlen <= 0 || !haystack || !needle)
        return NULL;
 
    /* ---- Preprocess ---- */
    /* Initialize the table to default value */
    /* When a character is encountered that does not occur
     * in the needle, we can safely skip ahead for the whole
     * length of the needle.
     */
    for (scan = 0; scan <= 256; scan = scan + 1)
        bad_char_skip[scan] = nlen;
 
    /* C arrays have the first byte at [0], therefore:
     * [nlen - 1] is the last byte of the array. */
    int last = nlen - 1;
 
    /* Then populate it with the analysis of the needle */
    for (scan = 0; scan < last; scan = scan + 1)
    {
        if ( (unsigned int) needle[scan] < 256 )
            bad_char_skip[needle[scan]] = last - scan;
    }
    /* ---- Do the matching ---- */
 
    /* Search the haystack, while the needle can still be within it. */
    while (hlen >= nlen)
    {
        /* scan from the end of the needle */
        for (scan = last; haystack[scan] == needle[scan]; scan = scan - 1)
            if (scan == 0) /* If the first byte matches, we've found it. */
                return haystack;
 
        /* otherwise, we need to skip some bytes and start again.
           Note that here we are getting the skip value based on the last byte
           of needle, no matter where we didn't match. So if needle is: "abcd"
           then we are skipping based on 'd' and that value will be 4, and
           for "abcdd" we again skip on 'd' but the value will be only 1.
           The alternative of pretending that the mismatched character was
           the last character is slower in the normal case (E.g. finding
           "abcd" in "...azcd..." gives 4 by using 'd' but only
           4-2==2 using 'z'. */
        if ( (unsigned int) haystack[last] < 256 )
        {
            hlen     -= bad_char_skip[haystack[last]];
            haystack += bad_char_skip[haystack[last]];
        }
        else
        {
            hlen--;
            haystack++;
        }
    }
 
    return NULL;
}


class eqString
{
public:

    eqString() {}

    int operator () (const string& s1, const string& s2) const
    {
        return s1 == s2;
    }
};


class hashString
{
public:

    hashString() {}

    int operator () (const string& s) const
    {
        return hash(s.c_str());
    }

private:
    static int hash( const char * str ) {
        int hash = 5381;
        int c;
        while( c = *str++ )
            hash = c + ( hash << 16 ) + ( hash << 6 ) - hash;

        return hash;
    }
};



class eqWstring
{
    public:

    eqWstring() {}

    int operator () (const wstring& s1, const wstring& s2) const
    {
        return s1 == s2;
    }
};


class hashWstring
{
public:

    hashWstring() {}

    int operator () (const wstring& s) const
    {
        return hash(s.c_str());
    }

private:
    static int hash( const wchar_t * str ) {
        int hash = 5381;
        int c;
        while( c = *str++ )
            hash = c + ( hash << 16 ) + ( hash << 6 ) - hash;

        return hash;
    }
};


#endif
