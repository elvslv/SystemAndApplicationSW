#include <iostream>
#include <fstream>
#include "Huffman.h"

using namespace std;

void Huffman::initQueue()
{	
	intType num = 0;
	for ( unsigned int i = 0; i < (1 << BITS_IN_BYTE); ++i )
	{
		if ( !freq[i] )
			continue;
		vector<unsigned char>* s = new vector<unsigned char>();
		s->push_back( i );
		Node* node = new Node( NULL, NULL, s, freq[i]);
		pq.push( node );
		++num;
	}
	for ( intType i = 0; i < num - 1; ++i )
	{
		Node* left = pq.top();
		pq.pop( );
		Node* right = pq.top();
		pq.pop( );
		vector<unsigned char>* v = new vector<unsigned char>( 
			left->getSymbols()->size() + right->getSymbols()->size() );
		set_union( left->getSymbols()->begin(), left->getSymbols()->end(), right->getSymbols()->begin(), 
			right->getSymbols()->end(), v->begin() );
		Node* node = new Node( left, right, v, left->getFreq() + right->getFreq() );
		pq.push(node);
	}
}

static ofstream os;

HuffmanEncoder::HuffmanEncoder( vector<string>* fns, string outputfns ): fileNames(fns), outputFileName(outputfns)
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
	memset( freq, 0, 256 * sizeof(intType) );
}

int HuffmanEncoder::encode()
{
	countFrequency( );
	initQueue( );
	buildCodes( );
	os.open( outputFileName.c_str(), ios::binary );
	printMetadata( );
	getResult( );
	os.close();
	return 0;
}
void HuffmanEncoder::countFrequency()
{	
	several = false;
	unsigned char ch = input[0];
	for ( string::iterator it = input.begin(); it != input.end(); ++it )
	{
		if ( *it != ch )
			several = true;
		++freq[(unsigned char)*it]; 
	}
}

void HuffmanEncoder::buildCodes( )
{
	if ( !several )
	{
		chCodes[pq.top()->getSymbols()->at(0)].push_back( 1 );
		return;
	}
	for ( unsigned int ch = 0; ch < (1 << BITS_IN_BYTE); ++ch )
	{
		if ( !freq[ch] )
			continue;
		Node* root = pq.top();
		while ( root->getLeft() && root->getRight() ) 
		{
			if ( binary_search(root->getLeft()->getSymbols()->begin(), root->getLeft()->getSymbols()->end(), ch) )
			{
				root = root->getLeft();
				chCodes[ch].push_back( 0 );
			}
			else
			{
				root = root->getRight();
				chCodes[ch].push_back( 1 );
			}
		}
	}
}

void HuffmanEncoder::getResult( )
{
	unsigned char next = 0;
	int i = 0;
	for ( string::iterator it = input.begin(); it != input.end(); ++it )
	{
		unsigned char ch = *it;
		for ( vector<int>::iterator it1 = chCodes[ch].begin(); it1 != chCodes[ch].end(); ++it1 )
		{
			next = (next << 1) | *it1;
			if ( !( ( ++i ) % 8 ) )
			{
				os << next;
				next = 0;
				i = 0;
			}
		}
	}
	if ( i )
	{
		while ( i )
		{
			next <<= 1;
			i = ( ++i ) % 8;
		}
		os << next;
	}
}

void HuffmanEncoder::printMetadata( )
{
	printInt( os, fileInfos.size() );
	for (vector<FileInfo>::iterator it = fileInfos.begin(); it != fileInfos.end(); ++it )
	{
		printInt( os, it->fileNameLength );
		os.write( it->fileName.c_str(), it->fileName.length() );
		printInt( os, it->numOfBytes );
	}
	for ( unsigned int i = 0; i < 256; ++i )
	{
		printInt( os, freq[i]);
	}
}
HuffmanDecoder::HuffmanDecoder( string infile, string d ): inFileName(infile), dir(d)
{
	bytesDecoded = 0;
	pos = 0;
	readFile( inFileName, input );
	dir += "\\";
}

