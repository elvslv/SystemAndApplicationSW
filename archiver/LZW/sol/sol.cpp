#pragma comment(linker, "/STACK:5000000")

#include <iostream>
#include <fstream>
#include <cassert>
#include <string>
#include <map>
#include <vector>

using namespace std;

const int BITS_IN_BYTE = 8;
const int LAST_BYTE = (1 << BITS_IN_BYTE) - 1;
const int BYTES_IN_INT = 4;
const int MAX_NUM_OF_BITS = 16;

struct FileInfo
{
	FileInfo( int fnl, string fn, int nb ): fileNameLength( fnl ), fileName( fn ), numOfBytes( nb ){};
	int fileNameLength;
	string fileName;
	int numOfBytes;
};

class LZW
{
protected:
	vector<FileInfo> fileInfos;
};

string result;

void readFile( string fileName, string& result )
{
	ifstream is;
	char* buffer;
	is.open( fileName.c_str(), ios::binary );
	is.seekg( 0, ios::end );
	int length = is.tellg( );
	is.seekg( 0, ios::beg );
	buffer = new char [length];
	is.read( buffer,length );
	is.close( );
    result = string( buffer, length );
	delete[] buffer;
}

void printInt( ofstream& os, int n )
{
	for ( int i = ( (BYTES_IN_INT - 1) * BITS_IN_BYTE ); i >= 0; i -= BITS_IN_BYTE )
	{
		unsigned char ch = (n & (LAST_BYTE << i)) >> i;
		os << ch;
	}
}

unsigned int getInt( const char* buffer )
{
	unsigned int result = 0;
	for ( int i = 0; i < BYTES_IN_INT; i += 1 )
	{
		result = (result << BITS_IN_BYTE) | (unsigned char)buffer[i];
	}
	return result;
}

unsigned int getIntFromStream( ifstream &is )
{
	unsigned int result = 0;
	for ( int i = 0; i < BYTES_IN_INT; i += 1 )
	{
		result = (result << BITS_IN_BYTE) | is.get();
	}
	return result;
}

void printBits( ofstream& os, unsigned short n )
{
	unsigned char ch1 = ( ( n & ( ( ( 1 << BITS_IN_BYTE ) - 1 ) << BITS_IN_BYTE ) ) >> BITS_IN_BYTE);
	unsigned char ch2 = ( n &  (( 1 << BITS_IN_BYTE ) - 1 ) );
	os << ch1 << ch2;
}

unsigned short getBits( ifstream& is, unsigned short n )
{
	unsigned char ch1;
	unsigned char ch2;
	is >> ch1 >> ch2;
	return (ch1 << BITS_IN_BYTE) | ch2;
}

ofstream os;
ifstream is;

class Encoder: public LZW
{
public:
	Encoder( vector<string>* fns, string outputfns ): fileNames(fns), outputFileName(outputfns)
	{
		if ( !fileNames->size() )
			throw "The list of input files is empty";
		for (vector<string>::iterator it = fileNames->begin(); it != fileNames->end(); ++it)
		{
			ifstream is;
			is.open( it->c_str(), ios::binary );
			is.seekg( 0, ios::end );
			fileInfos.push_back( FileInfo( it->length(), *it, is.tellg( ) ) );
			is.close( );
		}
		numOfBytes = input.length();
		for ( lastString = 0; lastString < LAST_BYTE + 1; ++lastString )
		{
			unsigned char ch = lastString;
			string str = "";
			str += ch;
			stringTable[str] = lastString;
		}
	}

	int encode()
	{
		os.open( outputFileName.c_str(), ios::binary );
		printMetadata();
		getResult();
		os.close();
		return 0;
	}

private:
	void printMetadata( )
	{
		printInt( os, fileInfos.size() );
		for (vector<FileInfo>::iterator it = fileInfos.begin(); it != fileInfos.end(); ++it )
		{
			printInt( os, it->fileNameLength );
			os.write( it->fileName.c_str(), it->fileName.length() );
			printInt( os, it->numOfBytes );
		}
	}

