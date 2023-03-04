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
    if (lexerlog.isSuccess) {
        Parser parser;
        parser.parse(lexer.getTokenList());
    }
    else {
        ui.textEdit->append(QString::fromStdString(lexerlog.log));
    }
    startNewLine();
}

/*
* Lexer Implementation
*/

void Lexer::printTokenList() {
    for (size_t i = 0; i < tokenList.size(); i++)    {
        tokenList[i].printToken();
    }
}

LexerLog Lexer::scan(std::string strIn) {
    LexerLog scanLog;
    for (size_t i = 0; i < strIn.length();i += 0) {
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
        else if (isComma(strIn[i])) {//,
            pushCommaToken(strIn, i);
        }
        else if (isParameter(strIn[i])) {
            pushParameterToken(strIn, i, scanLog);
            if (!scanLog.isSuccess) break;
        }
        else if (isQuote(strIn[i])) {// "
            pushPathToken(strIn, i, scanLog);
            if (!scanLog.isSuccess) break;
        }
        else if (isLSB(strIn[i])) {// [
            pushSequenceToken(strIn, i, scanLog);
            if (!scanLog.isSuccess) break;
        }

        else {
            scanLog.set_illegal_identifier_err(i, strIn);
            break;
        }
    }
    return scanLog;
}

bool Lexer::isLSB(char c) {
    return (c == '[');
}

bool Lexer::isRSB(char c) {
    return (c == ']');
}

void Lexer::pushSequenceToken(std::string s, size_t& index_now, LexerLog& log) {
    Token token;
    ++index_now;//cross the LSB
    while (index_now < s.length() && (!isRSB(s[index_now]))) {
        token.content += s[index_now];
        ++index_now;
    }
    if (index_now >= s.length()) {//miss the last RSB
        log.set_matching_symbol_missing_err(index_now - token.content.length() - 1, s);
    }
    else {
        ++index_now;//cross the RSB
        token.type = TokenType::SEQUENCE;
        tokenList.push_back(token);
    }
}


bool Lexer::isQuote(char c) {
    return (c == '\"');
}

void Lexer::pushPathToken(std::string s, size_t& index_now, LexerLog& log) {
    Token token;
    ++index_now;//cross the first quote
    while (index_now < s.length() && (!isQuote(s[index_now]))) {
        token.content += s[index_now];
        ++index_now;
    }
    if (index_now >= s.length()) {//miss the last quote
        log.set_matching_symbol_missing_err(index_now-token.content.length()-1, s);
    }
    else {
        ++index_now;//cross the last quote
        token.type = TokenType::PATH;
        tokenList.push_back(token);
    }
    
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
    else if (index_now < s.length() && isNumber(s[index_now])) {
        while (index_now < s.length() && isNumber(s[index_now])) {
            token.content += s[index_now];
            ++index_now;
        }
        token.type = TokenType::TOPK;
    }
    else {
        log.set_illegal_identifier_err(index_now, s);
    }
    tokenList.push_back(token);
}

bool Lexer::isComma(char c) {
    return (c == ',');
}

void Lexer::pushCommaToken(std::string s, size_t& index_now) {
    Token token;
    token.content = s[index_now];
    token.type = TokenType::COMMA;
    tokenList.push_back(token);
    ++index_now;
}



bool Lexer::isNumber(char c) {
    int i = c - '0';
    return (0 <= i && i <= 9);
}

