#include "RTBaseline.h"
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
    //
    hisIndex = 0;
    isNeverHitUp = true;
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
        else if (keyEvent->key() == Qt::Key_Up) {
            processUp();
            return true;//拦截
        }
        else if (keyEvent->key() == Qt::Key_Down) {
            processDown();
            return true;//拦截
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
    //qDebug() << "processInput" << "\n";
    QString strIn = ui.textEdit->toPlainText().sliced(cursorStartPosition);
    historyStr.push_back(strIn);
    hisIndex = historyStr.size() - 1;
    isNeverHitUp = true;
    Lexer lexer;
    lexer.scan(strIn.toStdString());
    Log lexerlog = lexer.getLexerLog();
    if (lexerlog.isSuccess) {
        Parser parser;
        parser.parse(lexer.getTokenList());
        Log parserLog = parser.getParserLog();
        if (!parserLog.isSuccess) {
            ui.textEdit->append(QString::fromStdString(parserLog.log));
        }
    }
    else {
        ui.textEdit->append(QString::fromStdString(lexerlog.log));
    }
    startNewLine();
}
//写的有点乱，不太擅长边界控制
void RTBaseline::processUp() {
    if (isNeverHitUp && historyStr.size() != 0) {
        ui.textEdit->setPlainText(ui.textEdit->toPlainText().sliced(0, cursorStartPosition)+ historyStr[hisIndex]);
        isNeverHitUp = false;
    }
    else if (hisIndex > 0) {
        hisIndex = hisIndex - 1;
        QString prefix = ui.textEdit->toPlainText().sliced(0, cursorStartPosition);
        QString strReplace = prefix + historyStr[hisIndex];
        ui.textEdit->setPlainText(strReplace);
    }
    else if (hisIndex == 0) {
        QString prefix = ui.textEdit->toPlainText().sliced(0, cursorStartPosition);
        QString strReplace = prefix + historyStr[hisIndex];
        ui.textEdit->setPlainText(strReplace);
    }
}
void RTBaseline::processDown() {
    if (historyStr.size() != 0 && hisIndex < historyStr.size() - 1) {
        hisIndex = hisIndex + 1;
        QString prefix = ui.textEdit->toPlainText().sliced(0, cursorStartPosition);
        QString strReplace = prefix + historyStr[hisIndex];
        ui.textEdit->setPlainText(strReplace);
    }
    else if (hisIndex == historyStr.size() - 1) {
        QString prefix = ui.textEdit->toPlainText().sliced(0, cursorStartPosition);
        QString strReplace = prefix + historyStr[hisIndex];
        ui.textEdit->setPlainText(strReplace);
    }
}

/*
* Lexer Implementation
*/

void Lexer::printTokenList() {
    for (size_t i = 0; i < tokenList.size(); i++)    {
        tokenList[i].printToken();
    }
}

void Lexer::scan(std::string strIn) {
    for (size_t i = 0; i < strIn.length();i += 0) {
        if (isSpace(strIn[i])) {
            ++i;
        }
        else if (isLetter(strIn[i])) {
            pushKeywordToken(strIn, i);
            if (!LexerLog.isSuccess) break;
        }
        else if (isNumber(strIn[i])) {//
            pushNumberToken(strIn, i);
        }
        else if (isComma(strIn[i])) {//,
            pushCommaToken(strIn, i);
        }
        else if (isParameter(strIn[i])) {
            pushParameterToken(strIn, i);
            if (!LexerLog.isSuccess) break;
        }
        else if (isQuote(strIn[i])) {// "
            pushPathToken(strIn, i);
            if (!LexerLog.isSuccess) break;
        }
        else if (isLSB(strIn[i])) {// [
            pushSequenceToken(strIn, i);
            if (!LexerLog.isSuccess) break;
        }

        else {
            LexerLog.set_illegal_identifier_err(i, strIn);
            break;
        }
    }
}

bool Lexer::isLSB(char c) {
    return (c == '[');
}

bool Lexer::isRSB(char c) {
    return (c == ']');
}

void Lexer::pushSequenceToken(std::string s, size_t& index_now) {
    Token token;
    ++index_now;//cross the LSB
    while (index_now < s.length() && (!isRSB(s[index_now]))) {
        token.content += s[index_now];
        ++index_now;
    }
    if (index_now >= s.length()) {//miss the last RSB
        LexerLog.set_matching_symbol_missing_err(index_now - token.content.length() - 1, s);
    }
    else {
        ++index_now;//cross the RSB
        token.type = SEQUENCE;
        tokenList.push_back(token);
    }
}


