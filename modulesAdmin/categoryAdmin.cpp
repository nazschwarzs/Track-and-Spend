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

vector<string> categoryIDs;

//  SAFE INPUT
int getValidatedChoiceAdmin1(int minChoice, int maxChoice)
{
    string input;

    while (true)
    {
        cout << "Enter Choice : ";
        getline(cin, input);

        try
        {
            if (input.empty())
                throw runtime_error("Empty");

            size_t pos;
            int choice = stoi(input, &pos);

            if (pos != input.length())
                throw runtime_error("Invalid");

            if (choice < minChoice || choice > maxChoice)
                throw out_of_range("Range");

            return choice;
        }
        catch (...)
        {
            cout << "Invalid input. Try again.\n";
        }
    }
}

// DISPLAY ALL CATEGORIES
void displayCategories()
{
    try
    {
        categoryIDs.clear();

        sql::Connection *con = connectDB();
        sql::PreparedStatement *ps =
            con->prepareStatement(
                "SELECT category_id, category_name FROM category ORDER BY category_id ASC");

        sql::ResultSet *res = ps->executeQuery();

        cout << "\n====================================================\n";
        cout << "                  CATEGORY LIST\n";
        cout << "====================================================\n";

        cout << left
             << setw(10) << "No"
             << setw(30) << "Category Name"
             << endl;

        cout << "----------------------------------------------------\n";

        int no = 1;

        while (res->next())
        {
            categoryIDs.push_back(res->getString("category_id"));

            cout << left
                 << setw(10) << no++
                 << setw(30) << res->getString("category_name")
                 << "\n";
        }
        cout << "====================================================\n";

        delete res;
        delete ps;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Display error: " << e.what() << endl;
    }
}

// ADD CATEGORY
void addCategory()

{
    try
    {
        displayCategories();

        string categoryName;

        cout << "\n================ ADD CATEGORY ================\n";
        cout << "\nEnter Category Name: ";
        getline(cin, categoryName);

        if (categoryName.empty())
            throw runtime_error("Category name empty");

        sql::Connection *con = connectDB();

        sql::PreparedStatement *ps =
            con->prepareStatement(
                "INSERT INTO category(category_name) VALUES(?)");

        ps->setString(1, categoryName);

        int rows = ps->executeUpdate();

        cout << (rows > 0 ? "Category added successfully!\n"
                          : "Insert failed!\n");

        delete ps;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Add error: " << e.what() << endl;
    }
}

// UPDATE CATEGORY
void updateCategory()
{
    try
    {
        displayCategories();

        string input;
        int no;
        cout << "\n================ UPDATE CATEGORY ================\n";
        cout << "\nEnter Category Number: ";
        getline(cin, input);

        stringstream ss(input);
        if (!(ss >> no) || no <= 0 || no > (int)categoryIDs.size())
            throw runtime_error("Invalid category number");

        string categoryID = categoryIDs[no - 1];

        cout << "Enter New Category Name: ";
        string newName;
        getline(cin, newName);

        if (newName.empty())
            throw runtime_error("Empty name");

        sql::Connection *con = connectDB();

        sql::PreparedStatement *ps =
            con->prepareStatement(
                "UPDATE category SET category_name=? WHERE category_id=?");

        ps->setString(1, newName);
        ps->setString(2, categoryID);

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

// DELETE CATEGORY
void deleteCategory()
{
    try
    {
        displayCategories();

        string input;
        int no;

        cout << "\n================ DELETE CATEGORY ================\n";
        cout << "\nEnter Category Number: ";
        getline(cin, input);

        stringstream ss(input);
        if (!(ss >> no) || no <= 0 || no > (int)categoryIDs.size())
            throw runtime_error("Invalid category number");

        string categoryID = categoryIDs[no - 1];

        sql::Connection *con = connectDB();

        sql::PreparedStatement *check =
            con->prepareStatement(
                "SELECT COUNT(*) AS total FROM expense WHERE category_id=?");

        check->setString(1, categoryID);

        sql::ResultSet *res = check->executeQuery();

        int total = 0;
        if (res->next())
            total = res->getInt("total");

        if (total > 0)
        {
            cout << "\n=============================================\n";
            cout << "Category linked with expenses\n";
            cout << "=============================================\n";
            return;
        }

        string confirm;
        cout << "Confirm delete (Y/N): ";
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
            con->prepareStatement(
                "DELETE FROM category WHERE category_id=?");

        del->setString(1, categoryID);

        int rows = del->executeUpdate();

        cout << (rows > 0 ? "Deleted successfully!\n"
                          : "Delete failed!\n");

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

// CATEGORY MANAGEMENT MENU
void manageCategory()
{
    while (true)
    {
        system("cls");
        displayCategories();
        
        cout << "\n=========== CATEGORY MANAGEMENT ===========\n";
        cout << "[1] Add\n";
        cout << "[2] Update\n";
        cout << "[3] Delete\n";
        cout << "[0] Back\n";
        cout << "===============================================\n";

        int choice = getValidatedChoiceAdmin1(0, 3);

        system("cls");

        switch (choice)
        {
        case 1:
            addCategory();
            break;

        case 2:
            updateCategory();
            break;

        case 3:
            deleteCategory();
            break;

        case 0:
            return;

        default:
            cout << "Invalid choice\n";
        }

        system("pause");
    }
}