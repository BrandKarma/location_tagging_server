//////////////////////////////////////////////////////////////////
//
// Class: Marshaler
// ------------------
//
// Marshals data
//
/////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <wchar.h>
#include "marshal.h"

// Global functions
int64_t makeInt64(int lowerInt, int upperInt)
{
	int64_t i64;
	char *ptr = (char *)&i64;
	memcpy(ptr, &lowerInt, sizeof(int));
	ptr += sizeof(int);
	memcpy(ptr, &upperInt, sizeof(int));
	return i64;
}

int64_t htoni64(int64_t hostOrder)
{
	int lower32 = (int)hostOrder;
	int upper32 = hostOrder >> 32;
	int networkLower32 = htonl(lower32);
	int networkUpper32 = htonl(upper32);
	return makeInt64(networkUpper32, networkLower32);
}

int64_t ntohi64(int64_t networkOrder)
{
	int lower32 = (int)networkOrder;
	int upper32 = networkOrder >> 32;
	int hostLower32 = ntohl(lower32);
	int hostUpper32 = ntohl(upper32);
	return makeInt64(hostUpper32, hostLower32);
}

Marshaler::Marshaler()
{
	_counter = sizeof(int); // leave room for the total size at the beginning
	_buffer = new char[MARSHALER_CHUNK_SIZE];
	_cursor = (_buffer + sizeof(int));
	_max = MARSHALER_CHUNK_SIZE;
}

Marshaler::~Marshaler()
{
	delete(_buffer);
}

void Marshaler::marshal_int(int i)
{
	if((_counter + sizeof(int)) > _max)
	{
		increase_buffer();
	}
	int networkOrder = htonl(i);
	memcpy(_cursor, &networkOrder, sizeof(int));
	_counter += sizeof(int);
	_cursor += sizeof(int);

}

void Marshaler::marshal_float(float f)
{
        if ( sizeof(int) != sizeof(float)) {
            printf("Unmatched sizes of float and int different. Abort!\n");
            exit(0);
        }

	if((_counter + sizeof(int)) > _max)
	{
		increase_buffer();
	}
	int networkOrder = htonl( *(int*)&f );
	memcpy(_cursor, &networkOrder, sizeof(int));
	_counter += sizeof(int);
	_cursor += sizeof(int);

}

void Marshaler::marshal_char(char c)
{
	if((_counter + sizeof(char)) > _max)
	{
		increase_buffer();
	}
	memcpy(_cursor, &c, sizeof(char));
	_counter += sizeof(char);
	_cursor += sizeof(char);
}

void Marshaler::marshal_int(int i, unsigned int index)
{
	int networkOrder = htonl(i);
	int total = sizeof(int) + index; // to account for the total packet size at the beginning
	if((total + sizeof(int)) < _max) // make sure that we are in range
	{
		memcpy((_buffer + total), &networkOrder, sizeof(int));
	}
}

void Marshaler::marshal_string(const char *str)
{
        size_t len;
        if(str != NULL)
        {
                len = strlen(str);

                marshal_int((int)len);
                if((_counter + len) > _max)
                {
                        if((_counter + len) > (_max + MARSHALER_CHUNK_SIZE))
                        {
                                increase_buffer(_counter + len + MARSHALER_CHUNK_SIZE);
                        }
                        else
                        {
                                increase_buffer();
                        }
                }
                memcpy(_cursor, str, len);
                _cursor += len;
                _counter += len;
        }

}

long Marshaler::get_data(char **data)
{
	char *temp = new char[_counter];
	memcpy(temp, _buffer, _counter);
	int *size = (int *)temp;
	int networkSize = htonl(_counter);
	memcpy(size, &networkSize, sizeof(int));
	*data = temp;
	return _counter;
}

void Marshaler::marshal_long(unsigned long ul)
{
	if((_counter + sizeof(unsigned long)) > _max)
	{
		increase_buffer();
	}
	unsigned long networkOrder = htonl(ul);
	memcpy(_cursor, &networkOrder, sizeof(unsigned long));
	_counter += sizeof(unsigned long);
	_cursor += sizeof(unsigned long);
}

void Marshaler::marshal_long(int64_t l)
{
	if((_counter + sizeof(int64_t)) > _max)
	{
		increase_buffer();
	}
	int64_t networkOrder = htoni64(l);
	memcpy(_cursor, &networkOrder, sizeof(int64_t));
	_counter += sizeof(int64_t);
	_cursor += sizeof(int64_t);
}

// Private methods



void Marshaler::increase_buffer()
{
	long newMax = _max + MARSHALER_CHUNK_SIZE;
	char *temp = new char[newMax];
	memcpy(temp, _buffer, _max);
	_max = newMax;
	delete[] (_buffer);
	_buffer = temp;
	_cursor = _buffer + _counter;
}

void Marshaler::increase_buffer(long newSize)
{
	char *temp = new char[newSize];
	memcpy(temp, _buffer, _max);
	_max = newSize;
	delete[] (_buffer);
	_buffer = temp;
	_cursor = _buffer + _counter;
}

//////////////////////////////////////////////////////////////////
//
// Class: Unmarshaler
// ------------------
//
// Unmarshals data
//
/////////////////////////////////////////////////////////////////
Unmarshaler::Unmarshaler(char *data)
{
	_data = data;
	_cursor = _data;
}

Unmarshaler::~Unmarshaler()
{
	delete[] _data;
}

char Unmarshaler::unmarshal_char()
{
	char c;
	memcpy(&c, _cursor, sizeof(char));
	_cursor += sizeof(char);
	return c;
}


int Unmarshaler::unmarshal_int()
{
	int networkInt;
	memcpy(&networkInt, _cursor, sizeof(int));
	_cursor += sizeof(int);
	return(ntohl(networkInt));
}

float Unmarshaler::unmarshal_float()
{
        if ( sizeof(int) != sizeof(float)) {
            printf("Unmatched sizes of float and int different. Abort!\n");
            exit(0);
        }
	int networkInt;
	memcpy(&networkInt, _cursor, sizeof(int));
	_cursor += sizeof(int);
	int i = (ntohl(networkInt));
        return *(float*)&i;
}


int64_t Unmarshaler::unmarshal_long()
{
	int64_t networkLong;
	memcpy(&networkLong, _cursor, sizeof(int64_t));
	_cursor += sizeof(int64_t);
	return(ntohi64(networkLong));
}

char *Unmarshaler::unmarshal_string(size_t max)
{
	char *str;
	size_t len = unmarshal_int();
        if (len > max ) len = max;

	str = new char[len+1];
	memcpy(str, _cursor, len);
        str[len] = '\0';	

if (strlen(str) != len) 
printf("unmatched string length! bytelen = %lu, expected len = %lu\n", strlen(str), len);

	_cursor += len;
	
	return str;
}


bool Unmarshaler::unmarshal_bool()
{
	bool b;
	memcpy(&b, _cursor, sizeof(bool));
	_cursor += sizeof(bool);
	return b;
}

