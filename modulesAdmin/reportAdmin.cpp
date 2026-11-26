#include <iostream>
#include <iomanip>
#include <vector>
#include <sstream>
#define NOMINMAX
#include <windows.h>
#include <iomanip>
#include <string>
#include <limits>
#include <algorithm>

#include "../db_connection.h"
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/prepared_statement.h>

using namespace std;

void pauseAdmin()
{
    cout << "\n==================================================\n";
    cout << "Press ENTER to continue...";
    cout << "\n==================================================\n";

    string dummy;
    getline(cin, dummy);
}

int getValidatedChoiceAdmin6(int minChoice, int maxChoice)
{
    string input;

    while (true)
    {
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
            {
                cout << "Invalid format!\n";
                continue;
            }

            if (choice < minChoice || choice > maxChoice)
            {
                cout << "Range must be " << minChoice << " - " << maxChoice << "\n";
                continue;
            }

            return choice;
        }
        catch (...)
        {
            cout << "Numbers only.\n";
        }
    }
}

void setColorAdmin(int c)
{
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), c);
}

string formatMoneyAdmin(double v)
{
    stringstream ss;
    ss << fixed << setprecision(2) << v;
    return ss.str();
}

string getMonth(int m)
{
    string month[] = {"", "Jan", "Feb", "Mar", "Apr", "May", "Jun",
                      "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    return (m >= 1 && m <= 12) ? month[m] : "NA";
}

sql::ResultSet *run(sql::Statement *stmt, string q)
{
    try
    {
        return stmt->executeQuery(q);
    }
    catch (exception &e)
    {
        cout << "SQL ERROR: " << e.what() << "\n";
        return nullptr;
    }
}

// CATEGORY USAGE REPORT
void categoryUsageReport()
{
    sql::Connection *con = connectDB();
    if (!con)
        return;

    try
    {
        sql::Statement *stmt = con->createStatement();
        if (!stmt)
        {
            cout << "Failed to create statement\n";
            delete con;
            return;
        }

        string query =
            "SELECT c.category_name, SUM(e.amount) AS total "
            "FROM expense e "
            "JOIN category c ON e.category_id = c.category_id "
            "GROUP BY c.category_name "
            "ORDER BY total DESC";

        sql::ResultSet *res = run(stmt, query);

        cout << "\n=======================================================================================================================\n";
        cout << "                                                    CATEGORY USAGE REPORT\n";
        cout << "=======================================================================================================================\n";

        cout << left
             << setw(6) << "No"
             << setw(25) << "Category"
             << setw(18) << "Amount (RM)"
             << setw(12) << "Usage"
             << setw(15) << "Rank"
             << "Status\n";

        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        vector<string> categories;
        vector<double> amounts;

        double grandTotal = 0;

        if (res)
        {
            while (res->next())
            {
                categories.push_back(res->getString("category_name"));
                amounts.push_back(res->getDouble("total"));

                grandTotal += res->getDouble("total");
            }
            delete res;
        }
        else
        {
            cout << "No data retrieved\n";
        }

        for (int i = 0; i < amounts.size(); i++)
        {
            double percentage = (grandTotal > 0)
                                    ? (amounts[i] / grandTotal) * 100
                                    : 0;

            string status;
            int statusColor = 7;

            if (i == 0)
            {
                status = "TOP";
                statusColor = 10; // Green
            }
            else if (percentage >= 20)
            {
                status = "HIGH";
                statusColor = 12; // Red
            }
            else if (percentage >= 10)
            {
                status = "MODERATE";
                statusColor = 14; // Yellow
            }
            else
            {
                status = "LOW";
                statusColor = 11; // Cyan
            }

            cout << left
                 << setw(6) << (i + 1)
                 << setw(25) << categories[i]
                 << setw(18) << formatMoneyAdmin(amounts[i])
                 << setw(11) << (to_string((int)percentage) + "%")
                 << ("#" + to_string(i + 1))
                 << setw(10 - to_string(i + 1).length()) << "";

            setColorAdmin(statusColor);
            cout << status;
            setColorAdmin(7);
            cout << "\n";
        }

        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        cout << "SUMMARY\n";
        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        cout << "Total Categories : " << categories.size() << "\n";
        cout << "Total Spending   : RM " << formatMoneyAdmin(grandTotal) << "\n";

        if (!categories.empty())
        {
            double topPercent = (grandTotal > 0)
                                    ? (amounts[0] / grandTotal) * 100
                                    : 0;

            cout << "Top Category     : " << categories[0]
                 << " (" << (int)topPercent << "% of total spending)\n";
        }

        cout << "=======================================================================================================================\n";

        delete stmt;
    }
    catch (exception &e)
    {
        cout << "Error in categoryUsageReport: " << e.what() << "\n";
    }

    delete con;
}

// FINANCIAL TREND REPORT
void financialTrendReport(int year)
{
    sql::Connection *con = connectDB();
    if (!con)
        return;

    try
    {
        sql::PreparedStatement *pstmt = nullptr;
        sql::ResultSet *res = nullptr;

        vector<double> income(13, 0);
        vector<double> expense(13, 0);

        string iq =
            "SELECT MONTH(date_received) AS m, SUM(amount) AS total "
            "FROM income "
            "WHERE YEAR(date_received)=? "
            "GROUP BY MONTH(date_received)";

        pstmt = con->prepareStatement(iq);
        if (pstmt)
        {
            pstmt->setInt(1, year);
            res = pstmt->executeQuery();

            if (res)
            {
                while (res->next())
                {
                    int m = res->getInt("m");
                    if (m >= 1 && m <= 12)
                        income[m] = res->getDouble("total");
                }
                delete res;
            }
            delete pstmt;
        }

        string eq =
            "SELECT MONTH(expense_date) AS m, SUM(amount) AS total "
            "FROM expense "
            "WHERE YEAR(expense_date)=? "
            "GROUP BY MONTH(expense_date)";

        pstmt = con->prepareStatement(eq);
        if (pstmt)
        {
            pstmt->setInt(1, year);
            res = pstmt->executeQuery();

            if (res)
            {
                while (res->next())
                {
                    int m = res->getInt("m");
                    if (m >= 1 && m <= 12)
                        expense[m] = res->getDouble("total");
                }
                delete res;
            }
            delete pstmt;
        }

        cout << "\n=======================================================================================================================\n";
        cout << "                                                       FINANCIAL TREND REPORT\n";
        cout << "=======================================================================================================================\n";

        cout << "YEAR: " << year << "\n\n";

        cout << left
             << setw(8) << "Month"
             << setw(18) << "Income (RM)"
             << setw(18) << "Expense (RM)"
             << setw(18) << "Balance (RM)"
             << "\n";

        cout << "=======================================================================================================================\n";

        double totalIncome = 0, totalExpense = 0;

        for (int m = 1; m <= 12; m++)
        {
            double balance = income[m] - expense[m];

            totalIncome += income[m];
            totalExpense += expense[m];

            cout << left
                 << setw(8) << getMonth(m)
                 << setw(18) << formatMoneyAdmin(income[m])
                 << setw(18) << formatMoneyAdmin(expense[m])
                 << setw(18) << formatMoneyAdmin(balance)
                 << "\n";
        }

        cout << "=======================================================================================================================\n";

        cout << left
             << setw(8) << "TOTAL"
             << setw(18) << formatMoneyAdmin(totalIncome)
             << setw(18) << formatMoneyAdmin(totalExpense)
             << setw(18) << formatMoneyAdmin(totalIncome - totalExpense)
             << "\n";

        cout << "=======================================================================================================================\n";

        if (totalIncome > totalExpense)
        {
            setColorAdmin(10);
            cout << "STATUS: PROFITABLE (Income > Expense)\n";
        }
        else
        {
            setColorAdmin(12);
            cout << "STATUS: LOSS WARNING (Expense > Income)\n";
        }

        setColorAdmin(7);

        cout << "=======================================================================================================================\n";
    }
    catch (exception &e)
    {
        cout << "Error in financialTrendReport: " << e.what() << "\n";
    }

    delete con;
}

// EXPENSE BAR CHART REPORT
void expenseBarChart(int year)
{
    sql::Connection *con = connectDB();
    if (!con)
        return;

    try
    {
        double expense[13] = {0};

        string q =
            "SELECT MONTH(expense_date) AS m, IFNULL(SUM(amount),0) AS total "
            "FROM expense "
            "WHERE YEAR(expense_date)=? "
            "GROUP BY MONTH(expense_date)";

        sql::PreparedStatement *pstmt = con->prepareStatement(q);
        if (!pstmt)
        {
            cout << "Failed to prepare statement\n";
            delete con;
            return;
        }

        pstmt->setInt(1, year);
        sql::ResultSet *res = pstmt->executeQuery();

        if (res)
        {
            while (res->next())
            {
                int m = res->getInt("m");
                if (m >= 1 && m <= 12)
                    expense[m] = res->getDouble("total");
            }
            delete res;
        }

        delete pstmt;

        double total = 0;
        double maxVal = 0;

        for (int i = 1; i <= 12; i++)
        {
            total += expense[i];
            if (expense[i] > maxVal)
                maxVal = expense[i];
        }

        cout << "\n=======================================================================================================================\n";
        cout << "                                           EXPENSE BAR CHART REPORT\n";
        cout << "=======================================================================================================================\n";

        cout << "Year : " << year << "\n";

        cout << "\n================================================ EXPENSE OVERVIEW ====================================================\n";
        cout << "Total Expense    : RM " << formatMoneyAdmin(total) << "\n";
        cout << "Average Monthly  : RM " << formatMoneyAdmin(total / 12) << "\n";
        cout << "Highest Expense  : RM " << formatMoneyAdmin(maxVal) << "\n";
        cout << "=======================================================================================================================\n";

        cout << "\nMONTHLY EXPENSE DISTRIBUTION\n";
        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        cout << left
             << setw(8) << "Month"
             << setw(18) << "Expense (RM)"
             << setw(18) << "Usage"
             << "Bar Chart\n";

        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        string highestMonth = "-";
        string lowestMonth = "-";

        double highestExpense = -1;
        double lowestExpense = -1;

        for (int i = 1; i <= 12; i++)
        {
            double ratio = (maxVal > 0)
                               ? (expense[i] / maxVal)
                               : 0;

            int bars = (int)(ratio * 10);
            bars = max(0, min(10, bars));

            string bar = string(bars, '#') +
                         string(10 - bars, '.');

            double percent = ratio * 100;

            if (expense[i] > highestExpense)
            {
                highestExpense = expense[i];
                highestMonth = getMonth(i);
            }

            if ((lowestExpense == -1 || expense[i] < lowestExpense))
            {
                lowestExpense = expense[i];
                lowestMonth = getMonth(i);
            }

            cout << left
                 << setw(8) << getMonth(i)
                 << setw(18) << formatMoneyAdmin(expense[i])
                 << setw(17) << (to_string((int)percent) + "%")
                 << "[" << bar << "]\n";
        }

        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        cout << "\nINSIGHT SUMMARY\n";
        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        cout << "Highest Spending Month : "
             << highestMonth
             << " (RM "
             << formatMoneyAdmin(highestExpense)
             << ")\n";

        cout << "Lowest Spending Month  : "
             << lowestMonth
             << " (RM "
             << formatMoneyAdmin(lowestExpense)
             << ")\n";

        double avg = total / 12;

        int spikeMonths = 0;

        for (int i = 1; i <= 12; i++)
        {
            if (expense[i] > avg * 1.5)
                spikeMonths++;
        }

        if (total == 0)
        {
            setColorAdmin(14);
            cout << "Status                  : No expense records found.\n";
            setColorAdmin(7);
        }
        else if (spikeMonths >= 3)
        {
            setColorAdmin(12);
            cout << "Status                  : HIGH VOLATILITY DETECTED\n";
            cout << "Recommendation          : Review spending spikes carefully.\n";
            setColorAdmin(7);
        }
        else if (spikeMonths >= 1)
        {
            setColorAdmin(14);
            cout << "Status                  : MODERATE SPENDING VARIATION\n";
            cout << "Recommendation          : Monitor high-expense months.\n";
            setColorAdmin(7);
        }
        else
        {
            setColorAdmin(10);
            cout << "Status                  : STABLE EXPENSE TREND\n";
            cout << "Recommendation          : Spending pattern is consistent.\n";
            setColorAdmin(7);
        }

        cout << "=======================================================================================================================\n";
    }
    catch (exception &e)
    {
        cout << "Error in expenseBarChart: " << e.what() << "\n";
    }

    delete con;
}

// EXPENSE CHANGE ANALYSIS REPORT (FIX: Parameterized queries)
void expenseMonthlyTrendReport(int year)
{
    sql::Connection *con = connectDB();
    if (!con)
        return;

    try
    {
        double expense[13] = {0};
        int zeroStreakStart = -1;
        int zeroStreakLength = 0;
        int maxZeroStreak = 0;
        int maxZeroStreakStart = -1;

        string q =
            "SELECT MONTH(expense_date) m, IFNULL(SUM(amount),0) total "
            "FROM expense WHERE YEAR(expense_date)=? "
            "GROUP BY MONTH(expense_date)";

        sql::PreparedStatement *pstmt = con->prepareStatement(q);
        if (!pstmt)
        {
            cout << "Failed to prepare statement\n";
            delete con;
            return;
        }

        pstmt->setInt(1, year);
        sql::ResultSet *res = pstmt->executeQuery();

        if (res)
        {
            while (res->next())
            {
                int m = res->getInt("m");
                if (m >= 1 && m <= 12)
                    expense[m] = res->getDouble("total");
            }
            delete res;
        }

        delete pstmt;

        // DETECT ZERO-SPENDING STREAKS
        for (int i = 1; i <= 12; i++)
        {
            if (expense[i] == 0)
            {
                if (zeroStreakStart == -1)
                    zeroStreakStart = i;
                zeroStreakLength++;
            }
            else
            {
                if (zeroStreakLength > maxZeroStreak)
                {
                    maxZeroStreak = zeroStreakLength;
                    maxZeroStreakStart = zeroStreakStart;
                }
                zeroStreakStart = -1;
                zeroStreakLength = 0;
            }
        }
        if (zeroStreakLength > maxZeroStreak)
        {
            maxZeroStreak = zeroStreakLength;
            maxZeroStreakStart = zeroStreakStart;
        }

        double total = 0;
        double avg = 0;

        for (int i = 1; i <= 12; i++)
            total += expense[i];

        avg = (total > 0) ? total / 12 : 0;

        int increaseCount = 0;
        int decreaseCount = 0;
        int stableCount = 0;
        int anomalyCount = 0;

        string highestChangeMonth = "-";
        double highestChange = 0;
        double largestAbsoluteIncrease = 0;
        string largestIncreaseMonth = "-";

        cout << "\n=======================================================================================================================\n";
        cout << "                                            EXPENSE CHANGE ANALYSIS REPORT\n";
        cout << "=======================================================================================================================\n";

        cout << "Year : " << year << "\n";

        cout << "\n================================================ EXPENSE OVERVIEW ====================================================\n";
        cout << "Total Expense       : RM " << formatMoneyAdmin(total) << "\n";
        cout << "Average Per Month   : RM " << formatMoneyAdmin(avg) << "\n";
        cout << "=======================================================================================================================\n";

        cout << "\nMONTHLY CHANGE ANALYSIS\n";
        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        cout << left
             << setw(8) << "Month"
             << setw(16) << "Expense"
             << setw(16) << "Direction"
             << setw(12) << "Change"
             << setw(18) << "Status"
             << "Insight\n";

        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        for (int i = 1; i <= 12; i++)
        {
            double prev = (i > 1) ? expense[i - 1] : 0;
            double curr = expense[i];

            double changePercent = 0;
            string changeText;

            string direction;
            string status;
            string insight;

            if (i == 1)
            {
                direction = "BASE";
                status = "REFERENCE";
                insight = "Starting month (no comparison)";
                changeText = "-";
            }
            else if (prev == 0 && curr > 0)
            {
                changePercent = 100;
                direction = "NEW";
                status = "NEW ACTIVITY";
                insight = "Expenses started this month";
                increaseCount++;
                changeText = "NEW";
            }
            else if (prev > 0 && curr == 0)
            {
                changePercent = 100;
                decreaseCount++;
                direction = "DOWN";
                status = "COMPLETE DROP";
                insight = "Expenses ceased (verify data integrity)";
                anomalyCount++;
                changeText = "100%";
            }
            else if (prev > 0 && curr > 0)
            {
                changePercent = ((curr - prev) / prev) * 100;

                if (changePercent > 0)
                {
                    increaseCount++;

                    direction = "UP";

                    double absoluteIncrease = curr - prev;
                    if (absoluteIncrease > largestAbsoluteIncrease)
                    {
                        largestAbsoluteIncrease = absoluteIncrease;
                        largestIncreaseMonth = getMonth(i);
                    }

                    if (changePercent >= 100)
                        status = "VERY HIGH";
                    else if (changePercent >= 50)
                        status = "HIGH";
                    else if (changePercent >= 20)
                        status = "MODERATE";
                    else
                        status = "LOW";

                    if (changePercent >= 100)
                    {
                        insight = "ALERT: Significant spike - verify legitimacy";
                        anomalyCount++;
                    }
                    else
                    {
                        insight = "Spending increased compared to last month";
                    }
                }
                else if (changePercent < 0)
                {
                    decreaseCount++;

                    changePercent = -changePercent;
                    direction = "DOWN";

                    if (changePercent >= 50)
                        status = "HIGH DROP";
                    else if (changePercent >= 20)
                        status = "MODERATE DROP";
                    else
                        status = "LOW DROP";

                    insight = "Spending decreased compared to last month";
                }
                else
                {
                    stableCount++;
                    direction = "NONE";
                    status = "STABLE";
                    insight = "No change from last month";
                }

                changeText = to_string((int)changePercent) + "%";
            }
            else
            {
                stableCount++;
                direction = "NONE";
                status = "STABLE";
                insight = "No spending activity";
                changeText = "0%";
            }

            if (changePercent > highestChange && changePercent > 0)
            {
                highestChange = changePercent;
                highestChangeMonth = getMonth(i);
            }

            cout << left
                 << setw(8) << getMonth(i)
                 << setw(16) << formatMoneyAdmin(curr)
                 << setw(16) << direction
                 << setw(12) << changeText;

            int statusColor = 7;
            if (status == "REFERENCE" || status == "STABLE")
                statusColor = 11; // Cyan
            else if (status == "VERY HIGH" || status == "HIGH" || status == "COMPLETE DROP" || status == "HIGH DROP")
                statusColor = 12; // Red
            else if (status == "MODERATE" || status == "MODERATE DROP" || status == "NEW ACTIVITY")
                statusColor = 14; // Yellow
            else if (status == "LOW" || status == "LOW DROP")
                statusColor = 11; // Cyan

            setColorAdmin(statusColor);
            cout << setw(18) << status;
            setColorAdmin(7);
            cout << insight << "\n";
        }

        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        cout << "\nINSIGHT SUMMARY\n";
        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        cout << "Months Increased : " << increaseCount << "\n";
        cout << "Months Decreased : " << decreaseCount << "\n";
        cout << "Stable Months    : " << stableCount << "\n";
        cout << "Anomalies Detected : " << anomalyCount << "\n";

        if (maxZeroStreak >= 3)
        {
            cout << "WARNING: " << maxZeroStreak << " consecutive months with zero spending starting "
                 << getMonth(maxZeroStreakStart) << "\n";
        }

        if (largestAbsoluteIncrease > 0)
        {
            cout << "Largest Absolute Increase : " << largestIncreaseMonth
                 << " (RM " << formatMoneyAdmin(largestAbsoluteIncrease) << ")\n";
        }

        cout << "\nRECOMMENDATION\n";
        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        if (total == 0)
        {
            setColorAdmin(14);
            cout << "No expense data found for this year.\n";
        }
        else if (maxZeroStreak >= 3)
        {
            setColorAdmin(12);
            cout << "CRITICAL: " << maxZeroStreak << " consecutive zero-expense months detected ("
                 << getMonth(maxZeroStreakStart) << "-" << getMonth(maxZeroStreakStart + maxZeroStreak - 1)
                 << ").\n";
            cout << "Action: Verify data entry and budget allocation.\n";
        }
        else if (anomalyCount >= 2)
        {
            setColorAdmin(12);
            cout << "Multiple spending anomalies detected. Review exceptional transactions for approval.\n";
        }
        else if (increaseCount > decreaseCount)
        {
            setColorAdmin(12);
            cout << "Expenses trending upward. Monitor spending closely and identify cost drivers.\n";
        }
        else if (decreaseCount > increaseCount)
        {
            setColorAdmin(10);
            cout << "Expenses declining. Confirm this reflects budget cuts, not missing transactions.\n";
        }
        else
        {
            setColorAdmin(11);
            cout << "Expense pattern stable. Continue current spending discipline.\n";
        }

        setColorAdmin(7);

        cout << "=======================================================================================================================\n";
    }
    catch (exception &e)
    {
        cout << "Error in expenseMonthlyTrendReport: " << e.what() << "\n";
    }

    delete con;
}

// USER ACCOUNT REPORT
void userAccountReport()
{
    sql::Connection *con = connectDB();
    if (!con)
        return;

    try
    {
        sql::Statement *stmt = con->createStatement();
        if (!stmt)
        {
            cout << "Failed to create statement\n";
            delete con;
            return;
        }

        string query =
            "SELECT u.user_id, u.username, "
            "COUNT(DISTINCT i.income_id) AS income_count, "
            "COUNT(DISTINCT e.expense_id) AS expense_count, "
            "IFNULL(SUM(e.amount),0) AS total_expense "
            "FROM users u "
            "LEFT JOIN income i ON i.user_id = u.user_id "
            "LEFT JOIN expense e ON e.user_id = u.user_id "
            "GROUP BY u.user_id, u.username "
            "ORDER BY total_expense DESC";

        sql::ResultSet *res = run(stmt, query);

        cout << "\n=======================================================================================================================\n";
        cout << "                                                 USER ACCOUNT ANALYTICS REPORT\n";
        cout << "=======================================================================================================================\n";

        cout << left
             << setw(5) << "No"
             << setw(12) << "User ID"
             << setw(20) << "Username"
             << setw(14) << "Income Tx"
             << setw(14) << "Expense Tx"
             << setw(18) << "Total Spent"
             << setw(12) << "Status"
             << "\n";

        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        int no = 1;
        int totalUsers = 0;
        int activeUsers = 0;

        double systemExpense = 0;
        double maxExpense = 0;
        string topUser = "-";

        int statusColor = 7;

        if (res)
        {
            while (res->next())
            {
                string uid = res->getString("user_id");
                string uname = res->getString("username");
                int incomeCount = res->getInt("income_count");
                int expenseCount = res->getInt("expense_count");
                double totalExpense = res->getDouble("total_expense");

                string status;
                // statusColor = 7;

                if (incomeCount == 0 && expenseCount == 0)
                {
                    status = "INACTIVE";
                    statusColor = 14; // Yellow
                }
                else if (expenseCount == 0)
                {
                    status = "NO EXPENSE";
                    statusColor = 11; // Cyan
                }
                else if (totalExpense > 1000)
                {
                    status = "HIGH SPENDER";
                    statusColor = 12; // Red
                }
                else if (totalExpense > 300)
                {
                    status = "ACTIVE";
                    statusColor = 10; // Green
                }
                else
                {
                    status = "LOW";
                    statusColor = 14; // Yellow
                }

                if (incomeCount > 0 || expenseCount > 0)
                    activeUsers++;

                if (totalExpense > maxExpense)
                {
                    maxExpense = totalExpense;
                    topUser = uname;
                }

                systemExpense += totalExpense;

                cout << left
                     << setw(5) << no++
                     << setw(12) << uid
                     << setw(20) << uname
                     << setw(14) << incomeCount
                     << setw(14) << expenseCount
                     << setw(18) << formatMoneyAdmin(totalExpense);

                setColorAdmin(statusColor);
                cout << setw(12) << status;
                setColorAdmin(7);
                cout << "\n";

                totalUsers++;
            }
            delete res;
        }

        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        double activeRate = (totalUsers > 0)
                                ? (activeUsers * 100.0 / totalUsers)
                                : 0;

        cout << "\nSYSTEM OVERVIEW\n";
        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        cout << "Total Users        : " << totalUsers << "\n";
        cout << "Active Users       : " << activeUsers << "\n";
        cout << "Inactive Users     : " << (totalUsers - activeUsers) << "\n";
        cout << "Active Rate        : " << (int)activeRate << "%\n";
        cout << "Total System Spend : RM " << formatMoneyAdmin(systemExpense) << "\n";
        cout << "Top Spender        : " << topUser << " (RM " << formatMoneyAdmin(maxExpense) << ")\n";

        cout << "\nINSIGHT\n";
        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        if (activeRate < 50)
        {
            statusColor = 12; // Red
            setColorAdmin(statusColor);
            cout << "System health is weak: many inactive users.\n";
            setColorAdmin(7);
        }

        else if (activeRate < 80)
        {
            statusColor = 14; // Yellow
            setColorAdmin(statusColor);
            cout << "System is moderately active.\n";
            setColorAdmin(7);
        }

        else
        {
            statusColor = 10; // Green
            setColorAdmin(statusColor);
            cout << "System is healthy with strong user engagement.\n";
            setColorAdmin(7);
        }
        //setColorAdmin(7);

        cout << "=======================================================================================================================\n";

        delete stmt;
    }
    catch (exception &e)
    {
        cout << "Error in userAccountReport: " << e.what() << "\n";
    }

    delete con;
}

// BUDGET SUMMARY REPORT
void adminBudgetSummaryReport()
{
    sql::Connection *con = connectDB();
    if (!con)
        return;

    try
    {
        sql::Statement *stmt = con->createStatement();
        if (!stmt)
        {
            cout << "Failed to create statement\n";
            delete con;
            return;
        }

        sql::ResultSet *rs = nullptr;

        double totalIncome = 0.0;
        double totalExpense = 0.0;
        double totalBudget = 0.0;

        rs = stmt->executeQuery("SELECT IFNULL(SUM(amount),0) AS total_income FROM income");
        if (rs && rs->next())
            totalIncome = rs->getDouble("total_income");
        if (rs)
            delete rs;

        rs = stmt->executeQuery("SELECT IFNULL(SUM(amount),0) AS total_expense FROM expense");
        if (rs && rs->next())
            totalExpense = rs->getDouble("total_expense");
        if (rs)
            delete rs;

        rs = stmt->executeQuery("SELECT IFNULL(SUM(amount_limit),0) AS total_budget FROM budget");
        if (rs && rs->next())
            totalBudget = rs->getDouble("total_budget");
        if (rs)
            delete rs;

        double systemBalance = totalIncome - totalExpense;

        cout << "\n=======================================================================================================================\n";
        cout << "                               ADMIN BUDGET SUMMARY DASHBOARD 2026\n";
        cout << "=======================================================================================================================\n";

        cout << "\n================================================ SYSTEM OVERVIEW ====================================================\n";

        cout << left
             << setw(22) << "Total Income" << ": RM " << fixed << setprecision(2) << totalIncome << "\n"
             << setw(22) << "Total Expense" << ": RM " << totalExpense << "\n"
             << setw(22) << "Total Budget" << ": RM " << totalBudget << "\n"
             << setw(22) << "System Balance" << ": RM " << systemBalance << "\n";

        cout << "=======================================================================================================================\n";

        cout << "\n================================================ USER FINANCIAL STATUS =================================================\n";

        string query =
            "SELECT u.user_id, u.name, "
            "IFNULL((SELECT SUM(amount) FROM income i WHERE i.user_id = u.user_id),0) AS income, "
            "IFNULL((SELECT SUM(amount) FROM expense e WHERE e.user_id = u.user_id),0) AS expense "
            "FROM users u";

        rs = stmt->executeQuery(query);

        cout << left
             << setw(5) << "ID"
             << setw(20) << "Name"
             << setw(15) << "Income"
             << setw(15) << "Expense"
             << setw(15) << "Balance"
             << setw(12) << "Status"
             << "\n";

        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        int totalUsers = 0;
        int riskUsers = 0;

        if (rs)
        {
            while (rs->next())
            {
                int id = rs->getInt("user_id");
                string name = rs->getString("name");
                double income = rs->getDouble("income");
                double expense = rs->getDouble("expense");
                double balance = income - expense;

                string status;
                int statusColor = 7;

                if (balance < 0)
                {
                    status = "NEGATIVE";
                    statusColor = 12; // Red
                    riskUsers++;
                }
                else if (balance < 100)
                {
                    status = "CRITICAL";
                    statusColor = 12; // Red
                    riskUsers++;
                }
                else if (balance < 500)
                {
                    status = "LOW";
                    statusColor = 14; // Yellow
                }
                else
                {
                    status = "OK";
                    statusColor = 10; // Green
                }

                cout << left
                     << setw(5) << id
                     << setw(20) << name
                     << setw(15) << fixed << setprecision(2) << income
                     << setw(15) << expense
                     << setw(15) << balance;

                setColorAdmin(statusColor);
                cout << setw(12) << status;
                setColorAdmin(7);
                cout << "\n";

                totalUsers++;
            }
            delete rs;
        }

        cout << "-----------------------------------------------------------------------------------------------------------------------\n";

        cout << "\n================================================ SYSTEM INSIGHT ======================================================\n";

        double usageRate = (totalBudget > 0)
                               ? (totalExpense / totalBudget) * 100
                               : 0;

        cout << "Budget Usage Rate : " << (int)usageRate << "%\n";
        cout << "At-Risk Users     : " << riskUsers << " / " << totalUsers << "\n";

        if (totalExpense > totalBudget)
        {
            setColorAdmin(12);
            cout << "STATUS            : OVER BUDGET\n";
            cout << "ACTION REQUIRED   : Immediate cost control needed\n";
        }
        else if (usageRate > 80)
        {
            setColorAdmin(14);
            cout << "STATUS            : HIGH USAGE WARNING\n";
            cout << "ACTION REQUIRED   : Monitor spending closely\n";
        }
        else
        {
            setColorAdmin(10);
            cout << "STATUS            : HEALTHY FINANCIAL POSITION\n";
            cout << "ACTION           : System within safe limits\n";
        }

        setColorAdmin(7);

        cout << "=======================================================================================================================\n";
        cout << "Report generated successfully.\n";
        cout << "=======================================================================================================================\n";

        delete stmt;
    }
    catch (exception &e)
    {
        cout << "Error in adminBudgetSummaryReport: " << e.what() << "\n";
    }

    delete con;
}

// MENU
void adminReportMenu()
{
    int choice;
    int year;

    do
    {
        system("cls");

        cout << "=============================================================================================\n";
        cout << "                                 ADMIN REPORT DASHBOARD\n";
        cout << "=============================================================================================\n";
        cout << "  Generate and analyse financial reports across the entire system.\n";
        cout << "=============================================================================================\n\n";

        cout << " REPORTS AVAILABLE\n";
        cout << "---------------------------------------------------------------------------------------------\n";
        cout << " [1] User Account Report          - User activities and spending overview\n";
        cout << " [2] Category Usage Report        - Most used expense categories\n";
        cout << " [3] Financial Trend Report       - Income vs Expense by year\n";
        cout << " [4] Expense Bar Chart            - Monthly expense visualization\n";
        cout << " [5] Expense Monthly Trend Report - Monthly expense analysis\n";
        cout << " [6] Budget Summary Report        - Overall financial health\n";
        cout << " [0] Back\n";
        cout << "---------------------------------------------------------------------------------------------\n";

        cout << "Enter Choice : ";
        choice = getValidatedChoiceAdmin6(0, 6);

        switch (choice)
        {
        case 1:
            system("cls");
            userAccountReport();
            break;

        case 2:
            system("cls");
            categoryUsageReport();
            break;

        case 3:
            system("cls");
            cout << "=============================================================================================\n";
            cout << "                              FINANCIAL TREND REPORT SETUP\n";
            cout << "=============================================================================================\n";
            cout << "Enter Year (2000 - 2100) : ";
            year = getValidatedChoiceAdmin6(2000, 2100);

            system("cls");
            financialTrendReport(year);
            break;

        case 4:
            system("cls");
            cout << "=============================================================================================\n";
            cout << "                             EXPENSE BAR CHART REPORT SETUP\n";
            cout << "=============================================================================================\n";
            cout << "Enter Year (2000 - 2100) : ";
            year = getValidatedChoiceAdmin6(2000, 2100);

            system("cls");
            expenseBarChart(year);
            break;

        case 5:
            system("cls");
            cout << "=============================================================================================\n";
            cout << "                           EXPENSE MONTHLY TREND REPORT SETUP\n";
            cout << "=============================================================================================\n";
            cout << "Enter Year (2000 - 2100) : ";
            year = getValidatedChoiceAdmin6(2000, 2100);

            system("cls");
            expenseMonthlyTrendReport(year);
            break;

        case 6:
            system("cls");
            adminBudgetSummaryReport();
            break;

        case 0:
            return;
        }

        pauseAdmin();

    } while (true);
}