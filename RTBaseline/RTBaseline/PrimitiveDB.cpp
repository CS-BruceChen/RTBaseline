#include "PrimitiveDB.h"

std::vector<Sequence> PrimitiveDB::TDBArray(0);
std::vector<Sequence> PrimitiveDB::PDBArray(0);
std::vector<Sequence> PrimitiveDB::QDBArray(0);
float PrimitiveDB::maxx = FLT_MIN;
float PrimitiveDB::maxy = FLT_MIN;
float PrimitiveDB::minx = FLT_MAX;
float PrimitiveDB::miny = FLT_MAX;

CoordBound PrimitiveDB::getBound(std::vector<Sequence>& primitive) {
    float MAXX=FLT_MIN, MAXY=FLT_MIN, MINX=FLT_MAX, MINY=FLT_MAX;
    for (size_t i = 0; i < primitive.size(); ++i) {
        for (size_t j = 0; j < primitive[i].size(); ++j) {
            float dx = primitive[i][j].x;
            float dy = primitive[i][j].y;
            if (dx > MAXX) MAXX = dx;
            if (dx < MINX) MINX = dx;
            if (dy > MAXY) MAXY = dy;
            if (dy < MINY) MINY = dy;
        }
    }
    return CoordBound(MAXX, MAXY, MINX, MINY);
}
errAndLeg PrimitiveDB::idRangeCheck(IDArray ids, std::vector<Sequence>& db) {
    errAndLeg eL;
    for (size_t i = 0; i < ids.size(); ++i) {
        if (ids[i] >= db.size())
            eL.errIds.push_back(ids[i]);
        else
            eL.legIds.push_back(ids[i]);
    }
    return eL;
}
Primitive** PrimitiveDB::createTrajectoryArray(std::vector<Sequence>* db, IDArray ids) {
    Primitive** primitiveArray = new Primitive * [ids.size()];
    for (size_t i = 0; i < ids.size(); ++i) {
        unsigned id = ids[i];
        Sequence seq = (*db)[id];
        unsigned vertsNum = seq.size();
        std::vector<float> verts;
        for (size_t j = 0; j < seq.size(); ++j) {
            verts.push_back(seq[j].x);
            verts.push_back(seq[j].y);
            verts.push_back(id+1);//start from 1
            //qDebug() << seq[j].x << " " << seq[j].y << " " << id << "\n";
        }
        primitiveArray[i] = new Primitive(vertsNum, verts.data());
    }
    return primitiveArray;
}
Primitive** PrimitiveDB::createPolygonArray(std::vector<Sequence>* db, IDArray ids) {
    Primitive** polyArray = new Primitive * [ids.size()];//指针数组的首地址是指向指针的指针
    for (size_t i = 0; i < ids.size(); ++i) {
        unsigned id = ids[i];
        Sequence seq = (*db)[id];
        std::vector<float> verts;
        triangulatePolygons(seq, verts, id+1);//start from 1
        unsigned vertsNum = verts.size() / 3;
        polyArray[i] = new Primitive(vertsNum, verts.data());
    }
    return polyArray;
}
void PrimitiveDB::setBound(Shader* shader,float maxn) {
    shader->use();
    shader->setFloat("MAXX", maxx);
    shader->setFloat("MAXY", maxy);
    shader->setFloat("MINX", minx);
    shader->setFloat("MINY", miny);
    shader->setFloat("MAXN", maxn);
}
void PrimitiveDB::addBound(Point p) {
    float fx = p.x;
    float fy = p.y;
    if (fx > maxx) maxx = fx;
    if (fx < minx) minx = fx;
    if (fy > maxy) maxy = fy;
    if (fy < miny) miny = fy;
}
void PrimitiveDB::deleteBound(Point p) {
    //也就是找第二大，但是这很难，至少不是一个常数复杂度的问题
}
IDArray PrimitiveDB::getTopk(std::vector<int> retFromShader, int topk) {
    std::vector<resultWithIndex> rI;
    for (size_t i = 0; i < retFromShader.size(); ++i) {
        rI.push_back(resultWithIndex(retFromShader[i], i));
    }
    sort(rI.begin(), rI.end(), [=](resultWithIndex a, resultWithIndex b)->bool {return a.value > b.value; });

    size_t realTopk = (rI.size() < topk) ? rI.size() : topk;
    IDArray resultIds;
    for (size_t i = 0; i < realTopk; ++i) {
        resultIds.push_back(rI[i].index);
    }
    return resultIds;
}
//ADD
void TrajectoryDB::ADD(std::vector<Sequence> seqArray) {
    for (size_t i = 0; i < seqArray.size(); ++i) {
        for (size_t j = 0; j < seqArray[i].size(); ++j) {
            addBound(seqArray[i][j]);
        }
        TDBArray.push_back(seqArray[i]);
    }
    DBLog.set_run_add_info("trajectory", seqArray.size());
}
void PolygonDB::ADD(std::vector<Sequence> seqArray) {
    for (size_t i = 0; i < seqArray.size(); ++i) {
        for (size_t j = 0; j < seqArray[i].size(); ++j) {
            addBound(seqArray[i][j]);
        }
        PDBArray.push_back(seqArray[i]);
    }
    DBLog.set_run_add_info("polygon", (int)seqArray.size());
}
void QueryTrajDB::ADD(std::vector<Sequence> seqArray) {
    for (size_t i = 0; i < seqArray.size(); ++i) {
        for (size_t j = 0; j < seqArray[i].size(); ++j) {
            addBound(seqArray[i][j]);
        }
        QDBArray.push_back(seqArray[i]);
    }
    DBLog.set_run_add_info("trajectory-to-be-queried", (int)seqArray.size());
}

