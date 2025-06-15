// fired.cpp
// Fired Terminal – Fast C++ Terminal File Manager with ANSI UI

#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <fstream>


namespace fs = std::filesystem;

// ANSI color codes
const std::string RESET  = "\033[0m";
const std::string BLUE   = "\033[1;34m";
const std::string CYAN   = "\033[1;36m";
const std::string GREEN  = "\033[1;32m";
const std::string YELLOW = "\033[1;33m";
const std::string WHITE  = "\033[1;37m";

// Convert bytes to human‐readable string
std::string human_size(uintmax_t bytes) {
    double b = static_cast<double>(bytes);
    std::ostringstream oss;
    if (b >= (1<<20)) {
        oss << std::fixed << std::setprecision(1) << (b/(1<<20)) << "MB";
    } else if (b >= (1<<10)) {
        oss << std::fixed << std::setprecision(1) << (b/(1<<10)) << "KB";
    } else {
        oss << b << "B";
    }
    return oss.str();
}

// Render the table UI with ANSI colors
void print_table(const std::vector<std::pair<std::string,bool>>& items) {
    auto cwd = fs::current_path();
    std::cout << BLUE << cwd.string() << RESET << "\n\n";

    // Header
    std::cout << WHITE
              << "-name-----------------------size-----------------------"
              << RESET << "\n";

    // Rows
    for (auto& [name, is_dir] : items) {
        uintmax_t size = 0;
        if (!is_dir) {
            try { size = fs::file_size(cwd / name); }
            catch(...) {}
        }
        std::string hs = human_size(size);

        std::cout
            << WHITE << "| " << RESET
            << (is_dir ? CYAN : GREEN)
            << std::left << std::setw(25) << name
            << RESET
            << WHITE << "| " << RESET
            << YELLOW << std::right << std::setw(7) << hs
            << RESET << "\n";
    }

    // Footer
    std::cout << WHITE
              << "--------------------------------------------------------"
              << RESET << "\n";
}

// Implementation of 'fd' command with flags
void fd_command(const std::vector<std::string>& args) {
    bool show_hidden   = false;
    bool no_extension  = false;
    bool sort_alpha    = false;
    std::string filter_ext;

    // Parse flags
    for (size_t i = 0; i < args.size(); ) {
        const auto& a = args[i];
        if (a == "--hidden") { show_hidden = true;  i++; }
        else if (a == "-e") { no_extension = true;  i++; }
        else if (a == "-p") {
            if (i+1 < args.size()) filter_ext = args[i+1];
            i += 2;
        }
        else if (a.rfind("-p",0) == 0 && a.size()>2) {
            filter_ext = a.substr(2);
            i++;
        }
        else if (a.size()==1 && std::isalpha(a[0])) {
            sort_alpha = true;
            i++;
        }
        else i++;
    }

    // Gather entries
    std::vector<std::pair<std::string,bool>> items;
    for (auto& entry : fs::directory_iterator(fs::current_path())) {
        auto name   = entry.path().filename().string();
        bool is_dir = entry.is_directory();

        if (!show_hidden && name.rfind(".",0)==0)           continue;
        if (no_extension && !is_dir && entry.path().has_extension()) continue;
        if (!filter_ext.empty() && entry.path().extension() != "."+filter_ext)
                                                            continue;

        items.emplace_back(name,is_dir);
    }

    // Sort if requested
    if (sort_alpha) {
        std::sort(items.begin(), items.end(),
                  [](auto& a, auto& b){ return a.first < b.first; });
    }

    print_table(items);
}

// Parse and execute a single command line
void process_command(const std::string& line) {
    std::istringstream iss(line);
    std::vector<std::string> tok;
    std::string w;
    while (iss >> w) tok.push_back(w);
    if (tok.empty()) return;

    auto cmd = tok[0];
    tok.erase(tok.begin());

    try {
        if (cmd == "fd" || cmd == "ls") {
            fd_command(tok);
        }
        else if (cmd == "pwd") {
            std::cout << fs::current_path() << "\n";
        }
        else if (cmd == "cd" && !tok.empty()) {
            fs::current_path(fs::path(tok[0]));
        }
        else if (cmd == "mkdir" && !tok.empty()) {
            fs::create_directory(tok[0]);
        }
        else if (cmd == "rmdir" && !tok.empty()) {
            fs::remove_all(tok[0]);
        }
        else if (cmd == "rm" && !tok.empty()) {
            fs::remove(tok[0]);
        }
        else if (cmd == "touch" && !tok.empty()) {
            std::ofstream file(tok[0]);
            if (!file)
                std::cerr << "Error al crear archivo\n";
            // file.close() 
        }

        else if (cmd == "mv" && tok.size()==2) {
            fs::rename(tok[0], tok[1]);
        }
        else if (cmd == "cp" && tok.size()==2) {
            fs::copy_file(tok[0], tok[1], fs::copy_options::overwrite_existing);
        }
        else if (cmd == "clear") {
            std::cout << "\033[2J\033[H";
        }
        else if (cmd == "exit") {
            std::exit(0);
        }
        else {
            std::cout << "Comando no reconocido: " << cmd << "\n";
        }
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
    }
}

int main() {
    std::string line;
    std::cout << "Fired Terminal - File Manager \n";
    while (true) {
        std::cout << BLUE << "> " << RESET;
        if (!std::getline(std::cin, line)) break;
        process_command(line);
    }
    return 0;
}
