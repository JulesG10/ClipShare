#include "clipboard.h"

ClipBoard::ClipBoard()
{

}


ClipBoardData* ClipBoard::GetData()
{
    this->data = new ClipBoardData();

    if (!OpenClipboard(NULL))
    {
        return data;
    }

    if (IsClipboardFormatAvailable(CF_HDROP))
    {
        this->handleFiles();
    }
    else if (IsClipboardFormatAvailable(CF_TEXT))
    {
        this->handleText();
    }

    CloseClipboard();
    return data;
}

void ClipBoard::handleFiles()
{
    HDROP hDrop = static_cast<HDROP>(GetClipboardData(CF_HDROP));
    if (hDrop == NULL)
    {
        return;
    }

    UINT numFiles = DragQueryFileA(hDrop, 0xFFFFFFFF, NULL, 0);
    if (numFiles <= 0)
    {
        return;
    }

    data->files.reserve(numFiles);
    for (UINT i = 0; i < numFiles; ++i)
    {
        UINT pathLength = DragQueryFileA(hDrop, i, NULL, 0);
        if (pathLength > 0)
        {
            std::vector<char> buffer(pathLength + 1);
            if (DragQueryFileA(hDrop, i, buffer.data(), pathLength + 1) > 0)
            {
                data->files.emplace_back(buffer.data());
            }
        }
    }
}

void ClipBoard::handleText()
{
    HANDLE hText = GetClipboardData(CF_TEXT);
    if (hText == NULL)
    {
        return;
    }

    char* pText = static_cast<char*>(GlobalLock(hText));
    GlobalUnlock(hText);

    if (pText == NULL)
    {
        return;
    }

    data->text = pText;
}

ClipBoard* ClipBoard::_instance = nullptr;