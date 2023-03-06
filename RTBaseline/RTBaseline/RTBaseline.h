#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_RTBaseline.h"
#include <QEvent>
#include <QKeyEvent>
#include "Lexer_Parser.h"

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








