#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdlib>

#define passwords_file "../src/passwords.dec"
#define credentials_path getenv("credentials_path")

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

void remove_trailing_space(std::string& tmp)
{
    int size = tmp.size() - 1;
    if (tmp[0] == ';') {
        while (!tmp.empty() && tmp[0] == ';') {
            tmp.erase(0, 1);
        }
    }

    size = tmp.size() - 1;
    if (tmp[size] == ';') {
        while (size >= 0 && tmp[size] == ';') {
            tmp.pop_back();
            size--;
        }
    }
}

void split_string(std::string& input_str)
{
    const char delimiter = ';';
    const int size = input_str.size() - 1;
    std::string tmp_str;
    std::vector<std::string> res_vec;

    for (int itr = 0; itr <= size; itr++) {
        if (input_str[itr] == delimiter) {
            if (!tmp_str.empty()) {
                remove_trailing_space(tmp_str);
                res_vec.push_back(tmp_str);
                tmp_str.clear();
            }
        } else {
            tmp_str.push_back(input_str[itr]);
        }
    }

    if (!tmp_str.empty()) {
        remove_trailing_space(tmp_str);
        res_vec.push_back(tmp_str);
    }
    entries.push_back(alloc(res_vec[0], res_vec[1], res_vec[2]));
}

void std_out(std::string&& log)
{
    std::cout << log << std::endl;
}

void load_passwords(std::string file_name)
{
    std::ifstream file(file_name);
    if (!file.good()) {
        std_out("[FILE NOT FOUND] passwords.dec");
        exit(1);
    }
    if(file.is_open()) {
        std::string curr_line;
        while (std::getline(file,curr_line)) {
            split_string(curr_line);
        }
    } else {
        std_out("[FILE] failed to read from passwords.dec");
    }
    file.close();
    std::ofstream filet("../src/passwords.dec", std::ofstream::out | std::ofstream::trunc);
    filet.close();
    system("rm -f ../src/passwords.dec");
}

void get_entry(const std::string& site_name)
{
    bool found = false;
    std::vector<entry> similar;
    const int entrie_size = entries.size();
    for (int i = 0; i < entrie_size; i++) {
        if (entries[i].site_name == site_name) {
            found = true;
            similar.push_back(entries[i]);
        }
    }

    if (!found) {
        std_out("[ERROR] no entry found");
    } else {
        if (similar.size() > 1) {
            std_out("[INFO] found " + std::to_string(similar.size()) + " entries for " + similar[0].site_name);
            std_out("[p] print all passwords\n[f] filter by email ");
            std::cout << "p/f >";
            char input;
            std::cin >> input;
            if (input == 'p') {
                const int similar_vec_size (similar.size());
                for (int i = 0; i < similar_vec_size; i++) {
                    std_out("\t" + similar[i].site_name);
                    std_out("email     : " + similar[i].email);
                    std_out("password  : " + similar[i].password);
                }
            } else if (input == 'f') {
                std_out("[INFO] Enter email");
                std::string email;
                std::cin >> email;
                const int similar_vec_size (similar.size());
                for (int i = 0; i < similar_vec_size; i++) {
                    if (similar[i].email == email) {
                        std_out("\t" + similar[i].site_name);
                        std_out("email     : " + similar[i].email);
                        std_out("password  : " + similar[i].password);
                    } else {
                        std_out("[ERROR] Did not find any entry with " + email);
                        exit(1);
                    }
                }
            } else {
                std_out("[ERROR] wrong input");
                exit(1);
            }
        } else {
            std_out("\t" + similar[0].site_name);
            std_out("email     : " + similar[0].email);
            std_out("password  : " + similar[0].password);
        }
    }
}

void usage()
{
    std_out("USAGE :");
    std_out(" passkey option [args]");
    std_out("");
    std_out("options : ");
    std_out("a, add        add new entry to vault");
    std_out("g, get        get entry from vault");
    std_out("d, delete     delete entry from vault");
    std_out("");
    std_out("args : ");
    std_out("a,            [sitename email password credentials_folder_path]");
    std_out("g,            [sitename credentials_folder_path]");
    std_out("d,            [sitename credentials_folder_path]");
    std_out("");
    std_out("credentials : ");
    std_out("credentials folder should have 'key.bin' and 'iv.bin' files");
}