void Lexer::pushNumberToken(std::string s, size_t& index_now) {
    Token token;
    while (isNumber(s[index_now]) && index_now < s.length()) {
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
    while (isLetter(s[index_now])&&index_now<s.length()) {
        token.content += s[index_now];
        ++index_now;
    }
    if (isKeyword(token.content)) {//add select from delete show
        token.type = getKeywordType(token.content);
    }
    else {
        log.set_illegal_identifier_err(index_now-token.content.length(), index_now, s);
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
    return (s == "add" || s == "select" || s == "from" || s == "delete" || s == "show" || s== "help");
}

TokenType Lexer::getKeywordType(std::string s) {
    if (s == "add") return TokenType::ADD;
    else if (s == "select") return TokenType::SELECT;
    else if (s == "from") return TokenType::FROM;
    else if (s == "delete") return TokenType::DELETE;
    else if (s == "show") return TokenType::SHOW;
    else if (s == "help") return TokenType::HELP;
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
    log += "\nLexer Error: <illegal identifier> at Position:" + std::to_string(errPos) + "\n";
    qDebug() << QString::fromStdString(log);
}

void LexerLog::set_illegal_identifier_err(int errPosition_begin, int errPosition_end, std::string& errStr) {
    errPos = errPosition_begin;
    isSuccess = false;
    log += "\n" + errStr + "\n";
    for (size_t j = 0; j < errStr.length(); ++j) {
        if (errPosition_begin <= j && j < errPosition_end) log += "^";
        else log += "-";
    }
    log += "\nLexer Error: <illegal identifier> at Position:" + std::to_string(errPos) + "\n";
    qDebug() << QString::fromStdString(log);
}

void LexerLog::set_matching_symbol_missing_err(int errPosition, std::string& errStr) {
    errPos = errPosition;
    isSuccess = false;
    log += "\n" + errStr + "\n";
    for (size_t j = 0; j < errStr.length(); ++j) {
        if (j == errPos) log += "^";
        else log += "-";
    }
    log += "\nLexer Error: <matching symbol missing> at Position:" + std::to_string(errPos) + "\n";
    qDebug() << QString::fromStdString(log);
}
/*
* Parser Implementation
*/

void Parser::parse(TokenList tokenList) {
    if (tokenList.size() == 0) {
        qDebug() << "empty command\n";
    }
    else if (isAddCommand(tokenList)) {
        qDebug() << "add command\n";
    }
    else if (isSelectCommand(tokenList)) {
        qDebug() << "select command\n";
    }
    else if (isDeleteCommand(tokenList)) {
        qDebug() << "delete command\n";
    }
    else if (isShowCommand(tokenList)) {
        qDebug() << "show command\n";
    }
    else if(isHelpCommand(tokenList)){
        qDebug() << "help command\n";
    }
    else {
        qDebug() << "Invalid Command\n";
    }
    
}

bool Parser::isAddCommand(TokenList tokenList) {
    if (tokenList[0].type == TokenType::ADD
        && tokenList.size() >= 3
        && (tokenList[1].type == TokenType::TRAJECTORY || tokenList[1].type == TokenType::POLYGON || tokenList[1].type == TokenType::QUERY_TRAJ)
        && (tokenList[2].type == TokenType::SEQUENCE || (tokenList[2].type == TokenType::PATH && tokenList.size() == 3))) {
        //读取seq类型的token，并且是，每遇到一个comma才会向下再读一位。非法的token则报错
        return true;
    }
    else if (tokenList[0].type == TokenType::ADD) {
        qDebug()
            << "Wrong Format Error: The format of the add command is as follows:\n"
            << "\tadd -t [(x0,y0),(x1,y1),...]\n"
            << "\tadd -t \"your/file/path\"\n"
            << "\tadd -p [(x0,y0),(x1,y1),...]\n"
            << "\tadd -p \"your/file/path\"\n"
            << "\tadd -q [(x0,y0),(x1,y1),...]\n"
            << "\tadd -q \"your/file/path\"\n"
            << "\tfor more information, please input help\n";
        return false;
    }
    else return false;
}

bool Parser::isSelectCommand(TokenList tokenList) {
    if (tokenList[0].type == TokenType::SELECT && (tokenList.size() == 5 || tokenList.size() == 6 || tokenList.size() == 7)) {
        if (tokenList[1].type == TokenType::TOPK) {
            if (tokenList[2].type == TokenType::RANGE
                && (tokenList[3].type == TokenType::TRAJECTORY || tokenList[3].type == TokenType::POLYGON)
                && tokenList[4].type == TokenType::SEQUENCE
                && tokenList[5].type == TokenType::FROM
                && tokenList[6].type == TokenType::SEQUENCE) {
                return true;
            }
            else if (tokenList[2].type == TokenType::SIMILARITY
                && tokenList[3].type == TokenType::SEQUENCE
                && tokenList[4].type == TokenType::FROM
                && tokenList[5].type == TokenType::SEQUENCE) {
                return true;
            }
        }
        else if (tokenList[1].type == TokenType::RANGE
            && (tokenList[2].type == TokenType::TRAJECTORY || tokenList[3].type == TokenType::POLYGON)
            && tokenList[3].type == TokenType::SEQUENCE
            && tokenList[4].type == TokenType::FROM
            && tokenList[5].type == TokenType::SEQUENCE) {
            return true;
        }
        else if (tokenList[1].type == TokenType::SIMILARITY
            && tokenList[2].type == TokenType::SEQUENCE
            && tokenList[3].type == TokenType::FROM
            && tokenList[4].type == TokenType::SEQUENCE) {
            return true;
        }
    }
    else if (tokenList[0].type == TokenType::SELECT) {
        qDebug()
            << "Wrong Format Error: The format of the select command is as follows:\n"
            << "\tselect -<top k num> -r -t [tid0,tid1,...] from [[pid0,pid1,...]\n"
            << "\tselect -<top k num> -r -p [pid0,pid1,...] from [tid0,tid1,...]\n"
            << "\tselect -<top k num> -s [tid0,tid1,...] from [qid0,qid1,...]\n"
            << "\tselect -r -t [tid0,tid1,...] from [pid0,pid1,...]\n"
            << "\tselect -r -p [pid0,pid1,...] from [tid0,tid1,...]\n"
            << "\tselect -s [tid0,tid1,...] from [qid0,qid1,...]\n"
            << "\tfor more information, please input help\n";
        return false;
    }
    else return false;
}

bool Parser::isDeleteCommand(TokenList tokenList) {
    if (tokenList[0].type == TokenType::DELETE
        && tokenList.size() == 3
        && (tokenList[1].type == TokenType::TRAJECTORY || tokenList[1].type == TokenType::POLYGON || tokenList[1].type == TokenType::QUERY_TRAJ)
        && (tokenList[2].type == TokenType::SEQUENCE)) {
        return true;
    }
    else if (tokenList[0].type == TokenType::DELETE) {
        qDebug()
            << "Wrong Format Error: The format of the delete command is as follows:\n"
            << "\tdelete -t [tid0,tid1,...]\n"
            << "\tdelete -p [pid0,pid1,...]\n"
            << "\tdelete -q [qid0,qid1,...]\n";
        return false;
    }
    else return false;
}

bool Parser::isShowCommand(TokenList tokenList) {
    if (tokenList[0].type == TokenType::SHOW
        && tokenList.size() == 3
        && (tokenList[1].type == TokenType::TRAJECTORY || tokenList[1].type == TokenType::POLYGON || tokenList[1].type == TokenType::QUERY_TRAJ)
        && (tokenList[2].type == TokenType::SEQUENCE)) {
        return true;
    }
    else if (tokenList[0].type == TokenType::SHOW) {
        qDebug()
            << "Wrong Format Error: The format of the show command is as follows:\n"
            << "\tshow -t [tid0,tid1,...]\n"
            << "\tshow -p [pid0,pid1,...]\n"
            << "\tshow -q [qid0,qid1,...]\n";
        return false;
    }
    else return false;
}

bool Parser::isHelpCommand(TokenList tokenList) {
    if (tokenList[0].type == TokenType::HELP && tokenList.size() == 1) {
        return true;
    }
    else if (tokenList[0].type == TokenType::HELP) {
        qDebug()
            << "Wrong Format Error: The format of the help command is as follows:\n"
            << "\thelp\n";
        return false;
    }
    else return false;
}


//设定：所有的合法性检查都在isxxx函数里面设计好，这里就认为所有的token序列格式都是合法的，不用考虑非法序列错误
void Parser::processAddCommand(TokenList tokenList) {
    TokenType addParameter = tokenList[1].type;
    if (tokenList[2].type == TokenType::PATH) {
        std::vector<Sequence> seqArray = fetchDataFromFile(tokenList[2].content);
        switch (addParameter) {
        case TRAJECTORY: tdb.ADD(seqArray); break;
        case POLYGON: pdb.ADD(seqArray); break;
        case QUERY_TRAJ: qdb.ADD(seqArray); break;
        default: break;
        }
        return;
    }
    else {
        size_t i = 2;
        std::vector<Sequence> seqArray;
        while (i < tokenList.size()) {
            if (tokenList[i].type == TokenType::SEQUENCE) {
                seqArray.push_back(fetchDataFromInput(tokenList[i].content));
            }
            ++i;
        }
        switch (addParameter) {
        case TRAJECTORY: tdb.ADD(seqArray); break;
        case POLYGON: pdb.ADD(seqArray); break;
        case QUERY_TRAJ: qdb.ADD(seqArray); break;
        default: break;
        }
        return;
    }
    
}

