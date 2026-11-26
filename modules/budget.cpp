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
int getValidatedChoice5(int minChoice, int maxChoice)
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

// VIEW ALL BUDGET
vector<int> allBudgetData()
{
    vector<int> budgetIDs;

    try
    {
        sql::Connection *con = connectDB();
        if (!con)
            return budgetIDs;

        int userID = stoi(currentUserID);

        sql::PreparedStatement *pstmt =
            con->prepareStatement(
                "SELECT b.budget_id, c.category_name, "
                "b.amount_limit, b.month, b.year, "
                "IFNULL(SUM(e.amount),0) AS total_spent "
                "FROM budget b "
                "JOIN category c ON b.category_id = c.category_id "
                "LEFT JOIN expense e "
                "ON b.category_id = e.category_id "
                "AND e.user_id = b.user_id "
                "AND MONTH(e.expense_date) = b.month "
                "AND YEAR(e.expense_date) = b.year "
                "WHERE b.user_id = ? "
                "GROUP BY b.budget_id "
                "ORDER BY b.year DESC, b.month DESC");

        pstmt->setInt(1, userID);

        sql::ResultSet *res = pstmt->executeQuery();

        cout << "\n====================================================================================================\n";
        cout << "                                         MY BUDGETS\n";
        cout << "====================================================================================================\n\n";

        cout << left
             << setw(5) << "No"
             << setw(18) << "Category"
             << setw(15) << "Limit"
             << setw(15) << "Spent"
             << setw(15) << "Balance"
             << setw(10) << "Month"
             << setw(8) << "Year"
             << setw(15) << "Status";

        cout << "\n----------------------------------------------------------------------------------------------------\n";

        int no = 1;

        while (res->next())
        {
            budgetIDs.push_back(res->getInt("budget_id"));

            double limit = res->getDouble("amount_limit");
            double spent = res->getDouble("total_spent");
            double balance = limit - spent;

            string status = (spent > limit) ? "Exceeded" : (spent >= limit * 0.8) ? "Warning": "Good";

            cout << left
                 << setw(5) << no++
                 << setw(18) << res->getString("category_name")
                 << setw(15) << fixed << setprecision(2) << limit
                 << setw(15) << spent
                 << setw(15) << balance
                 << setw(10) << res->getInt("month")
                 << setw(8) << res->getInt("year")
                 << setw(15) << status
                 << "\n";
        }

        if (budgetIDs.empty())
            cout << "No budget records found.\n";

        cout << "====================================================================================================\n";

        delete res;
        delete pstmt;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Error loading budget: " << e.what() << "\n";
    }

    system("pause");
    return budgetIDs;
}

// ADD BUDGET
void addBudget()
{
    system("cls");

    try
    {
        sql::Connection *con = connectDB();
        if (!con)
            return;

        int userID = stoi(currentUserID);

        cout << "\n=============== ADD BUDGET ===============\n";
        cout << "[1] Utility\n[2] Transport\n[3] Food\n[4] Entertainment\n[5] Education\n[6] Others\n";

        int category_id = getValidatedChoice5(1, 6);

        string input;
        double amount_limit;

        cout << "Budget Limit (RM): ";
        getline(cin, input);
        amount_limit = stod(input);

        if (amount_limit <= 0)
        {
            cout << "Invalid amount!\n";
            system("pause");
            return;
        }

        int month, year;

        cout << "Month (1-12): ";
        getline(cin, input);
        month = stoi(input);

        cout << "Year: ";
        getline(cin, input);
        year = stoi(input);

        if (month < 1 || month > 12)
        {
            cout << "Invalid month!\n";
            system("pause");
            return;
        }

        sql::PreparedStatement *check =
            con->prepareStatement(
                "SELECT COUNT(*) AS total FROM budget "
                "WHERE user_id=? AND category_id=? AND month=? AND year=?");

        check->setInt(1, userID);
        check->setInt(2, category_id);
        check->setInt(3, month);
        check->setInt(4, year);

        sql::ResultSet *res = check->executeQuery();

        if (res->next() && res->getInt("total") > 0)
        {
            cout << "Budget already exists!\n";

            delete res;
            delete check;
            delete con;
            return;
        }

        delete res;
        delete check;

        sql::PreparedStatement *pstmt =
            con->prepareStatement(
                "INSERT INTO budget (user_id, category_id, amount_limit, month, year) "
                "VALUES (?, ?, ?, ?, ?)");

        pstmt->setInt(1, userID);
        pstmt->setInt(2, category_id);
        pstmt->setDouble(3, amount_limit);
        pstmt->setInt(4, month);
        pstmt->setInt(5, year);

        pstmt->executeUpdate();

        cout << "Budget added successfully!\n";

        delete pstmt;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Error adding budget: " << e.what() << "\n";
    }

    system("pause");
}

