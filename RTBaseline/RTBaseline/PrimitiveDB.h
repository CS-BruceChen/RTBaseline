#pragma once
#include<vector>
#include<string>
#include <QDebug>
#include "GLObject.h"
#include "Log.h"
#include <algorithm>

struct errAndLeg
{
    IDArray errIds;//erro Ids
    IDArray legIds;//legal Ids
};

struct resultWithIndex {
    int value;
    size_t index;
    resultWithIndex(int val, size_t ind) { value = val; index = ind; }
};

class PrimitiveDB {
    //default constructor
protected://only visiable in derived classes
    static std::vector<Sequence> TDBArray;
    static std::vector<Sequence> PDBArray;
    static std::vector<Sequence> QDBArray;
    static float maxx;
    static float maxy;
    static float minx;
    static float miny;
    Log DBLog;

public:
    Log getDBLog() const { return DBLog; }
    CoordBound getBound(std::vector<Sequence>& primitive);
    void setBound(Shader* shader, float maxn);
    void addBound(Point p);
    void deleteBound(Point p);
    IDArray getTopk(std::vector<int> retFromShader, int topk);
    errAndLeg idRangeCheck(IDArray ids, std::vector<Sequence>& db);
    Primitive** createTrajectoryArray(std::vector<Sequence>* db, IDArray ids);
    Primitive** createPolygonArray(std::vector<Sequence>* db, IDArray ids);
    virtual void ADD(std::vector<Sequence> seqArray) = 0;
    virtual void DELETE(IDArray ids) = 0;
    virtual void SELECT(IDArray ids1, IDArray ids2, int topk) = 0;
    virtual void SHOW(IDArray ids) = 0;
    virtual void PRINT() = 0;
    virtual void PRINT(IDArray result_ids) = 0;
};

class TrajectoryDB :public PrimitiveDB {
public:
    void ADD(std::vector<Sequence> seqArray);
    void DELETE(IDArray ids);
    void SELECT(IDArray tids, IDArray pids, int topk);
    void SHOW(IDArray ids);
    void PRINT();
    void PRINT(IDArray result_ids);
};

class QueryTrajDB : public PrimitiveDB {
public:
    void ADD(std::vector<Sequence> seqArray);
    void DELETE(IDArray ids);
    void SELECT(IDArray tids, IDArray qids, int topk);
    void SHOW(IDArray ids);
    void PRINT();
    void PRINT(IDArray result_ids);
};

class PolygonDB : public PrimitiveDB {
public:
    void ADD(std::vector<Sequence> seqArray);
    void DELETE(IDArray ids);
    void SELECT(IDArray pids, IDArray tids, int topk);
    void SHOW(IDArray ids);
    void PRINT();
    void PRINT(IDArray result_ids);
};