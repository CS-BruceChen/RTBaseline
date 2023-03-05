#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_RTBaseline.h"
#include <QEvent>
#include <QKeyEvent>
#include <fstream>
class RTBaseline : public QMainWindow
{
    Q_OBJECT

public:
    RTBaseline(QWidget *parent = Q_NULLPTR);
private:
    Ui::RTBaselineClass ui;
    int cursorStartPosition;
    std::vector<QString> historyStr;
    size_t hisIndex;
    bool isNeverHitUp;
    void processInput();
    void startNewLine();
    void processUp();
    void processDown();
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
    //traj or polygon or id sequence
    SEQUENCE,
    //default
    DEFAULT,
};

struct Token {
    std::string content;
    TokenType type;
    Token():content(""),type(DEFAULT) {};
    void printToken() {
        qDebug() << "Token:( " << QString::fromStdString(content) << ", " << type << " )\n";
    }
};

struct Log {
    std::string log;
    bool isSuccess;
    int errPos;
    Log() :log(""), isSuccess(true), errPos(0) {};
    void set_illegal_identifier_err(int errPosition, std::string& errStr);
    void set_illegal_identifier_err(int errPosition_begin, int errPosition_end, std::string& errStr);
    void set_matching_symbol_missing_err(int errPosition, std::string& errStr);
    void set_wrong_add_command_format_err();
    void set_wrong_select_command_format_err();
    void set_wrong_delete_command_format_err();
    void set_wrong_show_command_format_err();
    void set_wrong_help_command_format_err();
    void set_wrong_data_format_err(int errPosition, std::string& errStr);
    void set_wrong_command_err();
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
    void pushKeywordToken(std::string s,size_t& index_now);
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

struct Point {
    double x, y;
    Point() { x = 0; y = 0; }
    Point(double xx, double yy) { x = xx; y = yy; }
    Point(const Point& pt) { x = pt.x; y = pt.y; };
    template<class T,class U> Point(T in_x,U in_y){x = static_cast<double>(in_x); y = static_cast<double>(in_y);}
};

typedef std::vector<Point> Sequence;
typedef std::vector<size_t> IDArray;

class PrimitiveDB {
    //default constructor
private:
    static std::vector<Sequence> TDBArray;
    static std::vector<Sequence> PDBArray;
    static std::vector<Sequence> QDBArray;
public:
    virtual void ADD(std::vector<Sequence> seqArray) = 0;
    virtual void DELETE(IDArray ids) = 0;
    virtual IDArray SELECT(IDArray ids1,IDArray ids2,int topk) = 0;
    virtual void SHOW(IDArray ids) = 0;
    virtual void PRINT(IDArray result_ids) = 0;
};

class TrajectoryDB :public PrimitiveDB {
public:
    void ADD(std::vector<Sequence> seqArray);
    void DELETE(IDArray ids);
    IDArray SELECT(IDArray tids,IDArray pids,int topk);
    void SHOW(IDArray ids);
    void PRINT(IDArray result_ids);
};

class QueryTrajDB : public PrimitiveDB {
public:
    void ADD(std::vector<Sequence> seqArray);
    void DELETE(IDArray ids);
    IDArray SELECT(IDArray tids,IDArray qids,int topk);
    void SHOW(IDArray ids);
    void PRINT(IDArray result_ids);
};

class PolygonDB : public PrimitiveDB {
public:
    void ADD(std::vector<Sequence> seqArray);
    void DELETE(IDArray ids);
    IDArray SELECT(IDArray pids,IDArray tids,int topk);
    void SHOW(IDArray ids);
    void PRINT(IDArray result_ids);
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
    

};


