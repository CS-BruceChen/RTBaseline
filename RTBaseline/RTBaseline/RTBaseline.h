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
    //float number
    NUMBER,
    //symbol,
    COMMA,
    DOT,
    QUOTE,
    LSB,
    RSB,
    LP,
    RP,
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
};


struct Lexer {//Lexical analyzer词法分析器
    std::vector<Token> tokenList;
    LexerLog scan(std::string s);
    void printTokenList();
    void pushKeywordToken(std::string s,size_t& index_now, LexerLog& log);
    void pushParameterToken(std::string s, size_t& index_now, LexerLog& log);
    void pushNumberToken(std::string s, size_t& index_now);
    void pushSymbolToken(std::string s, size_t& index_now);
    bool isLetter(char c);
    bool isNumber(char c);
    bool isSymbol(char c);
    bool isSpace(char c);
    bool isKeyword(std::string s);
    bool isParameter(char c);
    bool isParameterSuffix(char c);
    TokenType getKeywordType(std::string s);
    TokenType getSymbolType(char c);
    TokenType getParameterType(char c);
};

struct Parser {//Syntax parser语法解析器

};


