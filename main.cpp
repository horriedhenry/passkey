#include <iostream>
#include <fstream>
#include <map>
#include <queue>
#include <string>
#include <utility>
#include <vector>

#define endl std::endl
#define passwords_file "./passwords.txt"

std::vector<std::string> lines;
std::vector<std::vector<std::string>> passwords;
std::multimap<std::string, std::pair<std::string, std::string>> map; 

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

void parse_new_line_to_map(std::multimap<std::string, std::pair<std::string, std::string>>& map, std::vector<std::string> vec) {
    // when ever use adds a new entry in the vault, add it to the map.
    map.insert({vec[0], std::make_pair(vec[1], vec[2])});
}

void add_new_entry(std::multimap<std::string, std::pair<std::string, std::string>>& map) {
    std::string site_name, email, password;
    std::cout << "site name : " << endl;
    std::getline(std::cin, site_name);
    std::cout << "email     : " << endl;
    std::getline(std::cin, email);
    std::cout << "password : " << endl;
    std::getline(std::cin, password);
    std::string line;
    line.append(site_name+",");
    line.append(email+",");
    line.append(password+";");
    std::ofstream ofs;
    ofs.open(passwords_file, std::ios::app);
    if(ofs.is_open()) {
        ofs << line << endl;
    }
    std::vector<std::string> add_line;
    add_line.push_back(site_name);
    add_line.push_back(email);
    add_line.push_back(password);
    parse_new_line_to_map(map, add_line);
}

void load_vault() {
    load_passwords(passwords_file, lines);
    map_passwords(passwords, lines);
    parse_to_map(map, passwords);
    return;
}

int main(int argc, char *argv[]) {
    bool is_vault_open = true;
    do {
        std::cout << "usage - \n g - get entry \n a - add entry" << endl;
        std::cout << "Enter command" << endl;
        char command;
        std::cin >> command;
        switch (command) {
            case 'g' : {
                load_vault();
                std::cin.ignore(); // consume newline character
                // with .ignore() not used.. inside both get_password and add_new_entry 
                // std::getline() is not working.. the program is not taking any input.
                // chatgpt :
                // the issue you're experiencing where your program does not take input correctly without std::cin.ignore() is likely due to the fact that there's a newline character left in the input buffer after reading a single character using std::cin >> command in the main function. when you subsequently call std::getline(), it reads this newline character and interprets it as an empty line, causing your input to be skipped.
                // by using std::cin.ignore() after reading a single character, you're consuming this leftover newline character, allowing std::getline() to behave as expected.
                std::cout << "enter site name to retrieve email and password info :" << endl;
                std::string site_name;
                std::getline(std::cin, site_name);
                get_password(site_name, map);
            } break;
            case 'a':
                std::cout << "add entry : " << endl;
                std::cin.ignore(); // consume newline character
                add_new_entry(map);
                break;
            default :
                std::cout << "Wrong Input" << endl;
                break;
        }
        std::cout << "close vault or use it again, e - exit, c - keep vault opened" << endl;
        char in_out;
        std::cin >> in_out;
        std::cin.ignore(); // consume new line character.
        if (in_out == 'e') {
            is_vault_open = false;
            break;
        } else {
            continue;
        }
    } while (is_vault_open);

    return 0;
}
