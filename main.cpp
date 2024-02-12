#include <cstdio>
#include <iostream>
#include <fstream>
#include <queue>
#include <string>
#include <vector>

#define endl std::endl
#define passwords_file "./passwords.txt"

typedef struct entry {
    std::string site_name;
    std::string email;
    std::string password;
} entry;

std::vector<entry> entries;

entry alloc(std::string site_name, std::string email, std::string password) {
    entry e;
    e.site_name = site_name;
    e.email = email;
    e.password = password;
    return e;
}

void split_by_delim(std::string& str) {
    // {"site_name", "email", "pwd"}
    std::queue<char> q;
    std::vector<std::string> res_vec;
    for (auto it = str.begin(); it != str.end(); ++it) {
        if (*it != ',' && *it != ';') {
            q.push(*it);
        } else {
            std::string seperate = "";
            while(!q.empty()) {
                seperate += q.front();
                q.pop();
            }
            res_vec.push_back(seperate);
        }
    }
    entries.push_back(alloc(res_vec[0], res_vec[1], res_vec[2]));
}

void load_passwords(std::string file_name) {
    std::ifstream file;
    file.open(file_name);
    if(file.is_open()) {
        std::string curr_line;
        while (std::getline(file,curr_line)) {
            split_by_delim(curr_line);
        }
    } else {
        std::cout << "[INFO] File not found" << endl;
    }
    file.close();
}

void get_password(const std::string& site_name) {
    bool found = false;
    std::vector<entry> similar;
    for (int i = 0; i < entries.size(); i++) {
        if (entries[i].site_name == site_name) {
            found = true;
            similar.push_back(entries[i]);
        }
    }
    if (similar.size() > 1) {
        std::cout << "[INFO] found " << similar.size() << " entries for " << similar[0].site_name << endl;
        std::cout << "[INFO] <p - print all> or <f - filter by email> ";
        char input;
        std::cin >> input;
        if (input == 'p') {
            for (int i = 0; i < similar.size(); i++) {
                std::cout << "\t" << similar[i].site_name << endl;
                std::cout << "email     : " << similar[i].email << endl;
                std::cout << "password  : " << similar[i].password << endl;
            }
        } else if (input == 'f') {
            std::cout << "[INFO] Enter email" << endl;
            std::string email;
            std::cin >> email;
            for (int i = 0; i < similar.size(); i++) {
                if (similar[i].email == email) {
                    std::cout << "\t" << similar[i].site_name << endl;
                    std::cout << "email     : " << similar[i].email << endl;
                    std::cout << "password  : " << similar[i].password << endl;
                }
            }
        }
    } else {
        std::cout << "\t" << similar[0].site_name << endl;
        std::cout << "email     : " << similar[0].email << endl;
        std::cout << "password  : " << similar[0].password << endl;
    }
    if (!found) {
        std::cout << "[INFO] No entry found" << endl;
    }
}

void add_new_entry(std::string site_name, std::string email, std::string password) {
    std::string line;
    line.append(site_name+",");
    line.append(email+",");
    line.append(password+";");
    std::ofstream ofs;
    ofs.open(passwords_file, std::ios::app);
    if(ofs.is_open()) {
        ofs << line << endl;
    }
    ofs.close();
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        std::cerr << "USAGE : " << argv[0] << "  <g - Get Entry> <site_name> " << endl;
        std::cerr << "USAGE : " << argv[0] << "  <a - Add Entry> <site_name> <email> <password>" << endl;
        return 1;
    }

    const char arg1 = (char)*argv[1];
    if (arg1 == 'g') {
        if (argc != 3) {
            std::cerr << "USAGE : " << argv[0] << "  <g - Get Entry> <site_name> " << endl;
            return 1;
        } else {
            load_passwords(passwords_file);
            std::string site_name = (std::string)argv[2];
            get_password(site_name);
            return 0;
        }
    }

    if (arg1 == 'a') {
        if (argc != 5) {
            std::cerr << "USAGE : " << argv[0] << "  <a - Add Entry> <site_name> <email> <password>" << endl;
            return 1;
        } else {
            std::string site_name = (std::string )argv[2];
            std::string email = (std::string )argv[3];
            std::string password = (std::string )argv[4];
            add_new_entry(site_name, email, password);
            return 0;
        }
    }
    return 0;
}
