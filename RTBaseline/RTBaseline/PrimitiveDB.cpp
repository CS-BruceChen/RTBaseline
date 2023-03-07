#include "PrimitiveDB.h"

std::vector<Sequence> PrimitiveDB::TDBArray(0);
std::vector<Sequence> PrimitiveDB::PDBArray(0);
std::vector<Sequence> PrimitiveDB::QDBArray(0);
CoordBound PrimitiveDB::getBound(std::vector<Sequence> primitive) {
    float MAXX=DBL_MIN, MAXY=DBL_MIN, MINX=DBL_MAX, MINY=DBL_MAX;
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

//ADD
void TrajectoryDB::ADD(std::vector<Sequence> seqArray) {
    for (size_t i = 0; i < seqArray.size(); ++i) {
        TDBArray.push_back(seqArray[i]);
    }
    DBLog.set_run_add_info("trajectory", seqArray.size());
}
void PolygonDB::ADD(std::vector<Sequence> seqArray) {
    for (size_t i = 0; i < seqArray.size(); ++i) {
        PDBArray.push_back(seqArray[i]);
    }
    DBLog.set_run_add_info("polygon", (int)seqArray.size());
}
void QueryTrajDB::ADD(std::vector<Sequence> seqArray) {
    for (size_t i = 0; i < seqArray.size(); ++i) {
        QDBArray.push_back(seqArray[i]);
    }
    DBLog.set_run_add_info("trajectory-to-be-queried", (int)seqArray.size());
}

//DELETE
void TrajectoryDB::DELETE(IDArray ids) {
    std::vector<int> errIds;
    std::vector<int> deleteIds;
    for (size_t i = 0; i < ids.size(); ++i) {
        size_t eraseIndex = ids[i];
        if (eraseIndex >= TDBArray.size())
            errIds.push_back(eraseIndex);
        else
            deleteIds.push_back(eraseIndex);
    }
    DBLog.set_run_delete_info("trajectory", deleteIds.size(), errIds);
    //deleteIds 降序并去重，避免重复删除和动态删除引起的下标变化
    sort(deleteIds.begin(), deleteIds.end(), [=](size_t a, size_t b)->bool {return a > b;});
    for (size_t i = 0; i < deleteIds.size(); ++i) {
        if (i < deleteIds.size() - 1 && deleteIds[i] == deleteIds[i + 1]) continue;
        TDBArray.erase(TDBArray.begin() + deleteIds[i]);
    }
}
void PolygonDB::DELETE(IDArray ids) {
    std::vector<int> errIds;
    std::vector<int> deleteIds;
    for (size_t i = 0; i < ids.size(); ++i) {
        size_t eraseIndex = ids[i];
        if (eraseIndex >= PDBArray.size())
            errIds.push_back(eraseIndex);
        else
            deleteIds.push_back(eraseIndex);
    }
    DBLog.set_run_delete_info("trajectory", deleteIds.size(), errIds);
    sort(deleteIds.begin(), deleteIds.end(), [=](size_t a, size_t b)->bool {return a > b; });
    for (size_t i = 0; i < deleteIds.size(); ++i) {
        if (i < deleteIds.size() - 1 && deleteIds[i] == deleteIds[i + 1]) continue;
        PDBArray.erase(PDBArray.begin() + deleteIds[i]);
    }
}
void QueryTrajDB::DELETE(IDArray ids) {
    std::vector<int> errIds;
    std::vector<int> deleteIds;
    for (size_t i = 0; i < ids.size(); ++i) {
        size_t eraseIndex = ids[i];
        if (eraseIndex >= QDBArray.size())
            errIds.push_back(eraseIndex);
        else
            deleteIds.push_back(eraseIndex);
    }
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
void TrajectoryDB::PRINT(IDArray input_ids) {
    std::string printInfo = "";
    std::vector<int> errIds;
    std::vector<int> result_ids;
    for (size_t i = 0; i < input_ids.size(); ++i) {
        size_t printIndex = input_ids[i];
        if (printIndex >= TDBArray.size())
            errIds.push_back(printIndex);
        else
            result_ids.push_back(printIndex);
    }
    for (size_t i = 0; i < result_ids.size(); ++i) {
        size_t id = result_ids[i];
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
void PolygonDB::PRINT(IDArray input_ids) {
    std::string printInfo = "";
    std::vector<int> errIds;
    std::vector<int> result_ids;
    for (size_t i = 0; i < input_ids.size(); ++i) {
        size_t printIndex = input_ids[i];
        if (printIndex >= PDBArray.size())
            errIds.push_back(printIndex);
        else
            result_ids.push_back(printIndex);
    }
    for (size_t i = 0; i < result_ids.size(); ++i) {
        size_t id = result_ids[i];
        printInfo += "tid: " + std::to_string(id) + "\n";
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
void QueryTrajDB::PRINT(IDArray input_ids) {
    std::string printInfo = "";
    std::vector<int> errIds;
    std::vector<int> result_ids;
    for (size_t i = 0; i < input_ids.size(); ++i) {
        size_t printIndex = input_ids[i];
        if (printIndex >= QDBArray.size())
            errIds.push_back(printIndex);
        else
            result_ids.push_back(printIndex);
    }
    for (size_t i = 0; i < result_ids.size(); ++i) {
        size_t id = result_ids[i];
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


IDArray TrajectoryDB::SELECT(IDArray tids, IDArray pids, int topk) {
    qDebug() << "t slt\n";
    IDArray newid;
    return newid;
}
IDArray PolygonDB::SELECT(IDArray tids, IDArray pids, int topk) {
    qDebug() << "p slt\n";
    IDArray newid;
    return newid;
}
IDArray QueryTrajDB::SELECT(IDArray tids, IDArray pids, int topk) {
    qDebug() << "q slt\n";
    IDArray newid;
    return newid;
}


void TrajectoryDB::SHOW(IDArray ids) {
    std::vector<int> errIds;
    std::vector<int> showIds;
    for (size_t i = 0; i < ids.size(); ++i) {
        size_t showIndex = ids[i];
        if (showIndex >= TDBArray.size())
            errIds.push_back(showIndex);
        else
            showIds.push_back(showIndex);
    }
    std::vector<Sequence>* db = &TDBArray;
    const char* windowTitle = "RT Terminal-Trajectory Visualization";
    initOpenGL();
    GLFWwindow* window = createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle);
    initWindowAndGlad(window);

    CoordBound cb = PrimitiveDB::getBound(*db);
    Primitive** trajArray = new Primitive* [showIds.size()];//指针数组的首地址是指向指针的指针
    for (size_t i = 0; i < showIds.size(); ++i) {
        unsigned id = showIds[i];
        Sequence seq = (*db)[id];
        //todo:类似main中的line，生成一个line指针数组。每个指针都对应一个new出来的Primitive
        unsigned vertsNum = seq.size();
        std::vector<float> verts;
        for (size_t j = 0; j < seq.size(); ++j) {
            verts.push_back(seq[j].x);
            verts.push_back(seq[j].y);
            verts.push_back(id);
        }
        trajArray[i] = new Primitive(vertsNum, verts.data());
    }
    
    Shader* shader = Shader::newShader("drawPrimitives");
    shader->use();
    shader->setFloat("MAXX", cb.maxx);
    shader->setFloat("MAXY", cb.maxy);
    shader->setFloat("MINX", cb.minx);
    shader->setFloat("MINY", cb.miny);
    shader->setFloat("MAXN", (float)showIds.size());
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
    std::vector<int> errIds;
    std::vector<int> showIds;
    for (size_t i = 0; i < ids.size(); ++i) {
        size_t showIndex = ids[i];
        if (showIndex >= PDBArray.size())
            errIds.push_back(showIndex);
        else
            showIds.push_back(showIndex);
    }
    std::vector<Sequence>* db = &PDBArray;
    const char* windowTitle = "RT Terminal-Polygon Visualization";
    initOpenGL();
    GLFWwindow* window = createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle);
    initWindowAndGlad(window);

    CoordBound cb = PrimitiveDB::getBound(*db);
    Primitive** polyArray = new Primitive * [showIds.size()];//指针数组的首地址是指向指针的指针
    for (size_t i = 0; i < showIds.size(); ++i) {
        unsigned id = showIds[i];
        Sequence seq = (*db)[id];
        std::vector<float> verts;
        triangulatePolygons(seq, verts, id);
        unsigned vertsNum = verts.size() / 3;
        polyArray[i] = new Primitive(vertsNum, verts.data());
    }

    Shader* shader = Shader::newShader("drawPrimitives");
    shader->use();
    shader->setFloat("MAXX", cb.maxx);
    shader->setFloat("MAXY", cb.maxy);
    shader->setFloat("MINX", cb.minx);
    shader->setFloat("MINY", cb.miny);
    shader->setFloat("MAXN", (float)showIds.size());
    DBLog.set_show_info("trajectory", showIds.size(), errIds);
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
    std::vector<int> errIds;
    std::vector<int> showIds;
    for (size_t i = 0; i < ids.size(); ++i) {
        size_t showIndex = ids[i];
        if (showIndex >= QDBArray.size())
            errIds.push_back(showIndex);
        else
            showIds.push_back(showIndex);
    }
    std::vector<Sequence>* db = &QDBArray;
    const char* windowTitle = "RT Terminal-Trajectory-to-be queried Visualization";
    initOpenGL();
    GLFWwindow* window = createWindow(WINDOW_WIDTH, WINDOW_HEIGHT, windowTitle);
    initWindowAndGlad(window);

    CoordBound cb = PrimitiveDB::getBound(*db);
    Primitive** qtrajArray = new Primitive * [showIds.size()];//指针数组的首地址是指向指针的指针
    for (size_t i = 0; i < showIds.size(); ++i) {
        unsigned id = showIds[i];
        Sequence seq = (*db)[id];
        //todo:类似main中的line，生成一个line指针数组。每个指针都对应一个new出来的Trajectory
        unsigned vertsNum = seq.size();
        std::vector<float> verts;
        for (size_t j = 0; j < seq.size(); ++j) {
            verts.push_back(seq[j].x);
            verts.push_back(seq[j].y);
            verts.push_back(id);
        }
        qtrajArray[i] = new Primitive(vertsNum, verts.data());
    }

    Shader* shader = Shader::newShader("drawPrimitives");
    shader->use();
    shader->setFloat("MAXX", cb.maxx);
    shader->setFloat("MAXY", cb.maxy);
    shader->setFloat("MINX", cb.minx);
    shader->setFloat("MINY", cb.miny);
    shader->setFloat("MAXN", (float)showIds.size());
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