bool access_granted(const std::string& path)
{
    std::string cmd = "openssl enc -d -aes-256-cbc -pbkdf2 -in ../src/access.enc -out ../src/access.dec -pass ";
    std::string file = "file:"+ path + "key.bin ";
    std::string iv = "-iv $(cat " + path + "iv.bin)";
    std::string exec = cmd + file + iv;

    system(exec.c_str());

    std::ifstream f("../src/access.dec");
    std::string curr_line;
    bool flag = false;

    if (!f.good()) {
        std_out("[FILE NOT FOUND] ../src/access.enc no such file");
        exit(1);
    } else {
        if (f.is_open()) {
            std::getline(f, curr_line);
            if (curr_line == "true") {
                flag = true;
            }
        } else {
            std_out("[FILE ACCESS] cannot read from ../src/access.enc");
            f.close();
            exit(1);
        }
    }

    f.close();
    system("rm -f ../src/access.dec");

    if (!flag) {
        return false;
    } else {
        return true;
    }
}

void decrypt_vault(const std::string& path)
{
    if (access_granted(path)) {
        std::string cmd = "openssl enc -d -aes-256-cbc -pbkdf2 -in ../src/passwords.enc -out ../src/passwords.dec -pass ";
        std::string file = "file:"+ path + "key.bin ";
        std::string iv = "-iv $(cat " + path + "iv.bin)";
        std::string exec = cmd + file + iv;
        system(exec.c_str());
        return;
    } else {
        std_out("[ACCESS DENIED] cannot decrypt vault");
        exit(1);
    }
}

void encrypt_vault(const std::string& path)
{
    if (access_granted(path)) {
        std::ofstream filet("../src/passwords.enc", std::ofstream::out | std::ofstream::trunc);
        filet.close();
        std::string cmd = "openssl enc -aes-256-cbc -pbkdf2 -in ../src/passwords.dec -out ../src/passwords.enc -pass ";
        std::string file = "file:"+ path + "key.bin ";
        std::string iv = "-iv $(cat " + path + "iv.bin)";
        std::string exec = cmd + file + iv;
        system(exec.c_str());
        system("rm -rf ../src/passwords.dec");
        return;
    } else {
        std_out("[ACCESS DENIED] cannot encrypt vault");
        exit(1);
    }
}

void add_new_entry(std::string site_name, std::string email, std::string password, const std::string& path)
{
    decrypt_vault(path);
    std::string line;
    line.append(site_name+";");
    line.append(email+";");
    line.append(password+";");
    std::ofstream ofs;
    ofs.open("../src/passwords.dec", std::ios::app);
    if(ofs.is_open()) {
        ofs << line << "\n";
    } else {
        std_out("[FILE] failed to read from passwords.dec");
        exit(1);
    }
    ofs.close();
    encrypt_vault(path);
}

void delete_single_entry(const std::string& site_name, const std::string& path,  const int index)
{
    std::ofstream file("../src/passwords.dec", std::ios_base::app);
    const int entries_size (entries.size());
    for (int i = 0; i < entries_size; i++) {
        if (i != index) {
            std::string line;
            line.append(entries[i].site_name+";");
            line.append(entries[i].email+";");
            line.append(entries[i].password+";");
            file << line << "\n";
        }
    }
    file.close();
    encrypt_vault(path);
    std_out("[INFO] Deletion Successful");
}

