#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

namespace fs = std::filesystem;

std::string credentials_path = getenv("credentials_path");
std::string passwords_file_dec = credentials_path + "passwords.dec";
std::string passwords_file_enc = credentials_path + "passwords.enc";

typedef struct {
    std::string site_name;
    std::string email;
    std::string password;
} entry;

std::vector<entry> entries;

entry alloc(std::string &site_name, std::string &email, std::string &password)
{
    entry e;
    e.site_name = site_name;
    e.email = email;
    e.password = password;
    return e;
}

void remove_trailing_space(std::string &tmp)
{
    const char space{' '};
    if (tmp[0] == space) {
        int itr{0};
        while (tmp[itr] == space) {
            itr++;
        }
        tmp = tmp.substr(itr);
    }
    while (tmp.back() == space) {
        tmp.pop_back();
    }
}

void split_string(std::string &input_str)
{
    const char delimiter{';'};
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
        }
        else {
            tmp_str.push_back(input_str[itr]);
        }
    }

    if (!tmp_str.empty()) {
        remove_trailing_space(tmp_str);
        res_vec.push_back(tmp_str);
    }
    entries.push_back(alloc(res_vec[0], res_vec[1], res_vec[2]));
}

void std_out(std::string &&log) { std::cout << log << std::endl; }

void load_passwords()
{
    std::ifstream file(passwords_file_dec);

    if (!file.good()) {
        std_out("[FILE NOT FOUND] passwords.dec");
        exit(1);
    }
    if (file.is_open()) {
        std::string curr_line;

        while (std::getline(file, curr_line)) {
            split_string(curr_line);
        }
    }
    else {
        std_out("[FILE] failed to read from passwords.dec");
    }
    file.close();

    std::ofstream filet(passwords_file_dec,
                        std::ofstream::out | std::ofstream::trunc);
    filet.close();

    std::string delete_file_path{passwords_file_dec};
    std::string delete_command{"rm -f " + delete_file_path};
    system(delete_command.c_str());
}

