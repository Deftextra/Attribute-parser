attributeParser : attributeParser.cpp
	clear
	g++ -Wall -o attributeParser attributeParser.cpp 

run : attributeParser
	clear
	./attributeParser
