#include <iostream>
#include <fstream>
#include <queue>
#include <string>
#include <vector>

#define endl std::endl
#define passwords_file "./passwords.dec"

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
    std::ifstream file(file_name);
    if (!file.good()) {
        std::cout << "[FILE NOT FOUND] passwords.dec" << endl;
        exit(1);
    }
    if(file.is_open()) {
        std::string curr_line;
        while (std::getline(file,curr_line)) {
            split_by_delim(curr_line);
        }
    } else {
        std::cout << "[FILE] failed to read from passwords.dec" << endl;
    }
    file.close();
    std::ofstream filet("./passwords.dec", std::ofstream::out | std::ofstream::trunc);
    filet.close();
    system("rm -rf ./passwords.dec");
}

void get_entry(const std::string& site_name) {
    bool found = false;
    std::vector<entry> similar;
    for (int i = 0; i < entries.size(); i++) {
        if (entries[i].site_name == site_name) {
            found = true;
            similar.push_back(entries[i]);
        }
    }

    if (!found) {
        std::cout << "[ERROR] no entry found" << endl;
    } else {
        if (similar.size() > 1) {
            std::cout << "[INFO] found " << similar.size() << " entries for " << similar[0].site_name << endl;
            std::cout << "[p] print all passwords\n[f] filter by email " << endl;
            std::cout << "p/f >";
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
                    } else {
                        std::cout << "[ERROR] Did not find any entry with " << email << endl;
                        exit(1);
                    }
                }
            } else {
                std::cout << "[ERROR] wrong input" << endl;
                exit(1);
            }
        } else {
            std::cout << "\t" << similar[0].site_name << endl;
            std::cout << "email     : " << similar[0].email << endl;
            std::cout << "password  : " << similar[0].password << endl;
        }
    }
}

void usage() {
    std::cout << "USAGE :" << endl;
    std::cout << " passkey option [args]" << endl;
    std::cout << endl;
    std::cout << "options : " << endl;
    std::cout << "a, add        add new entry to vault" << endl;
    std::cout << "g, get        get entry from vault" << endl;
    std::cout << "d, delete     delete entry from vault" << endl;
    std::cout << endl;
    std::cout << "args : " << endl;
    std::cout << "a,            [sitename email password credentials_folder_path]" << endl;
    std::cout << "g,            [sitename credentials_folder_path]" << endl;
    std::cout << "d,            [sitename credentials_folder_path]" << endl;
    std::cout << endl;
    std::cout << "credentials : " << endl;
    std::cout << "credentials folder should have 'key.bin' and 'iv.bin' files" << endl;
}

bool access_granted(const std::string& path) {
    std::string cmd = "openssl enc -d -aes-256-cbc -pbkdf2 -in access.enc -out access.dec -pass ";
    std::string file = "file:"+ path + "key.bin ";
    std::string iv = "-iv $(cat " + path + "iv.bin)";
    std::string exec = cmd + file + iv;

    system(exec.c_str());

    std::ifstream f("./access.dec");
    std::string curr_line;
    bool flag = false;

    if (!f.good()) {
        std::cout << "[FILE NOT FOUND] ./access.enc no such file" << endl;
        exit(1);
    } else {
        if (f.is_open()) {
            std::getline(f, curr_line);
            if (curr_line == "true") {
                flag = true;
            }
        } else {
            std::cout << "[FILE ACCESS] cannot read from ./access.enc" << endl;
            f.close();
            exit(1);
        }
    }

    f.close();
    system("rm -rf ./access.dec");

    if (!flag) {
        return false;
    } else {
        return true;
    }
}

void decrypt_vault(const std::string& path) {
    if (access_granted(path)) {
        std::string cmd = "openssl enc -d -aes-256-cbc -pbkdf2 -in passwords.enc -out passwords.dec -pass ";
        std::string file = "file:"+ path + "key.bin ";
        std::string iv = "-iv $(cat " + path + "iv.bin)";
        std::string exec = cmd + file + iv;
        system(exec.c_str());
        return;
    } else {
        std::cout << "[ACCESS DENIED] cannot decrypt vault" << endl;
        exit(1);
    }
}

