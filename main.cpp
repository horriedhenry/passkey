#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#define endl std::endl

void parse_string(std::vector<std::string>& res_vec,std::string& str) {
    // parse_string splits string by delimeter, to vec of strings
    // from this : "site_name,email,pwd;"
    // to this : {{"site_name", "email", "pwd"}} which is pushed to res_vec.
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

void parse_to_map(std::multimap<std::string, std::pair<std::string, std::string>>& map,std::vector<std::vector<std::string>> vec) {
    // from this : {{"site_name", "email", "pwd"}}
    // to this :
    // { "site_name" : <"email", "pwd"> }
    // makes it easy to search
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::vector<std::string> temp = *it;
        // for (auto itr = it->begin(); itr != it->end(); ++itr) {
        //     std::cout << *itr << " ";
        // }
        map.insert({temp[0], std::make_pair(temp[1], temp[2])});
    }
}

int main (int argc, char *argv[]) {
    std::ifstream file;
    file.open("./passwords.txt");
    std::vector<std::string> lines;
    // {"site_name,email,pwd;"}
    if(file.is_open()) {
        std::string curr_line;
        while (std::getline(file,curr_line)) {
        // curr_line : {"site_name,email,pwd;"}
            lines.push_back(curr_line);
        }
    } else {
        std::cout << "cannot open file" << endl;
    }

    std::vector<std::vector<std::string>> passwords;
    // passwords = {{"site_name", "email", "pwd"}, {"site_name_2", "email", "pwd"}}
    for (auto it = lines.begin(); it != lines.end(); ++it) {
        std::string curr_line = *it;
        std::vector<std::string> res;
        parse_string(res,curr_line);
        passwords.push_back(res);
    }

    std::multimap<std::string, std::pair<std::string, std::string>> map; 
    // map : <site, <email, password>
    parse_to_map(map, passwords);
    std::string keyToFind;
    std::getline(std::cin, keyToFind);
    auto range = map.equal_range(keyToFind);

    std::cout << "Values associated with key " << keyToFind << ":\n";
    for (auto it = range.first; it != range.second; ++it) {
        std::cout << it->second.first << ", " << it->second.second << endl;
    }
}
