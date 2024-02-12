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
    // TODO : Do something with entries with simillar site_name
    // something (maybe or think of something better) : check no.of entries with simillar site_name and then allow user
    // to filter results further to search with email in simillar site_name list of entries.
    bool found = false;
    for (int i = 0; i < entries.size(); i++) {
        if (entries[i].site_name == site_name) {
            found = true;
            std::cout << "\t" << entries[i].site_name << endl;
            std::cout << "email     : " << entries[i].email << endl;
            std::cout << "password  : " << entries[i].password << endl;
        }
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
