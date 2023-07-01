#pragma once
#include "stdafx.h"

class ShareData {
public:
    std::string text = "";

    std::vector<std::string> files = {};
    std::vector<std::string> request_files = {};
    std::vector<std::vector<std::string>> response_files = {};

    int id = 0;

    ShareData();

    static ShareData parse(const std::string& data);
    std::string build() const;

};