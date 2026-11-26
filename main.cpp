#include <iostream>
#include <iomanip>
#include <string>
#include <limits>
#include <cstdlib>

#include "auth/login.cpp"

#include "modules/user.cpp"
#include "modules/income.cpp"
#include "modules/expense.cpp"
#include "modules/budget.cpp"
#include "modules/report.cpp"

#include "modulesAdmin/userAdmin.cpp"
#include "modulesAdmin/categoryAdmin.cpp"
#include "modulesAdmin/sourcesAdmin.cpp"
#include "modulesAdmin/incomeAdmin.cpp"
#include "modulesAdmin/expenseAdmin.cpp"
#include "modulesAdmin/budgetAdmin.cpp"
#include "modulesAdmin/reportAdmin.cpp"

#include "db_connection.cpp"

#define GREEN 10
#define RED 12
#define YELLOW 14
#define RESET 7

using namespace std;

extern string currentUserID;
extern string currentRoleID;

void setColorUser(int color)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void pauseScreen()
{
    cout << "\nPress ENTER to continue...";
    cin.get();
}

void printHeader(const string &title)
{
    system("cls");
    cout << "\n==================================================\n";
    cout << "              " << title << "\n";
    cout << "==================================================\n";
}

void printMenuLine()
{
    cout << "--------------------------------------------------\n";
}

int getChoice()
{
    int choice;
    cin >> choice;

    if (cin.fail())
    {
        cin.clear();
        return -1;
    }

    return choice;
}

int getValidatedChoice()
{
    string input;
    int choice;

    while (true)
    {
        cout << "Select option: ";
        getline(cin, input);

        if (input.empty())
        {
            cout << "Input cannot be empty. Please try again.\n";
            continue;
        }

        try
        {
            size_t pos;
            choice = stoi(input, &pos);

            if (pos != input.length())
            {
                throw invalid_argument("Invalid");
            }

            return choice;
        }
        catch (...)
        {
            cout << "Invalid input! Please enter a number.\n";
        }
    }
}

// FINANCIAL SUMMARY STRUCT
struct FinancialSummary
{
    double income;
    double expense;
    double balance;
};

// USER SUMMARY
FinancialSummary getFinancialSummary()
{
    FinancialSummary data{0, 0, 0};

    try
    {
        auto con = connectDB();
        auto stmt = con->createStatement();

        string query =
            "SELECT IFNULL(SUM(amount),0) AS total_income "
            "FROM income WHERE user_id='" +
            currentUserID + "'";

        auto incomeRes = stmt->executeQuery(query);
        if (incomeRes->next())
            data.income = incomeRes->getDouble("total_income");

        query =
            "SELECT IFNULL(SUM(amount),0) AS total_expense "
            "FROM expense WHERE user_id='" +
            currentUserID + "'";

        auto expenseRes = stmt->executeQuery(query);
        if (expenseRes->next())
            data.expense = expenseRes->getDouble("total_expense");

        data.balance = data.income - data.expense;

        delete incomeRes;
        delete expenseRes;
        delete stmt;
        delete con;
    }
    catch (...)
    {
        cout << "\nError loading financial summary!\n";
    }

    return data;
}

// ADMIN SUMMARY
FinancialSummary getAdminSummary()
{
    FinancialSummary data{0, 0, 0};

    try
    {
        auto con = connectDB();
        auto stmt = con->createStatement();

        auto incomeRes = stmt->executeQuery(
            "SELECT IFNULL(SUM(amount),0) AS total_income FROM income");

        if (incomeRes->next())
            data.income = incomeRes->getDouble("total_income");

        auto expenseRes = stmt->executeQuery(
            "SELECT IFNULL(SUM(amount),0) AS total_expense FROM expense");

        if (expenseRes->next())
            data.expense = expenseRes->getDouble("total_expense");

        data.balance = data.income - data.expense;

        delete incomeRes;
        delete expenseRes;
        delete stmt;
        delete con;
    }
    catch (...)
    {
        cout << "\nError loading admin summary!\n";
    }

    return data;
}

// ADMIN MENU
void adminMenu()
{
    int choice;

    do
    {
        printHeader("ADMIN DASHBOARD");

        cout << "| 1 | User Management\n";
        cout << "| 2 | Category Management\n";
        cout << "| 3 | Income Source Management\n";
        cout << "| 4 | Income Records Management\n";
        cout << "| 5 | Expense Records Management\n";
        cout << "| 6 | Budget Management\n";
        cout << "| 7 | Reports & Analytics\n";
        cout << "| 8 | Logout\n";
        cout << "| 0 | Exit Program\n";

        printMenuLine();

        choice = getValidatedChoice();

        switch (choice)
        {
        case 1:
            manageUsers();
            break;

        case 2:
            manageCategory();
            break;

        case 3:
            manageSource();
            break;

        case 4:
            manageIncome();
            break;

        case 5:
            manageExpense();
            break;

        case 6:
            manageBudget();
            break;

        case 7:
            adminReportMenu();
            break;

        case 8:
            cout << "Logging out...\n";
            return;

        case 0:
            exit(0);

        default:
            cout << "Invalid choice! Please select 0 - 8.\n";
            pauseScreen();
        }

    } while (true);
}

// USER MENU
void userMenu()
{
    int choice;

    do
    {
        FinancialSummary s = getFinancialSummary();

        printHeader("TRACK & SPEND SYSTEM");

        cout << "Balance : RM ";
        if (s.balance < 0)
        {
            setColorUser(RED);
        }
        else if (s.balance < 100)
        {
            setColorUser(YELLOW);
        }
        else
        {
            setColorUser(GREEN);
        }
        cout << fixed << setprecision(2) << s.balance;
        setColorUser(RESET);
        cout << "\n";

        cout << "Income  : RM ";
        setColorUser(GREEN);
        cout << fixed << setprecision(2) << s.income;
        setColorUser(RESET);
        cout << "\n";

        cout << "Expense : RM ";
        setColorUser(RED);
        cout << fixed << setprecision(2) << s.expense;
        setColorUser(RESET);
        cout << "\n";

        printMenuLine();

        cout << "| 1 | Profile\n";
        cout << "| 2 | Income\n";
        cout << "| 3 | Expense\n";
        cout << "| 4 | Budget\n";
        cout << "| 5 | Report\n";
        cout << "| 6 | Logout\n";
        cout << "| 0 | Exit Program\n";

        printMenuLine();

        choice = getValidatedChoice();

        switch (choice)
        {
        case 1:
            viewUser();
            break;

        case 2:
            viewIncome();
            break;

        case 3:
            viewExpense();
            break;

        case 4:
            viewBudget();
            break;

        case 5:
            reportMenu();
            break;

        case 6:
            return;

        case 0:
            exit(0);

        default:
            cout << "Invalid choice! Please select 0-6.\n";
            pauseScreen();
        }

    } while (true);
}

// MAIN
int main()
{
    try
    {
        while (true)
        {
            if (!loginUser())
                break;

            system("cls");

            if (currentRoleID == "R0001")
                adminMenu();
            else if (currentRoleID == "R0002")
                userMenu();
        }
    }
    catch (exception &e)
    {
        cout << "\nSystem Error: " << e.what() << endl;
    }

    return 0;
}