//DELETE
void TrajectoryDB::DELETE(IDArray ids) {
    errAndLeg eL = idRangeCheck(ids,TDBArray);
    IDArray errIds = eL.errIds;
    IDArray deleteIds = eL.legIds;
    DBLog.set_run_delete_info("trajectory", deleteIds.size(), errIds);
    //deleteIds 降序并去重，避免重复删除和动态删除引起的下标变化
    sort(deleteIds.begin(), deleteIds.end(), [=](size_t a, size_t b)->bool {return a > b;});
    for (size_t i = 0; i < deleteIds.size(); ++i) {
        if (i < deleteIds.size() - 1 && deleteIds[i] == deleteIds[i + 1]) continue;
        TDBArray.erase(TDBArray.begin() + deleteIds[i]);
    }
}
void PolygonDB::DELETE(IDArray ids) {
    errAndLeg eL = idRangeCheck(ids, PDBArray);
    IDArray errIds = eL.errIds;
    IDArray deleteIds = eL.legIds;
    DBLog.set_run_delete_info("polygon", deleteIds.size(), errIds);
    sort(deleteIds.begin(), deleteIds.end(), [=](size_t a, size_t b)->bool {return a > b; });
    for (size_t i = 0; i < deleteIds.size(); ++i) {
        if (i < deleteIds.size() - 1 && deleteIds[i] == deleteIds[i + 1]) continue;
        PDBArray.erase(PDBArray.begin() + deleteIds[i]);
    }
}
void QueryTrajDB::DELETE(IDArray ids) {
    errAndLeg eL = idRangeCheck(ids, QDBArray);
    IDArray errIds = eL.errIds;
    IDArray deleteIds = eL.legIds;
    DBLog.set_run_delete_info("trajectory-to-be-queried", deleteIds.size(), errIds);
    sort(deleteIds.begin(), deleteIds.end(), [=](size_t a, size_t b)->bool {return a > b; });
    for (size_t i = 0; i < deleteIds.size(); ++i) {
        if (i < deleteIds.size() - 1 && deleteIds[i] == deleteIds[i + 1]) continue;
        QDBArray.erase(QDBArray.begin() + deleteIds[i]);
    }
}

//PRINT-()
void TrajectoryDB::PRINT() {
    std::string printInfo = "";
    for (size_t i = 0; i < TDBArray.size(); ++i) {
        printInfo += "tid: " + std::to_string(i) + "\n";
        Sequence seq = TDBArray[i];
        for (size_t j = 0; j < seq.size(); ++j) {
            double dx = seq[j].x;
            double dy = seq[j].y;
            printInfo += "(" + std::to_string(dx) + ", " + std::to_string(dy) + ") ";
        }
        printInfo += "\n";
    }
    DBLog.set_print_info("trajectory", printInfo);
}
void PolygonDB::PRINT() {
    std::string printInfo = "";
    for (size_t i = 0; i < PDBArray.size(); ++i) {
        printInfo += "pid: " + std::to_string(i) + "\n";
        Sequence seq = PDBArray[i];
        for (size_t j = 0; j < seq.size(); ++j) {
            double dx = seq[j].x;
            double dy = seq[j].y;
            printInfo += "(" + std::to_string(dx) + ", " + std::to_string(dy) + ") ";
        }
        printInfo += "\n";
    }
    DBLog.set_print_info("polygon", printInfo);
}
void QueryTrajDB::PRINT() {
    std::string printInfo = "";
    for (size_t i = 0; i < QDBArray.size(); ++i) {
        printInfo += "qid: " + std::to_string(i) + "\n";
        Sequence seq = QDBArray[i];
        for (size_t j = 0; j < seq.size(); ++j) {
            double dx = seq[j].x;
            double dy = seq[j].y;
            printInfo += "(" + std::to_string(dx) + ", " + std::to_string(dy) + ") ";
        }
        printInfo += "\n";
    }
    DBLog.set_print_info("trajectory-to-be-queried", printInfo);
}

