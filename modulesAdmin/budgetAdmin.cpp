#include <iostream>
#include <iomanip>
#include <limits>
#include <vector>
#include <string>

#define NOMINMAX
#include <windows.h>

#include "../db_connection.h"
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>

using namespace std;

vector<int> budgetIDs;

int getValidatedChoiceAdmin5(int minChoice, int maxChoice)
{
    string input;

    while (true)
    {
        cout << "Enter Choice : ";
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

void showCategoryMenu()
{
    cout << "\n============= CATEGORY =============\n";
    cout << "[1] Utility\n";
    cout << "[2] Transport\n";
    cout << "[3] Food\n";
    cout << "[4] Entertainment\n";
    cout << "[5] Education\n";
    cout << "[6] Others\n";
    cout << "====================================\n";
}

bool isValidCategory(int category_id)
{
    return category_id >= 1 && category_id <= 6;
}

// DISPLAY BUDGET
void displayBudgetAdmin()
{
    budgetIDs.clear();

    sql::Connection *con = connectDB();
    sql::Statement *stmt = con->createStatement();

    string query =
        "SELECT b.budget_id, b.user_id, u.name, "
        "c.category_name, b.amount_limit, "
        "b.month, b.year "
        "FROM budget b "
        "JOIN users u ON b.user_id = u.user_id "
        "JOIN category c ON b.category_id = c.category_id "
        "ORDER BY b.year DESC, b.month DESC";

    sql::ResultSet *res = stmt->executeQuery(query);

    cout << "\n============================================================================================================\n";
    cout << "                                               BUDGET LIST\n";
    cout << "============================================================================================================\n";

    cout << left
         << setw(5) << "No"
         << setw(10) << "User"
         << setw(30) << "Name"
         << setw(20) << "Category"
         << setw(15) << "Limit"
         << setw(10) << "Month"
         << setw(10) << "Year"
         << endl;

    cout << "------------------------------------------------------------------------------------------------------------\n";

    int no = 1;

    while (res->next())
    {
        budgetIDs.push_back(res->getInt("budget_id"));

        cout << left
             << setw(5) << no
             << setw(10) << res->getInt("user_id")
             << setw(30) << res->getString("name")
             << setw(20) << res->getString("category_name")
             << "RM "
             << setw(12) << fixed << setprecision(2)
             << res->getDouble("amount_limit")
             << setw(10) << res->getInt("month")
             << setw(10) << res->getInt("year")
             << endl;

        no++;
    }

    cout << "============================================================================================================\n";

    delete res;
    delete stmt;
    delete con;
}

// ADD BUDGET
void addBudgetAdmin()
{
    try
    {
        system("cls");
        displayBudgetAdmin();

        string input;
        int user_id, category_id, month, year;
        double amount_limit;

        cout << "\n================ ADD BUDGET ================\n";

        cout << "User ID : ";
        getline(cin, input);
        user_id = stoi(input);

        showCategoryMenu();

        cout << "Category ID : ";
        getline(cin, input);
        category_id = stoi(input);

        if (!isValidCategory(category_id))
            throw runtime_error("Invalid Category ID");

        cout << "Budget Limit (RM) : ";
        getline(cin, input);
        amount_limit = stod(input);

        cout << "Month (1-12) : ";
        getline(cin, input);
        month = stoi(input);

        if (month < 1 || month > 12)
            throw runtime_error("Invalid Month");

        cout << "Year : ";
        getline(cin, input);
        year = stoi(input);

        sql::Connection *con = connectDB();

        sql::PreparedStatement *ps =
            con->prepareStatement(
                "INSERT INTO budget(user_id, category_id, amount_limit, month, year) "
                "VALUES (?, ?, ?, ?, ?)");

        ps->setInt(1, user_id);
        ps->setInt(2, category_id);
        ps->setDouble(3, amount_limit);
        ps->setInt(4, month);
        ps->setInt(5, year);

        int rows = ps->executeUpdate();

        cout << (rows > 0 ? "\nBudget added successfully!\n"
                          : "\nFailed to add budget!\n");

        delete ps;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Add error: " << e.what() << endl;
    }
}

// UPDATE BUDGET
void updateBudgetAdmin()
{
    try
    {
        displayBudgetAdmin();

        string input;
        int no, choice;

        cout << "\n================ UPDATE BUDGET ================\n";

        cout << "\nEnter Budget Number To Update : ";
        getline(cin, input);
        stringstream ss(input);

        if (!(ss >> no) || no <= 0 || no > (int)budgetIDs.size())
            throw runtime_error("invalid selection");

        string selectedID = to_string(budgetIDs[no - 1]);

        cout << "\n================  UPDATE MENU ================\n";
        cout << "[1] User ID\n";
        cout << "[2] Category ID\n";
        cout << "[3] Budget Limit\n";
        cout << "[4] Month\n";
        cout << "[5] Year\n";
        cout << "[0] Back\n";
        cout << "================================================\n";

        cout << "Choose : ";
        getline(cin, input);
        choice = stoi(input);

        string field, value;

        switch (choice)
        {
        case 1:
            field = "user_id";
            cout << "New User ID : ";
            break;

        case 2:
            field = "category_id";
            showCategoryMenu();
            cout << "New Category ID : ";
            break;

        case 3:
            field = "amount_limit";
            cout << "New Budget Limit : ";
            break;

        case 4:
            field = "month";
            cout << "New Month : ";
            break;

        case 5:
            field = "year";
            cout << "New Year : ";
            break;

        case 0:
            return;

        default:
            throw runtime_error("Invalid choice");
        }

        getline(cin, value);

        sql::Connection *con = connectDB();
        sql::PreparedStatement *ps =
            con->prepareStatement(
                ("UPDATE budget SET " + field + "=? WHERE budget_id=?").c_str());

        if (field == "user_id" || field == "category_id" ||
            field == "month" || field == "year")
        {
            ps->setInt(1, stoi(value));
        }
        else
        {
            ps->setDouble(1, stod(value));
        }

        ps->setInt(2, stoi(selectedID));

        int rows = ps->executeUpdate();

        cout << (rows > 0 ? "\nBudget updated successfully!\n"
                          : "\nUpdate failed!\n");

        delete ps;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Update error: " << e.what() << endl;
    }
}

// DELETE BUDGET
void deleteBudgetAdmin()
{
    try
    {
        displayBudgetAdmin();

        string input;
        int no;

        cout << "\nEnter Budget Number To Delete : ";
        getline(cin, input);

        stringstream ss(input);
        if (!(ss >> no) || no <= 0 || no > (int)budgetIDs.size())
            throw runtime_error("invalid selection");

        int budgetID = budgetIDs[no - 1];

        string confirm;
        cout << "Confirm Delete? (Y/N) : ";
        getline(cin, confirm);

        if (confirm != "Y" && confirm != "y")
        {
            cout << "Delete cancelled.\n";
            return;
        }

        sql::Connection *con = connectDB();
        sql::PreparedStatement *ps =
            con->prepareStatement("DELETE FROM budget WHERE budget_id=?");

        ps->setInt(1, budgetID);

        int rows = ps->executeUpdate();

        cout << (rows > 0 ? "\nBudget deleted successfully!\n"
                          : "\nDelete failed!\n");

        delete ps;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Delete error: " << e.what() << endl;
    }
}

// MAIN MENU
void manageBudget()
{
    while (true)
    {
        system("cls");
        displayBudgetAdmin();

        cout << "\n================ BUDGET MANAGEMENT ================\n";
        cout << "[1] Add Budget\n";
        cout << "[2] Update Budget\n";
        cout << "[3] Delete Budget\n";
        cout << "[0] Back\n";
        cout << "===================================================\n";

        int choice = getValidatedChoiceAdmin5(0, 3);

        system("cls");

        switch (choice)
        {
        case 1:
            addBudgetAdmin();
            break;

        case 2:
            updateBudgetAdmin();
            break;

        case 3:
            deleteBudgetAdmin();
            break;

        case 0:
            return;
        }

        system("pause");
    }
}