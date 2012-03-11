#pragma comment(linker, "/STACK:5000000")

#include <iostream>
#include <fstream>
#include <cassert>
#include "LZW.h"

using namespace std;

static ofstream os;
static ifstream is;

LZWEncoder::LZWEncoder( vector<string>* fns, string outputfns ): fileNames(fns), outputFileName(outputfns)
{
	if ( !fileNames->size() )
		throw "The list of input files is empty";
	for (vector<string>::iterator it = fileNames->begin(); it != fileNames->end(); ++it)
	{
		string tmp;
		readFile( *it, tmp );
		input += tmp;
		fileInfos.push_back( FileInfo( it->length(), *it, tmp.length() ) );
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

int LZWEncoder::encode()
{
	os.open( outputFileName.c_str(), ios::binary );
	printMetadata();
	getResult();
	os.close();
	return 0;
}

void LZWEncoder::printMetadata( )
{
	printInt( os, fileInfos.size() );
	for (vector<FileInfo>::iterator it = fileInfos.begin(); it != fileInfos.end(); ++it )
	{
		printInt( os, it->fileNameLength );
		os.write( it->fileName.c_str(), it->fileName.length() );
		printInt( os, it->numOfBytes );
	}
}

void LZWEncoder::getResult( )
{	
	string str = "";
	char ch, ch1;
	for ( vector<FileInfo>::iterator it = fileInfos.begin(); it != fileInfos.end(); ++it )
	{
		ifstream is;
		is.open( it->fileName.c_str(), ios::binary );
		intType bytesDecoded = 0;
		if ( it == fileInfos.begin() )
		{
			ch1 = is.get();
			str = "";
			str += ch1;
			bytesDecoded += 1;
		}
		
		bool f1 = false, f2 = false, f3 = false;
		while ( !is.eof() &&  bytesDecoded < it->numOfBytes )
		{
			if ( bytesDecoded == 3197189 )
			{
				bytesDecoded = bytesDecoded;
			}
			ch = is.get();
			++bytesDecoded;
			if ( stringTable.find(str + ch) != stringTable.end() )
			{
				str += ch;
			}
			else 
			{
				printBits(os, stringTable[str]);
				if ( lastString < ( 1 << MAX_NUM_OF_BITS ) - 1 )
				{
					stringTable[str + ch] = lastString++;
				}
				str = ch;
			}
		}
		//char ch2 = is.get();
		is.close( );
	}
	printBits(os, stringTable[str]);
}

LZWDecoder::LZWDecoder( string infile, string d ): inFileName(infile), dir(d)
{
	bytesDecoded = 0;
	pos = 0;
	for ( lastString = 0; lastString < (1 << MAX_NUM_OF_BITS); ++lastString )
	{
		stringTable[lastString] = "";
		if ( lastString < (1 << BITS_IN_BYTE) )
			stringTable[lastString] += (unsigned char)lastString;
	}
	lastString = 256;
	dir += "\\";
}

int LZWDecoder::decode()
{
	is.open( inFileName.c_str(), ios::binary );
	fillMetadata();
	getResult();
	is.close();
	return 0;
}
void LZWDecoder::fillMetadata()
{
	intType numOfFiles = getIntFromStream( is );
	for ( intType i = 0; i < numOfFiles; ++i )
	{
		intType nameLength = getIntFromStream( is );
		string name = "";
		for ( intType j = 0; j < nameLength; ++j )
		{
			name += is.get();
		}
		intType length = getIntFromStream( is );
		fileInfos.push_back( FileInfo( nameLength, name, length ) );
	}
}

void LZWDecoder::getResult()
{
	vector<FileInfo>::iterator curFile = fileInfos.begin();
	ofstream os;
	string path;		
	path = dir;
	path += curFile->fileName;
	os.open( path.c_str(), ios::binary );
	while ( curFile != fileInfos.end() && !curFile->numOfBytes )
	{
		os.close();		
		compareFiles( curFile->fileName, path );
		++curFile;
		if ( curFile == fileInfos.end() )
			return;
		path = dir;
		path += curFile->fileName;
		os.open( path.c_str(), ios::binary );
	}
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
		if ( bytesDecoded + str.length() > curFile->numOfBytes )
		{
			os << str.substr(0, curFile->numOfBytes - bytesDecoded);
			intType l = curFile->numOfBytes - bytesDecoded;
			bytesDecoded += str.length();
			str = str.substr(l);
		}
		else
		{
			os << str;
			bytesDecoded += str.length();
		}
		if ( bytesDecoded >= curFile->numOfBytes )
		{
			bytesDecoded = bytesDecoded - curFile->numOfBytes;
			os.close();
			compareFiles( curFile->fileName, path );
			++curFile;
			if ( curFile == fileInfos.end() )
			{
				//is.get();
				break;
			}
			path = dir;
			path += curFile->fileName;
			os.open( path.c_str(), ios::binary );
			while ( curFile != fileInfos.end() && !curFile->numOfBytes )
			{
				os.close();		
				compareFiles( curFile->fileName, path );
				++curFile;
				if ( curFile == fileInfos.end() )
					return;
				path = dir;
				path += curFile->fileName;
				os.open( path.c_str(), ios::binary );
			}
			if ( bytesDecoded && curFile != fileInfos.end() )
				os << str;
		}
		ch = str[0];
		if (lastString < (1 << MAX_NUM_OF_BITS) - 1)
		{	
			string tmp = stringTable[oldCode];
			tmp += ch;
			stringTable[lastString++] = tmp;
		}
		oldCode = newCode;
	}
	curFile = curFile;
	is.get();
	bool b1 = is.eof();
	bool b2 = curFile != fileInfos.end();
	if ( b2 || !b1 )
		throw "Archive is corrupted";
}