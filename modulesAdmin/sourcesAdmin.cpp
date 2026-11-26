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

vector<string> sourceIDs;

// SAFE INPUT
int getValidatedChoiceAdmin2(int minChoice, int maxChoice)
{
    string input;

    while (true)
    {
        cout << "Enter Choice : ";
        getline(cin, input);

        try
        {
            if (input.empty())
                throw runtime_error("empty");

            size_t pos;
            int choice = stoi(input, &pos);

            if (pos != input.length())
                throw runtime_error("invalid");

            if (choice < minChoice || choice > maxChoice)
                throw out_of_range("range");

            return choice;
        }
        catch (...)
        {
            cout << "Invalid input. Try again.\n";
        }
    }
}

// DISPLAY ALL INCOME SOURCES
void displaySources()
{
    try
    {
        sourceIDs.clear();

        sql::Connection *con = connectDB();
        sql::PreparedStatement *ps =
            con->prepareStatement(
                "SELECT source_id, source_name FROM source ORDER BY source_id ASC");

        sql::ResultSet *res = ps->executeQuery();

        cout << "\n====================================================\n";
        cout << "               INCOME SOURCE LIST\n";
        cout << "====================================================\n";

        cout << left
             << setw(10) << "No"
             << setw(30) << "Source Name"
             << endl;

        cout << "----------------------------------------------------\n";

        int no = 1;

        while (res->next())
        {
            sourceIDs.push_back(res->getString("source_id"));

            cout << left
                 << setw(10) << no++
                 << setw(30) << res->getString("source_name")
                 << "\n";
        }

        delete res;
        delete ps;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Display error: " << e.what() << endl;
    }
}

// ADD SOURCE
void addSource()
{
    try
    {
        displaySources();

        string sourceName;

        cout << "\n================ ADD INCOME SOURCE ================\n";

        cout << "\nEnter Source Name: ";
        getline(cin, sourceName);

        if (sourceName.empty())
            throw runtime_error("empty name");

        sql::Connection *con = connectDB();

        sql::PreparedStatement *ps =
            con->prepareStatement(
                "INSERT INTO source(source_name) VALUES(?)");

        ps->setString(1, sourceName);

        int rows = ps->executeUpdate();

        cout << (rows > 0 ? "Source added successfully!\n"
                          : "Insert failed!\n");

        delete ps;
        delete con;
    }
    catch (exception &e)
    {
        cout << "Add error: " << e.what() << endl;
    }
}

// UPDATE SOURCE
void updateSource()
{
    try
    {
        displaySources();

        string input;
        int no;

        cout << "\n================ UPDATE INCOME SOURCE ================\n";

        cout << "\nEnter Source Number: ";
        getline(cin, input);

        stringstream ss(input);
        if (!(ss >> no) || no <= 0 || no > (int)sourceIDs.size())
            throw runtime_error("invalid number");

        string sourceID = sourceIDs[no - 1];

        string newName;
        cout << "Enter New Source Name: ";
        getline(cin, newName);

        if (newName.empty())
            throw runtime_error("empty name");

        sql::Connection *con = connectDB();

        sql::PreparedStatement *ps =
            con->prepareStatement(
                "UPDATE source SET source_name=? WHERE source_id=?");

        ps->setString(1, newName);
        ps->setString(2, sourceID);

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

// DELETE SOURCE
void deleteSource()
{
    try
    {
        displaySources();

        string input;
        int no;

        cout << "\n================ DELETE INCOME SOURCE ================\n";

        cout << "\nEnter Source Number: ";
        getline(cin, input);

        stringstream ss(input);
        if (!(ss >> no) || no <= 0 || no > (int)sourceIDs.size())
            throw runtime_error("invalid selection");

        string sourceID = sourceIDs[no - 1];

        sql::Connection *con = connectDB();

        sql::PreparedStatement *check =
            con->prepareStatement(
                "SELECT COUNT(*) AS total FROM income WHERE source_id=?");

        check->setString(1, sourceID);

        sql::ResultSet *res = check->executeQuery();

        int total = 0;
        if (res->next())
            total = res->getInt("total");

        if (total > 0)
        {
            cout << "\n=============================================\n";
            cout << "Source linked with income records\n";
            cout << "Cannot delete this source\n";
            cout << "=============================================\n";

            delete res;
            delete check;
            delete con;
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
                "DELETE FROM source WHERE source_id=?");

        del->setString(1, sourceID);

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

// INCOME SOURCE MANAGEMENT MENU
void manageSource()
{
    int choice;

    do
    {
        system("cls");

        displaySources();

        cout << "\n=========== INCOME SOURCE MANAGEMENT ===========\n";
        cout << "[1] Add Source\n";
        cout << "[2] Update Source\n";
        cout << "[3] Delete Source\n";
        cout << "[0] Back\n";
        cout << "===============================================\n";

        choice = getValidatedChoiceAdmin2(0, 3);

        switch (choice)
        {
        case 1:
            system("cls");
            addSource();
            system("pause");
            break;

        case 2:
            system("cls");
            updateSource();
            system("pause");
            break;

        case 3:
            system("cls");
            deleteSource();
            system("pause");
            break;

        case 0:
            return;
        }

    } while (true);
}