void get_entry(const std::string &site_name)
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
    }
    else {
        if (similar.size() > 1) {
            std_out("[INFO] found " + std::to_string(similar.size()) +
                    " entries for " + similar[0].site_name);
            std_out("[p] print all passwords\n[f] filter by email ");

            std::cout << "p/f >";
            char input;
            std::cin >> input;

            if (input == 'p') {
                const int similar_vec_size(similar.size());
                for (int i = 0; i < similar_vec_size; i++) {
                    std_out("\t" + similar[i].site_name);
                    std_out("email     : " + similar[i].email);
                    std_out("password  : " + similar[i].password);
                }
            }
            else if (input == 'f') {
                std_out("[INFO] Enter email");
                std::string email;
                std::cin >> email;

                const int similar_vec_size(similar.size());

                for (int i = 0; i < similar_vec_size; i++) {
                    if (similar[i].email == email) {
                        std_out("\t" + similar[i].site_name);
                        std_out("email     : " + similar[i].email);
                        std_out("password  : " + similar[i].password);
                    }
                    else {
                        std_out("[ERROR] Did not find any entry with " + email);
                        exit(1);
                    }
                }
            }
            else {
                std_out("[ERROR] wrong input");
                exit(1);
            }
        }
        else {
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
    std_out("a,            [sitename email password]");
    std_out("g,            [sitename]");
    std_out("d,            [sitename]");
    std_out("");
    std_out("credentials : ");
    std_out("credentials folder should have 'key.bin' and 'iv.bin' files");
}

bool access_granted()
{
    std::string access_file_enc{credentials_path + "access.enc"};
    std::string access_file_dec{credentials_path + "access.dec"};

    std::string cmd = "openssl enc -d -aes-256-cbc -pbkdf2 -in " +
                      access_file_enc + " -out " + access_file_dec + " -pass ";
    std::string file = "file:" + credentials_path + "key.bin ";
    std::string iv = "-iv $(cat " + credentials_path + "iv.bin)";
    std::string exec = cmd + file + iv;

    system(exec.c_str());

    std::ifstream f(access_file_dec);
    std::string curr_line;
    bool flag = false;

    if (!f.good()) {
        std_out("[FILE NOT FOUND] access.enc no such file");
        exit(1);
    }
    else {
        if (f.is_open()) {
            std::getline(f, curr_line);
            if (curr_line == "true") {
                flag = true;
            }
        }
        else {
            std_out("[FILE ACCESS] cannot read from access.enc");
            f.close();
            exit(1);
        }
    }

    f.close();

    std::string delete_file_path{access_file_dec};
    std::string delete_command{"rm -f " + delete_file_path};
    system(delete_command.c_str());

    if (!flag) {
        return false;
    }
    else {
        return true;
    }
}

void decrypt_vault()
{
    if (access_granted()) {

        std::string cmd = "openssl enc -d -aes-256-cbc -pbkdf2 -in " +
                          passwords_file_enc + " -out " + passwords_file_dec +
                          " -pass ";
        std::string file = "file:" + credentials_path + "key.bin ";
        std::string iv = "-iv $(cat " + credentials_path + "iv.bin)";
        std::string exec = cmd + file + iv;

        system(exec.c_str());
        return;
    }
    else {
        std_out("[ACCESS DENIED] cannot decrypt vault");
        exit(1);
    }
}

void encrypt_vault()
{
    if (access_granted()) {

        std::ofstream filet(passwords_file_enc,
                            std::ofstream::out | std::ofstream::trunc);
        filet.close();

        std::string cmd = "openssl enc -aes-256-cbc -pbkdf2 -in " +
                          passwords_file_dec + " -out " + passwords_file_enc +
                          " -pass ";
        std::string file = "file:" + credentials_path + "key.bin ";
        std::string iv = "-iv $(cat " + credentials_path + "iv.bin)";
        std::string exec = cmd + file + iv;
        system(exec.c_str());

        std::string delete_file_path{passwords_file_dec};
        std::string delete_command{"rm -f " + delete_file_path};
        system(delete_command.c_str());

        return;
    }
    else {
        std_out("[ACCESS DENIED] cannot encrypt vault");
        exit(1);
    }
}

void add_new_entry(std::string &site_name, std::string &email,
                   std::string &password)
{
    decrypt_vault();

    std::string line;
    line.append(site_name + ";");
    line.append(email + ";");
    line.append(password + ";");

    std::ofstream ofs;

    ofs.open(passwords_file_dec, std::ios::app);

    if (ofs.is_open()) {
        ofs << line << "\n";
    }
    else {
        std_out("[FILE] failed to read from passwords.dec");
        exit(1);
    }
    ofs.close();
    encrypt_vault();
    std_out("[INFO] New entry added successfully -_-");
}

void delete_single_entry(const std::string &site_name, const int index)
{
    std::ofstream file(passwords_file_dec, std::ios_base::app);
    const int entries_size(entries.size());

    for (int i = 0; i < entries_size; i++) {
        if (i != index) {
            std::string line;
            line.append(entries[i].site_name + ";");
            line.append(entries[i].email + ";");
            line.append(entries[i].password + ";");
            file << line << "\n";
        }
    }

    file.close();
    encrypt_vault();

    std_out("[INFO] Deletion Successful");
}

void delete_entry(const std::string &site_name)
{
    decrypt_vault();
    load_passwords();

    bool found = false;
    std::vector<int> found_pos;

    int first_found_index;
    std::unordered_map<int, entry> map;

    const int entries_size(entries.size());

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
        }
        else {
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
            delete_single_entry(site_name, first_found_index);
            std_out("[INFO] Entry deleted from vault");
            exit(0);
        }
        else if (input == 'n') {
            std_out("[INFO] Operation Cancelled");
            exit(0);
        }
        else {
            std_out("[ABORT] Wrong input");
            exit(1);
        }
    }
    else {

        std_out("[INFO] found " + std::to_string(found_pos.size()) +
                " entries");

        for (int i = found_pos[0]; i <= found_pos.back(); i++) {
            map[i] = entries[i];
            std_out("\t[" + std::to_string(i) + "] " + entries[i].site_name);
            std_out("email     : " + entries[i].email);
            std_out("password  : " + entries[i].password);
        }

        std_out("[INFO] choose one of the options ");
        std_out("s->delete single entry, m-> delete multiple entries, "
                "c->clear/delete all");

        std::cout << "s/m/c > ";
        char operation;
        std::cin >> operation;

        if (operation == 's') {
            int index;
            std_out("[INFO] choose from above indices");
            std::cout << "i > ";
            std::cin >> index;
            const int entries_size(entries.size());
            if (index > entries_size - 1 || index < 0) {
                std_out("[ABORT] index out of range");
                exit(1);
            }
            else {
                std_out("[INFO] Are you sure..");
                std::cout << "y/n > ";
                char input;
                std::cin >> input;
                if (input == 'y') {
                    delete_single_entry(site_name, index);
                    return;
                }
                else if (input == 'n') {
                    std_out("[INFO] operation cancelled");
                    exit(0);
                }
                else {
                    std_out("[ABORT] wrong input");
                    exit(1);
                }
            }
            // end of 'i'
        }
        else if (operation == 'm') {
            std_out("[INFO] Enter all the indices you want to delete. choose "
                    "from above entries");
            std_out("[INFO] you can specify " +
                    std::to_string(found_pos.size()) + " indices");
            std_out("[INFO] use -1 to stop.");
            std::vector<int> delete_indices;
            int index;
            const int found_pos_size(found_pos.size());
            for (int i = 0; i < found_pos_size; i++) {
                std::cout << "in > ";
                std::cin >> index;
                if (index == -1) {
                    break;
                }
                else if (index > entries_size - 1 || index <= -2) {
                    std_out("[ABORT] index out of range");
                    exit(1);
                    break;
                }
                else {
                    auto it = map.find(index);
                    if (it != map.end()) {
                        delete_indices.push_back(index);
                    }
                    else {
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

                    const int delete_indices_size(delete_indices.size());

                    while (it < delete_indices_size) {
                        entries.erase(entries.begin() + delete_indices[it]);
                        it++;
                    }

                    std::ofstream file(passwords_file_dec, std::ios_base::app);
                    const int entries_size(entries.size());

                    for (int i = 0; i < entries_size; i++) {
                        std::string line;
                        line.append(entries[i].site_name + ";");
                        line.append(entries[i].email + ";");
                        line.append(entries[i].password + ";");
                        file << line << "\n";
                    }

                    file.close();
                    encrypt_vault();

                    std_out("[INFO] Deleted " +
                            std::to_string(delete_indices.size()) + " entries");
                }
                else if (input == 'n') {
                    std_out("[INFO] operation cancelled");
                    exit(0);
                }
                else {
                    std_out("[ABORT] wrong input");
                    exit(1);
                }
            }
            else {
                std_out("[INFO] No index specified" +
                        std::string("[INFO] No entry deleted"));
                exit(1);
            }
            // end of 'm'
        }
        else if (operation == 'c') {

            std_out("Are you sure ");
            std::cout << "y/n > ";
            char input;
            std::cin >> input;
            if (input == 'y') {

                const int found_pos_size(found_pos.size());

                for (int i = found_pos[0]; i <= found_pos_size; i++) {
                    entries.erase(entries.begin() + i);
                }

                std::ofstream file(passwords_file_dec, std::ios_base::app);

                for (int i = 0; i < entries_size; i++) {
                    std::string line;
                    line.append(entries[i].site_name + ";");
                    line.append(entries[i].email + ";");
                    line.append(entries[i].password + ";");
                    file << line << "\n";
                }

                file.close();
                encrypt_vault();

                std_out("[INFO] Deleted all entries");
                exit(0);
            }
            else if (input == 'n') {
                std_out("[INFO] operation cancelled");
                exit(0);
            }
            else {
                std_out("[ABORT] wrong input");
                exit(1);
            }
            // end of c
        }
        else {
            std_out("[ABORT] wrong input");
            exit(1);
        }
    }
}

