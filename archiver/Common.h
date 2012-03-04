#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <vector>

using namespace std;

const int MAX_NUM_OF_BITS = 16;
const int BITS_IN_BYTE = 8;
const int LAST_BYTE = (1 << BITS_IN_BYTE) - 1;
const int BYTES_IN_INT = 8;
typedef unsigned __int64 intType;

struct FileInfo
{
	FileInfo( intType fnl, string fn, intType nb ): fileNameLength( fnl ), fileName( fn ), numOfBytes( nb ){};
	intType fileNameLength;
	string fileName;
	intType numOfBytes;
};

class Encoder
{
public:
	Encoder(){};
	virtual int encode(){ return 0; }; 
};

class Decoder
{
public:
	Decoder(){};
	virtual int decode(){ return 0; };
};

void readFile( string fileName, string& result );
int compareFiles( string frstFile, string scndFile );
void printInt( ofstream& os, intType n );
intType getInt( const char* buffer );
intType getIntFromStream( ifstream &is );
void printBits( ofstream& os, unsigned short n );
unsigned short getBits( ifstream& is, unsigned short n );

#endif //COMMON_H
