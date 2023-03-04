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


struct Lexer {//Lexical analyzer词法分析器
public:
    void printTokenList();
    std::vector<Token> getTokenList() const { return tokenList; };
    LexerLog scan(std::string s);

private:
    std::vector<Token> tokenList;
    
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

struct Parser {//Syntax parser语法解析器
    void parse(std::vector<Token> tokenList);
};


