#include <iostream>
#include <iomanip>
#include <ctime>
#include <limits>
#include <vector>
#include <fstream>
#include <direct.h>
#include <windows.h>
#include <sstream>
#include <algorithm>

#define GREEN 10
#define RED 12
#define RESET 7

#include "../db_connection.h"
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/exception.h>
#include <jdbc/cppconn/prepared_statement.h>

using namespace std;

extern string currentUserID;

int getValidatedChoice6(int minChoice, int maxChoice)
{
    string input;

    while (true)
    {

        getline(cin, input);

        if (input.empty())
        {
            cout << "Input cannot be empty!\n";
            continue;
        }

        try
        {
            size_t pos;
            int choice = stoi(input, &pos);

            if (pos != input.length())
                throw invalid_argument("Invalid");

            if (choice < minChoice || choice > maxChoice)
            {
                cout << "Please enter a number between "
                     << minChoice << " and "
                     << maxChoice << ".\n";
                continue;
            }

            return choice;
        }
        catch (...)
        {
            cout << "Invalid input! Numbers only.\n";
        }
    }
}

// COLOR
void setColor(int color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

//  FORMAT
string formatMoney(double value)
{
    stringstream ss;
    ss << fixed << setprecision(2) << value;
    return ss.str();
}

string getCurrentDate()
{
    time_t now = time(0);
    return string(ctime(&now));
}

string getMonthName(int m)
{
    static const string months[] =
        {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
         "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

    return (m >= 1 && m <= 12) ? months[m] : "Unknown";
}

//  USER
string getUserName()
{
    sql::Connection *con = connectDB();
    sql::Statement *stmt = con->createStatement();

    string query = "SELECT name FROM users WHERE user_id=" + currentUserID;

    sql::ResultSet *res = stmt->executeQuery(query);

    string name = "Unknown";

    if (res->next())
        name = res->getString("name");

    delete res;
    delete stmt;
    delete con;

    return name;
}

//  FOLDER
void createReportFolders()
{
    _mkdir("savedReport");
    _mkdir("savedReport/monthlyExpense");
    _mkdir("savedReport/monthlySummary");
}

//  STAR BAR
void drawStarBar(const string &label, double value, double maxValue)
{
    double percent = (maxValue == 0) ? 0 : (value / maxValue) * 100;
    int stars = (int)(percent / 10);

    cout << left << setw(12) << label << " | ";

    for (int i = 0; i < stars; i++)
        cout << "*";
    for (int i = stars; i < 10; i++)
        cout << ".";

    cout << "  " << fixed << setprecision(1) << percent << "%";
    cout << "  RM " << formatMoney(value) << endl;
}

//  QUERY
sql::ResultSet *runQuery(sql::Statement *stmt, const string &query)
{
    try
    {
        return stmt->executeQuery(query);
    }
    catch (sql::SQLException &e)
    {
        cout << "[SQL ERROR] " << e.what() << endl;
        return nullptr;
    }
}

//  SAVE EXPENSE
void saveMonthlyExpenseReport(int month)
{
    createReportFolders();

    sql::Connection *con = connectDB();
    if (!con)
        return;

    sql::Statement *stmt = con->createStatement();

    int userID = stoi(currentUserID);

    string query =
        "SELECT e.expense_id, c.category_name, e.amount, e.description, e.expense_date "
        "FROM expense e JOIN category c ON e.category_id=c.category_id "
        "WHERE e.user_id=" +
        to_string(userID) +
        " AND MONTH(e.expense_date)=" + to_string(month);

    sql::ResultSet *res = runQuery(stmt, query);

    string username = getUserName();
    replace(username.begin(), username.end(), ' ', '_');

    ofstream file("savedReport/monthlyExpense/monthly_expense_" +
                  getMonthName(month) + "_" + username + ".txt");

    if (!file.is_open())
    {
        cout << "[ERROR SAVING FILE]\n";
        delete res;
        delete stmt;
        delete con;
        return;
    }
    file << "============================================================================================\n";
    file << "                             MONTHLY EXPENSE REPORT\n";
    file << "============================================================================================\n";

    file << "User : " << getUserName() << "\n";
    file << "Month: " << getMonthName(month) << "\n";
    file << "Date : " << getCurrentDate() << "\n";

    file << "--------------------------------------------------------------------------------------------\n";
    file << "EXPENSE DETAILS\n";
    file << "--------------------------------------------------------------------------------------------\n";

    file << left
         << setw(5) << "No"
         << setw(18) << "Category"
         << setw(15) << "Amount"
         << setw(25) << "Description"
         << setw(15) << "Date"
         << "\n";

    file << "--------------------------------------------------------------------------------------------\n";

    double total = 0;
    int no = 1;

    while (res && res->next())
    {
        double amount = res->getDouble("amount");
        total += amount;

        file << left
             << setw(5) << no++
             << setw(18) << res->getString("category_name")
             << setw(15) << ("RM " + formatMoney(amount))
             << setw(25) << res->getString("description")
             << setw(15) << res->getString("expense_date")
             << "\n";
    }

    file << "--------------------------------------------------------------------------------------------\n";
    file << "TOTAL EXPENSE : RM " << formatMoney(total) << "\n";
    file << "--------------------------------------------------------------------------------------------\n";

    file.close();

    cout << "\n[REPORT SAVED SUCCESSFULLY]\n";

    delete res;
    delete stmt;
    delete con;
}

//  EXPENSE REPORT
void monthlyExpenseReport(int month)
{
    sql::Connection *con = connectDB();
    if (!con)
        return;

    sql::Statement *stmt = con->createStatement();

    string query =
        "SELECT e.expense_id, c.category_name, e.amount, e.description, e.expense_date "
        "FROM expense e JOIN category c ON e.category_id=c.category_id "
        "WHERE e.user_id=" +
        currentUserID +
        " AND MONTH(e.expense_date)=" + to_string(month);

    sql::ResultSet *res = runQuery(stmt, query);

    cout << "=============================================================================================\n";
    cout << "                            MONTHLY EXPENSE REPORT\n";
    cout << "=============================================================================================\n";

    cout << "User : " << getUserName() << "\n";
    cout << "Month: " << getMonthName(month) << "\n";

    cout << "\n---------------------------------------------------------------------------------------------\n";
    cout << "EXPENSE DETAILS\n";
    cout << "---------------------------------------------------------------------------------------------\n";

    cout << left
         << setw(5) << "No"
         << setw(15) << "Category"
         << setw(12) << "Amount"
         << setw(20) << "Description"
         << setw(15) << "Date"
         << "\n";

    cout << "---------------------------------------------------------------------------------------------\n";

    double total = 0;
    int no = 1;

    while (res && res->next())
    {
        double amount = res->getDouble("amount");
        total += amount;

        cout << left
             << setw(5) << no++
             << setw(15) << res->getString("category_name")
             << setw(12) << ("RM " + formatMoney(amount))
             << setw(20) << res->getString("description")
             << setw(15) << res->getString("expense_date")
             << "\n";
    }

    cout << "---------------------------------------------------------------------------------------------\n\n";

    cout << "TOTAL SUMMARY\n";
    cout << "---------------------------------------------------------------------------------------------\n";
    cout << "TOTAL EXPENSE: RM " << formatMoney(total) << "\n";

    cout << "\n---------------------------------------------------------------------------------------------\n";
    cout << "OPTIONS\n";
    cout << "---------------------------------------------------------------------------------------------\n";
    cout << "1. Save Report\n0. Back\nChoice: ";

    int opt;
    cin >> opt;

    if (opt == 1)
        saveMonthlyExpenseReport(month);

    delete res;
    delete stmt;
    delete con;
}

//  SAVE SUMMARY
void saveMonthlySummaryReport(int month)
{
    createReportFolders();

    sql::Connection *con = connectDB();
    if (!con)
        return;

    sql::Statement *stmt = con->createStatement();

    string iq =
        "SELECT IFNULL(SUM(amount),0) AS income FROM income "
        "WHERE user_id=" +
        currentUserID +
        " AND MONTH(date_received)=" + to_string(month);

    string eq =
        "SELECT IFNULL(SUM(amount),0) AS expense FROM expense "
        "WHERE user_id=" +
        currentUserID +
        " AND MONTH(expense_date)=" + to_string(month);

    sql::ResultSet *i = runQuery(stmt, iq);
    sql::ResultSet *e = runQuery(stmt, eq);

    double income = 0, expense = 0;

    if (i && i->next())
        income = i->getDouble("income");
    if (e && e->next())
        expense = e->getDouble("expense");

    double balance = income - expense;

    string username = getUserName();
    replace(username.begin(), username.end(), ' ', '_');

    ofstream file("savedReport/monthlySummary/monthly_summary_" +
                  getMonthName(month) + "_" + username + ".txt");

    if (!file.is_open())
    {
        cout << "[ERROR SAVING FILE]\n";
        delete i;
        delete e;
        delete stmt;
        delete con;
        return;
    }
    file << "============================================================================================\n";
    file << "                             MONTHLY SUMMARY REPORT\n";
    file << "============================================================================================\n";

    file << "User : " << getUserName() << "\n";
    file << "Month: " << getMonthName(month) << "\n";
    file << "Date : " << getCurrentDate() << "\n";

    file << "--------------------------------------------------------------------------------------------\n";
    file << "FINANCIAL OVERVIEW\n";
    file << "--------------------------------------------------------------------------------------------\n";

    file << left
         << setw(15) << "ITEM"
         << "AMOUNT\n";

    file << "--------------------------------------------------------------------------------------------\n";

    file << left << setw(15) << "Income" << "RM " << formatMoney(income) << "\n";
    file << left << setw(15) << "Expense" << "RM " << formatMoney(expense) << "\n";
    file << left << setw(15) << "Balance" << "RM " << formatMoney(balance) << "\n";

    file << "--------------------------------------------------------------------------------------------\n";

    if (balance >= 0)
    {
        file << "STATUS : GOOD FINANCIAL CONTROL\n";
    }
    else
    {
        file << "STATUS : OVERSPENDING WARNING\n\n";
        file << "ADVICE:\n";
        file << "- Reduce unnecessary spending\n";
        file << "- Track expenses\n";
        file << "- Use 50/30/20 rule\n";
    }

    file << "============================================================================================\n";

    file.close();

    cout << "\n[SUMMARY REPORT SAVED SUCCESSFULLY]\n";

    delete i;
    delete e;
    delete stmt;
    delete con;
}

//  SUMMARY REPORT
void monthlySummaryReport(int month)
{
    sql::Connection *con = connectDB();
    if (!con)
        return;

    sql::Statement *stmt = con->createStatement();

    string iq =
        "SELECT IFNULL(SUM(amount),0) AS income FROM income "
        "WHERE user_id=" +
        currentUserID +
        " AND MONTH(date_received)=" + to_string(month);

    string eq =
        "SELECT IFNULL(SUM(amount),0) AS expense FROM expense "
        "WHERE user_id=" +
        currentUserID +
        " AND MONTH(expense_date)=" + to_string(month);

    sql::ResultSet *i = runQuery(stmt, iq);
    sql::ResultSet *e = runQuery(stmt, eq);

    double income = 0, expense = 0;

    if (i && i->next())
        income = i->getDouble("income");
    if (e && e->next())
        expense = e->getDouble("expense");

    double balance = income - expense;
    double maxVal = max(income, max(expense, balance));

    cout << "=============================================================================================\n";
    cout << "                              MONTHLY SUMMARY REPORT\n";
    cout << "=============================================================================================\n";

    cout << "User : " << getUserName() << "\n";
    cout << "Month: " << getMonthName(month) << "\n\n";

    cout << "---------------------------------------------------------------------------------------------\n";
    cout << "FINANCIAL OVERVIEW\n";
    cout << "---------------------------------------------------------------------------------------------\n\n";

    cout << left
         << setw(12) << "ITEM"
         << setw(12) << "BAR"
         << setw(8) << "%"
         << "AMOUNT\n";

    cout << "---------------------------------------------------------------------------------------------\n";

    drawStarBar("Income", income, maxVal);
    drawStarBar("Expense", expense, maxVal);
    drawStarBar("Balance", balance, maxVal);

    cout << "---------------------------------------------------------------------------------------------\n\n";

    cout << "STATUS : ";

    if (balance >= 0)
    {
        setColor(GREEN);
        cout << "GOOD FINANCIAL CONTROL\n";
        setColor(RESET);
    }
    else
    {
        setColor(RED);
        cout << "OVERSPENDING WARNING\n";
        setColor(RESET);

        cout << "\nADVICE:\n";
        cout << "- Reduce unnecessary spending\n";
        cout << "- Track daily expenses\n";
        cout << "- Use 50/30/20 rule\n";
    }

    cout << "\n---------------------------------------------------------------------------------------------\n";
    cout << "OPTIONS\n";
    cout << "---------------------------------------------------------------------------------------------\n";

    cout << "1. Save Report\n";
    cout << "0. Back\n";
    cout << "Choice: ";

    int opt;
    cin >> opt;

    if (opt == 1)
        saveMonthlySummaryReport(month);

    delete i;
    delete e;
    delete stmt;
    delete con;
}
void saveBudgetSummaryReport(int month)
{
    createReportFolders();

    sql::Connection *con = connectDB();
    if (!con)
        return;

    sql::Statement *stmt = con->createStatement();
    int userID = stoi(currentUserID);

    string iq =
        "SELECT IFNULL(SUM(amount),0) AS income FROM income "
        "WHERE user_id=" +
        to_string(userID) +
        " AND MONTH(date_received)=" + to_string(month);

    string eq =
        "SELECT IFNULL(SUM(amount),0) AS expense FROM expense "
        "WHERE user_id=" +
        to_string(userID) +
        " AND MONTH(expense_date)=" + to_string(month);

    sql::ResultSet *i = runQuery(stmt, iq);
    sql::ResultSet *e = runQuery(stmt, eq);

    double income = 0, expense = 0;

    if (i && i->next())
        income = i->getDouble("income");
    if (e && e->next())
        expense = e->getDouble("expense");

    double balance = income - expense;

    sql::PreparedStatement *reportStmt = con->prepareStatement(
        "INSERT INTO report (user_id, total_income, total_expense, balance, generated_date) "
        "VALUES (?, ?, ?, ?, CURDATE())");

    reportStmt->setInt(1, userID);
    reportStmt->setDouble(2, income);
    reportStmt->setDouble(3, expense);
    reportStmt->setDouble(4, balance);

    try
    {
        reportStmt->executeUpdate();
    }
    catch (sql::SQLException &e)
    {
        cout << "[DB ERROR] " << e.what() << endl;

        delete reportStmt;
        delete i;
        delete stmt;
        delete con;

        return;
    }

    delete reportStmt;

    string username = getUserName();
    replace(username.begin(), username.end(), ' ', '_');

    string filePath =
        "savedReport/budgetSummary/budget_summary_" +
        getMonthName(month) + "_" + username + ".txt";

    ofstream file(filePath);

    if (!file.is_open())
    {
        cout << "[ERROR] Cannot save report file\n";
    }
    else
    {
        file << "=============================================================================================\n";
        file << "                                 BUDGET SUMMARY REPORT\n";
        file << "=============================================================================================\n\n";

        file << "Name           : " << getUserName() << "\n";
        file << "Month          : " << getMonthName(month) << " 2026\n";
        file << "Generated Date : " << getCurrentDate() << "\n";

        file << "\n---------------------------------------------------------------------------------------------\n";
        file << "FINANCIAL OVERVIEW\n";
        file << "---------------------------------------------------------------------------------------------\n";

        file << "Total Income      : RM " << formatMoney(income) << "\n";
        file << "Total Expenses    : RM " << formatMoney(expense) << "\n";
        file << "Remaining Balance : RM " << formatMoney(balance) << "\n";

        file << "\nCATEGORY BREAKDOWN\n";

        string q =
            "SELECT c.category_name, b.amount_limit, "
            "IFNULL(SUM(e.amount),0) AS spent "
            "FROM budget b "
            "JOIN category c ON b.category_id=c.category_id "
            "LEFT JOIN expense e ON e.category_id=b.category_id "
            "AND e.user_id=b.user_id "
            "AND MONTH(e.expense_date)=b.month "
            "WHERE b.user_id=" +
            to_string(userID) +
            " AND b.month=" + to_string(month) +
            " GROUP BY b.budget_id";

        sql::ResultSet *res = runQuery(stmt, q);

        int overCount = 0;

        while (res && res->next())
        {
            string cat = res->getString("category_name");
            double budget = res->getDouble("amount_limit");
            double spent = res->getDouble("spent");

            double usage = (budget == 0) ? 0 : (spent / budget) * 100;
            if (usage > 100)
                usage = 100;

            if (spent > budget)
                overCount++;

            double balanceCat = budget - spent;

            int stars = usage / 10;
            string bar = "[";
            for (int i = 0; i < 10; i++)
                bar += (i < stars) ? "#" : "-";
            bar += "]";

            string status = (spent > budget) ? "Exceeded" : "Good";

            file << cat << " | "
                 << formatMoney(budget) << " | "
                 << formatMoney(spent) << " | "
                 << formatMoney(balanceCat) << " | "
                 << bar << " | "
                 << (to_string((int)usage) + "%") << " | "
                 << status << "\n";
        }

        file << "\nALERT: " << overCount << " categories exceeded budget.\n";

        file.close();

        cout << "\n[SAVED SUCCESSFULLY]\nFile: " << filePath << "\n";
    }

    // =======================
    // CLEANUP
    // =======================
    delete i;
    delete e;
    delete stmt;
    delete con;
}

void budgetSummaryReport(int month)
{
    sql::Connection *con = connectDB();
    if (!con)
        return;

    sql::Statement *stmt = con->createStatement();
    int userID = stoi(currentUserID);

    string iq =
        "SELECT IFNULL(SUM(amount),0) AS income FROM income "
        "WHERE user_id=" +
        to_string(userID) +
        " AND MONTH(date_received)=" + to_string(month);

    string eq =
        "SELECT IFNULL(SUM(amount),0) AS expense FROM expense "
        "WHERE user_id=" +
        to_string(userID) +
        " AND MONTH(expense_date)=" + to_string(month);

    sql::ResultSet *i = runQuery(stmt, iq);
    sql::ResultSet *e = runQuery(stmt, eq);

    double income = 0, expense = 0;

    if (i && i->next())
        income = i->getDouble("income");
    if (e && e->next())
        expense = e->getDouble("expense");

    double balance = income - expense;

    cout << "\n=============================================================================================\n";
    cout << "                                 BUDGET SUMMARY REPORT\n";
    cout << "=============================================================================================\n\n";

    cout << "Name           : " << getUserName() << "\n";
    cout << "Month          : " << getMonthName(month) << " 2026\n";
    cout << "Generated Date : " << getCurrentDate() << "\n";

    cout << "\n---------------------------------------------------------------------------------------------\n";
    cout << "FINANCIAL OVERVIEW\n";
    cout << "---------------------------------------------------------------------------------------------\n";

    cout << "Total Income      : RM " << formatMoney(income) << "\n";
    cout << "Total Expenses    : RM " << formatMoney(expense) << "\n";
    cout << "Remaining Balance : RM " << formatMoney(balance) << "\n";

    cout << "\n---------------------------------------------------------------------------------------------\n";
    cout << "CATEGORY BREAKDOWN\n";
    cout << "---------------------------------------------------------------------------------------------\n";

    cout << left
         << setw(15) << "Category"
         << setw(12) << "Budget"
         << setw(12) << "Spent"
         << setw(12) << "Balance"
         << setw(18) << "Bar"
         << setw(8) << "Usage"
         << "Status\n";

    cout << "---------------------------------------------------------------------------------------------\n";

    string q =
        "SELECT c.category_name, b.amount_limit, "
        "IFNULL(SUM(e.amount),0) AS spent "
        "FROM budget b "
        "JOIN category c ON b.category_id=c.category_id "
        "LEFT JOIN expense e ON e.category_id=b.category_id "
        "AND e.user_id=b.user_id "
        "AND MONTH(e.expense_date)=b.month "
        "WHERE b.user_id=" +
        to_string(userID) +
        " AND b.month=" + to_string(month) +
        " GROUP BY b.budget_id";

    sql::ResultSet *res = runQuery(stmt, q);

    int overCount = 0;

    while (res && res->next())
    {
        string cat = res->getString("category_name");
        double budget = res->getDouble("amount_limit");
        double spent = res->getDouble("spent");

        double usage = (budget == 0) ? 0 : (spent / budget) * 100;
        if (usage > 100)
            usage = 100;

        if (spent > budget)
            overCount++;

        double balanceCat = budget - spent;

        int stars = usage / 10;
        string bar = "[";

        for (int i = 0; i < 10; i++)
            bar += (i < stars) ? "#" : "-";

        bar += "]";

        string status = (spent > budget) ? "Exceeded" : "Good";

        cout << left
             << setw(15) << cat
             << setw(12) << formatMoney(budget)
             << setw(12) << formatMoney(spent)
             << setw(12) << formatMoney(balanceCat)
             << setw(18) << bar
             << setw(8) << (to_string((int)usage) + "%")
             << status << "\n";
    }

    cout << "---------------------------------------------------------------------------------------------\n";

    cout << "\nTOTAL SUMMARY\n";
    cout << "---------------------------------------------------------------------------------------------\n";

    cout << "Total Budget    : RM " << formatMoney(income) << "\n";
    cout << "Total Spent     : RM " << formatMoney(expense) << "\n";

    double usageTotal = (income == 0) ? 0 : (expense / income) * 100;
    if (usageTotal > 100)
        usageTotal = 100;

    cout << "Budget Usage    : " << fixed << setprecision(1) << usageTotal << "%\n";

    cout << "\n---------------------------------------------------------------------------------------------\n";
    cout << "ALERT SECTION\n";
    cout << "---------------------------------------------------------------------------------------------\n";

    if (overCount > 0)
    {
        setColor(RED);
        cout << "WARNING: " << overCount << " Categories exceeded budget limit!\n";
        setColor(RESET);
    }
    else
    {
        setColor(GREEN);
        cout << "ALL BUDGETS ARE UNDER CONTROL\n";
        setColor(RESET);
    }

    cout << "\n1. Save Report\n0. Back\nChoice: ";

    int opt;
    cin >> opt;

    if (opt == 1)
        saveBudgetSummaryReport(month);

    delete i;
    delete e;
    delete res;
    delete stmt;
    delete con;
}

//  MENU
void reportMenu()
{
    int choice, month;

    do
    {
        system("cls");
        cout << "====================================================\n";
        cout << "         REPORT SYSTEM 2026                \n";
        cout << "====================================================\n";
        cout << "1. Monthly Expense Report \n";
        cout << "2. Budget Summary Report \n";
        cout << "3. Monthly Summary Report \n";
        cout << "0. Back\n";
        cout << "====================================================\n";

        cout << "Enter Choice : ";
        choice = getValidatedChoice6(0, 3);

        switch (choice)
        {
        case 1:
            system("cls");
            cout << "\n================================================\n";
            cout << "         MONTHLY EXPENSE REPORT 2026                 \n";
            cout << "================================================\n";
            cout << "Enter Month (1 - 12) : ";
            month = getValidatedChoice6(1, 12);
            system("cls");
            monthlyExpenseReport(month);
            break;

        case 2:
            system("cls");
            cout << "\n================================================\n";
            cout << "         BUDGET SUMMARY REPORT 2026                   \n";
            cout << "================================================\n";
            cout << "Enter Month(1-12) : ";
            month = getValidatedChoice6(1, 12);
            system("cls");
            budgetSummaryReport(month);
            break;

        case 3:
            system("cls");
            cout << "\n================================================\n";
            cout << "         MONTHLY SUMMARY REPORT 2026                   \n";
            cout << "================================================\n";
            cout << "Enter Month(1-12) : ";
            month = getValidatedChoice6(1, 12);
            system("cls");
            monthlySummaryReport(month);
            break;

        case 0:
            return;
        }

        cout << "\nPress Enter to continue...";
        cin.get();

    } while (true);
}