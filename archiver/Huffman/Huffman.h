#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <queue>
#include "../Common.h"

using namespace std;

class Node
{
public:
	Node( ){}

	Node( Node* l, Node* r, vector<unsigned char>* sym, intType f ): left( l ), right( r ), symbols( sym ), freq( f ){}

	intType getFreq( )
	{
		return freq;
	}

	bool operator( )(Node* x, Node* y)
    {     
        return x->getFreq() > y->getFreq();
    }

	vector<unsigned char>* getSymbols( )
	{
		return symbols;
	}

	Node* getLeft( )
	{
		return left;
	}

	Node* getRight( )
	{
		return right;
	}

private:
	Node* left;
	Node* right;
	vector<unsigned char>* symbols;
	intType freq;
};


class Huffman
{
public:
	Huffman( ){};
protected:
	void initQueue( );

	vector<FileInfo> fileInfos;	
	intType freq[1 << BITS_IN_BYTE];
	priority_queue<Node*, vector<Node*>, Node> pq;
};

class HuffmanEncoder: public Huffman, public Encoder
{
public:
	HuffmanEncoder( vector<string>* fns, string outputfns );
	int encode( );
private:
	void countFrequency( );
	void buildCodes( );
	void getResult( );
	void printMetadata( );

	vector<string>* fileNames;
	string outputFileName;
	string input;
	intType numOfBytes;
	(vector<int> chCodes)[1 << BITS_IN_BYTE];
	queue<unsigned char> result;
	bool several;
};

class HuffmanDecoder: public Huffman, public Decoder
{
public:
	HuffmanDecoder( string infile, string d );
	int decode( );
private:
	void fillMetadata( );
	void getFrequences( );
	int getResult( );

	string inFileName;
	string dir;
	string input;
	intType pos;
	intType bytesDecoded;
};
#endif //HUFFMAN_H