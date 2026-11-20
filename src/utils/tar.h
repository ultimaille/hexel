#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstring>
#include <filesystem>
#include <array>

namespace fs = std::filesystem;

// Helper to write a basic ustar header for a file
void writeTarHeader(std::ofstream& tar, const std::string& filename, size_t file_size, bool is_directory) {
    char header[512];
    std::memset(header, 0, 512);

    // File name (max 100 chars in basic ustar)
    std::strncpy(header, filename.c_str(), 100);

    // File mode (octal) - standard read/write for owner/group/others
    std::snprintf(header + 100, 8, "%07o", 0644);
    // User ID
    std::snprintf(header + 108, 8, "%07o", 1000);
    // Group ID
    std::snprintf(header + 116, 8, "%07o", 1000);
    // File size in octal
    std::snprintf(header + 124, 12, "%011zo", file_size);
    // Modification time in octal
    std::snprintf(header + 136, 12, "%011llo", (long long)time(nullptr));

    // Typeflag: '0' or '\0' for regular file
    header[156] = is_directory ? '5' : '0';  // Set before checksum

    // Ustar indicator magic string
    std::memcpy(header + 257, "ustar", 5);
    header[257 + 5] = '\0';

    // Set checksum field to spaces before calculating the checksum
    std::memset(header + 148, ' ', 8);
    // Calculate checksum
    unsigned int checksum = 0;
    for (int i = 0; i < 512; ++i)
        checksum += static_cast<unsigned char>(header[i]);

    std::snprintf(header + 148, 8, "%06o", checksum);
    header[154] = '\0';
    header[155] = ' ';

    tar.write(header, 512);
}

bool create_tar(const std::string& path, const std::string& directory_path) {
    fs::path root = fs::absolute(directory_path);
    if (!fs::exists(root) || !fs::is_directory(root)) return false;

    // Make archive paths relative to the directory's parent.
    fs::path base = root.parent_path();

    std::ofstream tar(path, std::ios::binary);
    if (!tar) return false;

    auto addEntry = [&](const fs::path& path) -> bool {
        bool isDirectory = fs::is_directory(path);
        fs::path archivePath = fs::relative(path, base);
        std::string name = archivePath.generic_string();

        if (isDirectory && !name.empty() && name.back() != '/')
            name += '/';

        // Basic ustar headers support at most 100 filename bytes.
        if (name.size() > 100) return false;

        std::uintmax_t size = isDirectory ? 0 : fs::file_size(path);
        writeTarHeader(tar, name, static_cast<size_t>(size), isDirectory);

        if (!isDirectory) {
            std::ifstream src(path, std::ios::binary);
            if (!src) return false;

            std::array<char, 8192> buffer{};
            std::uintmax_t remaining = size;

            while (remaining > 0) {
                auto count = static_cast<std::streamsize>(
                    std::min<std::uintmax_t>(remaining, buffer.size()));
                src.read(buffer.data(), count);
                if (src.gcount() != count) return false;

                tar.write(buffer.data(), count);
                if (!tar) return false;
                remaining -= count;
            }

            std::size_t padding = (512 - (size % 512)) % 512;
            std::array<char, 512> zeros{};
            if (padding) tar.write(zeros.data(), padding);
        }

        return static_cast<bool>(tar);
    };

    if (!addEntry(root)) return false;

    for (const auto& entry : fs::recursive_directory_iterator(root)) {
        if (!addEntry(entry.path())) return false;
    }

    char endBlocks[1024] = {};
    tar.write(endBlocks, sizeof(endBlocks));
    return static_cast<bool>(tar);
}

static std::uint64_t parse_octal(const char* field, std::size_t length) {
    std::uint64_t value = 0;
    bool foundDigit = false;

    for (std::size_t i = 0; i < length && field[i] != '\0'; ++i) {
        if (field[i] == ' ') continue;
        if (field[i] < '0' || field[i] > '7')
            throw std::runtime_error("Invalid octal field in tar header");
        foundDigit = true;
        value = value * 8 + (field[i] - '0');
    }
    return foundDigit ? value : 0;
}

bool extract_tar(const std::string& path, const std::string& destination) {
    std::ifstream tar(path, std::ios::binary);
    if (!tar) return false;

    fs::path root = fs::absolute(destination);
    fs::create_directories(root);

    std::array<char, 512> header{};
    std::array<char, 8192> buffer{};

    while (tar.read(header.data(), header.size())) {
        bool allZero = true;
        for (char c : header) {
            if (c != '\0') {
                allZero = false;
                break;
            }
        }
        if (allZero) return true; // End of archive

        std::string name(header.data(), strnlen(header.data(), 100));
        if (name.empty()) return false;

        fs::path relative(name);
        if (relative.is_absolute()) return false;
        for (const auto& part : relative)
            if (part == "..") return false;

        fs::path output = root / relative;
        std::uint64_t size = parse_octal(header.data() + 124, 12);
        char type = header[156];

        if (type == '5') { // Directory
            fs::create_directories(output);
            if (size != 0) return false;
        } else if (type == '0' || type == '\0') { // Regular file
            fs::create_directories(output.parent_path());
            std::ofstream file(output, std::ios::binary);
            if (!file) return false;

            std::uint64_t remaining = size;
            while (remaining > 0) {
                auto count = static_cast<std::streamsize>(
                    std::min<std::uint64_t>(remaining, buffer.size()));
                if (!tar.read(buffer.data(), count)) return false;
                file.write(buffer.data(), count);
                if (!file) return false;
                remaining -= count;
            }
        } else {
            return false; // Reject unsupported types, such as links
        }

        // Skip padding up to the next 512-byte block.
        std::uint64_t padding = (512 - (size % 512)) % 512;
        tar.ignore(static_cast<std::streamsize>(padding));
        if (!tar) return false;
    }

    return false;
}