void encrypt_vault(const std::string& path) {
    if (access_granted(path)) {
        std::ofstream filet("./passwords.enc", std::ofstream::out | std::ofstream::trunc);
        filet.close();
        std::string cmd = "openssl enc -aes-256-cbc -pbkdf2 -in passwords.dec -out passwords.enc -pass ";
        std::string file = "file:"+ path + "key.bin ";
        std::string iv = "-iv $(cat " + path + "iv.bin)";
        std::string exec = cmd + file + iv;
        system(exec.c_str());
        system("rm -rf ./passwords.dec");
        return;
    } else {
        std::cout << "[ACCESS DENIED] cannot encrypt vault" << endl;
        exit(1);
    }
}

void add_new_entry(std::string site_name, std::string email, std::string password, const std::string& path) {
    decrypt_vault(path);
    std::string line;
    line.append(site_name+",");
    line.append(email+",");
    line.append(password+";");
    std::ofstream ofs;
    ofs.open(passwords_file, std::ios::app);
    if(ofs.is_open()) {
        ofs << line << endl;
    } else {
        std::cout << "[FILE] failed to read from passwords.dec" << endl;
        exit(1);
    }
    ofs.close();
    encrypt_vault(path);
}

void delete_entry(const std::string& site_name, const std::string& path) {
    // TODO : what if there are multiple entries with similar sitename
    decrypt_vault(path);
    load_passwords(passwords_file);
    bool found = false;
    int index;
    for (int i = 0; i < entries.size(); i++) {
        if (entries[i].site_name == site_name) {
            found = true;
            index = i;
            break;
        }
    }
    if (!found) {
        std::cout << "[ABORT] no entry with " << "'" << site_name << "'" << " found in vault" << endl;
        exit(1);
    } else {
        std::cout << "Are you sure you want to delete this entry" << endl;
        std::cout << "\t" << entries[index].site_name << endl;
        std::cout << "email     : " << entries[index].email << endl;
        std::cout << "password  : " << entries[index].password << endl;
        std::cout << "y/n >";
        char input;
        std::cin >> input;
        if (input == 'y') {
            std::ofstream file("passwords.dec", std::ios_base::app);
            if (file.is_open()) {
                for (int i = 0; i < entries.size(); i++) {
                    if (i != index) {
                        std::string line;
                        line.append(entries[i].site_name+",");
                        line.append(entries[i].email+",");
                        line.append(entries[i].password+";");
                        file << line << endl;
                    }
                }
                encrypt_vault(path);
                std::cout << "[INFO] Deletion Successful" << endl;
                return;
            } else {
                std::cout << "[FILE] cannot write to passwords.dec" << endl;
                std::cout << "[ABORT] write operation failed" << endl;
                exit(1);
            }
        } else if (input == 'n'){
            std::cout << "[ABORT] Deletion cancelled" << endl;
            exit(0);
        } else {
            std::cout << "[ABORT] Wrong option" << endl;
            exit(1);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        usage();
        exit(1);
    }

    const char arg1 = (char)*argv[1];
    if (arg1 == 'g') {
        if (argc != 4) {
            usage();
            exit(1);
        } else {
            decrypt_vault(argv[3]);
            load_passwords(passwords_file);
            get_entry(argv[2]);
            exit(0);
        }
    } else if (arg1 == 'a') {
        if (argc != 6) {
            usage();
            exit(1);
        } else {
            std::string site_name = (std::string)argv[2];
            std::string email = (std::string)argv[3];
            std::string password = (std::string)argv[4];
            add_new_entry(site_name, email, password, argv[5]);
            exit(0);
        }
    } else if (arg1 == 'd') {
        if (argc != 4) {
            usage();
            exit(1);
        } else {
            std::string site_name = (std::string)argv[2];
            delete_entry(site_name, argv[3]);
        }
    } else {
        usage();
        exit(1);
    }
    return 0;
}
