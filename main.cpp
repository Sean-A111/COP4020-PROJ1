#include "bank.h"
#include <iostream>
#include <fstream>
#include <cassert>

using namespace std;

void create_sample_file(const string& path) {
    ofstream out(path);
    out << "Jordan Lee, 123-45-6789, 0001234567, 1250.75\n";
    out << "Taylor Morgan, 987-65-4321, 0009876543, 500.00\n";
    out.close();
}

int main() {
    cout << "========================================" << endl;
    cout << "    BANK BACKEND TEST DRIVER (Part 1)   " << endl;
    cout << "========================================" << endl;

    const string test_file = "test_clients.csv";
    const string save_file = "test_clients_saved.csv";
    create_sample_file(test_file);

    try {
        // 1. Successful file loading and display of the number of clients
        Bank bank(test_file);
        cout << "[Test 1] Loaded clients. Count: " << bank.get_num_clients() << endl;
        assert(bank.get_num_clients() == 2);

        // 2. Searching for an existing account and a non-existing account
        cout << "[Test 2] Searching for existing account '0001234567':" << endl;
        Client c = bank.find_client("0001234567");
        cout << "         Found: " << c.client_name << " | Balance: $" << c.balance << endl;

        try {
            bank.find_client("9999999999");
            assert(false);
        } catch (const exception& e) {
            cout << "         Non-existent lookup caught: " << e.what() << endl;
        }

        // 3. A valid deposit and a valid withdrawal
        cout << "[Test 3] Valid Transactions:" << endl;
        double bal = bank.deposit("0001234567", 250.25);
        cout << "         Deposited 250.25. New balance: $" << bal << endl;
        assert(bal == 1501.00);

        bal = bank.withdraw("0001234567", 501.00);
        cout << "         Withdrew 501.00. New balance: $" << bal << endl;
        assert(bal == 1000.00);

        // 4. An invalid deposit, an invalid withdrawal, and an insufficient-funds withdrawal
        cout << "[Test 4] Invalid Transactions:" << endl;
        try {
            bank.deposit("0001234567", -50.0);
            assert(false);
        } catch (const exception& e) {
            cout << "         Negative deposit rejected: " << e.what() << endl;
        }

        try {
            bank.withdraw("0001234567", -25.0);
            assert(false);
        } catch (const exception& e) {
            cout << "         Negative withdrawal rejected: " << e.what() << endl;
        }

        try {
            bank.withdraw("0001234567", 5000.00);
            assert(false);
        } catch (const exception& e) {
            cout << "         Overdraft withdrawal rejected: " << e.what() << endl;
        }

        // 5. Adding a new client and attempting to add a duplicate account number
        cout << "[Test 5] Adding Client:" << endl;
        Client new_c("Alex Rivera", "456-78-1234", "0005554443", 320.00);
        bank.add_new_client(new_c);
        cout << "         New client added. New count: " << bank.get_num_clients() << endl;
        assert(bank.get_num_clients() == 3);

        try {
            bank.add_new_client(new_c);
            assert(false);
        } catch (const exception& e) {
            cout << "         Duplicate account rejected: " << e.what() << endl;
        }

        // 6. Saving the modified data to a new text file
        cout << "[Test 6] Saving data to: " << save_file << endl;
        bank.save_info(save_file);
        Bank reloaded_bank(save_file);
        assert(reloaded_bank.get_num_clients() == 3);
        cout << "         Successfully verified saved file reload." << endl;

        // 7. Deep-copy behavior (Copy constructor and Copy-assignment operator)
        cout << "[Test 7] Verifying Deep-Copy Behavior:" << endl;
        Bank copy_bank = bank;
        copy_bank.deposit("0001234567", 500.0);
        assert(copy_bank.find_client("0001234567").balance != bank.find_client("0001234567").balance);
        cout << "         Original bank balance: $" << bank.find_client("0001234567").balance << endl;
        cout << "         Copied bank balance:   $" << copy_bank.find_client("0001234567").balance << endl;

        Bank assigned_bank;
        assigned_bank = bank;
        assigned_bank.withdraw("0001234567", 100.0);
        assert(assigned_bank.find_client("0001234567").balance == 900.00);
        assert(bank.find_client("0001234567").balance == 1000.00);
        cout << "         Independent memory confirmed for both copy construction and copy assignment." << endl;

    } catch (const exception& ex) {
        cerr << "Unhandled Error: " << ex.what() << endl;
        return 1;
    }

    cout << "\nAll Part 1 backend tests passed successfully!" << endl;
    return 0;
}