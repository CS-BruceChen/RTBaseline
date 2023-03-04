#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_RTBaseline.h"
#include <QEvent>
#include <QKeyEvent>
class RTBaseline : public QMainWindow
{
    Q_OBJECT

public:
    RTBaseline(QWidget *parent = Q_NULLPTR);
private:
    Ui::RTBaselineClass ui;
    int max_line;
    int cursorStartPosition;
    void processInput();
    void startNewLine();
private slots:
    void on_textEdit_cursorPositionChanged();
protected:
    bool eventFilter(QObject* object, QEvent* event);
};

enum TokenType
{
    //keyword,
    ADD,
    SELECT,
    FROM,
    DELETE,
    SHOW,
    HELP,
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
    //traj or polygon sequence
    SEQUENCE,
    //default
    DEFAULT,
};

struct Token {
    std::string content;
    TokenType type;
    Token():content(""),type(TokenType::DEFAULT) {};
    void printToken() {
        qDebug() << "Token:( " << QString::fromStdString(content) << ", " << type << " )\n";
    }
};

struct LexerLog {
    std::string log;
    bool isSuccess;
    int errPos;
    LexerLog() :log(""), isSuccess("true"), errPos(0) {};
    void set_illegal_identifier_err(int errPosition, std::string& errStr);
    void set_illegal_identifier_err(int errPosition_begin, int errPosition_end, std::string& errStr);
    void set_matching_symbol_missing_err(int errPosition, std::string& errStr);
};

typedef std::vector<Token> TokenList;

struct Lexer {//Lexical analyzer词法分析器
public:
    void printTokenList();
    TokenList getTokenList() const { return tokenList; };
    LexerLog scan(std::string s);

private:
    TokenList tokenList;
    
private:
    void pushKeywordToken(std::string s,size_t& index_now, LexerLog& log);
    void pushParameterToken(std::string s, size_t& index_now, LexerLog& log);
    void pushPathToken(std::string s, size_t& index_now, LexerLog& log);
    void pushSequenceToken(std::string s, size_t& index_now, LexerLog& log);
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

struct Point {
    double x, y;
    Point() { x = 0; y = 0; }
    Point(double xx, double yy) { x = xx; y = yy; }
    Point(const Point& pt) { x = pt.x; y = pt.y; };
    template<class T,class U> Point(T in_x,U in_y){x = static_cast<double>(in_x); y = static_cast<double>(in_y);}
};

typedef std::vector<Point> Sequence;

class PrimitiveDB {
    //default constructor
private:
    std::vector<Sequence> DBArray;
public:
    void ADD(std::vector<Sequence> PrimitiveData);
    void DELETE();
public:
    virtual void SELECT();
    virtual void SHOW();
};

class TrajectoryDB :public PrimitiveDB {};
class QueryTrajDB : public PrimitiveDB {};
class PolygonDB : public PrimitiveDB {
public:
    void SELECT();
    void SHOW();
};


struct Parser {//Syntax parser语法解析器
    void parse(TokenList tokenList);
private:
    bool isAddCommand(TokenList tokenList);
    bool isSelectCommand(TokenList tokenList);
    bool isDeleteCommand(TokenList tokenList);
    bool isShowCommand(TokenList tokenList);
    bool isHelpCommand(TokenList tokenList);
    void parseSequence(TokenList tokenList);
    Sequence fetchDataFromInput(std::string inputSequence);//
    std::vector<Sequence> fetchDataFromFile(std::string filepath);
    void processAddCommand(TokenList tokenList);
    void processSelectCommand(TokenList tokenList);
    void processDeleteCommand(TokenList tokenList);
    void processShowCommand(TokenList tokenList);
    void processHelpCommand(TokenList tokenList);
private:
    TrajectoryDB tdb;
    PolygonDB pdb;
    QueryTrajDB qdb;
};


