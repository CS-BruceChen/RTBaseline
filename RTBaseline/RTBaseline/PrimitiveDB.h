#pragma once
#include<vector>
#include<string>
#include <QDebug>
#include "GLObject.h"
#include "Log.h"
#include <algorithm>

typedef std::vector<size_t> IDArray;
class PrimitiveDB {
    //default constructor
protected://only visiable in derived classes
    static std::vector<Sequence> TDBArray;
    static std::vector<Sequence> PDBArray;
    static std::vector<Sequence> QDBArray;
    static CoordBound getBound(std::vector<Sequence> primitive);
    Log DBLog;
public:
    Log getDBLog() const { return DBLog; }
    virtual void ADD(std::vector<Sequence> seqArray) = 0;
    virtual void DELETE(IDArray ids) = 0;
    virtual IDArray SELECT(IDArray ids1, IDArray ids2, int topk) = 0;
    virtual void SHOW(IDArray ids) = 0;
    virtual void PRINT() = 0;
    virtual void PRINT(IDArray result_ids) = 0;
};

class TrajectoryDB :public PrimitiveDB {
public:
    void ADD(std::vector<Sequence> seqArray);
    void DELETE(IDArray ids);
    IDArray SELECT(IDArray tids, IDArray pids, int topk);
    void SHOW(IDArray ids);
    void PRINT();
    void PRINT(IDArray result_ids);
};

class QueryTrajDB : public PrimitiveDB {
public:
    void ADD(std::vector<Sequence> seqArray);
    void DELETE(IDArray ids);
    IDArray SELECT(IDArray tids, IDArray qids, int topk);
    void SHOW(IDArray ids);
    void PRINT();
    void PRINT(IDArray result_ids);
};

class PolygonDB : public PrimitiveDB {
public:
    void ADD(std::vector<Sequence> seqArray);
    void DELETE(IDArray ids);
    IDArray SELECT(IDArray pids, IDArray tids, int topk);
    void SHOW(IDArray ids);
    void PRINT();
    void PRINT(IDArray result_ids);
};