bool Lexer::isQuote(char c) {
    return (c == '\"');
}

void Lexer::pushPathToken(std::string s, size_t& index_now) {
    Token token;
    ++index_now;//cross the first quote
    while (index_now < s.length() && (!isQuote(s[index_now]))) {
        token.content += s[index_now];
        ++index_now;
    }
    if (index_now >= s.length()) {//miss the last quote
        LexerLog.set_matching_symbol_missing_err(index_now-token.content.length()-1, s);
    }
    else {
        ++index_now;//cross the last quote
        token.type = PATH;
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
    if (c == 't') return TRAJECTORY;
    else if (c == 'p')return POLYGON;
    else if (c == 'q')return QUERY_TRAJ;
    else if (c == 'r')return RANGE;
    else if (c == 's')return SIMILARITY;
    else return DEFAULT;
}

void Lexer::pushParameterToken(std::string s, size_t& index_now) {
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
        token.type = TOPK;
    }
    else {
        LexerLog.set_illegal_identifier_err(index_now, s);
    }
    tokenList.push_back(token);
}

bool Lexer::isComma(char c) {
    return (c == ',');
}

void Lexer::pushCommaToken(std::string s, size_t& index_now) {
    Token token;
    token.content = s[index_now];
    token.type = COMMA;
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
    token.type = NUMBER;
    tokenList.push_back(token);
}

bool Lexer::isSpace(char c) {
    return (c == ' ');
}

