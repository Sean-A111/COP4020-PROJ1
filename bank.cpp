#include "bank.h"
#include <fstream>
#include <sstream>
#include <iomanip>
#include <cmath>
#include <algorithm>
#include <cctype>

using namespace std;

string Bank::trim(const string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

Bank::Bank() : num_clients(0), clients(nullptr) {}

Bank::Bank(const string& filename) : num_clients(0), clients(nullptr) {
    load_from_file(filename);
}

Bank::~Bank() {
    delete[] clients;
    clients = nullptr;
    num_clients = 0;
}

Bank::Bank(const Bank& other) : num_clients(other.num_clients), clients(nullptr) {
    if (other.num_clients > 0 && other.clients != nullptr) {
        clients = new Client[other.num_clients];
        for (int i = 0; i < other.num_clients; ++i) {
            clients[i] = other.clients[i];
        }
    }
}

Bank& Bank::operator=(const Bank& other) {
    if (this != &other) {
        Client* new_clients = nullptr;
        if (other.num_clients > 0 && other.clients != nullptr) {
            new_clients = new Client[other.num_clients];
            for (int i = 0; i < other.num_clients; ++i) {
                new_clients[i] = other.clients[i];
            }
        }
        delete[] clients;
        clients = new_clients;
        num_clients = other.num_clients;
    }
    return *this;
}

int Bank::find_client_index(const string& account_number) const {
    string target = trim(account_number);
    for (int i = 0; i < num_clients; ++i) {
        if (clients[i].bank_account_number == target) {
            return i;
        }
    }
    return -1;
}

void Bank::load_from_file(const string& filename) {
    if (trim(filename).empty()) {
        throw invalid_argument("Filename cannot be empty.");
    }

    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Could not open file: " + filename);
    }

    vector<Client> temp_list;
    string line;
    int line_num = 0;

    while (getline(file, line)) {
        line_num++;
        if (trim(line).empty()) continue;

        stringstream ss(line);
        string name, ssn, acc_num, bal_str, extra;

        if (!getline(ss, name, ',') ||
            !getline(ss, ssn, ',') ||
            !getline(ss, acc_num, ',') ||
            !getline(ss, bal_str)) {
            throw runtime_error("Malformed CSV record at line " + to_string(line_num) + ".");
        }

        // Reject lines with extra commas/fields
        if (getline(ss, extra)) {
            throw runtime_error("Too many fields in CSV record at line " + to_string(line_num) + ".");
        }

        name = trim(name);
        ssn = trim(ssn);
        acc_num = trim(acc_num);
        bal_str = trim(bal_str);

        if (name.empty() || ssn.empty() || acc_num.empty() || bal_str.empty()) {
            throw runtime_error("Missing required field at line " + to_string(line_num) + ".");
        }

        double balance = 0.0;
        try {
            size_t idx = 0;
            balance = stod(bal_str, &idx);
            if (idx != bal_str.length() || isnan(balance) || isinf(balance)) {
                throw invalid_argument("Trailing characters or invalid float");
            }
        } catch (const exception&) {
            throw runtime_error("Invalid balance value at line " + to_string(line_num) + ": " + bal_str);
        }

        if (balance < 0.0) {
            throw runtime_error("Negative balance in file at line " + to_string(line_num) + ".");
        }

        for (const auto& existing : temp_list) {
            if (existing.bank_account_number == acc_num) {
                throw runtime_error("Duplicate account number in file at line " + to_string(line_num) + ": " + acc_num);
            }
        }

        temp_list.emplace_back(name, ssn, acc_num, balance);
    }
    file.close();

    // Exception-safe replacement of dynamic array
    Client* new_clients = nullptr;
    int new_count = static_cast<int>(temp_list.size());
    if (new_count > 0) {
        new_clients = new Client[new_count];
        for (int i = 0; i < new_count; ++i) {
            new_clients[i] = temp_list[i];
        }
    }

    delete[] clients;
    clients = new_clients;
    num_clients = new_count;
}

void Bank::save_info(const string& filename) const {
    if (trim(filename).empty()) {
        throw invalid_argument("Output filename cannot be empty.");
    }

    ofstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Could not open file for writing: " + filename);
    }

    file << fixed << setprecision(2);
    for (int i = 0; i < num_clients; ++i) {
        file << clients[i].client_name << ", "
             << clients[i].ssn << ", "
             << clients[i].bank_account_number << ", "
             << clients[i].balance << "\n";
        if (file.fail()) {
            throw runtime_error("Error occurred while writing to file: " + filename);
        }
    }
    file.close();
}

int Bank::get_num_clients() const {
    return num_clients;
}

vector<Client> Bank::get_clients_info() const {
    vector<Client> list;
    list.reserve(num_clients);
    for (int i = 0; i < num_clients; ++i) {
        list.push_back(clients[i]);
    }
    return list;
}

Client Bank::find_client(const string& account_number) const {
    string clean_acc = trim(account_number);
    if (clean_acc.empty()) {
        throw invalid_argument("Account number cannot be empty.");
    }
    int idx = find_client_index(clean_acc);
    if (idx == -1) {
        throw invalid_argument("Client account not found: " + clean_acc);
    }
    return clients[idx];
}

double Bank::deposit(const string& account_number, double amount) {
    if (isnan(amount) || isinf(amount) || amount <= 0.0) {
        throw invalid_argument("Deposit amount must be strictly greater than 0.");
    }
    string clean_acc = trim(account_number);
    int idx = find_client_index(clean_acc);
    if (idx == -1) {
        throw invalid_argument("Account not found for deposit: " + clean_acc);
    }

    clients[idx].balance += amount;
    return clients[idx].balance;
}

double Bank::withdraw(const string& account_number, double amount) {
    if (isnan(amount) || isinf(amount) || amount <= 0.0) {
        throw invalid_argument("Withdrawal amount must be strictly greater than 0.");
    }
    string clean_acc = trim(account_number);
    int idx = find_client_index(clean_acc);
    if (idx == -1) {
        throw invalid_argument("Account not found for withdrawal: " + clean_acc);
    }
    if (clients[idx].balance < amount) {
        throw runtime_error("Insufficient funds for withdrawal.");
    }

    clients[idx].balance -= amount;
    return clients[idx].balance;
}

void Bank::add_new_client(const Client& new_client) {
    string name = trim(new_client.client_name);
    string ssn = trim(new_client.ssn);
    string acc_num = trim(new_client.bank_account_number);
    double bal = new_client.balance;

    if (name.empty() || ssn.empty() || acc_num.empty()) {
        throw invalid_argument("Client name, SSN, and account number cannot be empty.");
    }
    if (isnan(bal) || isinf(bal) || bal < 0.0) {
        throw invalid_argument("Initial balance cannot be negative.");
    }
    if (find_client_index(acc_num) != -1) {
        throw invalid_argument("Account number already exists: " + acc_num);
    }

    Client sanitized_client(name, ssn, acc_num, bal);

    Client* new_arr = new Client[num_clients + 1];
    for (int i = 0; i < num_clients; ++i) {
        new_arr[i] = clients[i];
    }
    new_arr[num_clients] = sanitized_client;

    delete[] clients;
    clients = new_arr;
    num_clients++;
}