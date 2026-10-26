#ifndef BANK_H
#define BANK_H

#include <string>
#include <vector>
#include <stdexcept>

using namespace std;

struct Client {
    string client_name;
    string ssn;
    string bank_account_number;
    double balance;

    Client() : client_name(""), ssn(""), bank_account_number(""), balance(0.0) {}
    Client(const string& name, const string& ssn_val, const string& acc_num, double bal)
        : client_name(name), ssn(ssn_val), bank_account_number(acc_num), balance(bal) {}
};

class Bank {
private:
    int num_clients;
    Client* clients;

    int find_client_index(const string& account_number) const;
    static string trim(const string& str);

public:
    Bank();
    explicit Bank(const string& filename);
    ~Bank();

    Bank(const Bank& other);
    Bank& operator=(const Bank& other);

    void load_from_file(const string& filename);
    void save_info(const string& filename) const;

    int get_num_clients() const;
    vector<Client> get_clients_info() const;
    Client find_client(const string& account_number) const;

    double deposit(const string& account_number, double amount);
    double withdraw(const string& account_number, double amount);
    void add_new_client(const Client& new_client);
};

#endif
