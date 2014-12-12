#ifndef MYMARSHAL_H
#define MYMARSHAL_H

#define MARSHALER_CHUNK_SIZE 1024
#define MAX_ERROR_STR 250
#define MAX_STR_LEN 1000000

#include <stdint.h>

// Global Functions
int64_t htoni64(int64_t hostOrder);
int64_t ntohi64(int64_t networkOrder);
int64_t makeInt64(int lowerInt, int upperInt);

//////////////////////////////////////////////////////////////////
//
// Class: Marshaler
// ------------------
//
// Marshals data
//
/////////////////////////////////////////////////////////////////
class Marshaler
{
public:
	Marshaler();
	virtual ~Marshaler();
	void marshal_char(char c);
	void marshal_int(int i);
	void marshal_float(float f);
	void marshal_int(int i, unsigned int index);
	void marshal_long(unsigned long i);
	void marshal_long(int64_t l);
	void marshal_string(const char *str);
	long get_data(char **data);

private:
	unsigned long _counter;
	unsigned long _max;
	char *_buffer;
	char *_cursor;

	void increase_buffer();
	void increase_buffer(long newSize);

};

//////////////////////////////////////////////////////////////////
//
// Class: Unmarshaler
// ------------------
//
// Marshals data
//
/////////////////////////////////////////////////////////////////
class Unmarshaler
{
public:
	Unmarshaler(char *data);
	virtual ~Unmarshaler();
        char unmarshal_char();
	int unmarshal_int();
	float unmarshal_float();
	int64_t unmarshal_long();
	char* unmarshal_string(size_t max);
	bool unmarshal_bool();

private:
	char *_data;
	char *_cursor;
	
};

#endif