void delete_entry(const std::string& site_name, const std::string& path)
{
    decrypt_vault(path);
    load_passwords(passwords_file);
    bool found = false;
    std::vector<int> found_pos;
    int first_found_index;
    std::unordered_map<int, entry> map;
    const int entries_size (entries.size());
    for (int i = 0; i < entries_size; i++) {
        if (entries[i].site_name == site_name) {
            found = true;
            first_found_index = i;
            break;
        }
    }
    
    for (int i = first_found_index; i < entries_size; i++) {
        if (entries[i].site_name == site_name) {
            found_pos.push_back(i);
        } else {
            break;
        }
    }

    if (!found) {
        std_out("[INFO] no entry with { " + site_name + " } found");
        exit(0);
    }

    if (found_pos.size() == 1) {
        std_out("[INFO] Are you sure you want to delete this entry");

        std_out("\t" + entries[first_found_index].site_name);
        std_out("email     : " + entries[first_found_index].email);
        std_out("password  : " + entries[first_found_index].password);
        std::cout << "y/n > ";
        char input;
        std::cin >> input;
        if (input == 'y') {
            delete_single_entry(site_name, path, first_found_index);
            std_out("[INFO] Entry deleted from vault");
            exit(0);
        } else if (input == 'n') {
            std_out("[INFO] Operation Cancelled");
            exit(0);
        } else {
            std_out("[ABORT] Wrong input");
            exit(1);
        }
    } else {

        std_out("[INFO] found " + std::to_string(found_pos.size()) + " entries");
        for (int i = found_pos[0]; i <= found_pos.back(); i++) {
            map[i] = entries[i];
            std_out("\t[" + std::to_string(i) + "] " + entries[i].site_name);
            std_out("email     : " + entries[i].email);
            std_out("password  : " + entries[i].password);
        }
        std_out("[INFO] choose one of the options ");
        std_out("s->delete single entry, m-> delete multiple entries, c->clear/delete all");
        std::cout << "s/m/c > ";
        char operation;
        std::cin >> operation;
        if (operation == 's') {
            int index;
            std_out("[INFO] choose from above indices");
            std::cout << "i > ";
            std::cin >> index;
            const int entries_size (entries.size());
            if (index > entries_size - 1 || index < 0) {
                std_out("[ABORT] index out of range");
                exit(1);
            } else {
                std_out("[INFO] Are you sure..");
                std::cout << "y/n > ";
                char input;
                std::cin >> input;
                if (input == 'y') {
                    delete_single_entry(site_name, path, index);
                    return;
                } else if (input == 'n') {
                    std_out("[INFO] operation cancelled");
                    exit(0);
                } else {
                    std_out("[ABORT] wrong input");
                    exit(1);
                }
            }
            // end of 'i'
        } else if (operation == 'm') {
            std_out("[INFO] Enter all the indices you want to delete. choose from above entries");
            std_out("[INFO] you can specify " + std::to_string(found_pos.size()) + " indices");
            std_out("[INFO] use -1 to stop.");
            std::vector<int> delete_indices;
            int index;
            const int found_pos_size (found_pos.size());
            for (int i = 0; i < found_pos_size; i++) {
                std::cout << "in > ";
                std::cin >> index;
                if (index == -1) {
                    break;
                } else if (index > entries_size - 1 || index <= -2) {
                    std_out("[ABORT] index out of range");
                    exit(1);
                    break;
                } else {
                    auto it = map.find(index);
                    if (it != map.end()) {
                        delete_indices.push_back(index);
                    } else {
                        std_out("[ABORT] index does not map to found entries");
                        exit(1);
                    }
                }
            }
            if (!delete_indices.empty()) {
                std_out(std::string("[INFO] Are you sure..") + "y/n > ");
                char input;
                std::cin >> input;
                if (input == 'y') {
                    int it = 0;

                    const int delete_indices_size (delete_indices.size());
                    while (it < delete_indices_size) {
                        entries.erase(entries.begin()+delete_indices[it]);
                        it++;
                    }

                    std::ofstream file("../src/passwords.dec", std::ios_base::app);
                    const int entries_size (entries.size());
                    for (int i = 0; i < entries_size; i++) {
                        std::string line;
                        line.append(entries[i].site_name+";");
                        line.append(entries[i].email+";");
                        line.append(entries[i].password+";");
                        file << line << "\n";
                    }

                    file.close();
                    encrypt_vault(path);
                    std_out("[INFO] Deleted " + std::to_string(delete_indices.size()) + " entries" );

                } else if (input == 'n') {
                    std_out("[INFO] operation cancelled");
                    exit(0);
                } else {
                    std_out("[ABORT] wrong input");
                    exit(1);
                }
            } else {
                std_out("[INFO] No index specified" + std::string("[INFO] No entry deleted"));
                exit(1);
            }
            // end of 'm'
        } else if ( operation == 'c' ) {

            std_out("Are you sure ");
            std::cout << "y/n > ";
            char input;
            std::cin >> input;
            if (input == 'y') {

                const int found_pos_size (found_pos.size());
                for (int i = found_pos[0]; i <= found_pos_size; i++) {
                    entries.erase(entries.begin()+i);
                }

                std::ofstream file("../src/passwords.dec", std::ios_base::app);
                for (int i = 0; i < entries_size; i++) {
                        std::string line;
                        line.append(entries[i].site_name+";");
                        line.append(entries[i].email+";");
                        line.append(entries[i].password+";");
                        file << line << "\n";
                }
                file.close();
                encrypt_vault(path);
                std_out("[INFO] Deleted all entries");
                exit(0);
            } else if (input == 'n') {
                std_out("[INFO] operation cancelled");
                exit(0);
            } else {
                std_out("[ABORT] wrong input");
                exit(1);
            }
            // end of c
        } else {
            std_out("[ABORT] wrong input");
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
        if (argc != 3) {
            usage();
            exit(1);
        } else {
            decrypt_vault(credentials_path);
            load_passwords(passwords_file);
            get_entry(argv[2]);
            exit(0);
        }
    } else if (arg1 == 'a') {
        if (argc != 5) {
            usage();
            exit(1);
        } else {
            std::string site_name = (std::string)argv[2];
            std::string email = (std::string)argv[3];
            std::string password = (std::string)argv[4];
            add_new_entry(site_name, email, password, credentials_path);
            exit(0);
        }
    } else if (arg1 == 'd') {
        if (argc != 3) {
            usage();
            exit(1);
        } else {
            std::string site_name = (std::string)argv[2];
            delete_entry(site_name, credentials_path);
        }
    } else {
        usage();
        exit(1);
    }
    return 0;
}
