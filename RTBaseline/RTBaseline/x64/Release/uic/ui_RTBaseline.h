/********************************************************************************
** Form generated from reading UI file 'RTBaseline.ui'
**
** Created by: Qt User Interface Compiler version 6.4.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_RTBASELINE_H
#define UI_RTBASELINE_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_RTBaselineClass
{
public:
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    QTextEdit *textEdit;

    void setupUi(QMainWindow *RTBaselineClass)
    {
        if (RTBaselineClass->objectName().isEmpty())
            RTBaselineClass->setObjectName("RTBaselineClass");
        RTBaselineClass->resize(800, 600);
        RTBaselineClass->setStyleSheet(QString::fromUtf8("background-color: rgb(50, 50, 50);"));
        centralWidget = new QWidget(RTBaselineClass);
        centralWidget->setObjectName("centralWidget");
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName("gridLayout");
        textEdit = new QTextEdit(centralWidget);
        textEdit->setObjectName("textEdit");
        textEdit->setMouseTracking(true);
        textEdit->setStyleSheet(QString::fromUtf8("color: rgb(204, 204, 204);\n"
"font: 14pt \"Consolas\";\n"
"border:0px;\n"
""));
        textEdit->setLineWidth(1);
        textEdit->setCursorWidth(2);

        gridLayout->addWidget(textEdit, 0, 0, 1, 1);

        RTBaselineClass->setCentralWidget(centralWidget);

        retranslateUi(RTBaselineClass);

        QMetaObject::connectSlotsByName(RTBaselineClass);
    } // setupUi

    void retranslateUi(QMainWindow *RTBaselineClass)
    {
        RTBaselineClass->setWindowTitle(QCoreApplication::translate("RTBaselineClass", "RTBaseline", nullptr));
    } // retranslateUi

};

namespace Ui {
    class RTBaselineClass: public Ui_RTBaselineClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_RTBASELINE_H