	void getResult( )
	{	
		string str = "";
		char ch;
		for ( vector<string>::iterator it = fileNames->begin(); it != fileNames->end(); ++it )
		{
			ifstream is;
			is.open( it->c_str(), ios::binary );
			if ( it == fileNames->begin() )
			{
				str = is.get();
			}
			while ( !is.eof() )
			{
				ch = is.get();
				if ( stringTable.find(str + ch) != stringTable.end() )
				{
					str += ch;
				}
				else 
				{
					printBits(os, stringTable[str]);
					if ( lastString < ( 1 << MAX_NUM_OF_BITS ) - 1 )
					{
						stringTable[str + ch] = ++lastString;
					}
					str = ch;
				}
			}
			is.close( );
		}
		printBits(os, stringTable[str]);
	}

	vector<string>* fileNames;
	string outputFileName;
	string input;
	int numOfBytes;
	map<string, unsigned short> stringTable;
	int lastString;
};

class Decoder: public LZW
{
public:
	Decoder( string infile ): inFileName(infile)
	{
		bytesDecoded = 0;
		pos = 0;
		for ( unsigned int lastString = 0; lastString < (1 << MAX_NUM_OF_BITS); ++lastString )
		{
			stringTable[lastString] = "";
			if ( lastString < (1 << BITS_IN_BYTE) )
				stringTable[lastString] += (unsigned char)lastString;
		}
	}

	int decode()
	{
		is.open( inFileName.c_str(), ios::binary );
		fillMetadata();
		getResult();
		is.close();
		return 0;
	}

private:
	void fillMetadata()
	{
		unsigned int numOfFiles = getIntFromStream( is );
		for ( int i = 0; i < numOfFiles; ++i )
		{
			unsigned int nameLength = getIntFromStream( is );
			string name = "";
			for ( int j = 0; j < nameLength; ++j )
			{
				name += is.get();
			}
			unsigned int length = getIntFromStream( is );
			fileInfos.push_back( FileInfo( nameLength, name, length ) );
		}
	}

	void getResult()
	{
		vector<FileInfo>::iterator curFile = fileInfos.begin();
		ofstream os;
		os.open( curFile->fileName.c_str(), ios::binary );
		unsigned short oldCode;
		oldCode = is.get() << BITS_IN_BYTE | is.get();
		unsigned char ch = oldCode;
		os << ch;
		bytesDecoded = 1;
		string str;
		while ( !is.eof() )
		{
			unsigned short newCode = is.get() << BITS_IN_BYTE | is.get();
			if ( stringTable[newCode] == "" )
			{
				str = stringTable[oldCode];
				str += ch;
			}
			else
			{
				str = stringTable[newCode];
			}
			os << str;
			bytesDecoded += str.length();
			if ( bytesDecoded == curFile->numOfBytes )
			{
				os.close();
				++curFile;
				if ( curFile == fileInfos.end() )
					break;
				os.open( curFile->fileName.c_str(), ios::binary );
			}
			ch = str[0];
			if (lastString < (1 << MAX_NUM_OF_BITS) - 1)
				stringTable[lastString++] = "" + (unsigned char) oldCode + ch;
			oldCode = newCode;
		}
		//if ( curFile != fileInfos.end() || !is.eof() )
		//	throw "Archive is corrupted";

	}
	string inFileName;
	string input;
	int pos;
	int bytesDecoded;
	unsigned int lastString;
	string stringTable[1 << MAX_NUM_OF_BITS];
};

int main()
{
	vector<string>fileNames;
	fileNames.push_back( "input.txt" );
	Encoder encoder = Encoder( &fileNames, "output.txt" );
	encoder.encode();
	//Decoder decoder = Decoder( "output.txt" );
	//decoder.decode();
	return 0;
}