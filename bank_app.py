import ctypes
import os
import sys
import tkinter as tk
from tkinter import ttk, filedialog, messagebox


class BankApp:
    ERROR_BUF_SIZE = 256
    FIELD_BUF_SIZE = 256

    def __init__(self, root: tk.Tk):
        self.root = root
        self.root.title("COP 4020 Banking Application")
        self.root.geometry("780x640")
        self.root.minsize(720, 600)

        # Pointer to C++ Bank object (ctypes.c_void_p)
        self.bank_ptr = None
        self.lib = None

        # Configure dark indigo/cosmic purple styling with cyan accents
        self._configure_styles()

        # Load shared library and bind ctypes signatures
        self._load_backend_library()

        # Build UI layout
        self._build_gui()

        # Ensure bank_destroy is called exactly once when closing
        self.root.protocol("WM_DELETE_WINDOW", self.on_close)

    def _configure_styles(self):
        self.bg_primary = "#131124"
        self.bg_card = "#1E1B36"
        self.fg_text = "#ECEFF4"
        self.fg_muted = "#A3A8C3"
        self.accent_cyan = "#00E5FF"
        self.success_green = "#00E676"
        self.error_red = "#FF5252"

        self.root.configure(bg=self.bg_primary)
        style = ttk.Style(self.root)
        style.theme_use("clam")

        style.configure("TFrame", background=self.bg_primary)
        style.configure("Card.TLabelframe", background=self.bg_card, foreground=self.accent_cyan,
                        borderwidth=1, relief="solid")
        style.configure("Card.TLabelframe.Label", background=self.bg_card, foreground=self.accent_cyan,
                        font=("Segoe UI", 10, "bold"))
        style.configure("TLabel", background=self.bg_card, foreground=self.fg_text,
                        font=("Segoe UI", 10))
        style.configure("Header.TLabel", background=self.bg_primary, foreground=self.accent_cyan,
                        font=("Segoe UI", 15, "bold"))
        style.configure("SubHeader.TLabel", background=self.bg_primary, foreground=self.fg_muted,
                        font=("Segoe UI", 10))
        style.configure("Value.TLabel", background=self.bg_card, foreground="#FFFFFF",
                        font=("Consolas", 11, "bold"))
        style.configure("TEntry", fieldbackground="#292548", foreground="#FFFFFF",
                        insertcolor="#FFFFFF", borderwidth=1)
        style.configure("Accent.TButton", background="#2E295E", foreground=self.accent_cyan,
                        font=("Segoe UI", 9, "bold"), padding=6)
        style.map("Accent.TButton",
                  background=[("active", "#3F387F")],
                  foreground=[("active", "#FFFFFF")])

    def _load_backend_library(self):
        base_dir = os.path.dirname(os.path.abspath(__file__))
        if sys.platform.startswith("win"):
            lib_names = ["bank.dll", "libbank.dll"]
            # Ensure MinGW runtime DLLs are discoverable on Windows
            if hasattr(os, "add_dll_directory"):
                os.add_dll_directory(base_dir)
                for path_dir in os.environ.get("PATH", "").split(os.pathsep):
                    if os.path.isdir(path_dir):
                        try:
                            os.add_dll_directory(path_dir)
                        except OSError:
                            pass
        elif sys.platform == "darwin":
            lib_names = ["libbank.dylib"]
        else:
            lib_names = ["libbank.so"]

        lib_path = None
        for name in lib_names:
            candidate = os.path.join(base_dir, name)
            if os.path.exists(candidate):
                lib_path = candidate
                break

        if not lib_path:
            messagebox.showerror(
                "Shared Library Missing",
                f"Could not find compiled C++ library ({', '.join(lib_names)}) in:\n{base_dir}\n\n"
                "Please compile bank.cpp and bank_wrapper.cpp first (see README.md)."
            )
            self.root.destroy()
            sys.exit(1)

        try:
            self.lib = ctypes.CDLL(lib_path)
        except OSError as e:
            messagebox.showerror("Library Load Error", f"Failed to load {lib_path}:\n{e}")
            self.root.destroy()
            sys.exit(1)

        # Declare argtypes and restype for every C wrapper function
        self.lib.bank_create_empty.argtypes = [ctypes.c_char_p, ctypes.c_int]
        self.lib.bank_create_empty.restype = ctypes.c_void_p

        self.lib.bank_create.argtypes = [ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
        self.lib.bank_create.restype = ctypes.c_void_p

        self.lib.bank_destroy.argtypes = [ctypes.c_void_p]
        self.lib.bank_destroy.restype = None

        self.lib.bank_load_file.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
        self.lib.bank_load_file.restype = ctypes.c_int

        self.lib.bank_save_file.argtypes = [ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p, ctypes.c_int]
        self.lib.bank_save_file.restype = ctypes.c_int

        self.lib.bank_get_num_clients.argtypes = [ctypes.c_void_p]
        self.lib.bank_get_num_clients.restype = ctypes.c_int

        self.lib.bank_find_client.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p,
            ctypes.c_char_p, ctypes.c_int,
            ctypes.c_char_p, ctypes.c_int,
            ctypes.c_char_p, ctypes.c_int,
            ctypes.POINTER(ctypes.c_double),
            ctypes.c_char_p, ctypes.c_int
        ]
        self.lib.bank_find_client.restype = ctypes.c_int

        self.lib.bank_deposit.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p, ctypes.c_double,
            ctypes.POINTER(ctypes.c_double), ctypes.c_char_p, ctypes.c_int
        ]
        self.lib.bank_deposit.restype = ctypes.c_int

        self.lib.bank_withdraw.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p, ctypes.c_double,
            ctypes.POINTER(ctypes.c_double), ctypes.c_char_p, ctypes.c_int
        ]
        self.lib.bank_withdraw.restype = ctypes.c_int

        self.lib.bank_add_client.argtypes = [
            ctypes.c_void_p, ctypes.c_char_p, ctypes.c_char_p,
            ctypes.c_char_p, ctypes.c_double, ctypes.c_char_p, ctypes.c_int
        ]
        self.lib.bank_add_client.restype = ctypes.c_int

    def _build_gui(self):
        main_frame = ttk.Frame(self.root, padding=14)
        main_frame.pack(fill=tk.BOTH, expand=True)

        # Top Header & File Management Bar
        header_frame = ttk.Frame(main_frame)
        header_frame.pack(fill=tk.X, pady=(0, 10))

        title_box = ttk.Frame(header_frame)
        title_box.pack(side=tk.LEFT)
        ttk.Label(title_box, text="C++ / Python Banking Portal", style="Header.TLabel").pack(anchor="w")
        self.file_status_var = tk.StringVar(value="No client file loaded (0 clients)")
        ttk.Label(title_box, textvariable=self.file_status_var, style="SubHeader.TLabel").pack(anchor="w")

        btn_box = ttk.Frame(header_frame)
        btn_box.pack(side=tk.RIGHT)
        ttk.Button(btn_box, text="Load Client File...", style="Accent.TButton",
                   command=self.load_client_file).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_box, text="Save Data As...", style="Accent.TButton",
                   command=self.save_client_file).pack(side=tk.LEFT, padx=5)

        # Middle Grid: Left (Search & Client Details + Transactions) | Right (Add New Client)
        content_frame = ttk.Frame(main_frame)
        content_frame.pack(fill=tk.BOTH, expand=True)
        content_frame.columnconfigure(0, weight=1)
        content_frame.columnconfigure(1, weight=1)

        # --- Section 1: Find & Display Client ---
        search_card = ttk.LabelFrame(content_frame, text=" 1. Find / Display Client ",
                                     style="Card.TLabelframe", padding=12)
        search_card.grid(row=0, column=0, sticky="nsew", padx=(0, 6), pady=6)

        ttk.Label(search_card, text="Account Number:").grid(row=0, column=0, sticky="w", pady=4)
        self.search_acc_var = tk.StringVar()
        ttk.Entry(search_card, textvariable=self.search_acc_var, width=22).grid(row=0, column=1, sticky="ew", padx=6, pady=4)
        ttk.Button(search_card, text="Find Client", style="Accent.TButton",
                   command=self.find_client).grid(row=0, column=2, padx=4, pady=4)

        ttk.Separator(search_card, orient="horizontal").grid(row=1, column=0, columnspan=3, sticky="ew", pady=8)

        self.disp_name_var = tk.StringVar(value="—")
        self.disp_ssn_var = tk.StringVar(value="—")
        self.disp_acc_var = tk.StringVar(value="—")
        self.disp_bal_var = tk.StringVar(value="—")

        details = [
            ("Client Name:", self.disp_name_var),
            ("Masked SSN:", self.disp_ssn_var),
            ("Account Number:", self.disp_acc_var),
            ("Current Balance:", self.disp_bal_var),
        ]
        for idx, (label_text, var) in enumerate(details, start=2):
            ttk.Label(search_card, text=label_text).grid(row=idx, column=0, sticky="w", pady=3)
            ttk.Label(search_card, textvariable=var, style="Value.TLabel").grid(
                row=idx, column=1, columnspan=2, sticky="w", padx=6, pady=3
            )
        search_card.columnconfigure(1, weight=1)

        # --- Section 2: Deposit & Withdraw ---
        tx_card = ttk.LabelFrame(content_frame, text=" 2. Account Transactions (Deposit / Withdraw) ",
                                 style="Card.TLabelframe", padding=12)
        tx_card.grid(row=1, column=0, sticky="nsew", padx=(0, 6), pady=6)

        ttk.Label(tx_card, text="Account Number:").grid(row=0, column=0, sticky="w", pady=6)
        self.tx_acc_var = tk.StringVar()
        ttk.Entry(tx_card, textvariable=self.tx_acc_var, width=25).grid(row=0, column=1, columnspan=2, sticky="ew", padx=6, pady=6)

        ttk.Label(tx_card, text="Amount ($):").grid(row=1, column=0, sticky="w", pady=6)
        self.tx_amount_var = tk.StringVar()
        ttk.Entry(tx_card, textvariable=self.tx_amount_var, width=25).grid(row=1, column=1, columnspan=2, sticky="ew", padx=6, pady=6)

        tx_btn_frame = ttk.Frame(tx_card)
        tx_btn_frame.grid(row=2, column=0, columnspan=3, pady=10)
        ttk.Button(tx_btn_frame, text="Deposit Funds", style="Accent.TButton",
                   command=self.deposit_funds).pack(side=tk.LEFT, padx=8)
        ttk.Button(tx_btn_frame, text="Withdraw Funds", style="Accent.TButton",
                   command=self.withdraw_funds).pack(side=tk.LEFT, padx=8)
        tx_card.columnconfigure(1, weight=1)

        # --- Section 3: Add New Client ---
        add_card = ttk.LabelFrame(content_frame, text=" 3. Add New Client ",
                                  style="Card.TLabelframe", padding=12)
        add_card.grid(row=0, column=1, rowspan=2, sticky="nsew", padx=(6, 0), pady=6)

        self.add_name_var = tk.StringVar()
        self.add_ssn_var = tk.StringVar()
        self.add_acc_var = tk.StringVar()
        self.add_bal_var = tk.StringVar()

        fields = [
            ("Full Name:", self.add_name_var),
            ("SSN (e.g., 123-45-6789):", self.add_ssn_var),
            ("Account Number:", self.add_acc_var),
            ("Initial Balance ($):", self.add_bal_var),
        ]
        for row_idx, (lbl, var) in enumerate(fields):
            ttk.Label(add_card, text=lbl).grid(row=row_idx * 2, column=0, sticky="w", pady=(6, 2))
            ttk.Entry(add_card, textvariable=var, width=30).grid(
                row=row_idx * 2 + 1, column=0, sticky="ew", pady=(0, 6)
            )

        ttk.Button(add_card, text="Create Client Account", style="Accent.TButton",
                   command=self.add_new_client).grid(row=8, column=0, sticky="ew", pady=14)
        add_card.columnconfigure(0, weight=1)

        # --- Bottom Status & Feedback Banner ---
        feedback_card = ttk.LabelFrame(main_frame, text=" Operation Feedback ",
                                       style="Card.TLabelframe", padding=10)
        feedback_card.pack(fill=tk.X, pady=(8, 0))

        self.status_var = tk.StringVar(value="Ready. Load a client CSV file or add a new client to begin.")
        self.status_label = tk.Label(
            feedback_card,
            textvariable=self.status_var,
            bg=self.bg_card,
            fg=self.accent_cyan,
            font=("Segoe UI", 10, "bold"),
            anchor="w",
            justify=tk.LEFT,
            wraplength=720
        )
        self.status_label.pack(fill=tk.X)

    # -------------------------------------------------------------------------
    # Helper Methods
    # -------------------------------------------------------------------------
    def _set_feedback(self, message: str, is_error: bool = False):
        self.status_var.set(message)
        self.status_label.configure(fg=self.error_red if is_error else self.success_green)

    @staticmethod
    def _mask_ssn(ssn: str) -> str:
        clean = ssn.strip()
        digits = [c for c in clean if c.isdigit()]
        if len(digits) >= 4:
            last4 = "".join(digits[-4:])
            return f"***-**-{last4}"
        elif len(clean) > 4:
            return "*" * (len(clean) - 4) + clean[-4:]
        return "****"

    def _update_client_count_label(self, source_label: str = None):
        if self.bank_ptr:
            count = self.lib.bank_get_num_clients(self.bank_ptr)
            if source_label:
                self._current_source = source_label
            src = getattr(self, "_current_source", "Active Session")
            self.file_status_var.set(f"{src} ({count} client{'s' if count != 1 else ''})")

    def _refresh_displayed_client(self, account_number: str):
        """Fetches and displays client records from C++ backend."""
        name_buf = ctypes.create_string_buffer(self.FIELD_BUF_SIZE)
        ssn_buf = ctypes.create_string_buffer(self.FIELD_BUF_SIZE)
        acc_buf = ctypes.create_string_buffer(self.FIELD_BUF_SIZE)
        err_buf = ctypes.create_string_buffer(self.ERROR_BUF_SIZE)
        balance_val = ctypes.c_double(0.0)

        status = self.lib.bank_find_client(
            self.bank_ptr,
            account_number.encode("utf-8"),
            name_buf, self.FIELD_BUF_SIZE,
            ssn_buf, self.FIELD_BUF_SIZE,
            acc_buf, self.FIELD_BUF_SIZE,
            ctypes.byref(balance_val),
            err_buf, self.ERROR_BUF_SIZE
        )

        if status == 0:
            name_str = name_buf.value.decode("utf-8", errors="replace")
            ssn_str = ssn_buf.value.decode("utf-8", errors="replace")
            acc_str = acc_buf.value.decode("utf-8", errors="replace")
            self.disp_name_var.set(name_str)
            self.disp_ssn_var.set(self._mask_ssn(ssn_str))
            self.disp_acc_var.set(acc_str)
            self.disp_bal_var.set(f"${balance_val.value:,.2f}")
            # Conveniently populate transaction account number
            self.tx_acc_var.set(acc_str)
            return True, ""
        else:
            err_msg = err_buf.value.decode("utf-8", errors="replace")
            return False, err_msg

    # -------------------------------------------------------------------------
    # GUI User Operations
    # -------------------------------------------------------------------------
    def load_client_file(self):
        filepath = filedialog.askopenfilename(
            title="Select Client CSV File",
            filetypes=[("CSV / Text Files", "*.csv *.txt"), ("All Files", "*.*")]
        )
        if not filepath:
            return

        err_buf = ctypes.create_string_buffer(self.ERROR_BUF_SIZE)

        if self.bank_ptr is None:
            new_ptr = self.lib.bank_create(
                filepath.encode("utf-8"), err_buf, self.ERROR_BUF_SIZE
            )
            if not new_ptr:
                err_msg = err_buf.value.decode("utf-8", errors="replace")
                self._set_feedback(f"Load Failed: {err_msg}", is_error=True)
                messagebox.showerror("File Load Error", err_msg)
                return
            self.bank_ptr = new_ptr
        else:
            status = self.lib.bank_load_file(
                self.bank_ptr, filepath.encode("utf-8"), err_buf, self.ERROR_BUF_SIZE
            )
            if status != 0:
                err_msg = err_buf.value.decode("utf-8", errors="replace")
                self._set_feedback(f"Load Failed: {err_msg}", is_error=True)
                messagebox.showerror("File Load Error", err_msg)
                return

        filename_only = os.path.basename(filepath)
        self._update_client_count_label(f"Loaded: {filename_only}")
        count = self.lib.bank_get_num_clients(self.bank_ptr)
        self._set_feedback(f"Successfully loaded {count} client(s) from '{filename_only}'.", is_error=False)

    def find_client(self):
        acc_num = self.search_acc_var.get().strip()
        if not acc_num:
            self._set_feedback("Validation Error: Please enter an account number to search.", is_error=True)
            messagebox.showwarning("Missing Input", "Please enter an account number to search.")
            return

        if self.bank_ptr is None:
            self._set_feedback("Error: Please load a client file before searching.", is_error=True)
            messagebox.showwarning("No Data Loaded", "Please load a client file first.")
            return

        ok, err_msg = self._refresh_displayed_client(acc_num)
        if ok:
            self._set_feedback(f"Client record found for account '{acc_num}'.", is_error=False)
        else:
            self.disp_name_var.set("—")
            self.disp_ssn_var.set("—")
            self.disp_acc_var.set("—")
            self.disp_bal_var.set("—")
            self._set_feedback(f"Search Error: {err_msg}", is_error=True)
            messagebox.showerror("Account Not Found", err_msg)

    def deposit_funds(self):
        self._execute_transaction(is_deposit=True)

    def withdraw_funds(self):
        self._execute_transaction(is_deposit=False)

    def _execute_transaction(self, is_deposit: bool):
        op_name = "Deposit" if is_deposit else "Withdrawal"
        acc_num = self.tx_acc_var.get().strip()
        amt_str = self.tx_amount_var.get().strip()

        if not acc_num or not amt_str:
            msg = f"Please enter both an Account Number and a {op_name} Amount."
            self._set_feedback(f"Validation Error: {msg}", is_error=True)
            messagebox.showwarning("Missing Input", msg)
            return

        try:
            amount = float(amt_str)
        except ValueError:
            msg = f"{op_name} amount must be a valid numeric value."
            self._set_feedback(f"Validation Error: {msg}", is_error=True)
            messagebox.showerror("Invalid Amount", msg)
            return

        if amount <= 0.0:
            msg = f"{op_name} amount must be strictly greater than 0."
            self._set_feedback(f"Validation Error: {msg}", is_error=True)
            messagebox.showerror("Invalid Amount", msg)
            return

        if self.bank_ptr is None:
            msg = "Please load a client file first."
            self._set_feedback(f"Error: {msg}", is_error=True)
            messagebox.showwarning("No Data Loaded", msg)
            return

        new_bal = ctypes.c_double(0.0)
        err_buf = ctypes.create_string_buffer(self.ERROR_BUF_SIZE)

        c_func = self.lib.bank_deposit if is_deposit else self.lib.bank_withdraw
        status = c_func(
            self.bank_ptr,
            acc_num.encode("utf-8"),
            ctypes.c_double(amount),
            ctypes.byref(new_bal),
            err_buf,
            self.ERROR_BUF_SIZE
        )

        if status == 0:
            self._refresh_displayed_client(acc_num)
            self.tx_amount_var.set("")
            self._set_feedback(
                f"{op_name} of ${amount:,.2f} on account '{acc_num}' succeeded. "
                f"Updated Balance: ${new_bal.value:,.2f}",
                is_error=False
            )
        else:
            err_msg = err_buf.value.decode("utf-8", errors="replace")
            self._set_feedback(f"{op_name} Failed: {err_msg}", is_error=True)
            messagebox.showerror(f"{op_name} Error", err_msg)

    def add_new_client(self):
        name = self.add_name_var.get().strip()
        ssn = self.add_ssn_var.get().strip()
        acc_num = self.add_acc_var.get().strip()
        bal_str = self.add_bal_var.get().strip()

        if not name or not ssn or not acc_num or not bal_str:
            msg = "All client fields (Name, SSN, Account Number, and Initial Balance) are required."
            self._set_feedback(f"Validation Error: {msg}", is_error=True)
            messagebox.showwarning("Missing Required Fields", msg)
            return

        try:
            initial_balance = float(bal_str)
        except ValueError:
            msg = "Initial balance must be a valid numeric value."
            self._set_feedback(f"Validation Error: {msg}", is_error=True)
            messagebox.showerror("Invalid Balance", msg)
            return

        if initial_balance < 0.0:
            msg = "Initial balance cannot be negative."
            self._set_feedback(f"Validation Error: {msg}", is_error=True)
            messagebox.showerror("Invalid Balance", msg)
            return

        err_buf = ctypes.create_string_buffer(self.ERROR_BUF_SIZE)

        # Lazily create an empty C++ Bank object if the user adds a client before loading a file
        if self.bank_ptr is None:
            self.bank_ptr = self.lib.bank_create_empty(err_buf, self.ERROR_BUF_SIZE)
            if not self.bank_ptr:
                err_msg = err_buf.value.decode("utf-8", errors="replace")
                self._set_feedback(f"Backend Error: {err_msg}", is_error=True)
                messagebox.showerror("Backend Error", err_msg)
                return

        status = self.lib.bank_add_client(
            self.bank_ptr,
            name.encode("utf-8"),
            ssn.encode("utf-8"),
            acc_num.encode("utf-8"),
            ctypes.c_double(initial_balance),
            err_buf,
            self.ERROR_BUF_SIZE
        )

        if status == 0:
            self._update_client_count_label()
            self._refresh_displayed_client(acc_num)
            self.add_name_var.set("")
            self.add_ssn_var.set("")
            self.add_acc_var.set("")
            self.add_bal_var.set("")
            self._set_feedback(
                f"Added new client '{name}' (Account: {acc_num}) with balance ${initial_balance:,.2f}.",
                is_error=False
            )
        else:
            err_msg = err_buf.value.decode("utf-8", errors="replace")
            self._set_feedback(f"Add Client Failed: {err_msg}", is_error=True)
            messagebox.showerror("Add Client Error", err_msg)

    def save_client_file(self):
        if self.bank_ptr is None:
            msg = "No active bank session to save. Load a file or add a client first."
            self._set_feedback(f"Save Error: {msg}", is_error=True)
            messagebox.showwarning("Nothing to Save", msg)
            return

        filepath = filedialog.asksaveasfilename(
            title="Save Client Data As",
            defaultextension=".csv",
            filetypes=[("CSV Files", "*.csv"), ("Text Files", "*.txt"), ("All Files", "*.*")]
        )
        if not filepath:
            return

        err_buf = ctypes.create_string_buffer(self.ERROR_BUF_SIZE)
        status = self.lib.bank_save_file(
            self.bank_ptr, filepath.encode("utf-8"), err_buf, self.ERROR_BUF_SIZE
        )

        if status == 0:
            filename_only = os.path.basename(filepath)
            self._set_feedback(f"Successfully saved all client records to '{filename_only}'.", is_error=False)
            messagebox.showinfo("Save Successful", f"Client data saved to:\n{filepath}")
        else:
            err_msg = err_buf.value.decode("utf-8", errors="replace")
            self._set_feedback(f"Save Failed: {err_msg}", is_error=True)
            messagebox.showerror("Save Error", err_msg)

    def on_close(self):
        if self.bank_ptr is not None and self.lib is not None:
            self.lib.bank_destroy(self.bank_ptr)
            self.bank_ptr = None
        self.root.destroy()


if __name__ == "__main__":
    root = tk.Tk()
    app = BankApp(root)
    root.mainloop()