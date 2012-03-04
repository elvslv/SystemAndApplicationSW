#include <fstream>
#include <iostream>
#include "Common.h"

using namespace std;

char* readBuffer( ifstream &is, intType& l )
{
	char* buffer;
	is.seekg( 0, ios::end );
	intType length = is.tellg( );
	is.seekg( 0, ios::beg );
	buffer = new char [length];
	is.read( buffer,length );
	l = length;
	return buffer;
}

void readFile( string fileName, string& result )
{
	ifstream is;
	char* buffer;
	intType length;
	is.open( fileName.c_str(), ios::binary );
	buffer = readBuffer( is, length );
	is.close( );
    result = string( buffer, length );
	delete[] buffer;
}

int compareFiles( string frstFile, string scndFile )
{
	ifstream is;
	char* buffer1;
	char* buffer2;
	intType length1, length2;
	int result;
	is.open( frstFile.c_str(), ios::binary );
	buffer1 = readBuffer( is, length1 );
	is.close();
	is.open( scndFile.c_str(), ios::binary );
	buffer2 = readBuffer( is, length2 );
	is.close();
	result = ( length1 != length2 && !memcmp( buffer1, buffer2, length1 ) );
	if ( result )
		cout << "Files " << frstFile << " and " << scndFile <<" doesn't match\n";
	return result;
}

void printInt( ofstream& os, intType n )
{
	for ( int i = ( (BYTES_IN_INT - 1) * BITS_IN_BYTE ); i >= 0; i -= BITS_IN_BYTE )
	{
		unsigned char ch = (n & (LAST_BYTE << i)) >> i;
		os << ch;
	}
}

intType getInt( const char* buffer )
{
	intType result = 0;
	for ( int i = 0; i < BYTES_IN_INT; i += 1 )
	{
		result = (result << BITS_IN_BYTE) | (unsigned char)buffer[i];
	}
	return result;
}

intType getIntFromStream( ifstream &is )
{
	intType result = 0;
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