#pragma once
#include<string>
#include<vector>
typedef std::vector<size_t> IDArray;
struct Log {
    Log() :log(""), isSuccess(true), errPos(0) {};
public:
    void setLog(Log newlog) { log = newlog.getLog(); isSuccess = newlog.getIsSuccess(); }
    std::string getLog() const { return log; }
    bool getIsSuccess() const {return isSuccess;}
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
    void set_run_add_info(const std::string& addObject,int objectNum);
    void set_run_delete_info(const std::string& deleteObject, int objectNum, IDArray errIds);
    void set_print_info(const std::string& printObject, std::string& printInfo);
    void set_print_info(const std::string& printObject, std::string& printInfo, IDArray errIds);
    void set_show_info(const std::string& showObject, int objectNum, IDArray errIds);
    void set_select_empty_err(const std::string& emptyObject);
    void set_select_info_RQ(const std::string& selectObject, IDArray retIds, IDArray errTids, IDArray errPids);
    void set_select_err_info_SQ(IDArray errTids, IDArray errQids);
    void set_select_info_SQ(IDArray retIds,size_t qid);
private:
    std::string log;
    bool isSuccess;
    int errPos;
};