void initialize_vault()
{
    // check if the credentials_path exist
    bool vault_exist = false;

    if (fs::exists(credentials_path)) {
        if (!fs::is_directory(credentials_path)) {
            std_out("[ERROR] Credentials path is not a directory\n");
            std_out(
                "[INFO]  Create a directory for the vault and add it to env");

            exit(1);
        }
        else {
            vault_exist = true;
        }
    }
    else {
        std_out("[ERROR] Credentials Directory does not exist.");
        std_out("[INFO]  Create a directory for the vault and add it to env");

        exit(1);
    }

    // if the vault exist check if passwords.enc and all the other files exist
    // or not ?? if a file does not exist let the user know

    if (vault_exist) {
        // check if key.bin & iv.bin files exist
        if (!fs::exists(credentials_path + "key.bin") &&
            !fs::exists(credentials_path + "iv.bin")) {
            std_out("[ERROR] key.bin file does not exist\n");
            std_out("[ERROR] iv.bin file does not exist\n");
            std_out("[INFO]  We need both key.bin and iv.bin files, it does "
                    "not work if only one exists\n");
            std_out("[INFO]  Create both key.bin and iv.bin");

            exit(1);
        }

        // if both key.bin and iv.bin exist then check if passwords.enc and
        // access.enc exist so that we don't have to over-write the existing
        // passwords.enc or access.enc files, as this is not the first time
        // the program is ran

        if (fs::exists(passwords_file_enc) &&
            fs::exists(credentials_path + "access.enc")) {
            // just return if both the files exist
            std_out("[DEBUG] both passwords and access files exist\n");
            return;
        }
    }

    // Create access.enc passwords.enc files
    if (vault_exist) {
        std_out("[INFO] Initializing Vault");
        std::string access_file_dec{credentials_path + "access.dec"};

        // create access.dec
        std::ofstream access_write(access_file_dec);

        if (access_write) {
            access_write << "true";
            access_write.close();

            std_out("[INFO] Created access file");
        }
        else {
            std_out("[ERROR] Failed to create access file");
            exit(1);
        }

        // create passwords.dec
        std::ofstream passwords_write(passwords_file_dec);
        if (passwords_write) {
            passwords_write.close();
            std_out("[INFO] Created passwords file");
        }
        else {
            std_out("[ERROR] Failed to create passwords file");
            exit(1);
        }

        // encrypt access.dec and passwords.dec and delete them
        // encrypt passwords.dec
        std::string cmd = "openssl enc -aes-256-cbc -pbkdf2 -in " +
                          passwords_file_dec + " -out " + passwords_file_enc +
                          " -pass ";
        std::string file = "file:" + credentials_path + "key.bin ";
        std::string iv = "-iv $(cat " + credentials_path + "iv.bin)";
        std::string exec = cmd + file + iv;
        system(exec.c_str());

        // delete passwords.dec
        std::string delete_file_path{passwords_file_dec};
        std::string delete_command{"rm -f " + delete_file_path};
        system(delete_command.c_str());

        // encrypt access.enc
        std::string access_file_enc{credentials_path + "access.enc"};

        cmd = "openssl enc -aes-256-cbc -pbkdf2 -in " + access_file_dec +
              " -out " + access_file_enc + " -pass ";
        file = "file:" + credentials_path + "key.bin ";
        iv = "-iv $(cat " + credentials_path + "iv.bin)";
        exec = cmd + file + iv;

        system(exec.c_str());

        // delete access.dec
        delete_file_path = access_file_dec;
        delete_command = "rm -f " + delete_file_path;
        system(delete_command.c_str());
    }
}

int main(int argc, char *argv[])
{
    initialize_vault();
    if (argc == 1) {
        usage();
        exit(1);
    }

    const char arg1 = (char)*argv[1];
    if (arg1 == 'g') {
        if (argc != 3) {
            usage();
            exit(1);
        }
        else {
            decrypt_vault();
            load_passwords();
            get_entry(argv[2]);
            exit(0);
        }
    }
    else if (arg1 == 'a') {
        if (argc != 5) {
            usage();
            exit(1);
        }
        else {
            std::string site_name = (std::string)argv[2];
            std::string email = (std::string)argv[3];
            std::string password = (std::string)argv[4];

            add_new_entry(site_name, email, password);
            exit(0);
        }
    }
    else if (arg1 == 'd') {
        if (argc != 3) {
            usage();
            exit(1);
        }
        else {
            std::string site_name = (std::string)argv[2];
            delete_entry(site_name);
        }
    }
    else {
        usage();
        exit(1);
    }
    return 0;
}
