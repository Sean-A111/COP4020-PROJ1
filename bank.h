#ifndef BANK_H
#define BANK_H

#include <string>
#include <vector>
#include <stdexcept>

struct Client {
    std::string client_name;
    std::string ssn;
    std::string bank_account_number;
    double balance;

    Client() : client_name(""), ssn(""), bank_account_number(""), balance(0.0) {}

    Client(const std::string& name, const std::string& ssn_val,
           const std::string& acc_num, double bal)
        : client_name(name), ssn(ssn_val), bank_account_number(acc_num), balance(bal) {}
};

class Bank {
private:
    int num_clients;
    Client* clients;

    static std::string trim(const std::string& str);
    int find_client_index(const std::string& account_number) const;

public:
    // Constructors & Rule of Three
    Bank();
    explicit Bank(const std::string& filename);
    ~Bank();
    Bank(const Bank& other);
    Bank& operator=(const Bank& other);

    // File I/O
    void load_from_file(const std::string& filename);
    void save_info(const std::string& filename) const;

    // Client Access & Search
    int get_num_clients() const;
    std::vector<Client> get_clients_info() const;
    Client find_client(const std::string& account_number) const;

    // Transactions & Mutators
    double deposit(const std::string& account_number, double amount);
    double withdraw(const std::string& account_number, double amount);
    void add_new_client(const Client& new_client);
};

#endif // BANK_H