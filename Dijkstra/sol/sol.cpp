#include <iostream>
#include <stack>
#include <map>
#include <vector>
#include <sstream>
#include "math.h"
using namespace std;

stack<char> st;
stack<float> rpnStack;
map<char, int> priority;
vector<string> result;

enum opType
{
	opUndef,
	opNumber,
	opOperator,
	opBracket,
};

template <class T>
T from_string( const std::string& s, std::ios_base& (*f)(std::ios_base&) )
{
	T t = 0;
	std::istringstream iss(s);
	if ( (iss >> f >> t).fail() )
	  throw "Invalid operand";
	return t;
}

int main()
{
	freopen( "input.txt", "r", stdin );
	freopen( "output.txt", "w", stdout );
	priority['+'] = priority['-'] = 1;
	priority['*'] = priority['/'] = 2;
	priority['^'] = 3;
	opType prevOp = opUndef;
	while ( !cin.eof() )
	{
		char ch;
		cin >> ch;
		if ( ch >= '0' && ch <= '9' || ch == '.' || 
			((ch == '-' || ch == '+') && prevOp != opNumber)) //parse float
		{
			string num = "";
			char pref = '+';
			while ( ch == '-' || ch == '+' )
			{
				if ( ch == '-' )
					pref = (pref == '-') ? '+' : '-';
				if ( !cin.eof() )
					cin >> ch;
				else 
					throw "Invalid expression";
			}
			if ( !(ch >= '0' && ch <= '9' || ch == '.') )
				throw "Invalid expresstion";
			if ( pref == '-' )
				num += pref;
			if ( !cin.eof() )
			{
				do
				{
					if ( ch == '.' && num.find('.') != -1 )
						throw "Invalid exception";
					num += ch;
					cin.get(ch);
				}while ( !cin.eof() && (ch >= '0' && ch <= '9' || ch == '.') );
				cin.putback(ch);
			}
			result.push_back(num);
			prevOp = opNumber;
		}
		else if ( ch == '(')
		{
			st.push(ch);
			prevOp = opBracket;

		}
		else if ( ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^' )
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
		else if ( ch == ')' )
		{
			while ( st.top() != '(' )
			{
				result.push_back( string(1, st.top()) );
				st.pop();
			}
			st.pop();
			prevOp = opBracket;

		} 
		else if ( !(ch == ' ' || ch == '\t' ) )
			throw "Invalid expression";
	}
	while ( st.size() )
	{
		char ch = st.top();
		if ( !(ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '^') )
			throw "Invalid expression";
		result.push_back( string(1, ch) );
		st.pop();
	}
	for ( vector<string>::iterator it  = result.begin(); it != result.end(); ++it )
	{
		if ( it->length() > 1 || (it->at(0) >= '0' && it->at(0) <= '9') )
			rpnStack.push(from_string<float>(*it, std::dec));
		else
		{
			float a1 = rpnStack.top();
			rpnStack.pop();
			float a2 = rpnStack.top();
			rpnStack.pop();
			float res;
			if ( *it == "+" )
				res = a1 + a2;
			else if ( *it == "-" )
				res = a2 - a1;
			else if ( *it == "*" )
				res = a1 * a2;
			else if ( *it == "/" )
				res = a2 / a1;
			else if ( *it == "^" )
				res = pow( a2, a1 );
			else
				throw "Invalid operation";
			rpnStack.push( res );
		}
	}
	if ( rpnStack.size() != 1 )
		throw "Invalid operation";
	cout << rpnStack.top();
	fclose( stdin );
	fclose( stdout );
	return 0;
}