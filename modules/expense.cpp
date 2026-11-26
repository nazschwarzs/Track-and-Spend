#include <iostream>
#include <iomanip>
#include <limits>
#include <vector>
#include <string>
#include "../db_connection.h"
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/prepared_statement.h>

using namespace std;

extern string currentUserID;

// INPUT VALIDATION
int getValidatedChoice4(int minChoice, int maxChoice)
{
    string input;

    while (true)
    {
        cout << "Enter Choice : ";
        getline(cin, input);

        if (input.find_first_not_of(" \t\r\n") == string::npos)
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

// VIEW ALL EXPENSES
vector<int> allExpenseData()
{
    vector<int> expenseIDs;

    try
    {
        sql::Connection *con = connectDB();
        if (!con)
            return expenseIDs;

        int userID = stoi(currentUserID);

        sql::PreparedStatement *pstmt =
            con->prepareStatement(
                "SELECT e.expense_id, c.category_name, "
                "e.description, e.amount, e.expense_date "
                "FROM expense e "
                "JOIN category c ON e.category_id = c.category_id "
                "WHERE e.user_id=? "
                "ORDER BY e.expense_date DESC");

        pstmt->setInt(1, userID);

        sql::ResultSet *res = pstmt->executeQuery();

        cout << "\n===============================================================\n";
        cout << "                         MY EXPENSES\n";
        cout << "===============================================================\n\n";

        cout << left
             << setw(5) << "No"
             << setw(15) << "Category"
             << setw(25) << "Description"
             << setw(15) << "Amount"
             << setw(20) << "Date\n";

        cout << "---------------------------------------------------------------\n";

        int no = 1;
        while (res->next())
        {
            expenseIDs.push_back(res->getInt("expense_id"));

            cout << left
                 << setw(5) << no++
                 << setw(15) << res->getString("category_name")
                 << setw(25) << res->getString("description")
                 << setw(15) << fixed << setprecision(2)
                 << res->getDouble("amount")
                 << setw(20) << res->getString("expense_date")
                 << "\n";
        }

        if (expenseIDs.empty())
            cout << "No expense found.\n";

        cout << "===============================================================\n";

        delete res;
        delete pstmt;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Error loading expense: " << e.what() << "\n";
    }

    system("pause");
    return expenseIDs;
}

// TOTAL EXPENSE
void totalExpense()
{
    try
    {
        sql::Connection *con = connectDB();
        if (!con)
            return;

        int userID = stoi(currentUserID);

        sql::PreparedStatement *pstmt =
            con->prepareStatement(
                "SELECT IFNULL(SUM(amount),0) AS total_expense "
                "FROM expense WHERE user_id=?");

        pstmt->setInt(1, userID);

        sql::ResultSet *res = pstmt->executeQuery();

        if (res->next())
        {
            cout << "Total Expense : RM "
                 << fixed << setprecision(2)
                 << res->getDouble("total_expense") << "\n";
        }

        delete res;
        delete pstmt;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Error calculating total expense: " << e.what() << "\n";
    }
}

// ADD EXPENSE
void addExpense()
{
    system("cls");

    try
    {
        sql::Connection *con = connectDB();
        if (!con)
            return;

        int userID = stoi(currentUserID);

        cout << "\n=============== ADD EXPENSE ===============\n";
        cout << "[1] Utility\n[2] Transport\n[3] Food\n[4] Entertainment\n[5] Education\n[6] Others\n";

        int category_id = getValidatedChoice4(1, 6);

        string input;
        cout << "Amount (RM): ";
        getline(cin, input);

        double amount = stod(input);
        if (amount <= 0)
            throw invalid_argument("invalid amount");

        string description, date;

        cout << "Description: ";
        getline(cin, description);

        cout << "Date (YYYY-MM-DD): ";
        getline(cin, date);

        if (description.empty() || date.empty())
        {
            cout << "Fields cannot be empty!\n";
            system("pause");
            return;
        }

        sql::PreparedStatement *pstmt =
            con->prepareStatement(
                "INSERT INTO expense (user_id, category_id, amount, description, expense_date) "
                "VALUES (?, ?, ?, ?, ?)");

        pstmt->setInt(1, userID);
        pstmt->setInt(2, category_id);
        pstmt->setDouble(3, amount);
        pstmt->setString(4, description);
        pstmt->setString(5, date);

        pstmt->executeUpdate();

        cout << "\nExpense added successfully!\n";

        delete pstmt;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Error adding expense: " << e.what() << "\n";
    }

    system("pause");
}

// UPDATE EXPENSE
void updateExpense()
{
    system("cls");

    vector<int> ids = allExpenseData();
    if (ids.empty())
        return;

    try
    {
        sql::Connection *con = connectDB();
        if (!con)
            return;

        int userID = stoi(currentUserID);

        cout << "\nSelect No to update: ";
        string input;
        getline(cin, input);

        int choice = stoi(input);
        int selectedID = ids.at(choice - 1);

        cout << "\n====================================================\n";
        cout << "                 UPDATE EXPENSE\n";
        cout << "====================================================\n";
        cout << "1. Category\n";
        cout << "2. Description\n";
        cout << "3. Amount\n";
        cout << "4. Date\n";
        cout << "0. Cancel\n";
        cout << "====================================================\n";

        cout << "Choice: ";
        getline(cin, input);

        sql::PreparedStatement *pstmt = nullptr;

        switch (stoi(input))
        {
        case 1:
            cout << "New Category ID: ";
            getline(cin, input);

            pstmt = con->prepareStatement(
                "UPDATE expense SET category_id=? WHERE expense_id=? AND user_id=?");
            pstmt->setInt(1, stoi(input));
            break;

        case 2:
            cout << "New Description: ";
            getline(cin, input);

            pstmt = con->prepareStatement(
                "UPDATE expense SET description=? WHERE expense_id=? AND user_id=?");
            pstmt->setString(1, input);
            break;

        case 3:
            cout << "New Amount: ";
            getline(cin, input);

            pstmt = con->prepareStatement(
                "UPDATE expense SET amount=? WHERE expense_id=? AND user_id=?");
            pstmt->setDouble(1, stod(input));
            break;

        case 4:
            cout << "New Date: ";
            getline(cin, input);

            pstmt = con->prepareStatement(
                "UPDATE expense SET expense_date=? WHERE expense_id=? AND user_id=?");
            pstmt->setString(1, input);
            break;

        case 0:
            cout << "Update cancelled.\n";
            delete con;
            return;

        default:
            cout << "Invalid option!\n";
            delete con;
            return;
        }

        pstmt->setInt(2, selectedID);
        pstmt->setInt(3, userID);

        pstmt->executeUpdate();

        cout << "\nUpdated successfully!\n";

        delete pstmt;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Error updating expense: " << e.what() << "\n";
    }

    system("pause");
}

// DELETE EXPENSE
void deleteExpense()
{
    system("cls");

    vector<int> ids = allExpenseData();
    if (ids.empty())
        return;

    try
    {
        sql::Connection *con = connectDB();
        if (!con)
            return;

        int userID = stoi(currentUserID);

        cout << "\nSelect No to delete: ";
        string input;
        getline(cin, input);

        int choice = stoi(input);
        int selectedID = ids.at(choice - 1);

        cout << "Confirm delete (Y/N): ";
        getline(cin, input);

        if (toupper(input[0]) != 'Y')
        {
            cout << "Cancelled.\n";
            system("pause");
            return;
        }

        sql::PreparedStatement *pstmt =
            con->prepareStatement(
                "DELETE FROM expense WHERE expense_id=? AND user_id=?");

        pstmt->setInt(1, selectedID);
        pstmt->setInt(2, userID);

        pstmt->executeUpdate();

        cout << "\nDeleted successfully!\n";

        delete pstmt;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Error deleting expense: " << e.what() << "\n";
    }

    system("pause");
}

// MENU
void viewExpense()
{
    while (true)
    {
        system("cls");

        cout << "\n================================================\n";
        cout << "                 EXPENSE MENU\n";
        cout << "================================================\n";

        totalExpense();

        cout << "\n1. View Expense\n";
        cout << "2. Add Expense\n";
        cout << "3. Update Expense\n";
        cout << "4. Delete Expense\n";
        cout << "0. Back\n";
        cout << "================================================\n";

        int choice = getValidatedChoice4(0, 4);

        switch (choice)
        {
        case 1:
            system("cls");
            allExpenseData();
            break;
        case 2:
            addExpense();
            break;
        case 3:
            updateExpense();
            break;
        case 4:
            deleteExpense();
            break;
        case 0:
            return;
        }
    }
}