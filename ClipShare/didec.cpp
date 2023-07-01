#include "didec.h"

DiDec::DiDec() {}

bool DiDec::encode_directory(const Path& indir, const Path& outfile) {
    if (!indir.is_dir())
    {
        return false;
    }

    std::fstream file((std::string)outfile, std::ios::out | std::ios::binary);
    if (!file) 
    {
        return false;
    }

    this->recursive_write(indir, Path(), file);
    file.close();

    return true;
}

bool DiDec::decode_directory(const Path& infile, Path outdir) {
    if (!infile.is_file())
    {
        return false;
    }

    std::ifstream ifs(infile);
    if (!ifs) {
        return false;
    }

    std::vector<char> content((std::istreambuf_iterator<char>(ifs)),
        (std::istreambuf_iterator<char>()));
    ifs.close();

    if (!create_if_nexists(outdir))
    {
        return false;
    }

    ParseState state = ParseState::NEXT_CHAR;

    std::string dirPath;

    size_t sizeCount = 0;
    size_t realFileSize = 0;

    std::string filePath;
    std::string fileSize;
    std::vector<char> fileContent;

    for (size_t i = 0; i < content.size(); i++) {
        char ch = content[i];
        switch (state) {
        case FILE_CONTENT:
            if (ch == SEPARATOR && sizeCount == realFileSize) {
                state = ParseState::NEXT_CHAR;

                Path full_filepath = outdir + filePath;
                std::fstream file((std::string)full_filepath, std::ios::out | std::ios::binary);
                if (file.good()) {
                    file.write(reinterpret_cast<const char*>(fileContent.data()),
                        fileContent.size());
                }
                file.close();

                fileContent.clear();
                fileSize.clear();
                filePath.clear();

                realFileSize = 0;
                sizeCount = 0;
            }
            else if (sizeCount >= realFileSize) {
                LOG("[DECODE]: File size error");
                return false;
            }
            else {
                sizeCount += 1;
                fileContent.push_back(ch);
            }
            break;
        case FILE_SIZE:
            if (ch == SEPARATOR) {
                state = ParseState::FILE_CONTENT;
                realFileSize = std::stoi(fileSize);

                if (realFileSize == 0) {
                    state = ParseState::NEXT_CHAR;
                    i++;

                    Path full_filepath = outdir + filePath;
                    create_if_nexists(full_filepath);
                    
                    fileSize.clear();
                    filePath.clear();

                    realFileSize = 0;
                }
            }
            else {
                fileSize += ch;
            }
            break;
        case FILE_NAME:
            if (ch == SEPARATOR) {
                state = ParseState::FILE_SIZE;
            }
            else {
                filePath += ch;
            }
            break;
        case DIR_NAME:
            if (ch == SEPARATOR) {
                state = ParseState::NEXT_CHAR;

                Path full_dirpath = outdir.copy().append(dirPath).to_dir();
                create_if_nexists(full_dirpath);
                dirPath.clear();
            }
            else {
                dirPath += ch;
            }
            break;
        case NEXT_CHAR:
            if (ch == DIRECTORY_CHAR) {
                state = ParseState::DIR_NAME;
            }
            else if (ch == FILE_CHAR) {
                state = ParseState::FILE_NAME;
            }
            else {
                LOG("[DECODE]: Parse error");
                return false;
            }
            break;
        default:
            LOG("[DECODE]: State not found");
            return false;
        }
    }

    return true;
}

void DiDec::recursive_write(Path path, Path base,
    std::fstream& file) {
    read_dir(path, [&](const WIN32_FIND_DATAA& findData) {
        Path itempath = path.copy().append(findData.cFileName);
        Path basepath = base.copy().append(findData.cFileName);

            if (is_valid_dir(itempath))
            {
                itempath.to_dir();
                basepath.to_dir();

                file << DIRECTORY_CHAR << basepath << SEPARATOR;
                this->recursive_write(itempath, basepath, file);
            }
            else if (file_exists(itempath))
            {
                std::ifstream ifs(itempath);
                if (ifs.good()) 
                {
                    std::vector<char> content((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));
                    file << FILE_CHAR << basepath << SEPARATOR << std::to_string(content.size()) << SEPARATOR;

                    file.write(reinterpret_cast<const char*>(content.data()), content.size());
                    file << SEPARATOR;
                }
            }

        return true;
        });
}
