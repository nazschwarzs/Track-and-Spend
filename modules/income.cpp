#include <iostream>
#include <iomanip>
#include <limits>
#include <vector>
#include <string>
#include <cctype>

#include "../db_connection.h"
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>

using namespace std;

extern string currentUserID;

// INPUT VALIDATION
int getValidatedChoice3(int minChoice, int maxChoice)
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
                     << minChoice << " and " << maxChoice << ".\n";
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

// VIEW INCOME
vector<int> allIncomeData()
{
    vector<int> incomeIDs;

    try
    {
        sql::Connection *con = connectDB();
        if (!con)
        {
            cout << "Database connection failed!\n";
            return incomeIDs;
        }

        sql::PreparedStatement *pstmt =
            con->prepareStatement(
                "SELECT i.income_id, s.source_name, i.amount, i.date_received "
                "FROM income i "
                "JOIN source s ON i.source_id = s.source_id "
                "WHERE i.user_id = ? "
                "ORDER BY i.date_received DESC");

        int userID = stoi(currentUserID);
        pstmt->setInt(1, userID);

        sql::ResultSet *res = pstmt->executeQuery();

        cout << "\n===============================================================\n";
        cout << "                         MY INCOME\n";
        cout << "===============================================================\n\n";

        cout << left
             << setw(5) << "No"
             << setw(25) << "Source"
             << setw(18) << "Amount (RM)"
             << setw(20) << "Date"
             << "\n";

        cout << "---------------------------------------------------------------\n";

        int no = 1;
        while (res->next())
        {
            incomeIDs.push_back(res->getInt("income_id"));

            cout << left
                 << setw(5) << no++
                 << setw(25) << res->getString("source_name")
                 << setw(18) << fixed << setprecision(2)
                 << res->getDouble("amount")
                 << setw(20) << res->getString("date_received")
                 << "\n";
        }

        if (incomeIDs.empty())
        {
            cout << "No income records found.\n";
        }

        cout << "===============================================================\n";

        delete res;
        delete pstmt;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Error loading income: " << e.what() << "\n";
    }

    return incomeIDs;
}

// TOTAL INCOME
void totalIncome()
{
    try
    {
        sql::Connection *con = connectDB();
        if (!con)
            return;

        sql::PreparedStatement *pstmt =
            con->prepareStatement(
                "SELECT IFNULL(SUM(amount),0) AS total_income "
                "FROM income WHERE user_id = ?");

        pstmt->setInt(1, stoi(currentUserID));

        sql::ResultSet *res = pstmt->executeQuery();

        if (res->next())
        {
            cout << "Total Income : RM "
                 << fixed << setprecision(2)
                 << res->getDouble("total_income") << "\n";
        }

        delete res;
        delete pstmt;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Error calculating total income: " << e.what() << "\n";
    }
}

// ADD INCOME
void addIncome()
{
    system("cls");

    try
    {
        sql::Connection *con = connectDB();
        if (!con)
            return;

        cout << "\n=============== ADD INCOME ===============\n";
        cout << "Select Income Source:\n";
        cout << "  [1] Parents Allowance\n";
        cout << "  [2] Scholarship / Aid\n";
        cout << "  [3] Part-time Job\n";
        cout << "  [4] Internship / Freelance\n";
        cout << "  [5] Others\n";
        cout << "-----------------------------------------\n";

        string sourceInput, amountInput, date_received;

        cout << "Source ID : ";
        getline(cin, sourceInput);

        cout << "Amount (RM): ";
        getline(cin, amountInput);

        cout << "Date (YYYY-MM-DD): ";
        getline(cin, date_received);

        if (sourceInput.empty() || amountInput.empty() || date_received.empty())
        {
            cout << "Fields cannot be empty!\n";
            delete con;
            return;
        }

        int source_id = stoi(sourceInput);
        double amount = stod(amountInput);

        sql::PreparedStatement *pstmt =
            con->prepareStatement(
                "INSERT INTO income (user_id, amount, date_received, source_id) "
                "VALUES (?, ?, ?, ?)");

        pstmt->setInt(1, stoi(currentUserID));
        pstmt->setDouble(2, amount);
        pstmt->setString(3, date_received);
        pstmt->setInt(4, source_id);

        pstmt->executeUpdate();

        cout << "\nIncome added successfully!\n";

        delete pstmt;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Error adding income: " << e.what() << "\n";
    }
    system("pause");
    system("cls");
}

