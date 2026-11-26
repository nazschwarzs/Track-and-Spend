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

vector<int> expenseIDs;

int getValidatedChoiceAdmin4(int minChoice, int maxChoice)
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

void showCategorryMenu()
{
    cout << "\n============= EXPENSE CATEGORY =============\n";
    cout << "  [1] Utility\n";
    cout << "  [2] Transport\n";
    cout << "  [3] Food\n";
    cout << "  [4] Entertainment\n";
    cout << "  [5] Education\n";
    cout << "  [6] Others\n";
    cout << "=========================================\n";
}

// DISPLAY ALL EXPENSE
void displayExpenseAdmin()
{
    expenseIDs.clear();

    sql::Connection *con = connectDB();
    sql::Statement *stmt = con->createStatement();

    string query =
        "SELECT e.expense_id, e.user_id, u.name AS name, "
        "c.category_name, e.amount, e.description, e.expense_date "
        "FROM expense e "
        "JOIN users u ON e.user_id = u.user_id "
        "JOIN category c ON e.category_id = c.category_id "
        "ORDER BY e.expense_date DESC";

    sql::ResultSet *res = stmt->executeQuery(query);

    cout << "\n=======================================================================================================================\n";
    cout << "                                                       EXPENSE LIST\n";
    cout << "=======================================================================================================================\n";

    cout << left
         << setw(5) << "No"
         << setw(10) << "User ID"
         << setw(30) << "Name"
         << setw(20) << "Category"
         << setw(15) << "Amount"
         << setw(25) << "Description"
         << setw(15) << "Date"
         << endl;

    cout << "-----------------------------------------------------------------------------------------------------------------------\n";

    int no = 1;

    while (res->next())
    {
        expenseIDs.push_back(res->getInt("expense_id"));

        cout << left
             << setw(5) << no
             << setw(10) << res->getInt("user_id")
             << setw(30) << res->getString("name")
             << setw(20) << res->getString("category_name")
             << setw(15) << fixed << setprecision(2) << res->getDouble("amount")
             << setw(25) << res->getString("description")
             << setw(15) << res->getString("expense_date")
             << endl;

        no++;
    }

    cout << "=======================================================================================================================\n";

    delete res;
    delete stmt;
    delete con;
}

// ADD EXPENSE
void addExpenseAdmin()
{
    try
    {
        displayExpenseAdmin();

        string input;
        string date, description;
        int user_id, category_id;
        double amount;

        cout << "\n================ ADD EXPENSE ================\n";

        cout << "User ID : ";
        getline(cin, input);
        user_id = stoi(input);

        showCategorryMenu();

        cout << "Category ID : ";
        getline(cin, input);
        category_id = stoi(input);

        cout << "Amount (RM) : ";
        getline(cin, input);
        amount = stod(input);

        cout << "Description : ";
        getline(cin, description);

        cout << "Date (YYYY-MM-DD) : ";
        getline(cin, date);

        if (date.empty())
            throw runtime_error("Date cannot be empty");

        sql::Connection *con = connectDB();

        sql::PreparedStatement *ps =
            con->prepareStatement(
                "INSERT INTO expense(user_id, category_id, amount, description, expense_date) "
                "VALUES (?, ?, ?, ?, ?)");

        ps->setInt(1, user_id);
        ps->setInt(2, category_id);
        ps->setDouble(3, amount);
        ps->setString(4, description);
        ps->setString(5, date);

        int rows = ps->executeUpdate();

        cout << (rows > 0 ? "Expense added successfully!\n" : "Failed to add expense!\n");

        delete ps;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Error: " << e.what() << endl;
    }
}

// UPDATE EXPENSE
void updateExpenseAdmin()
{
    displayExpenseAdmin();

    int no, choice;
    string input;

    cout << "\n================ UPDATE EXPENSE ================\n";

    cout << "\nEnter Expense Number To Update: ";
    cin >> no;

    if (cin.fail() || no <= 0 || no > expenseIDs.size())
    {
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "Invalid expense number!\n";
        return;
    }

    string selectedID = to_string(expenseIDs[no - 1]);

    cout << "\n================ UPDATE MENU ================\n";
    cout << "[1] Update User ID\n";
    cout << "[2] Update Category ID\n";
    cout << "[3] Update Amount\n";
    cout << "[4] Update Description\n";
    cout << "[5] Update Date\n";
    cout << "[0] Back\n";
    cout << "================================================\n";

    cout << "Choose: ";
    cin >> choice;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    string field;

    switch (choice)
    {
    case 1:
        field = "user_id";
        cout << "Enter New User ID: ";
        break;

    case 2:

        showCategorryMenu();
        
        field = "category_id";
        cout << "Enter New Category ID: ";
        break;

    case 3:
        field = "amount";
        cout << "Enter New Amount: ";
        break;

    case 4:
        field = "description";
        cout << "Enter New Description: ";
        break;

    case 5:
        field = "expense_date";
        cout << "Enter New Date (YYYY-MM-DD): ";
        break;

    case 0:
        return;

    default:
        cout << "Invalid choice!\n";
        return;
    }

    getline(cin, input);

    sql::Connection *con = connectDB();
    sql::Statement *stmt = con->createStatement();

    string query;

    if (field == "amount" || field == "user_id" || field == "category_id")
    {
        query = "UPDATE expense SET " + field + "=" + input +
                " WHERE expense_id=" + selectedID;
    }
    else
    {
        query = "UPDATE expense SET " + field + "='" + input +
                "' WHERE expense_id=" + selectedID;
    }

    int rows = stmt->executeUpdate(query);

    if (rows > 0)
        cout << "Expense updated successfully!\n";
    else
        cout << "Update failed!\n";

    delete stmt;
    delete con;
}

// DELETE EXPENSE
void deleteExpenseAdmin()
{
    try
    {
        displayExpenseAdmin();

        string input;
        int no;

        cout << "\n================ DELETE EXPENSE ================\n";

        cout << "\nEnter Expense Number: ";
        getline(cin, input);

        stringstream ss(input);
        if (!(ss >> no) || no <= 0 || no > (int)expenseIDs.size())
            throw runtime_error("invalid selection");

        int expenseID = expenseIDs[no - 1];

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
            con->prepareStatement("DELETE FROM expense WHERE expense_id=?");

        ps->setInt(1, expenseID);

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

// EXPENSE MENU
void manageExpense()
{
    while (true)
    {
        system("cls");
        displayExpenseAdmin();

        cout << "\n================ EXPENSE MANAGEMENT ================\n";
        cout << "[1] Add Expense\n";
        cout << "[2] Update Expense\n";
        cout << "[3] Delete Expense\n";
        cout << "[0] Back\n";
        cout << "====================================================\n";

        int choice = getValidatedChoiceAdmin4(0, 3);

        system("cls");

        switch (choice)
        {
        case 1:
            addExpenseAdmin();
            break;

        case 2:
            updateExpenseAdmin();
            break;

        case 3:
            deleteExpenseAdmin();
            break;

        case 0:
            return;
        }

        system("pause");
    }
}