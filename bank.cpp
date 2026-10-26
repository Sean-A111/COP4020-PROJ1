#include "bank.h"
#include <fstream>
#include <sstream>
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
    for (int i = 0; i < num_clients; ++i) {
        if (clients[i].bank_account_number == account_number) {
            return i;
        }
    }
    return -1;
}

void Bank::load_from_file(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Could not open file: " + filename);
    }

    vector<Client> temp_list;
    string line;
    while (getline(file, line)) {
        if (trim(line).empty()) continue;

        stringstream ss(line);
        string name, ssn, acc_num, bal_str;

        if (getline(ss, name, ',') &&
            getline(ss, ssn, ',') &&
            getline(ss, acc_num, ',') &&
            getline(ss, bal_str)) {
            
            name = trim(name);
            ssn = trim(ssn);
            acc_num = trim(acc_num);
            bal_str = trim(bal_str);

            double balance = stod(bal_str);
            temp_list.emplace_back(name, ssn, acc_num, balance);
        }
    }
    file.close();

    delete[] clients;
    num_clients = static_cast<int>(temp_list.size());
    if (num_clients > 0) {
        clients = new Client[num_clients];
        for (int i = 0; i < num_clients; ++i) {
            clients[i] = temp_list[i];
        }
    } else {
        clients = nullptr;
    }
}

void Bank::save_info(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Could not open file for writing: " + filename);
    }

    for (int i = 0; i < num_clients; ++i) {
        file << clients[i].client_name << ", "
             << clients[i].ssn << ", "
             << clients[i].bank_account_number << ", "
             << clients[i].balance << "\n";
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
    int idx = find_client_index(account_number);
    if (idx == -1) {
        throw invalid_argument("Client account not found: " + account_number);
    }
    return clients[idx];
}

double Bank::deposit(const string& account_number, double amount) {
    if (amount <= 0.0) {
        throw invalid_argument("Deposit amount must be strictly greater than 0.");
    }
    int idx = find_client_index(account_number);
    if (idx == -1) {
        throw invalid_argument("Account not found for deposit: " + account_number);
    }

    clients[idx].balance += amount;
    return clients[idx].balance;
}

double Bank::withdraw(const string& account_number, double amount) {
    if (amount <= 0.0) {
        throw invalid_argument("Withdrawal amount must be strictly greater than 0.");
    }
    int idx = find_client_index(account_number);
    if (idx == -1) {
        throw invalid_argument("Account not found for withdrawal: " + account_number);
    }
    if (clients[idx].balance < amount) {
        throw runtime_error("Insufficient funds for withdrawal.");
    }

    clients[idx].balance -= amount;
    return clients[idx].balance;
}

void Bank::add_new_client(const Client& new_client) {
    if (new_client.bank_account_number.empty() || new_client.client_name.empty()) {
        throw invalid_argument("Account number and client name cannot be empty.");
    }
    if (new_client.balance < 0.0) {
        throw invalid_argument("Initial balance cannot be negative.");
    }
    if (find_client_index(new_client.bank_account_number) != -1) {
        throw invalid_argument("Account number already exists: " + new_client.bank_account_number);
    }

    Client* new_arr = new Client[num_clients + 1];
    for (int i = 0; i < num_clients; ++i) {
        new_arr[i] = clients[i];
    }
    new_arr[num_clients] = new_client;

    delete[] clients;
    clients = new_arr;
    num_clients++;
}