int HuffmanDecoder::decode( )
{
	fillMetadata( );
	getFrequences( );
	initQueue( );
	input = string( input.c_str() + pos, input.length() - pos );
	if ( getResult( ) != 0 )
		throw "Archive is defective";
}
void HuffmanDecoder::fillMetadata( )
{
	intType filesNum = getInt( input.c_str() ); 
	pos += BYTES_IN_INT;
	for ( int i = 0; i < filesNum; ++i )
	{
		intType fileNameLength = getInt( input.c_str() + pos );
		pos += BYTES_IN_INT;
		string fileName = input.substr( pos, fileNameLength );
		pos += fileNameLength;
		intType fileLength = getInt( input.c_str() + pos );
		pos += BYTES_IN_INT;
		fileInfos.push_back( FileInfo( fileNameLength, fileName, fileLength ) );
	}
}

void HuffmanDecoder::getFrequences( )
{
	for ( unsigned int i = 0; i < 256; ++i )
	{
		intType fr = getInt( input.c_str() + pos );
		pos += BYTES_IN_INT;
		freq[i] = fr;
	}
}

int HuffmanDecoder::getResult( )
{
	vector<FileInfo>::iterator curFile = fileInfos.begin();
	ofstream os;
	string path = dir;
	path += curFile->fileName;
	os.open( path.c_str(), ios::binary );
	if ( !pq.size() )
	{
		os.close();		
		compareFiles( curFile->fileName, path );
		return 0;
	}
	int nextsym = 0;
	Node* root = pq.top();
	int j = BITS_IN_BYTE - 1;
	string::iterator it = input.begin();
	unsigned char sym = *it++;
	while ( it != input.end() && curFile != fileInfos.end())
	{
		if ( !( root->getLeft() && root->getRight() ) )
			--j;
		while ( root->getLeft() && root->getRight() )
		{
			int b = ( sym & ( 1 << j )  ) >> j;
			root = b ? root->getRight() : root->getLeft();
			if ( ( j == 0 ) && (root->getLeft() && root->getRight()) )
			{
				j = BITS_IN_BYTE - 1;
				sym = *it++;
				continue;
			}
			--j;
		}
		char ch = root->getSymbols()->front();
		os << ch;
		if ( ++bytesDecoded == curFile->numOfBytes )
		{
			bytesDecoded = 0;
			os.close();
			compareFiles( curFile->fileName, path );
			++curFile;
			if ( curFile != fileInfos.end() )
			{
				path = dir;
				path += curFile->fileName;
				os.open( path.c_str(), ios::binary );
				while ( curFile != fileInfos.end() && curFile->numOfBytes == bytesDecoded )
				{
					os.close();
					++curFile;
					path = dir;
					path += curFile->fileName;
					os.open( path.c_str(), ios::binary );
				}
			}
		}
		root = pq.top();
		if ( j < 0 )
		{
			j = BITS_IN_BYTE - 1;
			sym = *it++;
		}
		if ( curFile == fileInfos.end() && it != input.end() )
		{
			throw "Archive is defective 1";
		}
	}
	while ( curFile != fileInfos.end() && bytesDecoded < curFile->numOfBytes && j >= 0)
	{
		while ( root->getLeft() && root->getRight() )
		{
			int b = ( sym & ( 1 << j ) ) >> j;
			root = b ? root->getRight() : root->getLeft();
			--j;
		}
		char ch = root->getSymbols()->front();
		os << ch;
		if ( ++bytesDecoded == curFile->numOfBytes )
		{
			bytesDecoded = 0;
			os.close();
			compareFiles( curFile->fileName, path );
			++curFile;
			if ( curFile != fileInfos.end() )
			{
				path = dir;
				path += curFile->fileName;
				os.open( path.c_str(), ios::binary );
				while ( curFile != fileInfos.end() && curFile->numOfBytes == bytesDecoded )
				{
					os.close();
					++curFile;
					path = dir;
					path += curFile->fileName;
					os.open( path.c_str(), ios::binary );
				}
			}
		}
	}
	if ( curFile != fileInfos.end() )
	{
		throw "Archive is defective 2";
	}
	return 0;
}