// UPDATE INCOME
void updateIncome()
{
    system("cls");

    vector<int> ids = allIncomeData();
    if (ids.empty())
        return;

    try
    {
        sql::Connection *con = connectDB();
        if (!con)
            return;

        string input;
        cout << "\nSelect No to update: ";
        getline(cin, input);

        int choice = stoi(input);
        if (choice < 1 || choice > ids.size())
        {
            cout << "Invalid selection!\n";
            delete con;
            return;
        }

        int selectedID = ids[choice - 1];

        cout << "\n====================================================\n";
        cout << "                 UPDATE INCOME\n";
        cout << "====================================================\n";
        cout << "1. Source\n";
        cout << "2. Amount\n";
        cout << "3. Date\n";
        cout << "0. Cancel\n";
        cout << "====================================================\n";

        cout << "Choice: ";
        getline(cin, input);

        int option = stoi(input);

        sql::PreparedStatement *pstmt = nullptr;

        switch (option)
        {
        case 1:
        {
            cout << "New Source ID: ";
            getline(cin, input);

            pstmt = con->prepareStatement(
                "UPDATE income SET source_id=? WHERE income_id=? AND user_id=?");

            pstmt->setInt(1, stoi(input));
            pstmt->setInt(2, selectedID);
            pstmt->setInt(3, stoi(currentUserID));
            break;
        }

        case 2:
        {
            cout << "New Amount: ";
            getline(cin, input);

            pstmt = con->prepareStatement(
                "UPDATE income SET amount=? WHERE income_id=? AND user_id=?");

            pstmt->setDouble(1, stod(input));
            pstmt->setInt(2, selectedID);
            pstmt->setInt(3, stoi(currentUserID));
            break;
        }

        case 3:
        {
            cout << "New Date: ";
            getline(cin, input);

            pstmt = con->prepareStatement(
                "UPDATE income SET date_received=? WHERE income_id=? AND user_id=?");

            pstmt->setString(1, input);
            pstmt->setInt(2, selectedID);
            pstmt->setInt(3, stoi(currentUserID));
            break;
        }

        case 0:
            cout << "Update cancelled.\n";
            delete con;
            return;

        default:
            cout << "Invalid option!\n";
            delete con;
            return;
        }

        pstmt->executeUpdate();

        cout << "\nIncome updated successfully!\n";

        delete pstmt;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Error updating income: " << e.what() << "\n";
    }
    system("pause");
    system("cls");
}

// DELETE INCOME
void deleteIncome()
{
    system("cls");

    vector<int> ids = allIncomeData();
    if (ids.empty())
        return;

    try
    {
        sql::Connection *con = connectDB();
        if (!con)
            return;

        string input;

        cout << "\nSelect No to delete: ";
        getline(cin, input);

        int choice = stoi(input);

        if (choice < 1 || choice > ids.size())
        {
            cout << "Invalid selection!\n";
            delete con;
            return;
        }

        cout << "Are you sure? (Y/N): ";
        getline(cin, input);

        if (toupper(input[0]) != 'Y')
        {
            cout << "Cancelled.\n";
            delete con;
            return;
        }

        sql::PreparedStatement *pstmt =
            con->prepareStatement(
                "DELETE FROM income WHERE income_id=? AND user_id=?");

        pstmt->setInt(1, ids[choice - 1]);
        pstmt->setInt(2, stoi(currentUserID));

        pstmt->executeUpdate();

        cout << "\nIncome deleted successfully!\n";

        delete pstmt;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Error deleting income: " << e.what() << "\n";
    }
    system("pause");
    system("cls");
}

// MENU
void viewIncome()
{
    system("cls");

    while (true)
    {
        cout << "\n================================================\n";
        cout << "                  INCOME MENU\n";
        cout << "================================================\n";

        totalIncome();

        cout << "\n1. View Income\n";
        cout << "2. Add Income\n";
        cout << "3. Update Income\n";
        cout << "4. Delete Income\n";
        cout << "0. Back\n";
        cout << "================================================\n";

        int choice = getValidatedChoice3(0, 4);

        switch (choice)
        {
        case 1:
            system("cls");
            allIncomeData();
            break;

        case 2:
            system("cls");
            addIncome();
            break;

        case 3:
            system("cls");
            updateIncome();
            break;

        case 4:
            system("cls");
            deleteIncome();
            break;

        case 0:
            return;
        }
    }
}