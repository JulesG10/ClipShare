#pragma once
#include "stdafx.h"
#include "path.h"

#define DIRECTORY_CHAR 'D'
#define FILE_CHAR 'F'
#define SEPARATOR '|'

enum ParseState { FILE_SIZE, FILE_NAME, FILE_CONTENT, DIR_NAME, NEXT_CHAR };


class DiDec {
 public:
  DiDec();

  bool encode_directory(const Path& indir, const Path& outfile);
  bool decode_directory(const Path& infile, Path outdir);

 private:
  void recursive_write(Path path, Path base, std::fstream& file);
};