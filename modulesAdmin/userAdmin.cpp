#include <windows.h>
#include <iostream>
#include <iomanip>
#include <limits>
#include <vector>
#include <string>
#include <sstream>

#undef max
#undef min

#include "../db_connection.h"
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>

using namespace std;

vector<string> userIDs;

// SAFE INPUT
int getValidatedChoiceAdmin(int minChoice, int maxChoice)
{
    string input;
    int choice;

    while (true)
    {
        try
        {
            cout << "Enter Choice : ";
            getline(cin, input);

            stringstream ss(input);
            ss >> choice;

            if (ss.fail() || !ss.eof())
                throw runtime_error("Invalid number");

            if (choice < minChoice || choice > maxChoice)
                throw out_of_range("Out of range");

            return choice;
        }
        catch (...)
        {
            cout << "Invalid input. Try again.\n";
        }
    }
}

// DISPLAY USERS
void displayUsers()
{
    try
    {
        userIDs.clear();

        sql::Connection *con = connectDB();
        sql::Statement *stmt = con->createStatement();

        sql::ResultSet *res = stmt->executeQuery(
            "SELECT user_id, name, username, email, created_at FROM users ORDER BY user_id ASC");

        cout << "\n====================================================================================================\n";
        cout << "                                        USER LIST\n";
        cout << "====================================================================================================\n";

        cout << left
             << setw(5) << "No"
             << setw(25) << "Name"
             << setw(25) << "Username"
             << setw(30) << "Email"
             << setw(25) << "Created At"
             << endl;

        cout << "----------------------------------------------------------------------------------------------------\n";

        int no = 1;

        while (res->next())
        {
            userIDs.push_back(res->getString("user_id"));

            cout << left
                 << setw(5) << no++
                 << setw(25) << res->getString("name")
                 << setw(25) << res->getString("username")
                 << setw(30) << res->getString("email")
                 << setw(25) << res->getString("created_at")
                 << "\n";
        }
        cout << "----------------------------------------------------------------------------------------------------\n";

        delete res;
        delete stmt;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Display error: " << e.what() << endl;
    }
}

// ADD USER
void addUser()

{

    try
    {
        displayUsers();

        string name, username, email, password;

        cout << "\n================ ADD USER ================\n";

        cout << "\nName     : ";
        getline(cin, name);
        cout << "Username : ";
        getline(cin, username);
        cout << "Email    : ";
        getline(cin, email);
        cout << "Password : ";
        getline(cin, password);

        if (name.empty() || username.empty() || email.empty() || password.empty())
            throw runtime_error("Empty fields not allowed");

        sql::Connection *con = connectDB();

        sql::PreparedStatement *ps =
            con->prepareStatement(
                "INSERT INTO users(name, username, email, password, role_id) VALUES(?,?,?,?,?)");

        ps->setString(1, name);
        ps->setString(2, username);
        ps->setString(3, email);
        ps->setString(4, password);
        ps->setString(5, "R0002");

        int rows = ps->executeUpdate();

        cout << (rows > 0 ? "User added successfully!\n" : "Insert failed!\n");

        delete ps;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Add error: " << e.what() << endl;
    }
}

// UPDATE USER
void updateUser()
{
    try
    {
        displayUsers();

        string input;
        int no;

        cout << "\n================ UPDATE USER ================\n";

        cout << "\nEnter User Number: ";
        getline(cin, input);

        stringstream ss(input);
        if (!(ss >> no) || no <= 0 || no > (int)userIDs.size())
            throw runtime_error("Invalid user number");

        string userID = userIDs[no - 1];

        cout << "\n================ UPDATE MENU ================\n";
        cout << "[1] Name\n";
        cout << "[2] Username\n";
        cout << "[3] Password\n";
        cout << "[0] Back\n";

        int choice = getValidatedChoiceAdmin(0, 3);

        string field;

        switch (choice)
        {
        case 1:
            field = "name";
            break;

        case 2:
            field = "username";
            break;

        case 3:
            field = "password";
            break;

        case 0:
            return;

        default:
            throw runtime_error("Invalid option");
        }

        string newValue;
        cout << "Enter new value: ";
        getline(cin, newValue);

        if (newValue.empty())
            throw runtime_error("Value cannot be empty");

        sql::Connection *con = connectDB();

        sql::PreparedStatement *ps =
            con->prepareStatement(
                ("UPDATE users SET " + field + "=? WHERE user_id=?").c_str());

        ps->setString(1, newValue);
        ps->setString(2, userID);

        int rows = ps->executeUpdate();

        cout << (rows > 0 ? "User updated successfully!\n" : "Update failed!\n");

        delete ps;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Update error: " << e.what() << endl;
    }
}

// DELETE USER
void deleteUser()
{
    try
    {
        displayUsers();

        string input;
        int no;

        cout << "\n================ DELETE USER ================\n";

        cout << "\nEnter User Number: ";
        getline(cin, input);

        stringstream ss(input);
        if (!(ss >> no) || no <= 0 || no > (int)userIDs.size())
            throw runtime_error("Invalid selection");

        string userID = userIDs[no - 1];

        sql::Connection *con = connectDB();

        sql::PreparedStatement *check =
            con->prepareStatement("SELECT role_id FROM users WHERE user_id=?");

        check->setString(1, userID);
        sql::ResultSet *res = check->executeQuery();

        if (!res->next())
            throw runtime_error("User not found");

        if (res->getString("role_id") == "R0001")
            throw runtime_error("Cannot delete admin");

        string confirm;
        cout << "Confirm (Y/N): ";
        getline(cin, confirm);

        if (confirm != "Y" && confirm != "y")
        {
            cout << "Cancelled.\n";
            delete res;
            delete check;
            delete con;
            return;
        }

        sql::PreparedStatement *del =
            con->prepareStatement("DELETE FROM users WHERE user_id=?");

        del->setString(1, userID);

        int rows = del->executeUpdate();

        cout << (rows > 0 ? "Deleted successfully!\n" : "Delete failed!\n");

        delete res;
        delete check;
        delete del;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Delete error: " << e.what() << endl;
    }
}

// MENU
void manageUsers()
{
    while (true)
    {
        system("cls");
        displayUsers();
        cout << "\n=========== USER MANAGEMENT ===========\n";
        cout << "[1] Add User\n";
        cout << "[2] Update User\n";
        cout << "[3] Delete User\n";
        cout << "[0] Back\n";
        cout << "===============================================\n";

        int choice = getValidatedChoiceAdmin(0, 3);

        system("cls");

        switch (choice)
        {
        case 1:
            addUser();
            break;

        case 2:
            updateUser();
            break;

        case 3:
            deleteUser();
            break;

        case 0:
            return;

        default:
            cout << "Invalid choice\n";
        }

        system("pause");
    }
}