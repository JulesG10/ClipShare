#pragma once
#include "stdafx.h"

#define FILE_SEPARATOR '\\'

class Path {
public:
	Path();
	Path(std::string path);
	Path(Path& path);

	Path& append(std::string path, bool is_dir = false);
	Path& append(Path& path);

	Path& back();

	Path& to_dir();
	Path& to_file();

	operator std::string() const;
	explicit operator const char* () const;
	explicit operator char* () const;

	std::string get_name() const;
	std::string get_ext() const;

	bool is_dir() const;
	bool is_file() const;

	Path copy();

	Path operator+(Path other);
	Path operator+(std::string other);

	Path& operator+=(Path other);
	Path& operator+=(std::string other);

	Path& operator=(const Path& other);
	Path operator=(std::string& other);
	
	friend std::ostream& operator<<(std::ostream& os, const Path& path);
private:
	std::string fix_path(std::string path);

	bool m_directory = false;
	std::string m_value = std::string();
};

bool create_if_nexists(const Path& path);
bool file_exists(const Path& path);
bool dir_exists(const Path& path);
bool is_valid_dir(const Path& path);
void read_dir(Path path, std::function<bool(const WIN32_FIND_DATAA&)> handleFunc);
Path get_temp_path();
