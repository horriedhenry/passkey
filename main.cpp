#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#define endl std::endl

void split_by_delim(std::vector<std::string>& res_vec,std::string& str) {
    // parse_string splits string by delimeter, to vec of strings
    // from this : "site_name,email,pwd;"
    // to this : {"site_name", "email", "pwd"} which is pushed to res_vec.
    std::queue<char> q;
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
}

void parse_to_map(std::multimap<std::string, std::pair<std::string, std::string>>& map,std::vector<std::vector<std::string>>& vec) {
    // map : <site, <email, password>
    // from this : {{"site_name", "email", "pwd"}}
    // to this :
    // { "site_name" : <"email", "pwd"> }
    // makes it easy to search based on site_name
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::vector<std::string> temp = *it;
        // for (auto itr = it->begin(); itr != it->end(); ++itr) {
        //     std::cout << *itr << " ";
        // }
        map.insert({temp[0], std::make_pair(temp[1], temp[2])});
    }
}

void load_passwords(std::string file_name, std::vector<std::string>& lines) {
    // lines : {"site_name,email,pwd;"}
    std::ifstream file;
    file.open(file_name);
    if(file.is_open()) {
        std::string curr_line;
        while (std::getline(file,curr_line)) {
            lines.push_back(curr_line);
        }
    } else {
        std::cout << "cannot open file" << endl;
    }
}

void map_passwords(std::vector<std::vector<std::string>>& passwords, std::vector<std::string> lines) {
    // from this : {"site_name,email,pwd;", "site_name1,email,pwd;"} (lines)
    // to this : {"site_name", "email", "pwd"} (vecto<string> res)
    // to this : {{"site_name", "email", "pwd"}, {"site_name1", "email", "pwd"}} (res is pushed to passwords)
    for (auto it = lines.begin(); it != lines.end(); ++it) {
        std::string curr_line = *it;
        std::vector<std::string> res;
        split_by_delim(res,curr_line);
        passwords.push_back(res);
    }
}

void get_password(std::string& site_name, std::multimap<std::string, std::pair<std::string, std::string>> dict) {
    auto range = dict.equal_range(site_name);
    std::cout << "" << site_name << ":\n";
    for (auto it = range.first; it != range.second; ++it) {
        std::cout << "email    : " << it->second.first << "\n" << "password : " << it->second.second << endl;
    }
}

int main (int argc, char *argv[]) {
    std::vector<std::string> lines;
    load_passwords("./passwords.txt", lines);

    std::vector<std::vector<std::string>> passwords;
    map_passwords(passwords, lines);

    std::multimap<std::string, std::pair<std::string, std::string>> map; 
    parse_to_map(map, passwords);

    std::string site_name;
    std::getline(std::cin, site_name);
    get_password(site_name, map);
}
