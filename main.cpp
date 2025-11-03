#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <algorithm>
#include <cstring>
#include <iomanip>
#include <cmath>

using namespace std;

// ==================== Utility Functions ====================

string trim(const string& str) {
    size_t first = str.find_first_not_of(' ');
    if (first == string::npos) return "";
    size_t last = str.find_last_not_of(' ');
    return str.substr(first, last - first + 1);
}

vector<string> split(const string& str) {
    vector<string> result;
    string current;
    bool inQuote = false;
    
    for (char c : str) {
        if (c == '"') {
            inQuote = !inQuote;
            current += c; // Keep the quote
        } else if (c == ' ' && !inQuote) {
            if (!current.empty()) {
                result.push_back(current);
                current.clear();
            }
        } else {
            current += c;
        }
    }
    
    if (!current.empty()) {
        result.push_back(current);
    }
    
    return result;
}

bool isValidUserID(const string& s) {
    if (s.empty() || s.length() > 30) return false;
    for (char c : s) {
        if (!isalnum(c) && c != '_') return false;
    }
    return true;
}

bool isValidPassword(const string& s) {
    return isValidUserID(s);
}

bool isValidUsername(const string& s) {
    if (s.empty() || s.length() > 30) return false;
    for (char c : s) {
        if (c < 33 || c > 126) return false;
    }
    return true;
}

bool isValidISBN(const string& s) {
    if (s.empty() || s.length() > 20) return false;
    for (char c : s) {
        if (c < 33 || c > 126) return false;
    }
    return true;
}

bool isValidBookString(const string& s) {
    if (s.empty() || s.length() > 60) return false;
    for (char c : s) {
        if (c < 33 || c > 126 || c == '"') return false;
    }
    return true;
}

bool isValidKeyword(const string& s) {
    if (s.empty() || s.length() > 60) return false;
    vector<string> parts;
    string current;
    for (char c : s) {
        if (c == '|') {
            if (current.empty()) return false;
            parts.push_back(current);
            current.clear();
        } else {
            if (c < 33 || c > 126 || c == '"') return false;
            current += c;
        }
    }
    if (current.empty()) return false;
    parts.push_back(current);
    
    // Check for duplicates
    set<string> unique(parts.begin(), parts.end());
    return unique.size() == parts.size();
}

bool isValidPrice(const string& s) {
    if (s.empty() || s.length() > 13) return false;
    int dotCount = 0;
    for (size_t i = 0; i < s.length(); i++) {
        if (s[i] == '.') {
            dotCount++;
            if (dotCount > 1) return false;
        } else if (!isdigit(s[i])) {
            return false;
        }
    }
    return true;
}

bool isValidQuantity(const string& s) {
    if (s.empty() || s.length() > 10) return false;
    for (char c : s) {
        if (!isdigit(c)) return false;
    }
    return stoll(s) > 0;
}

bool isValidCount(const string& s) {
    if (s.empty() || s.length() > 10) return false;
    for (char c : s) {
        if (!isdigit(c)) return false;
    }
    return true;
}

// ==================== Data Structures ====================

struct Account {
    char userID[31];
    char password[31];
    char username[31];
    int privilege;
    
    Account() {
        memset(userID, 0, sizeof(userID));
        memset(password, 0, sizeof(password));
        memset(username, 0, sizeof(username));
        privilege = 0;
    }
    
    Account(const string& uid, const string& pwd, const string& uname, int priv) {
        strcpy(userID, uid.c_str());
        strcpy(password, pwd.c_str());
        strcpy(username, uname.c_str());
        privilege = priv;
    }
};

struct Book {
    char ISBN[21];
    char name[61];
    char author[61];
    char keyword[61];
    double price;
    long long quantity;
    
    Book() {
        memset(ISBN, 0, sizeof(ISBN));
        memset(name, 0, sizeof(name));
        memset(author, 0, sizeof(author));
        memset(keyword, 0, sizeof(keyword));
        price = 0.0;
        quantity = 0;
    }
};

struct Transaction {
    double amount;
    bool isIncome; // true for income (buy), false for expense (import)
    
    Transaction(double amt, bool income) : amount(amt), isIncome(income) {}
};