//PRINT-(ID)
void TrajectoryDB::PRINT(IDArray ids) {
    std::string printInfo = "";
    errAndLeg eL = idRangeCheck(ids,TDBArray);
    IDArray errIds=eL.errIds;
    IDArray printIds=eL.legIds;
    for (size_t i = 0; i < printIds.size(); ++i) {
        size_t id = printIds[i];
        printInfo += "tid: " + std::to_string(id) + "\n";
        Sequence seq = TDBArray[id];
        for (size_t j = 0; j < seq.size(); ++j) {
            double dx = seq[j].x;
            double dy = seq[j].y;
            printInfo += "(" + std::to_string(dx) + ", " + std::to_string(dy) + ") ";
        }
        printInfo += "\n";
    }
    DBLog.set_print_info("trajectory", printInfo, errIds);
}
void PolygonDB::PRINT(IDArray ids) {
    std::string printInfo = "";
    errAndLeg eL = idRangeCheck(ids, PDBArray);
    IDArray errIds = eL.errIds;
    IDArray printIds = eL.legIds;
    for (size_t i = 0; i < printIds.size(); ++i) {
        size_t id = printIds[i];
        printInfo += "pid: " + std::to_string(id) + "\n";
        Sequence seq = PDBArray[id];
        for (size_t j = 0; j < seq.size(); ++j) {
            double dx = seq[j].x;
            double dy = seq[j].y;
            printInfo += "(" + std::to_string(dx) + ", " + std::to_string(dy) + ") ";
        }
        printInfo += "\n";
    }
    DBLog.set_print_info("polygon", printInfo, errIds);
}
void QueryTrajDB::PRINT(IDArray ids) {
    std::string printInfo = "";
    errAndLeg eL = idRangeCheck(ids, QDBArray);
    IDArray errIds = eL.errIds;
    IDArray printIds = eL.legIds;
    for (size_t i = 0; i < printIds.size(); ++i) {
        size_t id = printIds[i];
        printInfo += "qid: " + std::to_string(id) + "\n";
        Sequence seq = QDBArray[id];
        for (size_t j = 0; j < seq.size(); ++j) {
            double dx = seq[j].x;
            double dy = seq[j].y;
            printInfo += "(" + std::to_string(dx) + ", " + std::to_string(dy) + ") ";
        }
        printInfo += "\n";
    }
    DBLog.set_print_info("trajectory-to-be-queried", printInfo, errIds);
}