// UPDATE BUDGET
void updateBudget()
{
    system("cls");

    vector<int> ids = allBudgetData();
    if (ids.empty())
        return;

    try
    {
        sql::Connection *con = connectDB();
        if (!con)
            return;

        int userID = stoi(currentUserID);

        string input;

        cout << "Select No to update: ";
        getline(cin, input);

        int choice = stoi(input);
        int selectedID = ids.at(choice - 1);

        cout << "\n====================================================\n";
        cout << "                 UPDATE BUDGET\n";
        cout << "====================================================\n";
        cout << "1. Category\n";
        cout << "2. Amount Limit\n";
        cout << "3. Month\n";
        cout << "4. Year\n";
        cout << "0. Cancel\n";
        cout << "====================================================\n";

        cout << "Choice: ";
        getline(cin, input);

        sql::PreparedStatement *pstmt = nullptr;

        switch (stoi(input))
        {
        case 1:
        {
            int category_id = getValidatedChoice5(1, 6);

            pstmt = con->prepareStatement(
                "UPDATE budget SET category_id=? WHERE budget_id=? AND user_id=?");
            pstmt->setInt(1, category_id);
            break;
        }

        case 2:
        {
            cout << "New Limit: ";
            getline(cin, input);

            pstmt = con->prepareStatement(
                "UPDATE budget SET amount_limit=? WHERE budget_id=? AND user_id=?");
            pstmt->setDouble(1, stod(input));
            break;
        }

        case 3:
        {
            cout << "New Month: ";
            getline(cin, input);

            pstmt = con->prepareStatement(
                "UPDATE budget SET month=? WHERE budget_id=? AND user_id=?");
            pstmt->setInt(1, stoi(input));
            break;
        }

        case 4:
        {
            cout << "New Year: ";
            getline(cin, input);

            pstmt = con->prepareStatement(
                "UPDATE budget SET year=? WHERE budget_id=? AND user_id=?");
            pstmt->setInt(1, stoi(input));
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

        pstmt->setInt(2, selectedID);
        pstmt->setInt(3, userID);

        pstmt->executeUpdate();

        cout << "Updated successfully!\n";

        delete pstmt;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Error updating budget: " << e.what() << "\n";
    }

    system("pause");
}

// DELETE BUDGET
void deleteBudget()
{
    system("cls");

    vector<int> ids = allBudgetData();
    if (ids.empty())
        return;

    try
    {
        sql::Connection *con = connectDB();
        if (!con)
            return;

        int userID = stoi(currentUserID);

        string input;

        cout << "Select No to delete: ";
        getline(cin, input);

        int choice = stoi(input);
        int selectedID = ids.at(choice - 1);

        cout << "Confirm (Y/N): ";
        getline(cin, input);

        if (toupper(input[0]) != 'Y')
        {
            cout << "Cancelled.\n";
            system("pause");
            return;
        }

        sql::PreparedStatement *pstmt =
            con->prepareStatement(
                "DELETE FROM budget WHERE budget_id=? AND user_id=?");

        pstmt->setInt(1, selectedID);
        pstmt->setInt(2, userID);

        pstmt->executeUpdate();

        cout << "Deleted successfully!\n";

        delete pstmt;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Error deleting budget: " << e.what() << "\n";
    }

    system("pause");
}

// MENU
void viewBudget()
{
    while (true)
    {
        system("cls");

        cout << "\n========================================\n";
        cout << "            BUDGET MENU\n";
        cout << "========================================\n";

        cout << "\n1. View Budget\n";
        cout << "2. Add Budget\n";
        cout << "3. Update Budget\n";
        cout << "4. Delete Budget\n";
        cout << "0. Back\n";
        cout << "========================================\n";

        int choice = getValidatedChoice5(0, 4);

        switch (choice)
        {
        case 1:
            system("cls");
            allBudgetData();
            break;
        case 2:
            addBudget();
            break;
        case 3:
            updateBudget();
            break;
        case 4:
            deleteBudget();
            break;
        case 0:
            return;
        }
    }
}