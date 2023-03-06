//Log implementation
#include "Log.h"
void Log::set_illegal_identifier_err(int errPosition, std::string& errStr) {
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
    log = "Parser Error: <wrong format>. The format of the select command is as follows:\n"
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
    log = "Parser Error: <wrong format>. The format of the show command is as follows:\n"
        "\tshow -t [tid0,tid1,...]\n"
        "\tshow -p [pid0,pid1,...]\n"
        "\tshow -q [qid0,qid1,...]\n";
    isSuccess = false;
}

void Log::set_wrong_help_command_format_err() {
    log = "Parser Error: <wrong format>. The format of the help command is as follows:\n"
        "\thelp\n";
    isSuccess = false;
}

void Log::set_wrong_data_format_err(int errPosition, std::string& errStr) {
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

void Log::set_run_add_info(const std::string& addObject,int objectNum) {
    log = "Execute <add command>: Add " + std::to_string(objectNum) + " " + addObject + " into DB\n";
    //isSuccess = true;
}

void Log::set_run_delete_info(const std::string& deleteObject, int objectNum, std::vector<int> errIds) {
    log = "Execute <delete command>: Delete " + std::to_string(objectNum) + " " + deleteObject + " from DB\n";
    if (errIds.size() > 0) {
        log += deleteObject + " with id: ";
        for (size_t i = 0; i < errIds.size(); ++i) {
            log += std::to_string(errIds[i]);
            log += " ";
        }
        log += " not in the DB, delete failed\n";
    }
    //isSuccess = true;
}

void Log::set_print_info(const std::string& printObject, std::string& printInfo) {
    log = "Execute <print command>\n";
    log += "The " + printObject + " in DB is as follows:\n";
    log += printInfo;
}

void Log::set_print_info(const std::string& printObject, std::string& printInfo, std::vector<int> errIds) {
    log = "Execute <print command>\n";
    if (errIds.size() > 0) {
        log += printObject + " with id: ";
        for (size_t i = 0; i < errIds.size(); ++i) {
            log += std::to_string(errIds[i]);
            log += " ";
        }
        log += " not in the DB, print failed\n";
    }    
    log += "The " + printObject + " in DB is as follows:\n";
    log += printInfo;
}
