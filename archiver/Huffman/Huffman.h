#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <queue>
#include "../Common.h"

using namespace std;

typedef pair<int, int> metadataCode;

class Node
{
public:
	Node( ){}

	Node( Node* l, Node* r, vector<unsigned char>* sym, int f ): left( l ), right( r ), symbols( sym ), freq( f ){}

	int getFreq( )
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
	int freq;
};


class Huffman
{
public:
	Huffman( ){};
protected:
	void initQueue( );

	vector<FileInfo> fileInfos;	
	int freq[1 << BITS_IN_BYTE];
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
	int numOfBytes;
	(vector<int> chCodes)[1 << BITS_IN_BYTE];
	metadataCode metadataCodes[1 << BITS_IN_BYTE];
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
	int pos;
	int bytesDecoded;
};
#endif //HUFFMAN_H