struct LogEntry {
    string operation;
    string userID;
    string details;
};

// ==================== Storage System ====================

class BookstoreSystem {
private:
    map<string, Account> accounts;
    map<string, Book> books;
    vector<Transaction> transactions;
    vector<LogEntry> logs;
    
    stack<pair<string, int>> loginStack; // userID, privilege
    map<string, string> selectedBooks; // userID -> selected ISBN
    
    bool initialized;
    
    void saveAccounts() {
        ofstream out("accounts.dat", ios::binary);
        int count = accounts.size();
        out.write((char*)&count, sizeof(count));
        for (auto& p : accounts) {
            out.write((char*)&p.second, sizeof(Account));
        }
        out.close();
    }
    
    void loadAccounts() {
        ifstream in("accounts.dat", ios::binary);
        if (!in) return;
        int count;
        in.read((char*)&count, sizeof(count));
        for (int i = 0; i < count; i++) {
            Account acc;
            in.read((char*)&acc, sizeof(Account));
            accounts[acc.userID] = acc;
        }
        in.close();
    }
    
    void saveBooks() {
        ofstream out("books.dat", ios::binary);
        int count = books.size();
        out.write((char*)&count, sizeof(count));
        for (auto& p : books) {
            out.write((char*)&p.second, sizeof(Book));
        }
        out.close();
    }
    
    void loadBooks() {
        ifstream in("books.dat", ios::binary);
        if (!in) return;
        int count;
        in.read((char*)&count, sizeof(count));
        for (int i = 0; i < count; i++) {
            Book book;
            in.read((char*)&book, sizeof(Book));
            books[book.ISBN] = book;
        }
        in.close();
    }
    
    void saveTransactions() {
        ofstream out("transactions.dat", ios::binary);
        int count = transactions.size();
        out.write((char*)&count, sizeof(count));
        for (auto& t : transactions) {
            out.write((char*)&t.amount, sizeof(t.amount));
            out.write((char*)&t.isIncome, sizeof(t.isIncome));
        }
        out.close();
    }
    
    void loadTransactions() {
        ifstream in("transactions.dat", ios::binary);
        if (!in) return;
        int count;
        in.read((char*)&count, sizeof(count));
        for (int i = 0; i < count; i++) {
            double amount;
            bool isIncome;
            in.read((char*)&amount, sizeof(amount));
            in.read((char*)&isIncome, sizeof(isIncome));
            transactions.push_back(Transaction(amount, isIncome));
        }
        in.close();
    }
    
    void saveInitFlag() {
        ofstream out("init.dat");
        out << "1";
        out.close();
    }
    
    bool checkInitFlag() {
        ifstream in("init.dat");
        return in.good();
    }
    
    int getCurrentPrivilege() {
        if (loginStack.empty()) return 0;
        return loginStack.top().second;
    }
    
    string getCurrentUserID() {
        if (loginStack.empty()) return "";
        return loginStack.top().first;
    }
    
    void addLog(const string& op, const string& details = "") {
        LogEntry entry;
        entry.operation = op;
        entry.userID = getCurrentUserID();
        entry.details = details;
        logs.push_back(entry);
    }
    
public:
    BookstoreSystem() : initialized(false) {
        if (!checkInitFlag()) {
            // First run - create root account
            accounts["root"] = Account("root", "sjtu", "root", 7);
            saveAccounts();
            saveInitFlag();
        } else {
            loadAccounts();
            loadBooks();
            loadTransactions();
        }
    }
    
    ~BookstoreSystem() {
        saveAccounts();
        saveBooks();
        saveTransactions();
    }
    
    // ==================== Account Commands ====================
    
    bool cmdSu(const vector<string>& params) {
        if (params.size() < 2 || params.size() > 3) return false;
        
        string userID = params[1];
        string password = params.size() == 3 ? params[2] : "";
        
        if (!isValidUserID(userID)) return false;
        if (!password.empty() && !isValidPassword(password)) return false;
        
        if (accounts.find(userID) == accounts.end()) return false;
        
        Account& acc = accounts[userID];
        
        if (password.empty()) {
            // Password can be omitted if current privilege is higher
            if (getCurrentPrivilege() <= acc.privilege) return false;
        } else {
            if (string(acc.password) != password) return false;
        }
        
        loginStack.push({userID, acc.privilege});
        addLog("su", userID);
        return true;
    }
    
