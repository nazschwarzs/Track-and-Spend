#include <iostream>
#include <iomanip>
#include <limits>
#include <vector>
#include <string>
#include <sstream>

#define NOMINMAX
#include <windows.h>

#include "../db_connection.h"
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>

using namespace std;

vector<int> incomeIDs;

// SAFE INPUT
int getValidatedChoiceAdmin3(int minChoice, int maxChoice)
{
    string input;

    while (true)
    {
        cout << "Enter Choice : ";
        getline(cin, input);

        try
        {
            if (input.empty())
                throw runtime_error("empty");

            size_t pos;
            int choice = stoi(input, &pos);

            if (pos != input.length())
                throw runtime_error("invalid");

            if (choice < minChoice || choice > maxChoice)
                throw out_of_range("range");

            return choice;
        }
        catch (...)
        {
            cout << "Invalid input.\n";
        }
    }
}

void showSourceMenu()
{
    cout << "\n============= INCOME SOURCE =============\n";
    cout << "[1] Parents Allowance\n";
    cout << "[2] Scholarship / Aid\n";
    cout << "[3] Part-time Job\n";
    cout << "[4] Internship / Freelance\n";
    cout << "[5] Others\n";
    cout << "=========================================\n";
}

bool isValidSource(int source_id)
{
    return source_id >= 1 && source_id <= 5;
}

// DISPLAY ALL INCOME
void displayIncomeAdmin()
{
    try
    {
        incomeIDs.clear();

        sql::Connection *con = connectDB();
        sql::PreparedStatement *ps =
            con->prepareStatement(
                "SELECT i.income_id, i.user_id, u.name, "
                "s.source_name, i.amount, i.date_received "
                "FROM income i "
                "JOIN users u ON i.user_id = u.user_id "
                "JOIN source s ON i.source_id = s.source_id "
                "ORDER BY i.date_received DESC");

        sql::ResultSet *res = ps->executeQuery();

        cout << "\n====================================================================================================\n";
        cout << "                                            INCOME LIST\n";
        cout << "====================================================================================================\n";

        cout << left
             << setw(5) << "No"
             << setw(10) << "User ID"
             << setw(35) << "Name"
             << setw(25) << "Source"
             << setw(15) << "Amount"
             << setw(20) << "Date"
             << endl;

        cout << "----------------------------------------------------------------------------------------------------\n";

        int no = 1;

        while (res->next())
        {
            incomeIDs.push_back(res->getInt("income_id"));

            cout << left
                 << setw(5) << no++
                 << setw(10) << res->getInt("user_id")
                 << setw(30) << res->getString("name")
                 << setw(25) << res->getString("source_name")
                 << setw(15) << fixed << setprecision(2) << res->getDouble("amount")
                 << setw(20) << res->getString("date_received")
                 << "\n";
        }
        cout << "====================================================================================================\n";

        delete res;
        delete ps;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Display error: " << e.what() << endl;
    }
}

// ADD INCOME
void addIncomeAdmin()
{
    try
    {
        system("cls");
        displayIncomeAdmin();

        string userInput;
        string sourceInput;
        string amountInput;
        string date;

        int userID;
        int sourceID;
        double amount;

        cout << "\n================ ADD INCOME ================\n";

        cout << "User ID : ";
        getline(cin, userInput);

        userID = stoi(userInput);

        showSourceMenu();

        cout << "Source ID : ";
        getline(cin, sourceInput);

        sourceID = stoi(sourceInput);

        if (!isValidSource(sourceID))
        {
            cout << "Invalid Source ID!\n";
            return;
        }

        cout << "Amount (RM) : ";
        getline(cin, amountInput);

        amount = stod(amountInput);

        if (amount <= 0)
        {
            cout << "Amount must be greater than 0!\n";
            return;
        }

        cout << "Date (YYYY-MM-DD) : ";
        getline(cin, date);

        if (date.empty())
        {
            cout << "Date cannot be empty!\n";
            return;
        }

        sql::Connection *con = connectDB();

        if (!con)
        {
            cout << "Database connection failed!\n";
            return;
        }

        sql::PreparedStatement *ps =
            con->prepareStatement(
                "INSERT INTO income(user_id, source_id, amount, date_received) "
                "VALUES (?, ?, ?, ?)");

        ps->setInt(1, userID);
        ps->setInt(2, sourceID);
        ps->setDouble(3, amount);
        ps->setString(4, date);

        int rows = ps->executeUpdate();

        if (rows > 0)
            cout << "\nIncome added successfully!\n";
        else
            cout << "\nFailed to add income!\n";

        delete ps;
        delete con;
    }
    catch (const invalid_argument &)
    {
        cout << "Invalid number entered!\n";
    }
    catch (const out_of_range &)
    {
        cout << "Value is out of range!\n";
    }
    catch (const sql::SQLException &e)
    {
        cout << "Database Error: " << e.what() << endl;
    }
    catch (const exception &e)
    {
        cout << "Error: " << e.what() << endl;
    }
}

