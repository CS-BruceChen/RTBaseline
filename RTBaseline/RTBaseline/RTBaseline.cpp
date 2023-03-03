#include "RTBaseline.h"
#include <QTextLayout>
#include <QTextBlock>
#include <QDebug>
RTBaseline::RTBaseline(QWidget *parent)
    : QMainWindow(parent)
{
    //init ui
    ui.setupUi(this);
    //set welcome text
    ui.textEdit->setText("@Copyright by brucechen@whu.edu.cn\nWelcome to Raster Trajectory Terminal v0.0.0 ! ");
    ui.textEdit->setWordWrapMode(QTextOption::WrapAnywhere);
    //move cursor to the end;
    startNewLine();
    //connect & install filter
    connect(ui.textEdit, SIGNAL(ui.textEdit->cursorPositionChanged()), SLOT(on_textEdit_cursorPositionChanged()));
    ui.textEdit->installEventFilter(this);
}

void RTBaseline::on_textEdit_cursorPositionChanged() {
    if (ui.textEdit->textCursor().position() < cursorStartPosition) {
        QTextCursor cursor = ui.textEdit->textCursor();
        cursor.setPosition(cursorStartPosition);
        ui.textEdit->setTextCursor(cursor);
    }
}

bool RTBaseline::eventFilter(QObject* object, QEvent* event) {
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Return) {
            processInput();
            return true;//拦截，在processInput()中会自己换行的
        }
        else if (keyEvent->key() == Qt::Key_Backspace) {
            if (ui.textEdit->textCursor().position() == cursorStartPosition) return true;//拦截，不允许删除
            return false;
        }
        else
            return false;
    }
    return false;
}

void RTBaseline::startNewLine() {
    ui.textEdit->append("root:\\> ");
    QTextCursor cursor = ui.textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui.textEdit->setTextCursor(cursor);
    cursorStartPosition = cursor.position();
}

void RTBaseline::processInput() {
    qDebug() << "processInput" << "\n";
    QString strIn = ui.textEdit->toPlainText().sliced(cursorStartPosition);
    Lexer lexer;
    LexerLog lexerlog = lexer.scan(strIn.toStdString());
    if (!lexerlog.isSuccess) {
        ui.textEdit->append(QString::fromStdString(lexerlog.log));
    }
    else {
        lexer.printTokenList();
    }
    //qDebug() << strIn << "\n";
    startNewLine();
}

void Lexer::printTokenList() {
    for (size_t i = 0; i < tokenList.size(); i++)    {
        tokenList[i].printToken();
    }
}

LexerLog Lexer::scan(std::string strIn) {
    LexerLog scanLog;
    for (size_t i = 0; i < strIn.length();i += 0) {
        qDebug() << i << "\n";
        if (isSpace(strIn[i])) {
            ++i;
        }
        else if (isLetter(strIn[i])) {
            pushKeywordToken(strIn, i, scanLog);
            if (!scanLog.isSuccess) break;
        }
        else if (isNumber(strIn[i])) {//
            pushNumberToken(strIn, i);
        }
        else if (isSymbol(strIn[i])) {//[ ] , . " " ( )
            pushSymbolToken(strIn, i);
        }
        else if (isParameter(strIn[i])) {
            pushParameterToken(strIn, i, scanLog);
            if (!scanLog.isSuccess) break;
        }
        else {
            scanLog.set_illegal_identifier_err(i, strIn);
            break;
        }
    }
    return scanLog;
}

bool Lexer::isParameter(char c) {
    return (c == '-');
}

bool Lexer::isParameterSuffix(char c) {
    return (c == 't' || c == 'p' || c == 'q' || c == 'r' || c == 's');
}

TokenType Lexer::getParameterType(char c) {
    if (c == 't') return TokenType::TRAJECTORY;
    else if (c == 'p')return TokenType::POLYGON;
    else if (c == 'q')return TokenType::QUERY_TRAJ;
    else if (c == 'r')return TokenType::RANGE;
    else if (c == 's')return TokenType::SIMILARITY;
    else return TokenType::DEFAULT;
}

void Lexer::pushParameterToken(std::string s, size_t& index_now, LexerLog& log) {
    Token token;
    token.content += s[index_now];
    ++index_now;//cross the "-"
    if (index_now < s.length() && isParameterSuffix(s[index_now])) {//会先计算第一个式子，不满足则不会计算第二个
        token.content += s[index_now];
        token.type = getParameterType(s[index_now]);
        ++index_now;//cross the parameter
    }
    else {
        log.set_illegal_identifier_err(index_now, s);
    }
    tokenList.push_back(token);
}

bool Lexer::isSymbol(char c) {
    return (c =='(' || c == ')' || c == '[' || c == ']' || c == ',' || c == '.' || c == '\"');
}

TokenType Lexer::getSymbolType(char c) {
    if (c == '(') return TokenType::LP;
    else if (c == ')') return TokenType::RP;
    else if (c == '[') return TokenType::LSB;
    else if (c == ']') return TokenType::RSB;
    else if (c == ',') return TokenType::COMMA;
    else if (c == '.') return TokenType::DOT;
    else if (c == '\"') return TokenType::QUOTE;
    else return TokenType::DEFAULT;
}

void Lexer::pushSymbolToken(std::string s, size_t& index_now) {
    Token token;
    token.content = s[index_now];
    token.type = getSymbolType(s[index_now]);
    tokenList.push_back(token);
    ++index_now;
}



bool Lexer::isNumber(char c) {
    int i = c - '0';
    return (0 <= i && i <= 9);
}

void Lexer::pushNumberToken(std::string s, size_t& index_now) {
    Token token;
    while (isNumber(s[index_now])) {
        token.content += s[index_now];
        ++index_now;
    }
    token.type = TokenType::NUMBER;
    tokenList.push_back(token);
}

bool Lexer::isSpace(char c) {
    return (c == ' ');
}

void Lexer::pushKeywordToken(std::string s, size_t& index_now, LexerLog& log) {
    Token token;
    while (isLetter(s[index_now])) {
        token.content += s[index_now];
        ++index_now;
    }
    if (isKeyword(token.content)) {//add select from delete show
        token.type = getKeywordType(token.content);
    }
    else {
        log.set_illegal_identifier_err(index_now, s);
        //break;
    }
    tokenList.push_back(token);
}

bool Lexer::isLetter(char c) {
    int i = c - 'a';
    int j = c - 'A';
    bool isLower = (0 <= i && i <= 25);
    bool isUpper = (0 <= j && j <= 25);
    return (isLower || isUpper);
}

bool Lexer::isKeyword(std::string s) {
    return (s == "add" || s == "select" || s == "from" || s == "delete" || s == "show");
}

TokenType Lexer::getKeywordType(std::string s) {
    if (s == "add") return TokenType::ADD;
    else if (s == "select") return TokenType::SELECT;
    else if (s == "from") return TokenType::FROM;
    else if (s == "delete") return TokenType::DELETE;
    else if (s == "show") return TokenType::SHOW;
    else return TokenType::DEFAULT;
}

void LexerLog::set_illegal_identifier_err(int errPosition,std::string& errStr) {
    errPos = errPosition;
    isSuccess = false;
    log += "\n" + errStr + "\n";
    for (size_t j = 0; j < errStr.length(); ++j) {
        if (j == errPos) log += "^";
        else log += "-";
    }
    log += "\nLexer Error at Position:" + std::to_string(errPos) + "\n";
    qDebug() << QString::fromStdString(log);
}




