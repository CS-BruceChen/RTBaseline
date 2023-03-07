#include "RTBaseline.h"
#include <QDebug>

RTBaseline::RTBaseline(QWidget *parent)
    : QMainWindow(parent)
{
    //init ui
    ui.setupUi(this);
    //set welcome text
    QString welcome = "Copyright (c) 2023, Bruce Chen, brucechen@whu.edu.cn\nWelcome to RT Terminal v0.0.0\n"
        "  _____ _______   _______                  _             _ \n"
        " |  __ \\__   __| |__   __|                (_)           | |\n"
        " | |__) | | |       | | ___ _ __ _ __ ___  _ _ __   __ _| |\n"
        " |  _  /  | |       | |/ _ \\ '__| '_ ` _ \\| | '_ \\ / _` | |\n"
        " | | \\ \\  | |       | |  __/ |  | | | | | | | | | | (_| | |\n"
        " |_|  \\_\\ |_|       |_|\\___|_|  |_| |_| |_|_|_| |_|\\__,_|_|\n";

    ui.textEdit->setText(welcome);
    ui.textEdit->setWordWrapMode(QTextOption::WrapAnywhere);
    //move cursor to the end;
    startNewLine();
    //connect & install filter
    connect(ui.textEdit, SIGNAL(ui.textEdit->cursorPositionChanged()), SLOT(on_textEdit_cursorPositionChanged()));
    ui.textEdit->installEventFilter(this);
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
    if (lexerlog.getIsSuccess()) {
        Parser parser;
        parser.parse(lexer.getTokenList());
        Log parserLog = parser.getParserLog();
        //parser log 可以任意输出，执行正确和执行错误都有对应反馈；
        ui.textEdit->append(QString::fromStdString(parserLog.getLog()));
    }
    else {
        ui.textEdit->append(QString::fromStdString(lexerlog.getLog()));
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
    QTextCursor cursor = ui.textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui.textEdit->setTextCursor(cursor);
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
    QTextCursor cursor = ui.textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    ui.textEdit->setTextCursor(cursor);
}