// UPDATE INCOME
void updateIncomeAdmin()
{
    try
    {
        displayIncomeAdmin();

        string input;
        int no;
        cout << "\n================ UPDATE INCOME ================\n";

        cout << "\nEnter Income Number: ";
        getline(cin, input);

        stringstream ss(input);
        if (!(ss >> no) || no <= 0 || no > (int)incomeIDs.size())
            throw runtime_error("invalid income number");

        int incomeID = incomeIDs[no - 1];

        cout << "\n================ UPDATE MENU ================\n";
        cout << "[1] User ID\n";
        cout << "[2] Source ID\n";
        cout << "[3] Amount\n";
        cout << "[4] Date Received\n";
        cout << "[0] Back\n";
        cout << "================================================\n";

        int choice = getValidatedChoiceAdmin3(0, 4);

        sql::Connection *con = connectDB();
        sql::PreparedStatement *ps = nullptr;

        switch (choice)
        {
        case 1:
        {
            cout << "New User ID: ";
            getline(cin, input);

            ps = con->prepareStatement("UPDATE income SET user_id=? WHERE income_id=?");
            ps->setInt(1, stoi(input));
            ps->setInt(2, incomeID);
            break;
        }

        case 2:
        {
            showSourceMenu();
            cout << "New Source ID: ";
            getline(cin, input);

            ps = con->prepareStatement("UPDATE income SET source_id=? WHERE income_id=?");
            ps->setInt(1, stoi(input));
            ps->setInt(2, incomeID);
            break;
        }

        case 3:
        {
            cout << "New Amount: ";
            getline(cin, input);

            ps = con->prepareStatement("UPDATE income SET amount=? WHERE income_id=?");
            ps->setDouble(1, stod(input));
            ps->setInt(2, incomeID);
            break;
        }

        case 4:
        {
            cout << "New Date: ";
            getline(cin, input);

            ps = con->prepareStatement("UPDATE income SET date_received=? WHERE income_id=?");
            ps->setString(1, input);
            ps->setInt(2, incomeID);
            break;
        }

        case 0:
            return;
        }

        int rows = ps->executeUpdate();

        cout << (rows > 0 ? "Updated successfully!\n"
                          : "Update failed!\n");

        delete ps;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Update error: " << e.what() << endl;
    }
}

// DELETE INCOME
void deleteIncomeAdmin()
{
    try
    {
        displayIncomeAdmin();

        string input;
        int no;

        cout << "\n================ DELETE INCOME ================\n";

        cout << "\nEnter Income Number: ";
        getline(cin, input);

        stringstream ss(input);
        if (!(ss >> no) || no <= 0 || no > (int)incomeIDs.size())
            throw runtime_error("invalid selection");

        int incomeID = incomeIDs[no - 1];

        string confirm;
        cout << "Confirm delete (Y/N): ";
        getline(cin, confirm);

        if (confirm != "Y" && confirm != "y")
        {
            cout << "Cancelled.\n";
            return;
        }

        sql::Connection *con = connectDB();
        sql::PreparedStatement *ps =
            con->prepareStatement("DELETE FROM income WHERE income_id=?");

        ps->setInt(1, incomeID);

        int rows = ps->executeUpdate();

        cout << (rows > 0 ? "Deleted successfully!\n"
                          : "Delete failed!\n");

        delete ps;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Delete error: " << e.what() << endl;
    }
}

// MAIN MENU
void manageIncome()
{
    while (true)
    {
        system("cls");
        displayIncomeAdmin();

        cout << "\n================ INCOME MANAGEMENT ================\n";
        cout << "[1] Add Income\n";
        cout << "[2] Update Income\n";
        cout << "[3] Delete Income\n";
        cout << "[0] Back\n";
        cout << "===================================================\n";

        int choice = getValidatedChoiceAdmin3(0, 3);

        system("cls");

        switch (choice)
        {
        case 1:
            addIncomeAdmin();
            break;

        case 2:
            updateIncomeAdmin();
            break;

        case 3:
            deleteIncomeAdmin();
            break;

        case 0:
            return;
        }

        system("pause");
    }
}