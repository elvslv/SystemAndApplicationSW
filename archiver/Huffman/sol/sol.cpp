#include <iostream>
#include <fstream>
#include <queue>
#include <string>
#include <map>
#include <vector>

using namespace std;

typedef pair<int, int> metadataCode;

const int BITS_IN_BYTE = 8;
const int LAST_BYTE = (1 << BITS_IN_BYTE) - 1;
const int BYTES_IN_INT = 4;

struct FileInfo
{
	FileInfo( int fnl, string fn, int nb ): fileNameLength( fnl ), fileName( fn ), numOfBytes( nb ){};
	int fileNameLength;
	string fileName;
	int numOfBytes;
};

class Node
{
public:
	Node( ){};
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

class Huffman
{
public:
	Huffman(){};


protected:
	void initQueue()
	{	
		int num = 0;
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
		for ( unsigned int i = 0; i < num - 1; ++i )
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

	vector<FileInfo> outFiles;	
	int freq[1 << BITS_IN_BYTE];
	priority_queue<Node*, vector<Node*>, Node> pq;
};

ofstream os;

class Encoder: public Huffman
{
public:
	Encoder( vector<string>* fns, string outputfns ): fileNames(fns), outputFileName(outputfns)
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
		memset( freq, 0, 256 * sizeof(int) );
	}

	int encode()
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
private:
	void countFrequency()
	{	
		for ( string::iterator it = input.begin(); it != input.end(); ++it )
		{
			++freq[(unsigned char)*it]; 
		}
	}

	void buildCodes( )
	{
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

	void getResult( )
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

	void printMetadata( )
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

	vector<string>* fileNames;
	string outputFileName;
	string input;
	int numOfBytes;
	(vector<int> chCodes)[256];
	metadataCode metadataCodes[256];
	queue<unsigned char> result;
};

class Decoder: public Huffman
{
public:
	Decoder( string infile ): inFileName(infile)
	{
		bytesDecoded = 0;
		pos = 0;
		readFile( inFileName, input );
	}

	int decode( )
	{
		fillMetadata( );
		getFrequences( );
		initQueue( );
		input = string( input.c_str() + pos, input.length() - pos );
		if ( getResult( ) != 0 )
			throw "Archive is defective";
	}
private:
	void fillMetadata( )
	{
		int filesNum = getInt( input.c_str() ); 
		pos += 4;
		for ( int i = 0; i < filesNum; ++i )
		{
			int fileNameLength = getInt( input.c_str() + pos );
			pos += 4;
			string fileName = input.substr( pos, fileNameLength );
			pos += fileNameLength;
			int fileLength = getInt( input.c_str() + pos );
			pos += 4;
			outFiles.push_back( FileInfo( fileNameLength, fileName, fileLength ) );
		}
	}

	void getFrequences( )
	{
		for ( unsigned int i = 0; i < 256; ++i )
		{
			unsigned int fr = getInt( input.c_str() + pos );
			if ( i == 255 )
				i = i;
			pos += 4;
			freq[i] = fr;
		}
	}

	int getResult( )
	{
		int nextsym = 0;
		Node* root = pq.top();
		int j = BITS_IN_BYTE - 1;
		string::iterator it = input.begin();
		vector<FileInfo>::iterator curFile = outFiles.begin();
		ofstream os;
		os.open( curFile->fileName.c_str(), ios::binary );
		unsigned char sym = *it++;
		queue<int> result_bits1;
		while ( it != input.end() && curFile != outFiles.end())
		{
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
				++curFile;
				os.close();
				if ( curFile != outFiles.end() )
					os.open( curFile->fileName.c_str(), ios::binary );
			}
			root = pq.top();
			if ( j < 0 )
			{
				j = BITS_IN_BYTE - 1;
				sym = *it++;
			}
			if ( curFile == outFiles.end() && it != input.end() )
			{
				//throw "Archive is defective";
			}
		}
		while ( curFile != outFiles.end() && bytesDecoded < curFile->numOfBytes && j >= 0)
		{
			while ( root->getLeft() && root->getRight() )
			{
				int b = ( sym & ( 1 << j ) ) >> j;
				result_bits1.push(b);
				root = b ? root->getRight() : root->getLeft();
				--j;
			}
			if ( ++bytesDecoded == curFile->numOfBytes )
			{
				bytesDecoded = 0;
				++curFile;
				os.close();
			}
			char ch = root->getSymbols()->front();
			os << ch;
		}
		if ( curFile != outFiles.end() )
		{
			//throw "Archive is defective";
		}
		return 0;
	}

	string inFileName;
	string input;
	int pos;
	int bytesDecoded;
};

int main( )
{
	vector<string>fileNames;
	fileNames.push_back( "input.txt" );
	//Encoder encoder = Encoder( &fileNames, "output.txt" );
	//encoder.encode();
	Decoder decoder = Decoder( "output.txt" );
	decoder.decode();
	return 0;
}