    bool cmdLogout(const vector<string>& params) {
        if (params.size() != 1) return false;
        if (getCurrentPrivilege() < 1) return false;
        
        string userID = getCurrentUserID();
        loginStack.pop();
        
        // Clear selected book for this user
        if (selectedBooks.find(userID) != selectedBooks.end()) {
            selectedBooks.erase(userID);
        }
        
        addLog("logout", userID);
        return true;
    }
    
    bool cmdRegister(const vector<string>& params) {
        if (params.size() != 4) return false;
        
        string userID = params[1];
        string password = params[2];
        string username = params[3];
        
        if (!isValidUserID(userID)) return false;
        if (!isValidPassword(password)) return false;
        if (!isValidUsername(username)) return false;
        
        if (accounts.find(userID) != accounts.end()) return false;
        
        accounts[userID] = Account(userID, password, username, 1);
        addLog("register", userID);
        return true;
    }
    
    bool cmdPasswd(const vector<string>& params) {
        if (params.size() < 3 || params.size() > 4) return false;
        if (getCurrentPrivilege() < 1) return false;
        
        string userID = params[1];
        string currentPassword = params.size() == 4 ? params[2] : "";
        string newPassword = params.size() == 4 ? params[3] : params[2];
        
        if (!isValidUserID(userID)) return false;
        if (!currentPassword.empty() && !isValidPassword(currentPassword)) return false;
        if (!isValidPassword(newPassword)) return false;
        
        if (accounts.find(userID) == accounts.end()) return false;
        
        Account& acc = accounts[userID];
        
        if (currentPassword.empty()) {
            // Can omit current password if privilege is 7
            if (getCurrentPrivilege() != 7) return false;
        } else {
            if (string(acc.password) != currentPassword) return false;
        }
        
        strcpy(acc.password, newPassword.c_str());
        addLog("passwd", userID);
        return true;
    }
    
    bool cmdUseradd(const vector<string>& params) {
        if (params.size() != 5) return false;
        if (getCurrentPrivilege() < 3) return false;
        
        string userID = params[1];
        string password = params[2];
        string privilegeStr = params[3];
        string username = params[4];
        
        if (!isValidUserID(userID)) return false;
        if (!isValidPassword(password)) return false;
        if (privilegeStr.length() != 1 || !isdigit(privilegeStr[0])) return false;
        if (!isValidUsername(username)) return false;
        
        int privilege = privilegeStr[0] - '0';
        if (privilege != 1 && privilege != 3 && privilege != 7) return false;
        
        if (privilege >= getCurrentPrivilege()) return false;
        
        if (accounts.find(userID) != accounts.end()) return false;
        
        accounts[userID] = Account(userID, password, username, privilege);
        addLog("useradd", userID);
        return true;
    }
    
    bool cmdDelete(const vector<string>& params) {
        if (params.size() != 2) return false;
        if (getCurrentPrivilege() < 7) return false;
        
        string userID = params[1];
        
        if (!isValidUserID(userID)) return false;
        
        if (accounts.find(userID) == accounts.end()) return false;
        
        // Check if user is logged in
        stack<pair<string, int>> tempStack = loginStack;
        while (!tempStack.empty()) {
            if (tempStack.top().first == userID) return false;
            tempStack.pop();
        }
        
        accounts.erase(userID);
        addLog("delete", userID);
        return true;
    }
    
    // ==================== Book Commands ====================
    
