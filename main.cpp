#include <iostream>
#include <fstream>
#include <queue>
#include <string>
#include <vector>
#include <openssl/sha.h>

#define endl std::endl
#define passwords_file "./passwords.dec"
#define HASH "c546d424016fb5b6a4cc6a0855e65113aaebcbe9933b4bd876dec776d10a5721"

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
    system("rm -rf ./passwords.dec");
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
        std::cout << "[p] print all passwords\n[f] filter by email " << endl;
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
        } else {
            std::cout << "[ERROR] wrong input" << endl;
            exit(1);
        }
    } else {
        std::cout << "\t" << similar[0].site_name << endl;
        std::cout << "email     : " << similar[0].email << endl;
        std::cout << "password  : " << similar[0].password << endl;
    }

    if (!found) {
        std::cout << "[ERROR] no entry found" << endl;
    }
}

void add_new_entry(std::string site_name, std::string email, std::string password) {
    // TODO : add openssl auth.
    std::string line;
    line.append(site_name+",");
    line.append(email+",");
    line.append(password+";");
    std::ofstream ofs(passwords_file);
    if (!ofs.good()) {
        std::cout << "[FILE NOT FOUND] passwords.txt" << endl;
    }
    if(ofs.is_open()) {
        ofs << line << endl;
        ofs.close();
    } else {
        std::cout << "[FILE] failed to write to passwords.txt" << endl;
    }
}

std::string sha256(const std::string str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, str.c_str(), str.size());
    SHA256_Final(hash, &sha256);

    std::string hashStr;
    hashStr.resize(SHA256_DIGEST_LENGTH * 2);
    for (size_t i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        sprintf(&hashStr[i * 2], "%02x", hash[i]);
    }
    return hashStr;
}

std::string load_hash_file(std::string hash_file) {
    std::ifstream file;
    std::string hash;
    file.open(hash_file);
    if (file.is_open()) {
        std::getline(file, hash);
    } else {
        std::cout << "[FILE NOT FOUND] hashfile" << endl;
        exit(1);
    }
    return hash;
}

void usage() {
    std::cout << "USAGE :" << endl;
    std::cout << " passkey option [args]" << endl;
    std::cout << endl;
    std::cout << "options : " << endl;
    std::cout << "-a, add        add new entry to vault" << endl;
    std::cout << "-g, get        get entry from vault" << endl;
    std::cout << endl;
    std::cout << "args : " << endl;
    std::cout << "-a,            [sitename email password credentials_path]" << endl;
    std::cout << "-g,            [sitename credentials_path]" << endl;
}

bool grant_access(const std::string& path) {
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
    std::string cmd = "openssl enc -d -aes-256-cbc -pbkdf2 -in passwords.enc -out passwords.dec -pass ";
    std::string file = "file:"+ path + "key.bin ";
    std::string iv = "-iv $(cat " + path + "iv.bin)";
    std::string exec = cmd + file + iv;
    system(exec.c_str());
    if (grant_access(path)) {
        return;
    } else {
        std::cout << "[ACCESS DENIED] cannot decrypt vault" << endl;
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    // TODO : use openssl to encrypt & decrypt password.txt
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
            get_password(argv[2]);
            exit(0);
        }
    }

    if (arg1 == 'a') {
        if (argc != 6) {
            usage();
            exit(1);
        } else {
            // TODO : Add openssl auth.
            decrypt_vault(argv[5]);
            std::string site_name = (std::string)argv[2];
            std::string email = (std::string)argv[3];
            std::string password = (std::string)argv[4];
            add_new_entry(site_name, email, password);
            exit(0);
        }
    }
    return 0;
}