void TrajectoryDB::SELECT(IDArray tids, IDArray pids, int topk) {
    //定义必要的数据成员
    errAndLeg eLt = idRangeCheck(tids, TDBArray);
    errAndLeg eLp = idRangeCheck(pids, PDBArray);
    IDArray errTids = eLt.errIds;
    IDArray errPids = eLp.errIds;
    IDArray queryTids = eLt.legIds;
    IDArray queryPids = eLp.legIds;
    
    //size check
    if (queryTids.size() == 0) {
        DBLog.set_select_empty_err("trajectory");
        return;
    }
    else if (queryPids.size() == 0) {
        DBLog.set_select_empty_err("polygon");
        return;
    }

    //定义必要的GL成员
    const char* windowTitle = "RT Terminal Visualization";
    initOpenGL();
    GLFWwindow* window = createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle);
    initWindowAndGlad(window);
    Primitive** trajArray = createTrajectoryArray(&TDBArray, queryTids);
    Primitive** polyArray = createPolygonArray(&PDBArray, queryPids);
    Shader* TShader = Shader::newShader("drawPrimitives");
    Shader* PShader = Shader::newShader("TRQ");
    FBO* fbo = new FBO(WINDOW_WIDTH, WINDOW_HEIGHT, FBO::Attachment::NoAttachment, GL_TEXTURE_2D, GL_RGB);
    unsigned framebuffer = fbo->getFBO();
    unsigned textureColorbuffer = fbo->texture();
    GLTextureBuffer texBuf;
    std::vector<int> resultData;
    texBuf.create(queryTids.size() * sizeof(int), GL_R32I, resultData.data());//size 为轨迹数目的size
    glBindImageTexture(0, texBuf.getTexId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32I);
    
    //render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        //查询轨迹，则首先在fbo绘制轨迹
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); {
            setBound(TShader, queryTids.size());
            for (unsigned i = 0; i < queryTids.size(); ++i) {
                glBindVertexArray(trajArray[i]->VAO);
                glDrawArrays(GL_LINE_STRIP, 0, trajArray[i]->VNUM);
                glBindVertexArray(0);
            }
        } glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //绘制polygon
        setBound(PShader, queryTids.size());//设置为tid数目，因为是要还原trajid
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        for (unsigned i = 0; i < queryPids.size(); ++i) {
            glBindVertexArray(polyArray[i]->VAO);
            glDrawArrays(GL_TRIANGLES, 0, polyArray[i]->VNUM);
            glBindVertexArray(0);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    resultData = texBuf.getBuffer();
    IDArray retIds = getTopk(resultData, topk);
    DBLog.set_select_info_RQ("trajectory overlap the given polygon", retIds, errTids, errPids);
    texBuf.destroy();
    glfwTerminate();
}
void PolygonDB::SELECT(IDArray pids, IDArray tids, int topk) {
    //定义必要的数据成员
    errAndLeg eLt = idRangeCheck(tids, TDBArray);
    errAndLeg eLp = idRangeCheck(pids, PDBArray);
    IDArray errTids = eLt.errIds;
    IDArray errPids = eLp.errIds;
    IDArray queryTids = eLt.legIds;
    IDArray queryPids = eLp.legIds;

    //size check
    if (queryTids.size() == 0) {
        DBLog.set_select_empty_err("trajectory");
        return;
    }
    else if (queryPids.size() == 0) {
        DBLog.set_select_empty_err("polygon");
        return;
    }

    //定义必要的GL成员
    const char* windowTitle = "RT Terminal Visualization";
    initOpenGL();
    GLFWwindow* window = createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle);
    initWindowAndGlad(window);
    Primitive** trajArray = createTrajectoryArray(&TDBArray, queryTids);
    Primitive** polyArray = createPolygonArray(&PDBArray, queryPids);
    Shader* PShader = Shader::newShader("drawPrimitives");
    Shader* TShader = Shader::newShader("TRQ");
    FBO* fbo = new FBO(WINDOW_WIDTH, WINDOW_HEIGHT, FBO::Attachment::NoAttachment, GL_TEXTURE_2D, GL_RGB);
    unsigned framebuffer = fbo->getFBO();
    unsigned textureColorbuffer = fbo->texture();
    GLTextureBuffer texBuf;
    std::vector<int> resultData;
    texBuf.create(queryPids.size() * sizeof(int), GL_R32I, resultData.data());//size 为polygon数目的size
    glBindImageTexture(0, texBuf.getTexId(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_R32I);

    //render loop
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        //查询多边形，则首先在fbo绘制多边形
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer); {
            setBound(PShader, queryPids.size());
            for (unsigned i = 0; i < queryPids.size(); ++i) {
                glBindVertexArray(polyArray[i]->VAO);
                glDrawArrays(GL_TRIANGLES, 0, polyArray[i]->VNUM);
                glBindVertexArray(0);
            }
        } glBindFramebuffer(GL_FRAMEBUFFER, 0);
        //绘制轨迹
        setBound(TShader, queryPids.size());//设置为pid数目，因为是要还原polyid
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer);
        for (unsigned i = 0; i < queryTids.size(); ++i) {
            glBindVertexArray(trajArray[i]->VAO);
            glDrawArrays(GL_LINE_STRIP, 0, trajArray[i]->VNUM);
            glBindVertexArray(0);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    resultData = texBuf.getBuffer();
    IDArray retIds = getTopk(resultData, topk);
    DBLog.set_select_info_RQ("polygon overlap the given trajectory", retIds, errTids, errPids);
    texBuf.destroy();
    glfwTerminate();

}
void QueryTrajDB::SELECT(IDArray tids, IDArray qids, int topk) {
    errAndLeg eLt = idRangeCheck(tids, TDBArray);
    errAndLeg eLq = idRangeCheck(qids, QDBArray);
    IDArray errTids = eLt.errIds;
    IDArray errQids = eLq.errIds;
    IDArray queryTids = eLt.legIds;
    IDArray queryQids = eLq.legIds;

    //size check
    if (queryTids.size() == 0) {
        DBLog.set_select_empty_err("trajectory");
        return;
    }
    else if (queryQids.size() == 0) {
        DBLog.set_select_empty_err("trajectory-to-be-queried");
        return;
    }
    DBLog.set_select_err_info_SQ(errTids, errQids);
    //定义必要的GL变量
    initOpenGL();
    GLFWwindow* window = createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE);
    initWindowAndGlad(window);
    Shader* EDR_shader = Shader::newComputeShader("computeEDR");

    for (size_t i = 0; i < queryQids.size(); ++i) {
        std::vector<resultWithIndex> rI;
        Sequence qSeq = QDBArray[queryQids[i]];
        for (size_t j = 0; j < queryTids.size(); ++j) {
            Sequence tSeq = TDBArray[queryTids[j]];
            DPInfo dpinfo(tSeq, qSeq);
            dpinfo.setUniformDPInfo(EDR_shader);
            for (unsigned i = 1; i <= dpinfo.get_total_step(); ++i) {
                EDR_shader->setUint("step_now", i);
                glDispatchCompute(1, 1, 1);
                int dpresult = *(dpinfo.get_DPBuf().end() - 1);
                qDebug() << dpresult;
                rI.push_back(resultWithIndex(dpresult, queryTids[j]));
            }
        }
        sort(rI.begin(), rI.end(), [=](resultWithIndex a, resultWithIndex b)->bool {return a.value > b.value; });
        size_t realTopk = (rI.size() < topk) ? rI.size() : topk;
        IDArray resultIds;
        for (size_t i = 0; i < realTopk; ++i) {
            resultIds.push_back(rI[i].index);
        }
        DBLog.set_select_info_SQ(resultIds, queryQids[i]);
    }
    glfwTerminate();
}