    bool cmdShow(const vector<string>& params) {
        if (getCurrentPrivilege() < 1) return false;
        
        vector<Book> results;
        
        if (params.size() == 1) {
            // Show all books
            for (auto& p : books) {
                results.push_back(p.second);
            }
        } else if (params.size() == 2) {
            string param = params[1];
            
            if (param.substr(0, 6) == "-ISBN=") {
                string isbn = param.substr(6);
                if (!isValidISBN(isbn)) return false;
                if (books.find(isbn) != books.end()) {
                    results.push_back(books[isbn]);
                }
            } else if (param.substr(0, 6) == "-name=") {
                if (param.length() < 9 || param[6] != '"' || param.back() != '"') return false;
                string name = param.substr(7, param.length() - 8);
                if (!isValidBookString(name)) return false;
                for (auto& p : books) {
                    if (string(p.second.name) == name) {
                        results.push_back(p.second);
                    }
                }
            } else if (param.substr(0, 8) == "-author=") {
                if (param.length() < 11 || param[8] != '"' || param.back() != '"') return false;
                string author = param.substr(9, param.length() - 10);
                if (!isValidBookString(author)) return false;
                for (auto& p : books) {
                    if (string(p.second.author) == author) {
                        results.push_back(p.second);
                    }
                }
            } else if (param.substr(0, 9) == "-keyword=") {
                if (param.length() < 12 || param[9] != '"' || param.back() != '"') return false;
                string keyword = param.substr(10, param.length() - 11);
                if (!isValidBookString(keyword)) return false;
                // Check for multiple keywords (should have no |)
                if (keyword.find('|') != string::npos) return false;
                for (auto& p : books) {
                    string kws = p.second.keyword;
                    // Split keywords and check
                    vector<string> kwList;
                    string current;
                    for (char c : kws) {
                        if (c == '|') {
                            kwList.push_back(current);
                            current.clear();
                        } else {
                            current += c;
                        }
                    }
                    kwList.push_back(current);
                    
                    bool found = false;
                    for (auto& kw : kwList) {
                        if (kw == keyword) {
                            found = true;
                            break;
                        }
                    }
                    if (found) {
                        results.push_back(p.second);
                    }
                }
            } else {
                return false;
            }
        } else {
            return false;
        }
        
        // Sort by ISBN
        sort(results.begin(), results.end(), [](const Book& a, const Book& b) {
            return string(a.ISBN) < string(b.ISBN);
        });
        
        if (results.empty()) {
            cout << "\n";
        } else {
            for (auto& book : results) {
                cout << book.ISBN << "\t"
                     << book.name << "\t"
                     << book.author << "\t"
                     << book.keyword << "\t"
                     << fixed << setprecision(2) << book.price << "\t"
                     << book.quantity << "\n";
            }
        }
        
        addLog("show");
        return true;
    }
    
    bool cmdBuy(const vector<string>& params) {
        if (params.size() != 3) return false;
        if (getCurrentPrivilege() < 1) return false;
        
        string isbn = params[1];
        string quantityStr = params[2];
        
        if (!isValidISBN(isbn)) return false;
        if (!isValidQuantity(quantityStr)) return false;
        
        long long quantity = stoll(quantityStr);
        
        if (books.find(isbn) == books.end()) return false;
        
        Book& book = books[isbn];
        if (book.quantity < quantity) return false;
        
        double totalCost = book.price * quantity;
        book.quantity -= quantity;
        
        transactions.push_back(Transaction(totalCost, true));
        
        cout << fixed << setprecision(2) << totalCost << "\n";
        
        addLog("buy", isbn + " " + quantityStr);
        return true;
    }
    
    bool cmdSelect(const vector<string>& params) {
        if (params.size() != 2) return false;
        if (getCurrentPrivilege() < 3) return false;
        
        string isbn = params[1];
        
        if (!isValidISBN(isbn)) return false;
        
        if (books.find(isbn) == books.end()) {
            // Create new book
            Book newBook;
            strcpy(newBook.ISBN, isbn.c_str());
            books[isbn] = newBook;
        }
        
        string userID = getCurrentUserID();
        selectedBooks[userID] = isbn;
        
        addLog("select", isbn);
        return true;
    }
    
