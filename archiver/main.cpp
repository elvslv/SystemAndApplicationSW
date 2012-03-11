#include <iostream>
#include <fstream>
#include "LZW/LZW.h"
#include "Huffman/Huffman.h"

using namespace std;

bool compress;
bool decompress;
bool lzw;
bool huffman;
vector<string> inputFiles;
string outputFile;
string dir;

int main( int argc, char* argv[] )
{
	try
	{
		if ( !argc )
			throw  "-c -- compress\n-d -- decompress\n"
				"-h -- huffman\n-l -- LZW\n-in -- name of input file\n"
				"-out -- name for output file (only for compressing mode)\n"
				"-dir -- directory to decompress";
		for ( int i = 0; i < argc; ++i )
		{
			compress = compress || !strcmp( argv[i], "-c");
			decompress = decompress || !strcmp( argv[i], "-d");
			lzw = lzw || !strcmp( argv[i], "-l");
			huffman = huffman || !strcmp( argv[i], "-h");
			if ( !strcmp( argv[i], "-in") )
				inputFiles.push_back( argv[i + 1] );
			if ( !strcmp( argv[i], "-out") )
				outputFile = argv[i + 1];
			if ( !strcmp( argv[i], "-dir") )
				dir = argv[i + 1];
		}
		if ( compress && decompress || !compress && !decompress )  
			throw "Invalid archiver format";
		if ( lzw && huffman || !lzw && !huffman )
			throw "Invalid compress format";
		if ( decompress && !dir.length() )
			throw "Invalid output directory";
		if ( compress && !(outputFile.length() && inputFiles.size()) || 
			 decompress && !(dir.length() && inputFiles.size() == 1) )
			throw "Invalid files";
		if ( compress )
		{
			if (lzw)
				LZWEncoder( &inputFiles, outputFile ).encode();
			else
				HuffmanEncoder( &inputFiles, outputFile ).encode();
		}
		else
		{	
			if (lzw)
				LZWDecoder( inputFiles[0], dir ).decode();
			else
				HuffmanDecoder( inputFiles[0], dir ).decode();
		}
		cout << "Finished" << endl;
		//char ch;
		//cin >> ch;
	}
	catch (char* str)
	{
		cout << str << endl;
	}
	return 0;
}