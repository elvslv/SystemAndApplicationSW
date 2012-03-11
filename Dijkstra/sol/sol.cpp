#include <iostream>
#include <stack>
#include <map>
#include <vector>
#include <sstream>
#include "math.h"
#include <limits>
#include <float.h>
using namespace std;

stack<char> st;
stack<double> rpnStack;
map<char, int> priority;
vector<string> result;

enum opType
{
	opUndef,
	opNumber,
	opOperator,
	opOpenBracket,
	opCloseBracket,
};

template <class T>
T from_string( const std::string& s, std::ios_base& (*f)(std::ios_base&) )
{
	T t = 0;
	istringstream iss(s);
	if ( (iss >> f >> t).fail() )
	  throw "Invalid operand";
	return t;
}

int main(int argc, char* argv[])
{
	string input = "";
	for ( int i = 1; i < argc; ++i )
	{
		input += argv[i];
	}
	int l = input.length();
	int cnt = 0;
	priority['+'] = priority['-'] = 1;
	priority['*'] = priority['/'] = 2;
	priority['^'] = 3;
	priority['_'] = 4; //unary minus
	opType prevOp = opUndef;
	istringstream iss(input);
	try
	{
		while ( !iss.eof() && cnt < l )
		{
			char ch;
			iss >> ch;
			cnt += 1;
			if ( ch >= '0' && ch <= '9' || ch == '.')
			{
				string num = "";
				if ( !iss.eof() )
				{
					do
					{
						if ( ch == '.' && num.find('.') != -1 )
							throw "Invalid espression";
						num += ch;
						iss.get(ch);
						cnt += 1;
					}while ( !iss.eof() && (ch >= '0' && ch <= '9' || ch == '.') );
					iss.putback(ch);
					cnt -= 1;
				}
				result.push_back(num);
				prevOp = opNumber;
			}
			else if ( ch == '(')
			{
				st.push(ch);
				prevOp = opOpenBracket;

			}
			else if ( ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^' )
			{
				if ( ( ch == '+' || ch == '-' ) && prevOp != opNumber && prevOp != opCloseBracket ) //unary
				{
					char res = '+';
					while( !iss.eof() && ( ch == '+' || ch == '-' ) )
					{
						if ( ch == '-' )
							res = res == '+' ? '-' : '+';
						iss.get(ch);
						cnt += 1;
					}
					if ( iss.eof() )
						throw "Invalid expression";
					iss.putback( ch );
					cnt -= 1;
					if ( res == '-' )
						st.push('_');
				}
				else
				{
					while ( st.size() && (ch == '^' ? 
						priority[ch] < priority[st.top()] : 
						priority[ch] <= priority[st.top()]) )
					{
						result.push_back( string(1, st.top()) );
						st.pop();
					}
					st.push(ch);
					prevOp = opOperator;
				}
			}
			else if ( ch == ')' )
			{
				while ( st.top() != '(' )
				{
					result.push_back( string(1, st.top()) );
					st.pop();
				}
				st.pop();
				prevOp = opCloseBracket;
			} 
			else if ( !(ch == ' ' || ch == '\t' ) )
				throw "Invalid expression";
		}
		while ( st.size() )
		{
			char ch = st.top();
			if ( !(ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^' || ch == '_') )
				throw "Invalid expression";
			result.push_back( string(1, ch) );
			st.pop();
		}
		for ( vector<string>::iterator it  = result.begin(); it != result.end(); ++it )
		{
			if ( it->length() > 1 || (it->at(0) >= '0' && it->at(0) <= '9') )
				rpnStack.push(from_string<double>(*it, std::dec));
			else
			{
				double res;
				double a1 = rpnStack.top();
				rpnStack.pop();
				if ( *it == "_")
				{
					res = -a1;
				}
				else
				{
					double a2 = rpnStack.top();
					rpnStack.pop();
					
					if ( *it == "+" )
						res = a1 + a2;
					else if ( *it == "-" )
						res = a2 - a1;
					else if ( *it == "*" )
						res = a1 * a2;
					else if ( *it == "/" )
					{
						if ( !a1 )
							throw "Division by zero";
						res = a2 / a1;
					}
					else if ( *it == "^" )
						res = pow( a2, a1 );
					else
						throw "Invalid operation";
				}
				if ( _isnan( res ) || !_finite( res ) )
				{
					throw "Precision error";
				}
				rpnStack.push( res );
			}
		}
		if ( rpnStack.size() != 1 )
			throw "Invalid operation";
		cout << rpnStack.top();
	}
	catch ( char* str)
	{
		cout << str << endl;
	}
	return 0;
}