    bool cmdModify(const vector<string>& params) {
        if (params.size() < 2) return false;
        if (getCurrentPrivilege() < 3) return false;
        
        string userID = getCurrentUserID();
        if (selectedBooks.find(userID) == selectedBooks.end()) return false;
        
        string isbn = selectedBooks[userID];
        
        set<string> paramTypes;
        string newISBN = "";
        string newName = "";
        string newAuthor = "";
        string newKeyword = "";
        double newPrice = -1.0;
        bool hasName = false, hasAuthor = false, hasKeyword = false, hasPrice = false;
        
        // First pass: validate and collect all modifications
        for (size_t i = 1; i < params.size(); i++) {
            string param = params[i];
            
            if (param.substr(0, 6) == "-ISBN=") {
                if (paramTypes.count("ISBN")) return false;
                paramTypes.insert("ISBN");
                newISBN = param.substr(6);
                if (!isValidISBN(newISBN)) return false;
                if (newISBN == isbn) return false; // Cannot change to same ISBN
                if (books.find(newISBN) != books.end()) return false; // New ISBN already exists
            } else if (param.substr(0, 6) == "-name=") {
                if (paramTypes.count("name")) return false;
                paramTypes.insert("name");
                if (param.length() < 9 || param[6] != '"' || param.back() != '"') return false;
                newName = param.substr(7, param.length() - 8);
                if (!isValidBookString(newName)) return false;
                hasName = true;
            } else if (param.substr(0, 8) == "-author=") {
                if (paramTypes.count("author")) return false;
                paramTypes.insert("author");
                if (param.length() < 11 || param[8] != '"' || param.back() != '"') return false;
                newAuthor = param.substr(9, param.length() - 10);
                if (!isValidBookString(newAuthor)) return false;
                hasAuthor = true;
            } else if (param.substr(0, 9) == "-keyword=") {
                if (paramTypes.count("keyword")) return false;
                paramTypes.insert("keyword");
                if (param.length() < 12 || param[9] != '"' || param.back() != '"') return false;
                newKeyword = param.substr(10, param.length() - 11);
                if (!isValidKeyword(newKeyword)) return false;
                hasKeyword = true;
            } else if (param.substr(0, 7) == "-price=") {
                if (paramTypes.count("price")) return false;
                paramTypes.insert("price");
                string priceStr = param.substr(7);
                if (!isValidPrice(priceStr)) return false;
                newPrice = stod(priceStr);
                hasPrice = true;
            } else {
                return false;
            }
        }
        
        // Apply modifications
        Book& book = books[isbn];
        
        if (hasName) strcpy(book.name, newName.c_str());
        if (hasAuthor) strcpy(book.author, newAuthor.c_str());
        if (hasKeyword) strcpy(book.keyword, newKeyword.c_str());
        if (hasPrice) book.price = newPrice;
        
        // Handle ISBN change (must be done after other modifications)
        if (!newISBN.empty()) {
            books[newISBN] = book;
            strcpy(books[newISBN].ISBN, newISBN.c_str());
            books.erase(isbn);
            selectedBooks[userID] = newISBN;
        }
        
        addLog("modify");
        return true;
    }
    
    bool cmdImport(const vector<string>& params) {
        if (params.size() != 3) return false;
        if (getCurrentPrivilege() < 3) return false;
        
        string userID = getCurrentUserID();
        if (selectedBooks.find(userID) == selectedBooks.end()) return false;
        
        string quantityStr = params[1];
        string totalCostStr = params[2];
        
        if (!isValidQuantity(quantityStr)) return false;
        if (!isValidPrice(totalCostStr)) return false;
        
        long long quantity = stoll(quantityStr);
        double totalCost = stod(totalCostStr);
        
        if (totalCost <= 0) return false;
        
        string isbn = selectedBooks[userID];
        Book& book = books[isbn];
        book.quantity += quantity;
        
        transactions.push_back(Transaction(totalCost, false));
        
        addLog("import", quantityStr + " " + totalCostStr);
        return true;
    }
    
    // ==================== Log Commands ====================
    
    bool cmdShowFinance(const vector<string>& params) {
        if (params.size() < 2 || params.size() > 3) return false;
        if (getCurrentPrivilege() < 7) return false;
        
        if (params[1] != "finance") return false;
        
        int count = -1;
        if (params.size() == 3) {
            string countStr = params[2];
            if (!isValidCount(countStr)) return false;
            count = stoi(countStr);
            if (count == 0) {
                cout << "\n";
                return true;
            }
            if (count > (int)transactions.size()) return false;
        }
        
        double income = 0.0, expense = 0.0;
        
        int start = 0;
        if (count != -1) {
            start = transactions.size() - count;
        }
        
        for (int i = start; i < (int)transactions.size(); i++) {
            if (transactions[i].isIncome) {
                income += transactions[i].amount;
            } else {
                expense += transactions[i].amount;
            }
        }
        
        cout << "+ " << fixed << setprecision(2) << income 
             << " - " << fixed << setprecision(2) << expense << "\n";
        
        addLog("show finance");
        return true;
    }
    
