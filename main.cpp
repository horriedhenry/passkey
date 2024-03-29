#include <iostream>
#include <fstream>
#include <queue>
#include <string>
#include <unordered_map>
#include <vector>

#define endl std::endl
#define cout std::cout
#define passwords_file "./passwords.dec"

typedef struct entry
{
    std::string site_name;
    std::string email;
    std::string password;
} entry;

std::vector<entry> entries;

entry alloc(std::string site_name, std::string email, std::string password)
{
    entry e;
    e.site_name = site_name;
    e.email = email;
    e.password = password;
    return e;
}

void split_by_delim(std::string& str)
{
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

void load_passwords(std::string file_name)
{
    std::ifstream file(file_name);
    if (!file.good()) {
        cout << "[FILE NOT FOUND] passwords.dec" << endl;
        exit(1);
    }
    if(file.is_open()) {
        std::string curr_line;
        while (std::getline(file,curr_line)) {
            split_by_delim(curr_line);
        }
    } else {
        cout << "[FILE] failed to read from passwords.dec" << endl;
    }
    file.close();
    std::ofstream filet("./passwords.dec", std::ofstream::out | std::ofstream::trunc);
    filet.close();
    system("rm -rf ./passwords.dec");
}

void get_entry(const std::string& site_name)
{
    bool found = false;
    std::vector<entry> similar;
    for (int i = 0; i < entries.size(); i++) {
        if (entries[i].site_name == site_name) {
            found = true;
            similar.push_back(entries[i]);
        }
    }

    if (!found) {
        cout << "[ERROR] no entry found" << endl;
    } else {
        if (similar.size() > 1) {
            cout << "[INFO] found " << similar.size() << " entries for " << similar[0].site_name << endl;
            cout << "[p] print all passwords\n[f] filter by email " << endl;
            cout << "p/f >";
            char input;
            std::cin >> input;
            if (input == 'p') {
                for (int i = 0; i < similar.size(); i++) {
                    cout << "\t" << similar[i].site_name << endl;
                    cout << "email     : " << similar[i].email << endl;
                    cout << "password  : " << similar[i].password << endl;
                }
            } else if (input == 'f') {
                cout << "[INFO] Enter email" << endl;
                std::string email;
                std::cin >> email;
                for (int i = 0; i < similar.size(); i++) {
                    if (similar[i].email == email) {
                        cout << "\t" << similar[i].site_name << endl;
                        cout << "email     : " << similar[i].email << endl;
                        cout << "password  : " << similar[i].password << endl;
                    } else {
                        cout << "[ERROR] Did not find any entry with " << email << endl;
                        exit(1);
                    }
                }
            } else {
                cout << "[ERROR] wrong input" << endl;
                exit(1);
            }
        } else {
            cout << "\t" << similar[0].site_name << endl;
            cout << "email     : " << similar[0].email << endl;
            cout << "password  : " << similar[0].password << endl;
        }
    }
}

void usage()
{
    cout << "USAGE :" << endl;
    cout << " passkey option [args]" << endl;
    cout << endl;
    cout << "options : " << endl;
    cout << "a, add        add new entry to vault" << endl;
    cout << "g, get        get entry from vault" << endl;
    cout << "d, delete     delete entry from vault" << endl;
    cout << endl;
    cout << "args : " << endl;
    cout << "a,            [sitename email password credentials_folder_path]" << endl;
    cout << "g,            [sitename credentials_folder_path]" << endl;
    cout << "d,            [sitename credentials_folder_path]" << endl;
    cout << endl;
    cout << "credentials : " << endl;
    cout << "credentials folder should have 'key.bin' and 'iv.bin' files" << endl;
}

bool access_granted(const std::string& path)
{
    std::string cmd = "openssl enc -d -aes-256-cbc -pbkdf2 -in access.enc -out access.dec -pass ";
    std::string file = "file:"+ path + "key.bin ";
    std::string iv = "-iv $(cat " + path + "iv.bin)";
    std::string exec = cmd + file + iv;

    system(exec.c_str());

    std::ifstream f("./access.dec");
    std::string curr_line;
    bool flag = false;

    if (!f.good()) {
        cout << "[FILE NOT FOUND] ./access.enc no such file" << endl;
        exit(1);
    } else {
        if (f.is_open()) {
            std::getline(f, curr_line);
            if (curr_line == "true") {
                flag = true;
            }
        } else {
            cout << "[FILE ACCESS] cannot read from ./access.enc" << endl;
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

void decrypt_vault(const std::string& path)
{
    if (access_granted(path)) {
        std::string cmd = "openssl enc -d -aes-256-cbc -pbkdf2 -in passwords.enc -out passwords.dec -pass ";
        std::string file = "file:"+ path + "key.bin ";
        std::string iv = "-iv $(cat " + path + "iv.bin)";
        std::string exec = cmd + file + iv;
        system(exec.c_str());
        return;
    } else {
        cout << "[ACCESS DENIED] cannot decrypt vault" << endl;
        exit(1);
    }
}

void encrypt_vault(const std::string& path)
{
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
        cout << "[ACCESS DENIED] cannot encrypt vault" << endl;
        exit(1);
    }
}

void add_new_entry(std::string site_name, std::string email, std::string password, const std::string& path)
{
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
        cout << "[FILE] failed to read from passwords.dec" << endl;
        exit(1);
    }
    ofs.close();
    encrypt_vault(path);
}

void delete_single_entry(const std::string& site_name, const std::string& path,  const int index)
{
    std::ofstream file("./passwords.dec", std::ios_base::app);
    for (int i = 0; i < entries.size(); i++) {
        if (i != index) {
            std::string line;
            line.append(entries[i].site_name+",");
            line.append(entries[i].email+",");
            line.append(entries[i].password+";");
            file << line << endl;
        }
    }
    file.close();
    encrypt_vault(path);
    cout << "[INFO] Deletion Successful" << endl;
}

void delete_entry(const std::string& site_name, const std::string& path)
{
    decrypt_vault(path);
    load_passwords(passwords_file);
    bool found = false;
    std::vector<int> found_pos;
    int first_found_index;
    std::unordered_map<int, entry> map;
    for (int i = 0; i < entries.size(); i++) {
        if (entries[i].site_name == site_name) {
            found = true;
            first_found_index = i;
            break;
        }
    }
    
    for (int i = first_found_index; i < entries.size(); i++) {
        if (entries[i].site_name == site_name) {
            found_pos.push_back(i);
        } else {
            break;
        }
    }

    if (!found) {
        cout << "[INFO] no entry with { " << site_name << " } found" << endl;
        exit(0);
    }

    if (found_pos.size() == 1) {
        cout << "[INFO] Are you sure you want to delete this entry" << endl;
        cout << "\t" << entries[first_found_index].site_name << endl;
        cout << "email    : "<< entries[first_found_index].email << endl;
        cout << "password : "<< entries[first_found_index].password << endl;
        cout << "y/n > ";
        char input;
        std::cin >> input;
        if (input == 'y') {
            delete_single_entry(site_name, path, first_found_index);
            cout << "[INFO] Entry deleted from vault" << endl;
            exit(0);
        } else if (input == 'n') {
            cout << "[INFO] Operation Cancelled" << endl;
            exit(0);
        } else {
            cout << "[ABORT] Wrong input" << endl;
            exit(1);
        }
    } else {

        cout << "[INFO] found " << found_pos.size() << " entries" << endl;
        cout << endl;
        for (int i = found_pos[0]; i <= found_pos.back(); i++) {
            map[i] = entries[i];
            cout << "\t[" << i << "] " << entries[i].site_name << endl;
            cout << "email    : "<< entries[i].email << endl;
            cout << "password : "<< entries[i].password << endl;
            cout << endl;
        }
        cout << "[INFO] choose one of the options " << endl;
        cout << "s->delete single entry, m-> delete multiple entries, c->clear/delete all" << endl;
        cout << "s/m/c > ";
        char operation;
        std::cin >> operation;
        if (operation == 's') {
            int index;
            cout << "[INFO] choose from above indices" << endl;
            cout << "i > ";
            std::cin >> index;
            if (index > entries.size() - 1 || index < 0) {
                cout << "[ABORT] index out of range" << endl;
                exit(1);
            } else {
                cout << "[INFO] Are you sure.." << endl;
                cout << "y/n > ";
                char input;
                std::cin >> input;
                if (input == 'y') {
                    delete_single_entry(site_name, path, index);
                    return;
                } else if (input == 'n') {
                    cout << "[INFO] operation cancelled" << endl;
                    exit(0);
                } else {
                    cout << "[ABORT] wrong input" << endl;
                    exit(1);
                }
            }
            // end of 'i'
        } else if (operation == 'm') {
            cout << "[INFO] Enter all the indices you want to delete. choose from above entries" << endl;
            cout << "[INFO] you can specify " << found_pos.size() << " indices" << endl;
            cout << "[INFO] use -1 to stop." << endl;
            std::vector<int> delete_indices;
            int index;
            for (int i = 0; i < found_pos.size(); i++) {
                cout << "in > ";
                std::cin >> index;
                if (index == -1) {
                    break;
                } else if (index > entries.size() - 1 || index <= -2) {
                    cout << "[ABORT] index out of range" << endl;
                    exit(1);
                    break;
                } else {
                    auto it = map.find(index);
                    if (it != map.end()) {
                        delete_indices.push_back(index);
                    } else {
                        cout << "[ABORT] index does not map to found entries" << endl;
                        exit(1);
                    }
                }
            }
            if (!delete_indices.empty()) {
                cout << "[INFO] Are you sure.." << endl;
                cout << "y/n > ";
                char input;
                std::cin >> input;
                if (input == 'y') {
                    int it = 0;

                    while (it < delete_indices.size()) {
                        entries.erase(entries.begin()+delete_indices[it]);
                        it++;
                    }

                    std::ofstream file("./passwords.dec", std::ios_base::app);
                    for (int i = 0; i < entries.size(); i++) {
                        std::string line;
                        line.append(entries[i].site_name+",");
                        line.append(entries[i].email+",");
                        line.append(entries[i].password+";");
                        file << line << endl;
                    }

                    file.close();
                    encrypt_vault(path);
                    cout << "[INFO] Deleted " << delete_indices.size() << " entries" << endl;

                } else if (input == 'n') {
                    cout << "[INFO] operation cancelled" << endl;
                    exit(0);
                } else {
                    cout << "[ABORT] wrong input" << endl;
                    exit(1);
                }
            } else {
                cout << "[INFO] No index specified" << endl;
                cout << "[INFO] No entry deleted" << endl;
                exit(1);
            }
            // end of 'm'
        } else if ( operation == 'c' ) {

            cout << "Are you sure " << endl;
            cout << "y/n > ";
            char input;
            std::cin >> input;
            if (input == 'y') {

                for (int i = found_pos[0]; i <= found_pos.back(); i++) {
                    entries.erase(entries.begin()+i);
                }

                std::ofstream file("./passwords.dec", std::ios_base::app);
                for (int i = 0; i < entries.size(); i++) {
                        std::string line;
                        line.append(entries[i].site_name+",");
                        line.append(entries[i].email+",");
                        line.append(entries[i].password+";");
                        file << line << endl;
                }
                file.close();
                encrypt_vault(path);
                cout << "[INFO] Deleted all entries" << endl;
                exit(0);
            } else if (input == 'n') {
                cout << "[INFO] operation cancelled" << endl;
                exit(0);
            } else {
                cout << "[ABORT] wrong input" << endl;
                exit(1);
            }
        } else {
            cout << "[ABORT] wrong input" << endl;
            exit(1);
        }
    }

}

int main(int argc, char *argv[])
{
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
