#pragma once
#include "PrimitiveDB.h"
#include <fstream>
#include "Log.h"
enum TokenType
{
    //keyword,
    ADD,
    SELECT,
    FROM,
    DELETE,
    SHOW,
    HELP,
    PRINT,
    //parameter,
    TRAJECTORY,
    POLYGON,
    QUERY_TRAJ,
    RANGE,
    SIMILARITY,
    TOPK,
    //float number
    NUMBER,
    //symbol,
    COMMA,
    DOT,
    LP,
    RP,
    //filepath
    PATH,
    //traj or polygon or id sequence
    SEQUENCE,
    //default
    DEFAULT,
};

struct Token {
    std::string content;
    TokenType type;
    Token() :content(""), type(DEFAULT) {};
    void printToken() {
        qDebug() << "Token:( " << QString::fromStdString(content) << ", " << type << " )\n";
    }
};

typedef std::vector<Token> TokenList;



struct Lexer {//Lexical analyzer词法分析器
public://其实应该把LexerLog作为一个数据对象的
    void printTokenList();
    TokenList getTokenList() const { return tokenList; }
    void scan(std::string s);
    Log getLexerLog() const { return LexerLog; }
private:
    TokenList tokenList;
    Log LexerLog;

private:
    void pushKeywordToken(std::string s, size_t& index_now);
    void pushParameterToken(std::string s, size_t& index_now);
    void pushPathToken(std::string s, size_t& index_now);
    void pushSequenceToken(std::string s, size_t& index_now);
    void pushNumberToken(std::string s, size_t& index_now);
    void pushCommaToken(std::string s, size_t& index_now);
    bool isLetter(char c);
    bool isNumber(char c);
    bool isComma(char c);
    bool isSpace(char c);
    bool isKeyword(std::string s);
    bool isParameter(char c);
    bool isParameterSuffix(char c);
    bool isQuote(char c);
    bool isLSB(char c);
    bool isRSB(char c);
    TokenType getKeywordType(std::string s);
    TokenType getParameterType(char c);
};

struct Parser {//Syntax parser语法解析器
public:
    void parse(TokenList tokenList);
    Log getParserLog() const { return parserLog; }
private:
    TrajectoryDB tdb;
    PolygonDB pdb;
    QueryTrajDB qdb;
    Log parserLog;
    enum Command {
        CADDS,//add from sequence
        CADDP,//add from filePath
        CSLT,//select
        CDEL,//delete
        CSHOW,//show
        CHELP,//help
        CPRTA,//print all
        CPRTD,//print form ID
        CERR//erro
    };
private:
    //void parseSequence(TokenList tokenList);
    Command fetchCommand(TokenList tokenList);
    Sequence fetchDataFromInput(std::string inputSequence);
    std::vector<Sequence> fetchDataFromFile(std::string filepath);
    IDArray fectchIDFromInput(std::string inputIDArray);
    void doSelectInDB(PrimitiveDB* db, std::string str1, std::string str2);
    void doSelectInDB(PrimitiveDB* db, std::string str1, std::string str2, int topk);
    void processAddFromFileCommand(TokenList tokenList);
    void processAddFromSequenceCommand(TokenList tokenList);
    void processSelectCommand(TokenList tokenList);
    void processDeleteCommand(TokenList tokenList);
    void processShowCommand(TokenList tokenList);
    void processHelpCommand(TokenList tokenList);
    void processPrintAllCommand(TokenList tokenList);
    void processPrintFromIdCommand(TokenList tokenList);
};