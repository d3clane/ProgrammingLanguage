CXX = g++-13
CXXFLAGS = -D _DEBUG -ggdb3 -std=c++17 -O0 -Wall -Wextra -Weffc++ -Waggressive-loop-optimizations	  \
		   -Wc++14-compat -Wmissing-declarations -Wcast-align -Wcast-qual -Wchar-subscripts 		  \
		   -Wconditionally-supported -Wconversion -Wctor-dtor-privacy -Wempty-body -Wfloat-equal      \
		   -Wformat-nonliteral -Wformat-security -Wformat-signedness -Wformat=2 -Winline -Wlogical-op \
		   -Wnon-virtual-dtor -Wopenmp-simd -Woverloaded-virtual -Wpacked -Wpointer-arith -Winit-self \
		   -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel 		  \
		   -Wstrict-overflow=2 -Wsuggest-attribute=noreturn -Wsuggest-final-methods 				  \
		   -Wsuggest-final-types -Wsuggest-override -Wswitch-default -Wswitch-enum -Wsync-nand 		  \
		   -Wundef -Wunreachable-code -Wunused -Wuseless-cast -Wvariadic-macros -Wno-literal-suffix   \
		   -Wno-missing-field-initializers -Wno-narrowing -Wno-old-style-cast -Wno-varargs 			  \
		   -Wstack-protector -fcheck-new -fsized-deallocation -fstack-protector -fstrict-overflow 	  \
		   -flto-odr-type-merging -fno-omit-frame-pointer -Wlarger-than=8192 -Wstack-usage=8192 -pie  \
		   -fPIE -Werror=vla -I/Library/Frameworks/SDL2.framework/Headers -F/Library/Frameworks	  	  \
		   -framework SDL2

HOME = $(shell pwd)
CXXFLAGS += -I $(HOME)

TARGET = BackEnd/backEnd.exe
DOXYFILE = Others/Doxyfile

HEADERS  = Tree/Tree.h Tree/Operations.h Common/Colors.h Common/DoubleFuncs.h Common/Errors.h \
		 	Common/Log.h Common/StringFuncs.h FastInput/StringFuncs.h FastInput/InputOutput.h \
			FrontEnd/TokensArr/TokensArr.h FrontEnd/TokensArr/TokenTypes.h FrontEnd/TokensArr/HashFuncs.h FrontEnd/TokensArr/ArrayFuncs.h 			\
			FrontEnd/LexicalParserTokenType	\
			Tree/DSL.h Tree/NameTable/ArrayFuncs.h Tree/NameTable/HashFuncs.h 				\
			Tree/NameTable/NameTable.h Tree/NameTable/NameTableTypes.h FrontEnd/LexicalParser.h		\
			FrontEnd/SyntaxParser.h BackEnd/BackEnd.h
			
FILESCPP = Tree/Tree.cpp Common/DoubleFuncs.cpp \
			Common/Errors.cpp \
		 	Common/Log.cpp Common/StringFuncs.cpp FastInput/StringFuncs.cpp \
			FastInput/InputOutput.cpp BackEnd/main.cpp \
			FrontEnd/TokensArr/ArrayFuncs.cpp FrontEnd/TokensArr/HashFuncs.cpp FrontEnd/TokensArr/TokensArr.cpp \
			Tree/DSL.cpp Tree/NameTable/ArrayFuncs.cpp Tree/NameTable/HashFuncs.cpp	\
			Tree/NameTable/NameTable.cpp FrontEnd/LexicalParser.cpp FrontEnd/SyntaxParser.cpp	\
			BackEnd/BackEnd.cpp 

objects = $(FILESCPP:%.cpp=%.o)

.PHONY: all docs clean buildDirs

all: $(TARGET)

$(TARGET): $(objects) 
	$(CXX) $^ -o $(TARGET) $(CXXFLAGS)

%.o : %.cpp $(HEADERS)
	$(CXX) -c $< -o $@ $(CXXFLAGS) 

docs: 
	doxygen $(DOXYFILE)

clean:
	rm -rf *.o
	rm -rf FastInput/*.o
	rm -rf FrontEnd/*.o
	rm -rf Common/*.o
	rm -rf FrontEnd/TokensArr/*.o
	rm -rf Tree/*.o
	rm -rf Tree/NameTable/*.o
	rm -rf BackEnd/*.o


buildDirs:
	mkdir $(OBJECTDIR)
	mkdir $(PROGRAMDIR)