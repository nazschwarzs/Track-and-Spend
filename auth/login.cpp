#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <cctype>
#include <conio.h>
#include "../db_connection.h"
#include <jdbc/cppconn/prepared_statement.h>

using namespace std;

string currentUserID = "";
string currentRoleID = "";

// HIDE PASSWORD
string hiddenPassword()
{
    string password;
    char ch;

    while ((ch = _getch()) != 13)
    {
        if (ch == 8)
        {
            if (!password.empty())
            {
                password.pop_back();
                cout << "\b \b";
            }
        }
        else
        {
            password += ch;
            cout << "*";
        }
    }

    cout << endl;
    return password;
}

void printCentered(const string &text, int width)
{
    int padding = (width - text.length()) / 2;
    if (padding < 0)
        padding = 0;

    cout << string(padding, ' ') << text << "\n";
}

void pressToContinue()
{
    cout << "\n=====================================\n";
    cout << " Press ENTER to continue...";
    cout << "\n=====================================\n";

    cin.get();

    system("cls");
}

int getValidatedChoice1(int minChoice, int maxChoice)
{
    string input;

    while (true)
    {
        cout << "Choice : ";
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
            pressToContinue();
        }
    }
}

// LOGIN
bool login(sql::Connection *con)
{
    try
    {
        string username, password;

        cout << "\n===================================";
        cout << "\n               LOGIN \n";
        cout << "===================================\n";

        cout << "Username : ";
        getline(cin, username);

        cout << "Password : ";
        password = hiddenPassword();

        if (username.empty() || password.empty())
        {
            cout << "Fields cannot be empty!\n";
            return false;
        }

        sql::PreparedStatement *pstmt = con->prepareStatement(
            "SELECT user_id, name, role_id FROM users WHERE username=? AND password=SHA2(?,256)");

        pstmt->setString(1, username);
        pstmt->setString(2, password);

        sql::ResultSet *res = pstmt->executeQuery();

        if (res->next())
        {
            currentUserID = res->getString("user_id");
            currentRoleID = res->getString("role_id");

            cout << "\nLogin Successful!\n";
            cout << "Welcome, " << res->getString("name") << endl;

            if (currentRoleID == "R0001")
                cout << "Role : ADMIN\n";
            else
                cout << "Role : USER\n";

            delete res;
            delete pstmt;

            pressToContinue();
            return true;
        }

        cout << "\nWrong username,password or not register!\n";
        delete res;
        delete pstmt;

        return false;
    }
    catch (sql::SQLException &e)
    {
        cout << "Database Error: " << e.what() << endl;
    }
    return false;
}

// REGISTER
void registerUser(sql::Connection *con)
{
    try
    {
        string name, username, email, password;

        cout << "\n===================================";
        cout << "\n               REGISTER \n";
        cout << "===================================\n";

        cout << "Name     : ";
        getline(cin, name);
        transform(name.begin(), name.end(), name.begin(), ::toupper);

        cout << "Username : ";
        getline(cin, username);
        transform(username.begin(), username.end(), username.begin(), ::tolower);

        cout << "Email    : ";
        getline(cin, email);
        transform(email.begin(), email.end(), email.begin(), ::tolower);

        cout << "Password : ";
        password = hiddenPassword();

        // EMPTY CHECK
        if (name.empty() || username.empty() || email.empty() || password.empty())
        {
            cout << "All fields are required!\n";
            return;
        }

        // VALIDATION
        if (username.find(' ') != string::npos)
        {
            cout << "Username cannot contain spaces!\n";
            return;
        }

        if (email.find("@gmail.com") == string::npos)
        {
            cout << "Only Gmail is allowed!\n";
            return;
        }

        bool hasUpper = false, hasLower = false, hasDigit = false;

        for (char ch : password)
        {
            if (isupper(ch))
                hasUpper = true;
            if (islower(ch))
                hasLower = true;
            if (isdigit(ch))
                hasDigit = true;
        }

        if (password.length() < 8 || !hasUpper || !hasLower || !hasDigit)
        {
            cout << "Password must contain:\n";
            cout << "- At least 8 characters\n";
            cout << "- 1 uppercase letter\n";
            cout << "- 1 lowercase letter\n";
            cout << "- 1 number\n";
            return;
        }

        // CHECK IF USER ALREADY EXISTS
        sql::PreparedStatement *checkStmt = con->prepareStatement(
            "SELECT user_id FROM users WHERE username=? OR email=?");

        checkStmt->setString(1, username);
        checkStmt->setString(2, email);

        sql::ResultSet *checkRes = checkStmt->executeQuery();

        if (checkRes->next())
        {
            cout << "\nUser already registered!\n";
            cout << "Please login instead.\n";

            delete checkRes;
            delete checkStmt;
            return;
        }

        delete checkRes;
        delete checkStmt;

        // INSERT NEW USER
        try
        {
            sql::PreparedStatement *pstmt = con->prepareStatement(
                "INSERT INTO users(name, username, email, password, role_id) "
                "VALUES (?, ?, ?, SHA2(?,256), 'R0002')");

            pstmt->setString(1, name);
            pstmt->setString(2, username);
            pstmt->setString(3, email);
            pstmt->setString(4, password);
            pstmt->execute();

            cout << "\nRegister Successful!\n";
            cout << "Please login.\n";

            delete pstmt;
        }
        catch (sql::SQLException &e)
        {
            if (e.getErrorCode() == 1062)
            {
                cout << "\nDuplicate entry detected! Username or email already exists.\n";
            }
            else
            {
                cout << "Database Error: " << e.what() << endl;
            }
        }
    }
    catch (exception &e)
    {
        cout << "System Error: " << e.what() << endl;
    }
}

// AUTH MENU
bool loginUser()
{
    try
    {
        system("cls");
        int choice;

        do
        {
            cout << "\n================================\n";
            cout << "      TRACK & SPEND SYSTEM      \n";
            cout << "================================\n";
            cout << " No | Option                   \n";
            cout << "================================\n";
            cout << " 1  | Login                    \n";
            cout << " 2  | Register                 \n";
            cout << " 3  | Exit                     \n";
            cout << "================================\n\n";

            choice = getValidatedChoice1(1, 3);

            sql::Connection *con = connectDB();

            if (!con)
                return false;

            switch (choice)
            {
            case 1:
                system("cls");

                if (login(con))
                {
                    delete con;
                    return true;
                }
                pressToContinue();
                break;

            case 2:
                system("cls");
                registerUser(con);
                pressToContinue();
                break;

            case 3:
                cout << "\n==============================\n";
                cout << "         EXITING SYSTEM       \n";
                cout << "==============================\n";
                delete con;
                return false;
            }

            delete con;

        } while (true);
    }
    catch (sql::SQLException &e)
    {
        cout << "Database Error: " << e.what() << endl;
    }
    catch (exception &e)
    {
        cout << "System Error: " << e.what() << endl;
    }

    return false;
}