void Lexer::pushKeywordToken(std::string s, size_t& index_now) {
    Token token;
    while (isLetter(s[index_now])&&index_now<s.length()) {
        token.content += s[index_now];
        ++index_now;
    }
    if (isKeyword(token.content)) {//add select from delete show
        token.type = getKeywordType(token.content);
    }
    else {
        LexerLog.set_illegal_identifier_err(index_now-token.content.length(), index_now, s);
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
    if (s == "add") return ADD;
    else if (s == "select") return SELECT;
    else if (s == "from") return FROM;
    else if (s == "delete") return DELETE;
    else if (s == "show") return SHOW;
    else if (s == "help") return HELP;
    else return DEFAULT;
}

void Log::set_illegal_identifier_err(int errPosition,std::string& errStr) {
    errPos = errPosition;
    isSuccess = false;
    log += "\n" + errStr + "\n";
    for (size_t j = 0; j < errStr.length(); ++j) {
        if (j == errPos) log += "^";
        else log += "-";
    }
    log += "\nLexer Error: <illegal identifier> at Position:" + std::to_string(errPos) + "\n";
    //qDebug() << QString::fromStdString(log);
}

void Log::set_illegal_identifier_err(int errPosition_begin, int errPosition_end, std::string& errStr) {
    errPos = errPosition_begin;
    isSuccess = false;
    log += "\n" + errStr + "\n";
    for (size_t j = 0; j < errStr.length(); ++j) {
        if (errPosition_begin <= j && j < errPosition_end) log += "^";
        else log += "-";
    }
    log += "\nLexer Error: <illegal identifier> at Position:" + std::to_string(errPos) + "\n";
    //qDebug() << QString::fromStdString(log);
}

void Log::set_matching_symbol_missing_err(int errPosition, std::string& errStr) {
    errPos = errPosition;
    isSuccess = false;
    log += "\n" + errStr + "\n";
    for (size_t j = 0; j < errStr.length(); ++j) {
        if (j == errPos) log += "^";
        else log += "-";
    }
    log += "\nLexer Error: <matching symbol missing> at Position:" + std::to_string(errPos) + "\n";
    //qDebug() << QString::fromStdString(log);
}

void Log::set_wrong_add_command_format_err() {
    log = "Parser Error: <wrong format>. The format of the add command is as follows:\n"
        "\tadd -t [(x0,y0),(x1,y1),...],[(x2,y2),(x3,y3),...]...\n"
        "\tadd -t \"your/file/path\"\n"
        "\tadd -p [(x0,y0),(x1,y1),...],[(x2,y2),(x3,y3),...]...\n"
        "\tadd -p \"your/file/path\"\n"
        "\tadd -q [(x0,y0),(x1,y1),...],[(x2,y2),(x3,y3),...]...\n"
        "\tadd -q \"your/file/path\"\n"
        "\tfor more information, please input help\n";
    isSuccess = false;
}

void Log::set_wrong_select_command_format_err() {
    log ="Parser Error: <wrong format>. The format of the select command is as follows:\n"
        "\tselect -<top k num> -r -t [tid0,tid1,...] from [[pid0,pid1,...]\n"
        "\tselect -<top k num> -r -p [pid0,pid1,...] from [tid0,tid1,...]\n"
        "\tselect -<top k num> -s [tid0,tid1,...] from [qid0,qid1,...]\n"
        "\tselect -r -t [tid0,tid1,...] from [pid0,pid1,...]\n"
        "\tselect -r -p [pid0,pid1,...] from [tid0,tid1,...]\n"
        "\tselect -s [tid0,tid1,...] from [qid0,qid1,...]\n"
        "\tfor more information, please input help\n";
    isSuccess = false;
}

void Log::set_wrong_delete_command_format_err() {
    log = "Parser Error: <wrong format>. The format of the delete command is as follows:\n"
        "\tdelete -t [tid0,tid1,...]\n"
        "\tdelete -p [pid0,pid1,...]\n"
        "\tdelete -q [qid0,qid1,...]\n";
    isSuccess = false;
}

void Log::set_wrong_show_command_format_err() {
    log="Parser Error: <wrong format>. The format of the show command is as follows:\n"
        "\tshow -t [tid0,tid1,...]\n"
        "\tshow -p [pid0,pid1,...]\n"
        "\tshow -q [qid0,qid1,...]\n";
    isSuccess = false;
}

void Log::set_wrong_help_command_format_err() {
    log="Parser Error: <wrong format>. The format of the help command is as follows:\n"
        "\thelp\n";
    isSuccess = false;
}

void Log::set_wrong_data_format_err(int errPosition,std::string& errStr) {
    errPos = errPosition;
    isSuccess = false;
    log += "\n" + errStr + "\n";
    for (size_t j = 0; j < errStr.length(); ++j) {
        if (j == errPos) log += "^";
        else log += "-";
    }
    log += "\nParser Error: <wrong data format> at Position:" + std::to_string(errPos) + "\n";
}

void Log::set_wrong_command_err() {
    log = "Parser Error: <unknown command>. Enter help for more information\n";
    isSuccess = false;
}
/*
* Parser Implementation
*/

void Parser::parse(TokenList tokenList) {
    if (tokenList.size() == 0) {
        ;//do nothing
    }
    else {
        Command cmd = fetchCommand(tokenList);
        switch (cmd)
        {
        case Parser::CADDS:
            processAddFromSequenceCommand(tokenList);
            break;
        case Parser::CADDP:
            processAddFromFileCommand(tokenList);
            break;
        case Parser::CSLT:
            processSelectCommand(tokenList);
            break;
        case Parser::CDEL:
            processDeleteCommand(tokenList);
            break;
        case Parser::CSHOW:
            processShowCommand(tokenList);
            break;
        case Parser::CHELP:
            processHelpCommand(tokenList);
            break;
        case Parser::CERR:
            parserLog.set_wrong_command_err();
            break;
        default:
            break;
        }
    }
}

//有穷状态机检查tokenList
Parser::Command Parser::fetchCommand(TokenList tokenList) {
    enum State {
        START,
        ADDB,//add begin
        ADDD,//add data
        ADDF,//add file
        ADDS,//add seq
        ADDNS,//add new seq
        DELB,//delete begin
        DELP,//delete parameter
        DELD,//delete data
        HLP,//help
        SHOWB,//show begin
        SHOWP,//show parameter
        SHOWD,//show data
        SLTB,//select begin
        SLTK,//select topk
        SLTR,//select range
        SLTS,//select similarity
        SLTRTP,//select range in trajectory or polygon
        SLTD1,//select data 1
        SLTF,//select from
        SLTD2,//select data 2
        ERR,//ERRO
    };
    State stateNow = START;
    size_t i = 0;
    Command command = CERR;
    while (i < tokenList.size()) {//实现每一个有向边
        if (stateNow == START && tokenList[i].type == ADD) {
            stateNow = ADDB;
        }
        else if (stateNow == ADDB && (tokenList[i].type == TRAJECTORY || tokenList[i].type == POLYGON || tokenList[i].type == QUERY_TRAJ)) {
            stateNow = ADDD;
        }
        else if (stateNow == ADDD && tokenList[i].type == PATH) {
            stateNow = ADDF;
        }
        else if (stateNow == ADDD && tokenList[i].type == SEQUENCE) {
            stateNow = ADDS;
        }
        else if (stateNow == ADDS && tokenList[i].type == COMMA) {
            stateNow = ADDNS;
        }
        else if (stateNow == ADDNS && tokenList[i].type == SEQUENCE) {
            stateNow = ADDS;
        }
        else if (stateNow == START && tokenList[i].type == HELP) {
            stateNow = HLP;
        }
        else if (stateNow == START && tokenList[i].type == DELETE) {
            stateNow = DELB;
        }
        else if (stateNow == DELB && (tokenList[i].type == TRAJECTORY || tokenList[i].type == POLYGON || tokenList[i].type == QUERY_TRAJ)) {
            stateNow = DELP;
        }
        else if (stateNow == DELP && tokenList[i].type == SEQUENCE) {
            stateNow = DELD;
        }
        else if (stateNow == START && tokenList[i].type == SHOW) {
            stateNow = SHOWB;
        }
        else if (stateNow == SHOWB && (tokenList[i].type == TRAJECTORY || tokenList[i].type == POLYGON || tokenList[i].type == QUERY_TRAJ)) {
            stateNow = SHOWP;
        }
        else if (stateNow == SHOWP && tokenList[i].type == SEQUENCE) {
            stateNow = SHOWD;
        }
        else if (stateNow == START && tokenList[i].type == SELECT) {
            stateNow = SLTB;
        }
        else if (stateNow == SLTB && tokenList[i].type == RANGE) {
            stateNow = SLTR;
        }
        else if (stateNow == SLTB && tokenList[i].type == TOPK) {
            stateNow = SLTK;
        }
        else if (stateNow == SLTB && tokenList[i].type == SIMILARITY) {
            stateNow = SLTS;
        }
        else if (stateNow == SLTK && tokenList[i].type == RANGE) {
            stateNow = SLTR;
        }
        else if (stateNow == SLTK && tokenList[i].type == SIMILARITY) {
            stateNow = SLTS;
        }
        else if (stateNow == SLTR && (tokenList[i].type == TRAJECTORY || tokenList[i].type == POLYGON)) {
            stateNow = SLTRTP;
        }
        else if (stateNow == SLTRTP && tokenList[i].type == SEQUENCE) {
            stateNow = SLTD1;
        }
        else if (stateNow == SLTS && tokenList[i].type == SEQUENCE) {
            stateNow = SLTD1;
        }
        else if (stateNow == SLTD1 && tokenList[i].type == FROM) {
            stateNow = SLTF;
        }
        else if (stateNow == SLTF && tokenList[i].type == SEQUENCE) {
            stateNow = SLTD2;
        }
        else {
            stateNow = ERR;
            break;
        }
        ++i;
    }
    switch (stateNow)
    {
    case ADDF:
        return CADDP;
        break;
    case ADDS:
        return CADDS;
        break;
    case DELD:
        return CDEL;
        break;
    case HLP:
        return CHELP;
        break;
    case SHOWD:
        return CSHOW;
        break;
    case SLTD2:
        return CSLT;
        break;
    case ERR:
        return CERR;
        break;
    default:
        return CERR;
        break;
    }
}

//设定：所有的合法性检查在状态机中已经完成
void Parser::processAddFromFileCommand(TokenList tokenList) {
    TokenType addParameter = tokenList[1].type;
    std::vector<Sequence> seqArray = fetchDataFromFile(tokenList[2].content);
    if (!parserLog.isSuccess) return;
    switch (addParameter) {
        case TRAJECTORY: tdb.ADD(seqArray); break;
        case POLYGON: pdb.ADD(seqArray); break;
        case QUERY_TRAJ: qdb.ADD(seqArray); break;
        default: break;
    }
}

void Parser::processAddFromSequenceCommand(TokenList tokenList) {
    TokenType addParameter = tokenList[1].type;
    size_t i = 2;
    std::vector<Sequence> seqArray;
    while (i < tokenList.size()) {
        seqArray.push_back(fetchDataFromInput(tokenList[i].content));
        if (!parserLog.isSuccess) return;
        ++i;
    }
    switch (addParameter) {
        case TRAJECTORY: tdb.ADD(seqArray); break;
        case POLYGON: pdb.ADD(seqArray); break;
        case QUERY_TRAJ: qdb.ADD(seqArray); break;
        default: break;
    }
}

void Parser::processSelectCommand(TokenList tokenList) {
    if (tokenList[1].type == TOPK) {
        int topk = std::stoi(tokenList[1].content.substr(1));
        if (tokenList[2].type == RANGE) {
            if (tokenList[3].type == TRAJECTORY) {
                doSelectInDB(&tdb, tokenList[4].content, tokenList[6].content, topk);
            }
            else if (tokenList[3].type == POLYGON) {
                doSelectInDB(&pdb, tokenList[4].content, tokenList[6].content, topk);
            }
        }
        else if (tokenList[2].type == SIMILARITY) {
            doSelectInDB(&qdb, tokenList[3].content, tokenList[5].content, topk);
        }
    }
    else {//no topk
        if (tokenList[1].type == RANGE) {
            if (tokenList[2].type == TRAJECTORY) {
                doSelectInDB(&tdb, tokenList[3].content, tokenList[5].content);
            }
            else if (tokenList[2].type == POLYGON) {
                doSelectInDB(&pdb, tokenList[3].content, tokenList[5].content);
            }
        }
        else if (tokenList[1].type == SIMILARITY) {
            doSelectInDB(&qdb, tokenList[2].content, tokenList[4].content);
        }
    }
}

void Parser::doSelectInDB(PrimitiveDB* db, std::string str1, std::string str2) {
    IDArray ids1 = fectchIDFromInput(str1);
    IDArray ids2 = fectchIDFromInput(str2);
    if (!parserLog.isSuccess) return;
    IDArray result_ids = db->SELECT(ids1, ids2, ids1.size());
    db->PRINT(result_ids);
}

void Parser::doSelectInDB(PrimitiveDB* db, std::string str1, std::string str2, int topk) {
    IDArray ids1 = fectchIDFromInput(str1);
    IDArray ids2 = fectchIDFromInput(str2);
    if (!parserLog.isSuccess) return;
    IDArray result_ids = db->SELECT(ids1, ids2, topk);
    db->PRINT(result_ids);
}

void Parser::processDeleteCommand(TokenList tokenList) {
    IDArray ids = fectchIDFromInput(tokenList[2].content);
    if (!parserLog.isSuccess) return;
    switch (tokenList[1].type) {
        case TRAJECTORY : tdb.DELETE(ids); break;
        case POLYGON : pdb.DELETE(ids); break;
        case QUERY_TRAJ : qdb.DELETE(ids); break;
        default:break;
    }
}

void Parser::processShowCommand(TokenList tokenList) {
    IDArray ids = fectchIDFromInput(tokenList[2].content);
    if (!parserLog.isSuccess) return;
    switch (tokenList[1].type) {
        case TRAJECTORY: tdb.SHOW(ids); break;
        case POLYGON: pdb.SHOW(ids); break;
        case QUERY_TRAJ: qdb.SHOW(ids); break;
        default:break;
    }
}

void Parser::processHelpCommand(TokenList tokenList) {
    qDebug() << "help\n";
}

//使用有穷状态机从字符串获取数据
Sequence Parser::fetchDataFromInput(std::string inputSequence) {
    enum State
    {
        START,
        XI,
        XF,
        YI,
        YF,
        END,
        ERR
    };
    State stateNow = START;
    size_t i = 0;
    std::string x = "";
    std::string y = "";
    double dx = 0;
    double dy = 0;
    Sequence newSeq;
    while (i < inputSequence.length()) {//实现状态转移图的每一个有向边
        if (stateNow == START && inputSequence[i] == '(') {
            stateNow = XI;
            x = "";
            y = "";
            dx = 0;
            dy = 0;
        }
        else if (stateNow == XI && 0 <= (inputSequence[i] - '0') && ((inputSequence[i] - '0')) <= 9) {
            stateNow = XI;
            x += inputSequence[i];
        }
        else if (stateNow == XI && inputSequence[i] == '.') {
            stateNow = XF;
            x += '.';
        }
        else if (stateNow == XI && inputSequence[i] == ',') {
            stateNow = YI;
            dx = std::stod(x);
        }
        else if (stateNow == XF && 0 <= (inputSequence[i] - '0') && ((inputSequence[i] - '0')) <= 9) {
            stateNow = XF;
            x += inputSequence[i];
        }
        else if (stateNow == XF && inputSequence[i] == ',') {
            stateNow = YI;
            dx = std::stod(x);
        }
        else if (stateNow == YI && 0 <= (inputSequence[i] - '0') && ((inputSequence[i] - '0')) <= 9) {
            stateNow = YI;
            y += inputSequence[i];
        }
        else if (stateNow == YI && inputSequence[i] == '.') {
            stateNow = YF;
            y += inputSequence[i];
        }
        else if (stateNow == YI && inputSequence[i] == ')') {
            stateNow = END;
            dy = std::stod(y);
            newSeq.push_back(Point(dx, dy));
        }
        else if (stateNow == YF && 0 <= (inputSequence[i] - '0') && ((inputSequence[i] - '0')) <= 9) {
            stateNow = YF;
            y += inputSequence[i];
        }
        else if (stateNow == YF && inputSequence[i] == ')') {
            stateNow = END;
            dy = std::stod(y);
            newSeq.push_back(Point(dx, dy));
        }
        else if (stateNow == END && inputSequence[i] == ',') {
            stateNow = START;
        }
        else {
            stateNow = ERR;
            break;
        }
        ++i;
    }
    if (stateNow != END) {
        parserLog.set_wrong_data_format_err(i, inputSequence);
    }
    return newSeq;
}

//使用有穷状态机获取id
IDArray Parser::fectchIDFromInput(std::string inputIDArray) {
    enum State
    {
        START,
        ID,
        ERR
    };
    State stateNow = START;
    IDArray newIDArray;
    size_t i = 0;
    size_t id = 0;
    std::string idstr = "";
    while (i < inputIDArray.length()) {
        if (stateNow == START && 0 <= (inputIDArray[i] - '0') && (inputIDArray[i] - '0') <= 9) {
            stateNow = ID;
            id = 0;
            idstr = "";
            idstr += inputIDArray[i];
        }
        else if (stateNow == ID && 0 <= (inputIDArray[i] - '0') && (inputIDArray[i] - '0') <= 9) {
            stateNow = ID;
            idstr += inputIDArray[i];
        }
        else if (stateNow == ID && inputIDArray[i] == ',') {
            stateNow = START;
            id = (size_t)std::stoi(idstr);
            newIDArray.push_back(id);
        }
        else {
            stateNow = ERR;
            break;
        }
        ++i;
    }
    if (stateNow != ID) {
        parserLog.set_wrong_data_format_err(i, inputIDArray);
    }
    return newIDArray;
}

std::vector<Sequence> Parser::fetchDataFromFile(std::string filepath) {
    std::fstream sequenceFile(filepath);
    std::string line;
    std::vector<Sequence> newSeqArray;
    while (getline(sequenceFile, line)) {
        Sequence newSeq = fetchDataFromInput(line);
        if (!parserLog.isSuccess) break;
        newSeqArray.push_back(newSeq);
    }
    return newSeqArray;    
}

//PrimitiveDB static member
std::vector<Sequence> PrimitiveDB::TDBArray(0);
std::vector<Sequence> PrimitiveDB::PDBArray(0);
std::vector<Sequence> PrimitiveDB::QDBArray(0);

void TrajectoryDB::ADD(std::vector<Sequence> seqArray) {
    qDebug() << "t add\n";
}
void TrajectoryDB::DELETE(IDArray ids) {
    qDebug() << "t del\n";
}
IDArray TrajectoryDB::SELECT(IDArray tids, IDArray pids, int topk) {
    qDebug() << "t slt\n";
    IDArray newid;
    return newid;
}
void TrajectoryDB::SHOW(IDArray ids) {
    qDebug() << "t show\n";
}
void TrajectoryDB::PRINT(IDArray result_ids) {
    qDebug() << "t print\n";
}

void PolygonDB::ADD(std::vector<Sequence> seqArray) {
    qDebug() << "p add\n";
}
void PolygonDB::DELETE(IDArray ids) {
    qDebug() << "p del\n";
}
IDArray PolygonDB::SELECT(IDArray tids, IDArray pids, int topk) {
    qDebug() << "p slt\n";
    IDArray newid;
    return newid;
}
void PolygonDB::SHOW(IDArray ids) {
    qDebug() << "p show\n";
}
void PolygonDB::PRINT(IDArray result_ids) {
    qDebug() << "p print\n";
}

void QueryTrajDB::ADD(std::vector<Sequence> seqArray) {
    qDebug() << "q add\n";
}
void QueryTrajDB::DELETE(IDArray ids) {
    qDebug() << "q del\n";
}
IDArray QueryTrajDB::SELECT(IDArray tids, IDArray pids, int topk) {
    qDebug() << "q slt\n";
    IDArray newid;
    return newid;
}
void QueryTrajDB::SHOW(IDArray ids) {
    qDebug() << "q show\n";
}
void QueryTrajDB::PRINT(IDArray result_ids) {
    qDebug() << "q print\n";
}

