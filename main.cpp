#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#define endl std::endl

std::vector<std::string> parse_string(std::string& str) {
    std::vector<std::string> res;
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
            res.push_back(seperate);
        }
    }
    // for (auto it = res.begin(); it != res.end(); ++it) {
    //     // std::string curr_line = *it;
    //     std::cout << *it << endl;
    // }
    return res;
}

std::multimap<std::string, std::pair<std::string, std::string>> parse_to_map(std::vector<std::vector<std::string>> vec) {
    std::multimap<std::string, std::pair<std::string, std::string>> map;
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::vector<std::string> temp = *it;
        // for (auto itr = it->begin(); itr != it->end(); ++itr) {
        //     std::cout << *itr << " ";
        // }
        map.insert({temp[0], std::make_pair(temp[1], temp[2])});
    }
    return map;
}

int main (int argc, char *argv[]) {
    std::ifstream file;
    file.open("./passwords.txt");
    std::vector<std::string> lines;
    if(file.is_open()) {
        std::string curr_line;
        while (std::getline(file,curr_line)) {
            // std::cout << curr_line << endl;
            lines.push_back(curr_line);
        }
    } else {
        std::cout << "cannot open file" << endl;
    }

    std::vector<std::vector<std::string>> passwords;
    for (auto it = lines.begin(); it != lines.end(); ++it) {
        std::string curr_line = *it;
        std::vector<std::string> res = parse_string(curr_line);
        passwords.push_back(res);
    }

    std::multimap<std::string, std::pair<std::string, std::string>> map = parse_to_map(passwords);
    std::string keyToFind;
    std::getline(std::cin, keyToFind);
    auto range = map.equal_range(keyToFind);

    std::cout << "Values associated with key " << keyToFind << ":\n";
    // for (auto it = range.first; it != range.second; ++it) {
    //     std::cout << it->second.first << ", " << it->second.second << endl;
    // }
    for (auto it = range.first; it != range.second; ++it) {
        std::cout << it->second.first << ", " << it->second.second << endl;
    }
}
