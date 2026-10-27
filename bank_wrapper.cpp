#include "bank.h"
#include <cstring>
#include <exception>

#if defined(_WIN32) || defined(__CYGWIN__)
    #define BANK_API __declspec(dllexport)
#else
    #define BANK_API __attribute__((visibility("default")))
#endif

static void copy_str(const std::string& src, char* dest, int dest_size) {
    if (dest != nullptr && dest_size > 0) {
        std::strncpy(dest, src.c_str(), dest_size - 1);
        dest[dest_size - 1] = '\0';
    }
}

extern "C" {

    // Creates an empty Bank instance (allows launching GUI without a file loaded)
    BANK_API Bank* bank_create_empty(char* error, int error_size) {
        try {
            copy_str("", error, error_size);
            return new Bank();
        } catch (const std::exception& e) {
            copy_str(e.what(), error, error_size);
            return nullptr;
        } catch (...) {
            copy_str("Unknown error creating empty Bank instance.", error, error_size);
            return nullptr;
        }
    }

    // Creates a Bank instance and loads client records from filename
    BANK_API Bank* bank_create(const char* filename, char* error, int error_size) {
        try {
            copy_str("", error, error_size);
            if (filename == nullptr) {
                copy_str("Filename pointer is null.", error, error_size);
                return nullptr;
            }
            return new Bank(std::string(filename));
        } catch (const std::exception& e) {
            copy_str(e.what(), error, error_size);
            return nullptr;
        } catch (...) {
            copy_str("Unknown error initializing Bank from file.", error, error_size);
            return nullptr;
        }
    }

    // Destroys the Bank instance and releases heap memory
    BANK_API void bank_destroy(Bank* bank) {
        delete bank;
    }

    // Loads or reloads client records into an existing Bank instance
    BANK_API int bank_load_file(Bank* bank, const char* filename, char* error, int error_size) {
        if (bank == nullptr) {
            copy_str("Bank backend is not initialized.", error, error_size);
            return 1;
        }
        try {
            copy_str("", error, error_size);
            bank->load_from_file(filename ? std::string(filename) : "");
            return 0;
        } catch (const std::exception& e) {
            copy_str(e.what(), error, error_size);
            return 1;
        } catch (...) {
            copy_str("Unknown error loading file.", error, error_size);
            return 1;
        }
    }

    // Saves current Bank state to filename
    BANK_API int bank_save_file(Bank* bank, const char* filename, char* error, int error_size) {
        if (bank == nullptr) {
            copy_str("No bank data loaded to save.", error, error_size);
            return 1;
        }
        try {
            copy_str("", error, error_size);
            bank->save_info(filename ? std::string(filename) : "");
            return 0;
        } catch (const std::exception& e) {
            copy_str(e.what(), error, error_size);
            return 1;
        } catch (...) {
            copy_str("Unknown error saving file.", error, error_size);
            return 1;
        }
    }

    // Returns the number of clients currently stored
    BANK_API int bank_get_num_clients(Bank* bank) {
        if (bank == nullptr) return 0;
        return bank->get_num_clients();
    }

    // Finds a client by account number and writes fields to output buffers
    BANK_API int bank_find_client(Bank* bank, const char* account_number,
                                  char* name_out, int name_size,
                                  char* ssn_out, int ssn_size,
                                  char* acc_out, int acc_size,
                                  double* balance_out,
                                  char* error, int error_size) {
        if (bank == nullptr) {
            copy_str("Please load a client file first.", error, error_size);
            return 1;
        }
        try {
            copy_str("", error, error_size);
            Client c = bank->find_client(account_number ? std::string(account_number) : "");
            copy_str(c.client_name, name_out, name_size);
            copy_str(c.ssn, ssn_out, ssn_size);
            copy_str(c.bank_account_number, acc_out, acc_size);
            if (balance_out != nullptr) {
                *balance_out = c.balance;
            }
            return 0;
        } catch (const std::exception& e) {
            copy_str(e.what(), error, error_size);
            return 1;
        } catch (...) {
            copy_str("Unknown error searching for client.", error, error_size);
            return 1;
        }
    }

    // Deposits amount into account_number and writes updated balance to new_balance
    BANK_API int bank_deposit(Bank* bank, const char* account_number, double amount,
                              double* new_balance, char* error, int error_size) {
        if (bank == nullptr) {
            copy_str("Please load a client file first.", error, error_size);
            return 1;
        }
        try {
            copy_str("", error, error_size);
            double updated = bank->deposit(account_number ? std::string(account_number) : "", amount);
            if (new_balance != nullptr) {
                *new_balance = updated;
            }
            return 0;
        } catch (const std::exception& e) {
            copy_str(e.what(), error, error_size);
            return 1;
        } catch (...) {
            copy_str("Unknown error during deposit.", error, error_size);
            return 1;
        }
    }

    // Withdraws amount from account_number and writes updated balance to new_balance
    BANK_API int bank_withdraw(Bank* bank, const char* account_number, double amount,
                               double* new_balance, char* error, int error_size) {
        if (bank == nullptr) {
            copy_str("Please load a client file first.", error, error_size);
            return 1;
        }
        try {
            copy_str("", error, error_size);
            double updated = bank->withdraw(account_number ? std::string(account_number) : "", amount);
            if (new_balance != nullptr) {
                *new_balance = updated;
            }
            return 0;
        } catch (const std::exception& e) {
            copy_str(e.what(), error, error_size);
            return 1;
        } catch (...) {
            copy_str("Unknown error during withdrawal.", error, error_size);
            return 1;
        }
    }

    // Adds a new client to the Bank
    BANK_API int bank_add_client(Bank* bank, const char* name, const char* ssn,
                                 const char* account_number, double initial_balance,
                                 char* error, int error_size) {
        if (bank == nullptr) {
            copy_str("Bank backend is not initialized.", error, error_size);
            return 1;
        }
        try {
            copy_str("", error, error_size);
            Client c(
                name ? std::string(name) : "",
                ssn ? std::string(ssn) : "",
                account_number ? std::string(account_number) : "",
                initial_balance
            );
            bank->add_new_client(c);
            return 0;
        } catch (const std::exception& e) {
            copy_str(e.what(), error, error_size);
            return 1;
        } catch (...) {
            copy_str("Unknown error adding new client.", error, error_size);
            return 1;
        }
    }

} // extern "C"