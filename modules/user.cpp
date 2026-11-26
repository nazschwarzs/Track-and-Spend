#include <iostream>
#include <iomanip>
#include <limits>
#include <string>
#include <cctype>

#include "../db_connection.h"
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <jdbc/cppconn/resultset.h>

using namespace std;

extern string currentUserID;

//INPUT VALIDATION
int getValidatedChoice2(int minChoice, int maxChoice)
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

//PROFILE
void myProfile()
{
    sql::Connection *con = connectDB();
    if (!con) return;

    sql::PreparedStatement *pstmt =
        con->prepareStatement("SELECT name, username, email, created_at FROM users WHERE user_id = ?");

    int userID = stoi(currentUserID);
    pstmt->setInt(1, userID);

    sql::ResultSet *res = pstmt->executeQuery();

    cout << "\n====================================================\n";
    cout << "                    MY PROFILE\n";
    cout << "====================================================\n";

    if (res->next())
    {
        cout << left << setw(20) << "Name"       << ": " << res->getString("name") << "\n";
        cout << left << setw(20) << "Username"   << ": " << res->getString("username") << "\n";
        cout << left << setw(20) << "Email"      << ": " << res->getString("email") << "\n";
        cout << left << setw(20) << "Created At" << ": " << res->getString("created_at") << "\n";
    }
    else
    {
        cout << "User not found!\n";
    }

    cout << "====================================================\n";

    delete res;
    delete pstmt;
    delete con;
}

//UPDATE PROFILE
void updateProfile()
{
    system("cls");

    sql::Connection *con = connectDB();
    if (!con) return;

    sql::PreparedStatement *pstmt = nullptr;
    int userID = stoi(currentUserID);

    cout << "\n====================================================\n";
    cout << "                 UPDATE PROFILE\n";
    cout << "====================================================\n";
    cout << "1. Name\n";
    cout << "2. Username\n";
    cout << "3. Email\n";
    cout << "4. Password\n";
    cout << "0. Cancel\n";
    cout << "====================================================\n";

    int choice = getValidatedChoice2(0, 4);

    string input;

    switch (choice)
    {
    case 1:
        cout << "Enter New Name: ";
        getline(cin, input);

        if (!input.empty())
        {
            pstmt = con->prepareStatement("UPDATE users SET name = ? WHERE user_id = ?");
            pstmt->setString(1, input);
            pstmt->setInt(2, userID);
        }
        break;

    case 2:
        cout << "Enter New Username: ";
        getline(cin, input);

        if (!input.empty())
        {
            pstmt = con->prepareStatement("UPDATE users SET username = ? WHERE user_id = ?");
            pstmt->setString(1, input);
            pstmt->setInt(2, userID);
        }
        break;

    case 3:
        cout << "Enter New Email: ";
        getline(cin, input);

        if (!input.empty())
        {
            pstmt = con->prepareStatement("UPDATE users SET email = ? WHERE user_id = ?");
            pstmt->setString(1, input);
            pstmt->setInt(2, userID);
        }
        break;

    case 4:
        cout << "Enter New Password: ";
        getline(cin, input);

        if (!input.empty())
        {
            pstmt = con->prepareStatement("UPDATE users SET password = ? WHERE user_id = ?");
            pstmt->setString(1, input);
            pstmt->setInt(2, userID);
        }
        break;

    case 0:
        cout << "Update cancelled.\n";
        delete con;
        return;
    }

    if (pstmt)
    {
        int rows = pstmt->executeUpdate();

        cout << (rows > 0 ? "\nProfile updated successfully!\n"
                          : "\nUpdate failed.\n");

        delete pstmt;
    }

    delete con;
}

//DELETE ACCOUNT
void deleteAccount()
{
    system("cls");

    sql::Connection *con = connectDB();
    if (!con) return;

    cout << "\n====================================================\n";
    cout << "                  DELETE ACCOUNT\n";
    cout << "====================================================\n";
    cout << "WARNING: This action cannot be undone!\n";
    cout << "Are you sure? (Y/N): ";

    string input;
    getline(cin, input);

    char confirm = input.empty() ? 'N' : toupper(input[0]);

    if (confirm != 'Y')
    {
        cout << "Delete cancelled.\n";
        delete con;
        return;
    }

    sql::PreparedStatement *pstmt =
        con->prepareStatement("DELETE FROM users WHERE user_id = ?");

    int userID = stoi(currentUserID);
    pstmt->setInt(1, userID);

    int rows = pstmt->executeUpdate();

    if (rows > 0)
    {
        cout << "\nAccount deleted successfully!\n";
        currentUserID = "";
    }
    else
    {
        cout << "\nFailed to delete account.\n";
    }

    delete pstmt;
    delete con;
}

//MENU
void viewUser()
{
    system("cls");

    int choice;

    do
    {
        cout << "\n================================================\n";
        cout << "                  USER MENU\n";
        cout << "================================================\n";
        cout << "1. View Profile\n";
        cout << "2. Update Profile\n";
        cout << "3. Delete Account\n";
        cout << "0. Back\n";
        cout << "================================================\n";

        choice = getValidatedChoice2(0, 3);

        switch (choice)
        {
        case 1:
            system("cls");
            myProfile();
            break;

        case 2:
            system("cls");
            updateProfile();
            break;

        case 3:
            system("cls");
            deleteAccount();
            break;

        case 0:
            cout << "Returning...\n";
            return;
        }

    } while (true);
}