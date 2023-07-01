#pragma once
#include "stdafx.h"

typedef struct ClipBoardData {
    std::vector<std::string> files;
    std::string text;
}ClipBoardData;

class ClipBoard {
public:

    ClipBoardData* GetData();

    static ClipBoard* Instance()
    {
        if (_instance == nullptr)
        {
            _instance = new ClipBoard();
        }

        return _instance;
    }


    HWND nextViewer = NULL;
private:
    static ClipBoard* _instance;
    ClipBoardData* data = nullptr;

    ClipBoard();

    void handleFiles();
    void handleText();
};
