#ifndef LZW_H
#define LZW_H

#include <map>
#include "../Common.h"

using namespace std;

class LZW
{
protected:
	vector<FileInfo> fileInfos;
};

class LZWEncoder: public LZW, public Encoder
{
public:
	LZWEncoder( vector<string>* fns, string outputfns );
	int encode();
private:
	void printMetadata( );
	void getResult( );

	vector<string>* fileNames;
	string outputFileName;
	string input;
	int numOfBytes;
	map<string, unsigned short> stringTable;
	unsigned int lastString;
};

class LZWDecoder: public LZW, public Decoder
{
public:
	LZWDecoder( string infile, string d );
	int decode();
private:
	void fillMetadata();
	void getResult();

	string inFileName;
	string dir;
	string input;
	int pos;
	__int64 bytesDecoded;
	unsigned int lastString;
	string stringTable[1 << MAX_NUM_OF_BITS];
};

#endif //LZW_H