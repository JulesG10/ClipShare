#include "path.h"

Path::Path()
{
}

Path::Path(std::string path)
{
    if (path.back() == '\\' || path.back() == '/')
    {
        this->append(path, true);
    }
    else {
        this->append(path);
    }
}

Path::Path(Path& path)
{
    this->m_value = path.m_value;
    this->m_directory = path.m_directory;
}


Path& Path::append(std::string path, bool is_dir) {
    if (!this->m_value.empty() && this->m_directory) {
        this->m_value.pop_back();
    }

    this->m_directory = is_dir;
    this->m_value = this->fix_path(this->m_value + FILE_SEPARATOR + path);
    return *this;
}

Path& Path::append(Path& path) {
    return this->append(path.m_value, path.m_directory);
}

Path& Path::back()
{
    if(this->m_value.empty())
    {
        return *this;
    }

    std::string tmp = this->m_value;
    if(this->m_directory)
    {
        tmp.pop_back();
    }

    size_t found = tmp.find_last_of(FILE_SEPARATOR);
    if (found != std::string::npos) {
        this->m_value = tmp.substr(0, found + 1);
    }

    this->m_directory = true;
    
    return *this;
}

std::string Path::fix_path(std::string path) {
    if (path.empty())
    {
        return path;
    }

    std::vector<std::string> parts;

    std::string current_part;
    for (size_t i = 0; i < path.size(); i++) {
        if (path[i] == '\\' || path[i] == '/') {
            parts.emplace_back(std::move(current_part));
        }
        else {
            current_part += path[i];
            if (i == path.size() - 1)
            {
                parts.emplace_back(std::move(current_part));
            }
        }
    }

    std::string fixed;
    for (std::string& part : parts) {
        if (!part.empty()) {
            fixed += part + FILE_SEPARATOR;
        }
    }

    if (!this->m_directory && !fixed.empty()) {
        fixed.pop_back();
    }

    return fixed;
}

std::string Path::get_name() const {
    if (this->m_value.empty()) {
        return std::string();
    }

    std::string tmp_name = this->m_value;

    if (this->m_directory) {
        tmp_name.pop_back();
    }

    size_t index = tmp_name.find_last_of(FILE_SEPARATOR) + 1;
    if (index == std::string::npos) {
        return std::string();
    }

    return std::string(tmp_name.substr(index));
}




std::string Path::get_ext() const {
    if (this->m_directory) {
        return std::string();
    }

    size_t index = this->m_value.find_last_of(".");

    if (index == std::string::npos) {
        return std::string();
    }

    return this->m_value.substr(index);
}


bool Path::is_dir() const
{
    return this->m_directory;
}

bool Path::is_file() const
{
    return !this->m_directory;
}

Path::operator std::string() const
{
    return this->m_value;
}

Path Path::operator+(Path other)
{
    Path newp = *this;
    newp.append(other);
    return newp;
}

Path::operator const char* () const
{
    return this->m_value.c_str();
}

Path Path::operator+(std::string other)
{
    Path newp = *this;
    newp.append(other);
    return newp;
}

Path::operator char* () const
{
    return (char*)this->m_value.c_str();
}

Path& Path::to_dir()
{
    if (!this->m_directory)
    {
        this->m_value += FILE_SEPARATOR;
        this->m_directory = true;
    }

    return *this;
}

Path& Path::to_file()
{
    if (this->m_directory)
    {
        this->m_value.pop_back();
        this->m_directory = false;
    }

    return *this;
}

Path& Path::operator+=(Path other)
{
    this->append(other);
    return *this;
}

Path& Path::operator+=(std::string other)
{
    this->append(other);
    return *this;
}

Path Path::copy()
{
    return Path(*this);
}

Path& Path::operator=(const Path& other)
{
    if (this != &other) {
        this->m_value = other.m_value;
        this->m_directory = other.m_directory;
    }
    return *this;
}

Path Path::operator=(std::string& other)
{
    return Path(other);
}

/*
std::ostream& operator<<(std::ostream& os, const Path& path)
{
    os << path.m_value;
    return os;
}
*/

bool create_if_nexists(const Path& path)
{
    if (path.is_file() && !file_exists(path))
    {
        HANDLE hFile = CreateFileA((char*)path, GENERIC_WRITE, NULL, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
        if (hFile == INVALID_HANDLE_VALUE) 
        {
            return false;
        }
        CloseHandle(hFile);
    }
    else if (path.is_dir() && !dir_exists(path))
    {
        return CreateDirectoryA((LPCSTR)path, NULL);
    }

    return true;
}

bool file_exists(const Path& path)
{
    if (!path.is_file())
    {
        return false;
    }
    DWORD dwAttrib = GetFileAttributesA((LPCSTR)path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool dir_exists(const Path& path)
{
    if (!path.is_dir())
    {
        return false;
    }
    DWORD dwAttrib = GetFileAttributesA((LPCSTR)path);
    return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
        (dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool is_valid_dir(const Path& path)
{
    std::string name = path.get_name();
    DWORD dwAttrib = GetFileAttributesA((LPCSTR)path);
    return (dwAttrib & FILE_ATTRIBUTE_DIRECTORY) && name != "." && name != "..";
}


void read_dir(Path path, std::function<bool(const WIN32_FIND_DATAA&)> handleFunc)
{
    WIN32_FIND_DATAA findData;
    HANDLE findHandle = FindFirstFileA(((std::string)path.to_file() + "\\*").c_str(), &findData);

    if (findHandle == INVALID_HANDLE_VALUE) {
        FindClose(findHandle);
        return;
    }

    do {
        if (!handleFunc(findData))
        {
            break;
        }
    } while (FindNextFileA(findHandle, &findData));

    FindClose(findHandle);
}

Path get_temp_path()
{
    Path tmp;
    char tempPath[MAX_PATH];

    DWORD result = GetTempPathA(MAX_PATH, tempPath);
    if (result <= 0 && result >= MAX_PATH)
    {
        return tmp;
    }

    tmp.append(tempPath, true);
    return tmp;
}

std::ostream& operator<<(std::ostream& os, const Path& path)
{
    os << path.m_value;
    return os;
}