void TrajectoryDB::SHOW(IDArray ids) {
    errAndLeg eL = idRangeCheck(ids, TDBArray);
    IDArray errIds = eL.errIds;
    IDArray showIds = eL.legIds;
    std::vector<Sequence>* db = &TDBArray;
    const char* windowTitle = "RT Terminal-Trajectory Visualization";
    initOpenGL();
    GLFWwindow* window = createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle);
    initWindowAndGlad(window);
    Primitive** trajArray = createTrajectoryArray(db, showIds);
    
    Shader* shader = Shader::newShader("drawPrimitives");
    setBound(shader, showIds.size());

    DBLog.set_show_info("trajectory", showIds.size(), errIds);
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        shader->use();
        for (unsigned i = 0; i < showIds.size(); ++i) {
            glBindVertexArray(trajArray[i]->VAO);
            glDrawArrays(GL_LINE_STRIP, 0, trajArray[i]->VNUM);
            glBindVertexArray(0);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}
void PolygonDB::SHOW(IDArray ids) {
    errAndLeg eL = idRangeCheck(ids, PDBArray);
    IDArray errIds = eL.errIds;
    IDArray showIds = eL.legIds;
    std::vector<Sequence>* db = &PDBArray;
    const char* windowTitle = "RT Terminal-Polygon Visualization";
    initOpenGL();
    GLFWwindow* window = createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle);
    initWindowAndGlad(window);

    Primitive** polyArray = createPolygonArray(db, showIds);
    Shader* shader = Shader::newShader("drawPrimitives");
    setBound(shader, showIds.size());

    DBLog.set_show_info("polygon", showIds.size(), errIds);
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        shader->use();
        for (unsigned i = 0; i < showIds.size(); ++i) {
            glBindVertexArray(polyArray[i]->VAO);
            glDrawArrays(GL_TRIANGLES, 0, polyArray[i]->VNUM);
            glBindVertexArray(0);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();


}
void QueryTrajDB::SHOW(IDArray ids) {
    errAndLeg eL = idRangeCheck(ids, QDBArray);
    IDArray errIds = eL.errIds;
    IDArray showIds = eL.legIds;
    std::vector<Sequence>* db = &QDBArray;
    const char* windowTitle = "RT Terminal-Trajectory-to-be queried Visualization";
    initOpenGL();
    GLFWwindow* window = createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle);
    initWindowAndGlad(window);

    Primitive** qtrajArray = createTrajectoryArray(db, showIds);
    Shader* shader = Shader::newShader("drawPrimitives");
    setBound(shader, showIds.size());

    DBLog.set_show_info("trajectory-to-be-queried", showIds.size(), errIds);
    while (!glfwWindowShouldClose(window)) {
        processInput(window);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        shader->use();
        for (unsigned i = 0; i < showIds.size(); ++i) {
            glBindVertexArray(qtrajArray[i]->VAO);
            glDrawArrays(GL_LINE_STRIP, 0, qtrajArray[i]->VNUM);
            glBindVertexArray(0);
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwTerminate();
}