    bool cmdReportFinance(const vector<string>& params) {
        if (params.size() != 2) return false;
        if (getCurrentPrivilege() < 7) return false;
        if (params[1] != "finance") return false;
        
        // Generate financial report (self-defined format)
        cout << "=== Financial Report ===\n";
        cout << "Total Transactions: " << transactions.size() << "\n";
        
        double totalIncome = 0.0, totalExpense = 0.0;
        for (auto& t : transactions) {
            if (t.isIncome) totalIncome += t.amount;
            else totalExpense += t.amount;
        }
        
        cout << "Total Income: " << fixed << setprecision(2) << totalIncome << "\n";
        cout << "Total Expense: " << fixed << setprecision(2) << totalExpense << "\n";
        cout << "Net Profit: " << fixed << setprecision(2) << (totalIncome - totalExpense) << "\n";
        
        addLog("report finance");
        return true;
    }
    
    bool cmdReportEmployee(const vector<string>& params) {
        if (params.size() != 2) return false;
        if (getCurrentPrivilege() < 7) return false;
        if (params[1] != "employee") return false;
        
        // Generate employee work report (self-defined format)
        cout << "=== Employee Work Report ===\n";
        map<string, int> userOps;
        for (auto& log : logs) {
            if (!log.userID.empty()) {
                userOps[log.userID]++;
            }
        }
        
        for (auto& p : userOps) {
            cout << "User: " << p.first << ", Operations: " << p.second << "\n";
        }
        
        addLog("report employee");
        return true;
    }
    
    bool cmdLog(const vector<string>& params) {
        if (params.size() != 1) return false;
        if (getCurrentPrivilege() < 7) return false;
        
        // Generate log (self-defined format)
        cout << "=== System Log ===\n";
        for (auto& log : logs) {
            cout << "[" << log.userID << "] " << log.operation;
            if (!log.details.empty()) {
                cout << " (" << log.details << ")";
            }
            cout << "\n";
        }
        
        addLog("log");
        return true;
    }
    
    // ==================== Command Processor ====================
    
    void saveAll() {
        saveAccounts();
        saveBooks();
        saveTransactions();
    }
    
    void processCommand(const string& line) {
        string trimmed = trim(line);
        if (trimmed.empty()) return;
        
        vector<string> params = split(trimmed);
        if (params.empty()) return;
        
        string cmd = params[0];
        bool success = false;
        
        if (cmd == "quit" || cmd == "exit") {
            saveAll();
            exit(0);
        } else if (cmd == "su") {
            success = cmdSu(params);
        } else if (cmd == "logout") {
            success = cmdLogout(params);
        } else if (cmd == "register") {
            success = cmdRegister(params);
        } else if (cmd == "passwd") {
            success = cmdPasswd(params);
        } else if (cmd == "useradd") {
            success = cmdUseradd(params);
        } else if (cmd == "delete") {
            success = cmdDelete(params);
        } else if (cmd == "show") {
            if (params.size() >= 2 && params[1] == "finance") {
                success = cmdShowFinance(params);
            } else {
                success = cmdShow(params);
            }
        } else if (cmd == "buy") {
            success = cmdBuy(params);
        } else if (cmd == "select") {
            success = cmdSelect(params);
        } else if (cmd == "modify") {
            success = cmdModify(params);
        } else if (cmd == "import") {
            success = cmdImport(params);
        } else if (cmd == "report") {
            if (params.size() >= 2 && params[1] == "finance") {
                success = cmdReportFinance(params);
            } else if (params.size() >= 2 && params[1] == "employee") {
                success = cmdReportEmployee(params);
            } else {
                success = false;
            }
        } else if (cmd == "log") {
            success = cmdLog(params);
        } else {
            success = false;
        }
        
        if (!success) {
            cout << "Invalid\n";
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    BookstoreSystem system;
    
    string line;
    while (getline(cin, line)) {
        system.processCommand(line);
    }
    
    return 0;
}
