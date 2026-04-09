#include "CoopTracker.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <limits>
#include <ctime>
#include <algorithm>
#include <map>
#include <cmath>

using namespace std;

namespace ANSI {
    const string RESET = "\033[0m";
    const string BOLD = "\033[1m";
    const string DIM = "\033[2m";
    const string RED = "\033[31m";
    const string GREEN = "\033[32m";
    const string YELLOW = "\033[33m";
    const string MAGENTA = "\033[35m";
    const string CYAN = "\033[36m";
    const string BRIGHT_BLACK = "\033[90m";
    const string BRIGHT_GREEN = "\033[92m";
    const string BRIGHT_CYAN = "\033[96m";
}

namespace UI {
    string money(double value) {
        ostringstream out;
        out << fixed << setprecision(2) << "$" << value;
        return ANSI::BRIGHT_GREEN + ANSI::BOLD + out.str() + ANSI::RESET;
    }

    string decimal(double value) {
        ostringstream out;
        out << fixed << setprecision(2) << value;
        return out.str();
    }

    string success(const string& message) {
        return ANSI::GREEN + "✔ " + message + ANSI::RESET;
    }

    string error(const string& message) {
        return ANSI::RED + "✖ " + message + ANSI::RESET;
    }

    string warning(const string& message) {
        return ANSI::YELLOW + "⚠ " + message + ANSI::RESET;
    }

    string value(const string& message) {
        return ANSI::BRIGHT_GREEN + ANSI::BOLD + message + ANSI::RESET;
    }

    void printDivider(char fill = '-') {
        cout << ANSI::DIM << ANSI::BRIGHT_BLACK << string(48, fill) << ANSI::RESET << "\n";
    }
}


// ================= HELPERS =================
void CoopTracker::printSectionHeader(const string& title) const {
    cout << "\n";
    UI::printDivider('=');
    cout << ANSI::BOLD << ANSI::BRIGHT_CYAN << "  " << title << ANSI::RESET << "\n";
    UI::printDivider('=');
}

void CoopTracker::pauseForEnter() const {
    cout << "\n" << ANSI::BOLD << ANSI::YELLOW << "Press Enter to continue..." << ANSI::RESET;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

string CoopTracker::getLineInput(const string& prompt) const {
    string input;
    cout << prompt;
    getline(cin, input);
    return input;
}

int CoopTracker::getValidatedInt(const string& prompt, int min, int max) const {
    int value;
    while (true) {
        cout << prompt;
        cin >> value;

        if (!cin.fail() && value >= min && value <= max) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << UI::error("Invalid input. Please try again.") << "\n";
    }
}

double CoopTracker::getValidatedDouble(const string& prompt, double min, double max) const {
    double value;
    while (true) {
        cout << prompt;
        cin >> value;

        if (!cin.fail() && value >= min && value <= max) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }

        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cout << UI::error("Invalid input. Please try again.") << "\n";
    }
}

string CoopTracker::promptForDate(const string& prompt) const {
    return getLineInput(prompt);
}

string CoopTracker::escapeCSV(const string& value) const {
    string escaped = value;
    size_t pos = 0;
    while ((pos = escaped.find('"', pos)) != string::npos) {
        escaped.insert(pos, 1, '"');
        pos += 2;
    }
    return "\"" + escaped + "\"";
}

// ================= DATE HELPERS =================
bool CoopTracker::parseDate(const string& date, int& month, int& day, int& year) const {
    char slash1, slash2;
    stringstream ss(date);

    ss >> month >> slash1 >> day >> slash2 >> year;

    if (ss.fail() || slash1 != '/' || slash2 != '/') return false;
    if (month < 1 || month > 12) return false;
    if (day < 1 || day > 31) return false;
    if (year < 1900 || year > 3000) return false;

    return true;
}

bool CoopTracker::isDateInMonthYear(const string& date, int month, int year) const {
    int m, d, y;
    if (!parseDate(date, m, d, y)) return false;
    return m == month && y == year;
}

bool CoopTracker::isDateInYear(const string& date, int year) const {
    int m, d, y;
    if (!parseDate(date, m, d, y)) return false;
    return y == year;
}

bool CoopTracker::isDateInRange(const string& date, const string& startDate, const string& endDate) const {
    int dateValue = dateToSortableValue(date);
    int startValue = dateToSortableValue(startDate);
    int endValue = dateToSortableValue(endDate);

    if (dateValue == 0 || startValue == 0 || endValue == 0) {
        return false;
    }

    return dateValue >= startValue && dateValue <= endValue;
}

int CoopTracker::dateToSortableValue(const string& date) const {
    int m, d, y;
    if (!parseDate(date, m, d, y)) return 0;
    return y * 10000 + m * 100 + d;
}

string CoopTracker::monthYearLabel(int month, int year) const {
    static const string months[12] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    if (month < 1 || month > 12) {
        return "Unknown";
    }

    return months[month - 1] + " " + to_string(year);
}

int CoopTracker::getCurrentMonth() const {
    time_t now = time(nullptr);
    tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localTime = *localtime(&now);
#endif
    return localTime.tm_mon + 1;
}

int CoopTracker::getCurrentYear() const {
    time_t now = time(nullptr);
    tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localTime = *localtime(&now);
#endif
    return localTime.tm_year + 1900;
}

string CoopTracker::getLastCleaningDate() const {
    if (cleaningRecords.empty()) {
        return "N/A";
    }

    string latestDate = cleaningRecords[0].getDate();
    int latestValue = dateToSortableValue(latestDate);

    for (size_t i = 1; i < cleaningRecords.size(); i++) {
        int currentValue = dateToSortableValue(cleaningRecords[i].getDate());
        if (currentValue > latestValue) {
            latestValue = currentValue;
            latestDate = cleaningRecords[i].getDate();
        }
    }

    return latestDate;
}

int CoopTracker::getDaysBetween(const string& earlierDate, const string& laterDate) const {
    int m1, d1, y1;
    int m2, d2, y2;

    if (!parseDate(earlierDate, m1, d1, y1) || !parseDate(laterDate, m2, d2, y2)) {
        return -1;
    }

    tm first{};
    first.tm_year = y1 - 1900;
    first.tm_mon = m1 - 1;
    first.tm_mday = d1;
    first.tm_hour = 12;

    tm second{};
    second.tm_year = y2 - 1900;
    second.tm_mon = m2 - 1;
    second.tm_mday = d2;
    second.tm_hour = 12;

    time_t t1 = mktime(&first);
    time_t t2 = mktime(&second);

    if (t1 == -1 || t2 == -1) {
        return -1;
    }

    double diffSeconds = difftime(t2, t1);
    return static_cast<int>(diffSeconds / (60 * 60 * 24));
}

int CoopTracker::getDaysSinceDate(const string& date) const {
    int m, d, y;
    if (!parseDate(date, m, d, y)) {
        return -1;
    }

    time_t now = time(nullptr);
    tm today{};
#ifdef _WIN32
    localtime_s(&today, &now);
#else
    today = *localtime(&now);
#endif

    tm past{};
    past.tm_year = y - 1900;
    past.tm_mon = m - 1;
    past.tm_mday = d;
    past.tm_hour = 12;

    today.tm_hour = 12;
    today.tm_min = 0;
    today.tm_sec = 0;

    time_t pastTime = mktime(&past);
    time_t todayTime = mktime(&today);

    if (pastTime == -1 || todayTime == -1) {
        return -1;
    }

    double diffSeconds = difftime(todayTime, pastTime);
    return static_cast<int>(diffSeconds / (60 * 60 * 24));
}

// ================= LOAD / SAVE ALL =================
void CoopTracker::loadAllData() {
    loadChickensFromTxt();
    loadFeedRecordsFromTxt();
    loadExpensesFromTxt();
    loadEggRecordsFromTxt();
    loadHealthNotesFromTxt();
    loadCleaningRecordsFromTxt();
}

void CoopTracker::saveAllData() const {
    saveChickensToTxt();
    saveFeedRecordsToTxt();
    saveExpensesToTxt();
    saveEggRecordsToTxt();
    saveHealthNotesToTxt();
    saveCleaningRecordsToTxt();
}

// ================= LOAD TXT =================
void CoopTracker::loadChickensFromTxt() {
    chickens.clear();
    ifstream file("data/chickens.txt");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string name, breed, ageStr, notes;

        getline(ss, name, '|');
        getline(ss, breed, '|');
        getline(ss, ageStr, '|');
        getline(ss, notes);

        int age = 0;
        try {
            age = stoi(ageStr);
        }
        catch (...) {
            age = 0;
        }

        chickens.push_back(Chicken(name, breed, age, notes));
    }
}

void CoopTracker::loadFeedRecordsFromTxt() {
    feedRecords.clear();
    ifstream file("data/feedrecords.txt");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string date, feedType, quantityStr, costStr;

        getline(ss, date, '|');
        getline(ss, feedType, '|');
        getline(ss, quantityStr, '|');
        getline(ss, costStr);

        double quantity = 0.0;
        double cost = 0.0;

        try { quantity = stod(quantityStr); }
        catch (...) { quantity = 0.0; }

        try { cost = stod(costStr); }
        catch (...) { cost = 0.0; }

        feedRecords.push_back(FeedRecord(date, feedType, quantity, cost));
    }
}

void CoopTracker::loadExpensesFromTxt() {
    expenses.clear();
    ifstream file("data/expenses.txt");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string date, category, description, amountStr;

        getline(ss, date, '|');
        getline(ss, category, '|');
        getline(ss, description, '|');
        getline(ss, amountStr);

        double amount = 0.0;
        try { amount = stod(amountStr); }
        catch (...) { amount = 0.0; }

        expenses.push_back(Expense(date, category, description, amount));
    }
}

void CoopTracker::loadEggRecordsFromTxt() {
    eggRecords.clear();
    ifstream file("data/eggrecords.txt");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string date, countStr, notes;

        getline(ss, date, '|');
        getline(ss, countStr, '|');
        getline(ss, notes);

        int count = 0;
        try { count = stoi(countStr); }
        catch (...) { count = 0; }

        eggRecords.push_back(EggRecord(date, count, notes));
    }
}

void CoopTracker::loadHealthNotesFromTxt() {
    healthNotes.clear();
    ifstream file("data/healthnotes.txt");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string date, chickenName, note;

        getline(ss, date, '|');
        getline(ss, chickenName, '|');
        getline(ss, note);

        healthNotes.push_back(HealthNote(date, chickenName, note));
    }
}

void CoopTracker::loadCleaningRecordsFromTxt() {
    cleaningRecords.clear();
    ifstream file("data/cleaningrecords.txt");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string date, task, notes;

        getline(ss, date, '|');
        getline(ss, task, '|');
        getline(ss, notes);

        cleaningRecords.push_back(CleaningRecord(date, task, notes));
    }
}

// ================= SAVE TXT =================
void CoopTracker::saveChickensToTxt() const {
    ofstream file("data/chickens.txt");
    for (const auto& chicken : chickens) {
        file << chicken.getName() << "|"
            << chicken.getBreed() << "|"
            << chicken.getAge() << "|"
            << chicken.getNotes() << "\n";
    }
}

void CoopTracker::saveFeedRecordsToTxt() const {
    ofstream file("data/feedrecords.txt");
    for (const auto& record : feedRecords) {
        file << record.getDate() << "|"
            << record.getFeedType() << "|"
            << record.getQuantity() << "|"
            << record.getCost() << "\n";
    }
}

void CoopTracker::saveExpensesToTxt() const {
    ofstream file("data/expenses.txt");
    for (const auto& expense : expenses) {
        file << expense.getDate() << "|"
            << expense.getCategory() << "|"
            << expense.getDescription() << "|"
            << expense.getAmount() << "\n";
    }
}

void CoopTracker::saveEggRecordsToTxt() const {
    ofstream file("data/eggrecords.txt");
    for (const auto& record : eggRecords) {
        file << record.getDate() << "|"
            << record.getEggCount() << "|"
            << record.getNotes() << "\n";
    }
}

void CoopTracker::saveHealthNotesToTxt() const {
    ofstream file("data/healthnotes.txt");
    for (const auto& note : healthNotes) {
        file << note.getDate() << "|"
            << note.getChickenName() << "|"
            << note.getNote() << "\n";
    }
}

void CoopTracker::saveCleaningRecordsToTxt() const {
    ofstream file("data/cleaningrecords.txt");
    for (const auto& record : cleaningRecords) {
        file << record.getDate() << "|"
            << record.getTask() << "|"
            << record.getNotes() << "\n";
    }
}

// ================= EXPORT CSV =================
void CoopTracker::exportChickensToCSV() const {
    ofstream file("exports/chickens.csv");
    file << "Name,Breed,Age,Notes\n";
    for (const auto& chicken : chickens) {
        file << escapeCSV(chicken.getName()) << ","
            << escapeCSV(chicken.getBreed()) << ","
            << chicken.getAge() << ","
            << escapeCSV(chicken.getNotes()) << "\n";
    }
}

void CoopTracker::exportFeedRecordsToCSV() const {
    ofstream file("exports/feedrecords.csv");
    file << "Date,Feed Type,Quantity,Cost\n";
    for (const auto& record : feedRecords) {
        file << escapeCSV(record.getDate()) << ","
            << escapeCSV(record.getFeedType()) << ","
            << record.getQuantity() << ","
            << record.getCost() << "\n";
    }
}

void CoopTracker::exportExpensesToCSV() const {
    ofstream file("exports/expenses.csv");
    file << "Date,Category,Description,Amount\n";
    for (const auto& expense : expenses) {
        file << escapeCSV(expense.getDate()) << ","
            << escapeCSV(expense.getCategory()) << ","
            << escapeCSV(expense.getDescription()) << ","
            << expense.getAmount() << "\n";
    }
}

void CoopTracker::exportEggRecordsToCSV() const {
    ofstream file("exports/eggrecords.csv");
    file << "Date,Egg Count,Notes\n";
    for (const auto& record : eggRecords) {
        file << escapeCSV(record.getDate()) << ","
            << record.getEggCount() << ","
            << escapeCSV(record.getNotes()) << "\n";
    }
}

void CoopTracker::exportHealthNotesToCSV() const {
    ofstream file("exports/healthnotes.csv");
    file << "Date,Chicken Name,Note\n";
    for (const auto& note : healthNotes) {
        file << escapeCSV(note.getDate()) << ","
            << escapeCSV(note.getChickenName()) << ","
            << escapeCSV(note.getNote()) << "\n";
    }
}

void CoopTracker::exportCleaningRecordsToCSV() const {
    ofstream file("exports/cleaningrecords.csv");
    file << "Date,Task,Notes\n";
    for (const auto& record : cleaningRecords) {
        file << escapeCSV(record.getDate()) << ","
            << escapeCSV(record.getTask()) << ","
            << escapeCSV(record.getNotes()) << "\n";
    }
}

void CoopTracker::exportAllToCSV() const {
    exportChickensToCSV();
    exportFeedRecordsToCSV();
    exportExpensesToCSV();
    exportEggRecordsToCSV();
    exportHealthNotesToCSV();
    exportCleaningRecordsToCSV();
}

// ================= STARTUP / DASHBOARD =================
void CoopTracker::showStartupStatus() const {
    printSectionHeader("DATA LOAD STATUS");
    cout << ANSI::DIM << "Loaded successfully:" << ANSI::RESET << "\n";
    cout << "  • Chickens:         " << UI::value(to_string(chickens.size())) << "\n";
    cout << "  • Feed records:     " << UI::value(to_string(feedRecords.size())) << "\n";
    cout << "  • Expenses:         " << UI::value(to_string(expenses.size())) << "\n";
    cout << "  • Egg records:      " << UI::value(to_string(eggRecords.size())) << "\n";
    cout << "  • Health notes:     " << UI::value(to_string(healthNotes.size())) << "\n";
    cout << "  • Cleaning records: " << UI::value(to_string(cleaningRecords.size())) << "\n";
}

void CoopTracker::showDashboard() const {
    int month = getCurrentMonth();
    int year = getCurrentYear();

    double expenseTotal = getExpenseTotalForMonth(month, year);
    double feedTotal = getFeedCostTotalForMonth(month, year);
    int eggTotalMonth = getEggTotalForMonth(month, year);
    int eggTotalYear = getEggTotalForYear(year);
    int healthCount = getHealthCountForMonth(month, year);
    int cleaningCount = getCleaningCountForMonth(month, year);
    double costPerDozen = getCostPerDozenForMonth(month, year);

    time_t now = time(nullptr);
    tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localTime = *localtime(&now);
#endif

    int todayMonth = localTime.tm_mon + 1;
    int todayDay = localTime.tm_mday;
    int todayYear = localTime.tm_year + 1900;

    int eggsToday = 0;
    for (const auto& record : eggRecords) {
        int m, d, y;
        if (parseDate(record.getDate(), m, d, y) && m == todayMonth && d == todayDay && y == todayYear) {
            eggsToday += record.getEggCount();
        }
    }

    double avgMonthlyFeedCost = getFeedCostTotalForYear(year) / 12.0;

    string status = "Flock stable";
    if (eggTotalMonth > 0 && costPerDozen > 0.0) {
        status = "Production active";
    }
    if (healthCount > 0) {
        status = "Recent health activity logged";
    }

    string lastFeedDate = "N/A";
    if (!feedRecords.empty()) {
        int latestValue = 0;
        for (const auto& record : feedRecords) {
            int value = dateToSortableValue(record.getDate());
            if (value > latestValue) {
                latestValue = value;
                lastFeedDate = record.getDate();
            }
        }
    }

    string lastHealthDate = "N/A";
    if (!healthNotes.empty()) {
        int latestValue = 0;
        for (const auto& note : healthNotes) {
            int value = dateToSortableValue(note.getDate());
            if (value > latestValue) {
                latestValue = value;
                lastHealthDate = note.getDate();
            }
        }
    }

    printSectionHeader("COOPKEEPER DASHBOARD");

    cout << ANSI::DIM << "Current period: " << monthYearLabel(month, year) << ANSI::RESET << "\n\n";

    cout << ANSI::BOLD << ANSI::MAGENTA << "FLOCK" << ANSI::RESET << "\n";
    cout << "Chickens:                 " << UI::value(to_string(chickens.size())) << "\n\n";

    cout << ANSI::BOLD << ANSI::CYAN << "PRODUCTION" << ANSI::RESET << "\n";
    cout << "Eggs (Today):             " << UI::value(to_string(eggsToday)) << "\n";
    cout << "Eggs (This Month):        " << UI::value(to_string(eggTotalMonth)) << "\n";
    cout << "Eggs (This Year):         " << UI::value(to_string(eggTotalYear)) << "\n\n";

    cout << ANSI::BOLD << ANSI::YELLOW << "FINANCIAL" << ANSI::RESET << "\n";
    cout << "Feed Cost (This Month):   " << UI::money(feedTotal) << "\n";
    cout << "Expenses (This Month):    " << UI::money(expenseTotal) << "\n";
    if (eggTotalMonth > 0) {
        cout << "Cost per Dozen:           " << UI::money(costPerDozen) << "\n";
    }
    else {
        cout << "Cost per Dozen:           " << ANSI::DIM << "N/A" << ANSI::RESET << "\n";
    }

    cout << "\n" << ANSI::BOLD << ANSI::GREEN << "OPERATIONS" << ANSI::RESET << "\n";
    cout << "Last Feed Purchase:       " << UI::value(lastFeedDate) << "\n";
    cout << "Avg Monthly Feed Cost:    " << UI::money(avgMonthlyFeedCost) << "\n";
    cout << "Health Notes (Month):     " << UI::value(to_string(healthCount)) << "\n";
    cout << "Last Health Check:        " << UI::value(lastHealthDate) << "\n";
    cout << "Cleanings (This Month):   " << UI::value(to_string(cleaningCount)) << "\n";
    cout << "Last Cleaning Date:       " << UI::value(getLastCleaningDate()) << "\n\n";

    UI::printDivider();
    cout << ANSI::BOLD << ANSI::MAGENTA << "Status: " << ANSI::RESET << UI::value(status) << "\n";
    UI::printDivider();
}

void CoopTracker::showInsights() const {
    int month = getCurrentMonth();
    int year = getCurrentYear();

    double avgEggsPerDay = getAverageEggsPerDayForMonth(month, year);
    double costPerEgg = getCostPerEggForMonth(month, year);
    string bestDay = getBestProductionDayForMonth(month, year);
    double productionChange = getProductionChangePercentFromPreviousMonth(month, year);

    cout << "\n" << ANSI::BOLD << ANSI::MAGENTA << "INSIGHTS" << ANSI::RESET << "\n";
    cout << "Average Eggs/Day:         " << UI::value(UI::decimal(avgEggsPerDay)) << "\n";
    if (getEggTotalForMonth(month, year) > 0) {
        cout << "Cost Per Egg:             " << UI::money(costPerEgg) << "\n";
    }
    else {
        cout << "Cost Per Egg:             " << ANSI::DIM << "N/A" << ANSI::RESET << "\n";
    }
    cout << "Best Production Day:      " << UI::value(bestDay) << "\n";

    ostringstream change;
    change << fixed << setprecision(2);
    if (productionChange > 0.0) {
        change << "+" << productionChange << "%";
    }
    else {
        change << productionChange << "%";
    }
    cout << "Vs Last Month:            " << UI::value(change.str()) << "\n";
}

void CoopTracker::showAlerts() const {
    printSectionHeader("ALERTS");

    bool hasAlerts = false;

    if (!eggRecords.empty()) {
        string latestEggDate = eggRecords[0].getDate();
        int latestEggValue = dateToSortableValue(latestEggDate);

        for (size_t i = 1; i < eggRecords.size(); i++) {
            int value = dateToSortableValue(eggRecords[i].getDate());
            if (value > latestEggValue) {
                latestEggValue = value;
                latestEggDate = eggRecords[i].getDate();
            }
        }

        int daysSinceEgg = getDaysSinceDate(latestEggDate);
        if (daysSinceEgg >= 3) {
            cout << UI::warning("No eggs recorded in " + to_string(daysSinceEgg) + " days.") << "\n";
            hasAlerts = true;
        }
    }

    if (!cleaningRecords.empty()) {
        string latestCleaningDate = cleaningRecords[0].getDate();
        int latestCleaningValue = dateToSortableValue(latestCleaningDate);

        for (size_t i = 1; i < cleaningRecords.size(); i++) {
            int value = dateToSortableValue(cleaningRecords[i].getDate());
            if (value > latestCleaningValue) {
                latestCleaningValue = value;
                latestCleaningDate = cleaningRecords[i].getDate();
            }
        }

        int daysSinceCleaning = getDaysSinceDate(latestCleaningDate);
        if (daysSinceCleaning >= 14) {
            cout << UI::warning("No cleaning logged in " + to_string(daysSinceCleaning) + " days.") << "\n";
            hasAlerts = true;
        }
    }
    else {
        cout << UI::warning("No cleaning records have been logged yet.") << "\n";
        hasAlerts = true;
    }

    int currentMonth = getCurrentMonth();
    int currentYear = getCurrentYear();
    int previousMonth = currentMonth - 1;
    int previousYear = currentYear;

    if (previousMonth == 0) {
        previousMonth = 12;
        previousYear--;
    }

    double currentFeed = getFeedCostTotalForMonth(currentMonth, currentYear);
    double previousFeed = getFeedCostTotalForMonth(previousMonth, previousYear);

    if (previousFeed > 0.0 && currentFeed > previousFeed) {
        double increasePercent = ((currentFeed - previousFeed) / previousFeed) * 100.0;
        cout << UI::warning("Feed spending is up " + UI::decimal(increasePercent) + "% vs last month.") << "\n";
        hasAlerts = true;
    }

    if (!hasAlerts) {
        cout << UI::success("No alerts right now.") << "\n";
    }
}

void CoopTracker::showMonthlyReport() const {
    int month = getValidatedInt("Enter month (1-12): ", 1, 12);
    int year = getValidatedInt("Enter year: ", 1900, 3000);

    printSectionHeader("MONTHLY REPORT - " + monthYearLabel(month, year));

    double expenseTotal = getExpenseTotalForMonth(month, year);
    double feedTotal = getFeedCostTotalForMonth(month, year);
    int eggTotal = getEggTotalForMonth(month, year);
    int healthCount = getHealthCountForMonth(month, year);
    int cleaningCount = getCleaningCountForMonth(month, year);
    double costPerEgg = getCostPerEggForMonth(month, year);
    double costPerDozen = getCostPerDozenForMonth(month, year);

    cout << fixed << setprecision(2);
    cout << "Expenses: $" << expenseTotal << "\n";
    cout << "Feed Cost: $" << feedTotal << "\n";
    cout << "Eggs Collected: " << eggTotal << "\n";
    cout << "Health Notes: " << healthCount << "\n";
    cout << "Cleaning Records: " << cleaningCount << "\n";

    if (eggTotal > 0) {
        cout << "Cost Per Egg: $" << costPerEgg << "\n";
        cout << "Cost Per Dozen: $" << costPerDozen << "\n";
    }
    else {
        cout << "Cost Per Egg: N/A\n";
        cout << "Cost Per Dozen: N/A\n";
    }
}

void CoopTracker::showYearlyReport() const {
    int year = getValidatedInt("Enter year: ", 1900, 3000);

    printSectionHeader("YEARLY REPORT - " + to_string(year));

    double expenseTotal = getExpenseTotalForYear(year);
    double feedTotal = getFeedCostTotalForYear(year);
    int eggTotal = getEggTotalForYear(year);
    int healthCount = getHealthCountForYear(year);
    int cleaningCount = getCleaningCountForYear(year);
    double costPerEgg = getCostPerEggForYear(year);
    double costPerDozen = getCostPerDozenForYear(year);

    cout << fixed << setprecision(2);
    cout << "Expenses: $" << expenseTotal << "\n";
    cout << "Feed Cost: $" << feedTotal << "\n";
    cout << "Eggs Collected: " << eggTotal << "\n";
    cout << "Health Notes: " << healthCount << "\n";
    cout << "Cleaning Records: " << cleaningCount << "\n";

    if (eggTotal > 0) {
        cout << "Cost Per Egg: $" << costPerEgg << "\n";
        cout << "Cost Per Dozen: $" << costPerDozen << "\n";
    }
    else {
        cout << "Cost Per Egg: N/A\n";
        cout << "Cost Per Dozen: N/A\n";
    }
}

void CoopTracker::showDateRangeReport() const {
    printSectionHeader("DATE RANGE REPORT");

    string startDate = promptForDate("Enter start date (MM/DD/YYYY): ");
    string endDate = promptForDate("Enter end date (MM/DD/YYYY): ");

    int sm, sd, sy, em, ed, ey;
    if (!parseDate(startDate, sm, sd, sy) || !parseDate(endDate, em, ed, ey)) {
        cout << "Invalid date format. Please use MM/DD/YYYY.\n";
        return;
    }

    if (dateToSortableValue(startDate) > dateToSortableValue(endDate)) {
        cout << "Start date must be before or equal to end date.\n";
        return;
    }

    int eggTotal = 0;
    double expenseTotal = 0.0;
    double feedTotal = 0.0;
    int healthCount = 0;
    int cleaningCount = 0;
    map<string, int> eggTotalsByDate;

    for (const auto& record : eggRecords) {
        if (isDateInRange(record.getDate(), startDate, endDate)) {
            eggTotal += record.getEggCount();
            eggTotalsByDate[record.getDate()] += record.getEggCount();
        }
    }

    for (const auto& expense : expenses) {
        if (isDateInRange(expense.getDate(), startDate, endDate)) {
            expenseTotal += expense.getAmount();
        }
    }

    for (const auto& record : feedRecords) {
        if (isDateInRange(record.getDate(), startDate, endDate)) {
            feedTotal += record.getCost();
        }
    }

    for (const auto& note : healthNotes) {
        if (isDateInRange(note.getDate(), startDate, endDate)) {
            healthCount++;
        }
    }

    for (const auto& record : cleaningRecords) {
        if (isDateInRange(record.getDate(), startDate, endDate)) {
            cleaningCount++;
        }
    }

    int daysInRange = getDaysBetween(startDate, endDate) + 1;
    if (daysInRange < 1) {
        daysInRange = 1;
    }

    double averageEggsPerDay = static_cast<double>(eggTotal) / daysInRange;
    double totalCost = expenseTotal + feedTotal;
    double costPerEgg = (eggTotal > 0) ? totalCost / eggTotal : 0.0;
    double costPerDozen = costPerEgg * 12.0;

    string bestDay = "N/A";
    int bestDayCount = -1;
    for (const auto& entry : eggTotalsByDate) {
        if (entry.second > bestDayCount) {
            bestDayCount = entry.second;
            bestDay = entry.first + " (" + to_string(entry.second) + " eggs)";
        }
    }

    cout << fixed << setprecision(2);
    cout << "Range: " << startDate << " to " << endDate << "\n";
    cout << "Days in Range: " << daysInRange << "\n";
    cout << "Eggs Collected: " << eggTotal << "\n";
    cout << "Average Eggs/Day: " << averageEggsPerDay << "\n";
    cout << "Feed Cost: $" << feedTotal << "\n";
    cout << "Expenses: $" << expenseTotal << "\n";
    cout << "Health Notes: " << healthCount << "\n";
    cout << "Cleaning Records: " << cleaningCount << "\n";

    if (eggTotal > 0) {
        cout << "Cost Per Egg: $" << costPerEgg << "\n";
        cout << "Cost Per Dozen: $" << costPerDozen << "\n";
    }
    else {
        cout << "Cost Per Egg: N/A\n";
        cout << "Cost Per Dozen: N/A\n";
    }

    cout << "Best Production Day: " << bestDay << "\n";
}

// ================= SUMMARY HELPERS =================
double CoopTracker::getExpenseTotalForMonth(int month, int year) const {
    double total = 0.0;
    for (const auto& expense : expenses) {
        if (isDateInMonthYear(expense.getDate(), month, year)) {
            total += expense.getAmount();
        }
    }
    return total;
}

double CoopTracker::getExpenseTotalForYear(int year) const {
    double total = 0.0;
    for (const auto& expense : expenses) {
        if (isDateInYear(expense.getDate(), year)) {
            total += expense.getAmount();
        }
    }
    return total;
}

double CoopTracker::getFeedCostTotalForMonth(int month, int year) const {
    double total = 0.0;
    for (const auto& record : feedRecords) {
        if (isDateInMonthYear(record.getDate(), month, year)) {
            total += record.getCost();
        }
    }
    return total;
}

double CoopTracker::getFeedCostTotalForYear(int year) const {
    double total = 0.0;
    for (const auto& record : feedRecords) {
        if (isDateInYear(record.getDate(), year)) {
            total += record.getCost();
        }
    }
    return total;
}

int CoopTracker::getEggTotalForMonth(int month, int year) const {
    int total = 0;
    for (const auto& record : eggRecords) {
        if (isDateInMonthYear(record.getDate(), month, year)) {
            total += record.getEggCount();
        }
    }
    return total;
}

int CoopTracker::getEggTotalForYear(int year) const {
    int total = 0;
    for (const auto& record : eggRecords) {
        if (isDateInYear(record.getDate(), year)) {
            total += record.getEggCount();
        }
    }
    return total;
}

int CoopTracker::getEggTotalForDate(const string& date) const {
    int total = 0;
    for (const auto& record : eggRecords) {
        if (record.getDate() == date) {
            total += record.getEggCount();
        }
    }
    return total;
}

int CoopTracker::getHealthCountForMonth(int month, int year) const {
    int count = 0;
    for (const auto& note : healthNotes) {
        if (isDateInMonthYear(note.getDate(), month, year)) {
            count++;
        }
    }
    return count;
}

int CoopTracker::getHealthCountForYear(int year) const {
    int count = 0;
    for (const auto& note : healthNotes) {
        if (isDateInYear(note.getDate(), year)) {
            count++;
        }
    }
    return count;
}

int CoopTracker::getCleaningCountForMonth(int month, int year) const {
    int count = 0;
    for (const auto& record : cleaningRecords) {
        if (isDateInMonthYear(record.getDate(), month, year)) {
            count++;
        }
    }
    return count;
}

int CoopTracker::getCleaningCountForYear(int year) const {
    int count = 0;
    for (const auto& record : cleaningRecords) {
        if (isDateInYear(record.getDate(), year)) {
            count++;
        }
    }
    return count;
}

double CoopTracker::getCostPerEggForMonth(int month, int year) const {
    int eggs = getEggTotalForMonth(month, year);
    if (eggs <= 0) return 0.0;

    double totalCost = getExpenseTotalForMonth(month, year) + getFeedCostTotalForMonth(month, year);
    return totalCost / eggs;
}

double CoopTracker::getCostPerDozenForMonth(int month, int year) const {
    return getCostPerEggForMonth(month, year) * 12.0;
}

double CoopTracker::getCostPerEggForYear(int year) const {
    int eggs = getEggTotalForYear(year);
    if (eggs <= 0) return 0.0;

    double totalCost = getExpenseTotalForYear(year) + getFeedCostTotalForYear(year);
    return totalCost / eggs;
}

double CoopTracker::getCostPerDozenForYear(int year) const {
    return getCostPerEggForYear(year) * 12.0;
}

double CoopTracker::getAverageEggsPerDayForMonth(int month, int year) const {
    map<string, int> dailyEggTotals;

    for (const auto& record : eggRecords) {
        if (isDateInMonthYear(record.getDate(), month, year)) {
            dailyEggTotals[record.getDate()] += record.getEggCount();
        }
    }

    if (dailyEggTotals.empty()) {
        return 0.0;
    }

    int totalEggs = 0;
    for (const auto& entry : dailyEggTotals) {
        totalEggs += entry.second;
    }

    return static_cast<double>(totalEggs) / dailyEggTotals.size();
}

string CoopTracker::getBestProductionDayForMonth(int month, int year) const {
    map<string, int> dailyEggTotals;

    for (const auto& record : eggRecords) {
        if (isDateInMonthYear(record.getDate(), month, year)) {
            dailyEggTotals[record.getDate()] += record.getEggCount();
        }
    }

    if (dailyEggTotals.empty()) {
        return "N/A";
    }

    string bestDate = "";
    int bestCount = -1;

    for (const auto& entry : dailyEggTotals) {
        if (entry.second > bestCount) {
            bestCount = entry.second;
            bestDate = entry.first;
        }
    }

    return bestDate + " (" + to_string(bestCount) + " eggs)";
}

double CoopTracker::getProductionChangePercentFromPreviousMonth(int month, int year) const {
    int previousMonth = month - 1;
    int previousYear = year;

    if (previousMonth == 0) {
        previousMonth = 12;
        previousYear--;
    }

    int currentTotal = getEggTotalForMonth(month, year);
    int previousTotal = getEggTotalForMonth(previousMonth, previousYear);

    if (previousTotal == 0) {
        if (currentTotal == 0) {
            return 0.0;
        }
        return 100.0;
    }

    return ((static_cast<double>(currentTotal - previousTotal) / previousTotal) * 100.0);
}

// ================= DISPLAY HELPERS =================
void CoopTracker::printFeedRecordList(const vector<FeedRecord>& records) const {
    for (size_t i = 0; i < records.size(); i++) {
        cout << i + 1 << ". "
            << records[i].getDate() << " | "
            << records[i].getFeedType() << " | Qty: "
            << records[i].getQuantity() << " | Cost: $"
            << fixed << setprecision(2) << records[i].getCost() << "\n";
    }
}

void CoopTracker::printExpenseList(const vector<Expense>& records) const {
    for (size_t i = 0; i < records.size(); i++) {
        cout << i + 1 << ". "
            << records[i].getDate() << " | "
            << records[i].getCategory() << " | "
            << records[i].getDescription() << " | $"
            << fixed << setprecision(2) << records[i].getAmount() << "\n";
    }
}

void CoopTracker::printEggRecordListSorted(const vector<EggRecord>& records) const {
    vector<EggRecord> sortedRecords = records;

    sort(sortedRecords.begin(), sortedRecords.end(),
        [this](const EggRecord& a, const EggRecord& b) {
            return dateToSortableValue(a.getDate()) < dateToSortableValue(b.getDate());
        });

    for (size_t i = 0; i < sortedRecords.size(); i++) {
        cout << i + 1 << ". "
            << sortedRecords[i].getDate() << " | Count: "
            << sortedRecords[i].getEggCount() << " | Notes: "
            << sortedRecords[i].getNotes() << "\n";
    }
}

void CoopTracker::printHealthNoteList(const vector<HealthNote>& records) const {
    for (size_t i = 0; i < records.size(); i++) {
        cout << i + 1 << ". "
            << records[i].getDate() << " | "
            << records[i].getChickenName() << " | "
            << records[i].getNote() << "\n";
    }
}

void CoopTracker::printCleaningRecordList(const vector<CleaningRecord>& records) const {
    for (size_t i = 0; i < records.size(); i++) {
        cout << i + 1 << ". "
            << records[i].getDate() << " | "
            << records[i].getTask() << " | "
            << records[i].getNotes() << "\n";
    }
}

// ================= RUN =================
void CoopTracker::run() {
    loadAllData();
    showStartupStatus();
    showDashboard();
    showInsights();
    showAlerts();
    pauseForEnter();

    int choice;
    do {
        cout << "\n\n";
        UI::printDivider();
        cout << ANSI::BOLD << ANSI::BRIGHT_CYAN << "                MAIN MENU" << ANSI::RESET << "\n";
        UI::printDivider();
        cout << "  " << ANSI::BOLD << "1." << ANSI::RESET << "  Chickens\n";
        cout << "  " << ANSI::BOLD << "2." << ANSI::RESET << "  Feed Records\n";
        cout << "  " << ANSI::BOLD << "3." << ANSI::RESET << "  Expenses\n";
        cout << "  " << ANSI::BOLD << "4." << ANSI::RESET << "  Egg Records\n";
        cout << "  " << ANSI::BOLD << "5." << ANSI::RESET << "  Health Notes\n";
        cout << "  " << ANSI::BOLD << "6." << ANSI::RESET << "  Cleaning Records\n";
        cout << "  " << ANSI::BOLD << "7." << ANSI::RESET << "  Monthly Report\n";
        cout << "  " << ANSI::BOLD << "8." << ANSI::RESET << "  Yearly Report\n";
        cout << "  " << ANSI::BOLD << "9." << ANSI::RESET << "  Date Range Report\n";
        cout << "  " << ANSI::BOLD << "10." << ANSI::RESET << " Export All to CSV\n";
        cout << "  " << ANSI::BOLD << "11." << ANSI::RESET << " Exit\n\n";

        choice = getValidatedInt("Select an option: ", 1, 11);

        switch (choice) {
        case 1: chickenMenu(); break;
        case 2: feedMenu(); break;
        case 3: expenseMenu(); break;
        case 4: eggMenu(); break;
        case 5: healthMenu(); break;
        case 6: cleaningMenu(); break;
        case 7: showMonthlyReport(); break;
        case 8: showYearlyReport(); break;
        case 9: showDateRangeReport(); break;
        case 10:
            exportAllToCSV();
            cout << UI::success("All CSV exports created in the exports folder.") << "\n";
            break;
        case 11:
            saveAllData();
            cout << UI::success("Goodbye.") << "\n";
            break;
        }
    } while (choice != 11);
}

// ================= CHICKENS =================
void CoopTracker::chickenMenu() {
    int choice;
    do {
        printSectionHeader("CHICKENS");
        cout << "1. Add\n";
        cout << "2. View\n";
        cout << "3. Edit\n";
        cout << "4. Delete\n";
        cout << "5. Back\n";

        choice = getValidatedInt("Choice: ", 1, 5);

        switch (choice) {
        case 1: addChicken(); break;
        case 2: viewChickens(); break;
        case 3: editChicken(); break;
        case 4: deleteChicken(); break;
        }
    } while (choice != 5);
}

void CoopTracker::addChicken() {
    printSectionHeader("ADD CHICKEN");

    string name = getLineInput("Name: ");
    string breed = getLineInput("Breed: ");
    int age = getValidatedInt("Age: ", 0, 100);
    string notes = getLineInput("Notes: ");

    chickens.push_back(Chicken(name, breed, age, notes));
    saveChickensToTxt();

    cout << UI::success("Chicken added.") << "\n";
}

void CoopTracker::viewChickens() const {
    printSectionHeader("CHICKENS");

    if (chickens.empty()) {
        cout << UI::warning("No chickens found.") << "\n";
        return;
    }

    for (size_t i = 0; i < chickens.size(); i++) {
        cout << i + 1 << ". "
            << chickens[i].getName() << " | "
            << chickens[i].getBreed() << " | Age: "
            << chickens[i].getAge() << " | Notes: "
            << chickens[i].getNotes() << "\n";
    }
}

void CoopTracker::editChicken() {
    if (chickens.empty()) {
        cout << UI::warning("No chickens to edit.") << "\n";
        return;
    }

    viewChickens();
    int choice = getValidatedInt("Select chicken: ", 1, static_cast<int>(chickens.size()));

    string name = getLineInput("New name: ");
    string breed = getLineInput("New breed: ");
    int age = getValidatedInt("New age: ", 0, 100);
    string notes = getLineInput("New notes: ");

    chickens[choice - 1].setName(name);
    chickens[choice - 1].setBreed(breed);
    chickens[choice - 1].setAge(age);
    chickens[choice - 1].setNotes(notes);

    saveChickensToTxt();
    cout << UI::success("Chicken updated.") << "\n";
}

void CoopTracker::deleteChicken() {
    if (chickens.empty()) {
        cout << UI::warning("No chickens to delete.") << "\n";
        return;
    }

    viewChickens();
    int choice = getValidatedInt("Select chicken to delete: ", 1, static_cast<int>(chickens.size()));
    chickens.erase(chickens.begin() + (choice - 1));
    saveChickensToTxt();

    cout << UI::success("Chicken deleted.") << "\n";
}

// ================= FEED =================
void CoopTracker::feedMenu() {
    int choice;
    do {
        printSectionHeader("FEED RECORDS");
        cout << "1. Add\n";
        cout << "2. View\n";
        cout << "3. Edit\n";
        cout << "4. Delete\n";
        cout << "5. Back\n";

        choice = getValidatedInt("Choice: ", 1, 5);

        switch (choice) {
        case 1: addFeedRecord(); break;
        case 2: viewFeedRecords(); break;
        case 3: editFeedRecord(); break;
        case 4: deleteFeedRecord(); break;
        }
    } while (choice != 5);
}

void CoopTracker::addFeedRecord() {
    printSectionHeader("ADD FEED RECORD");

    string date = promptForDate("Date (MM/DD/YYYY): ");
    string feedType = getLineInput("Feed type: ");
    double quantity = getValidatedDouble("Quantity: ", 0.0, 1000000.0);
    double cost = getValidatedDouble("Cost: ", 0.0, 1000000.0);

    feedRecords.push_back(FeedRecord(date, feedType, quantity, cost));
    saveFeedRecordsToTxt();

    cout << UI::success("Feed record added.") << "\n";
}

void CoopTracker::viewFeedRecords() const {
    if (feedRecords.empty()) {
        printSectionHeader("FEED RECORDS");
        cout << UI::warning("No feed records found.") << "\n";
        return;
    }

    printSectionHeader("VIEW FEED RECORDS");
    cout << "1. View All\n";
    cout << "2. View By Month\n";
    int option = getValidatedInt("Choice: ", 1, 2);

    if (option == 1) {
        printSectionHeader("FEED RECORDS - ALL");
        double totalCost = 0.0;
        for (const auto& record : feedRecords) totalCost += record.getCost();

        cout << fixed << setprecision(2);
        cout << "Total Feed Cost: $" << totalCost << "\n";
        cout << "Records Found: " << feedRecords.size() << "\n\n";

        printFeedRecordList(feedRecords);
    }
    else {
        int month = getValidatedInt("Enter month (1-12): ", 1, 12);
        int year = getValidatedInt("Enter year: ", 1900, 3000);

        vector<FeedRecord> filtered;
        double totalCost = 0.0;

        for (const auto& record : feedRecords) {
            if (isDateInMonthYear(record.getDate(), month, year)) {
                filtered.push_back(record);
                totalCost += record.getCost();
            }
        }

        printSectionHeader("FEED RECORDS - " + monthYearLabel(month, year));

        if (filtered.empty()) {
            cout << "No feed records found for this month.\n";
            return;
        }

        cout << fixed << setprecision(2);
        cout << "Total Feed Cost: $" << totalCost << "\n";
        cout << "Records Found: " << filtered.size() << "\n\n";

        printFeedRecordList(filtered);
    }
}

void CoopTracker::editFeedRecord() {
    if (feedRecords.empty()) {
        cout << UI::warning("No feed records to edit.") << "\n";
        return;
    }

    printSectionHeader("FEED RECORDS");
    printFeedRecordList(feedRecords);

    int choice = getValidatedInt("Select record: ", 1, static_cast<int>(feedRecords.size()));

    string date = promptForDate("New date (MM/DD/YYYY): ");
    string type = getLineInput("New type: ");
    double qty = getValidatedDouble("New qty: ", 0.0, 1000000.0);
    double cost = getValidatedDouble("New cost: ", 0.0, 1000000.0);

    feedRecords[choice - 1].setDate(date);
    feedRecords[choice - 1].setFeedType(type);
    feedRecords[choice - 1].setQuantity(qty);
    feedRecords[choice - 1].setCost(cost);

    saveFeedRecordsToTxt();
    cout << UI::success("Feed record updated.") << "\n";
}

void CoopTracker::deleteFeedRecord() {
    if (feedRecords.empty()) {
        cout << UI::warning("No feed records to delete.") << "\n";
        return;
    }

    printSectionHeader("FEED RECORDS");
    printFeedRecordList(feedRecords);

    int choice = getValidatedInt("Select record to delete: ", 1, static_cast<int>(feedRecords.size()));
    feedRecords.erase(feedRecords.begin() + (choice - 1));
    saveFeedRecordsToTxt();

    cout << UI::success("Feed record deleted.") << "\n";
}

// ================= EXPENSES =================
void CoopTracker::expenseMenu() {
    int choice;
    do {
        printSectionHeader("EXPENSES");
        cout << "1. Add\n";
        cout << "2. View\n";
        cout << "3. Edit\n";
        cout << "4. Delete\n";
        cout << "5. Summary by Category\n";
        cout << "6. Back\n";

        choice = getValidatedInt("Choice: ", 1, 6);

        switch (choice) {
        case 1: addExpense(); break;
        case 2: viewExpenses(); break;
        case 3: editExpense(); break;
        case 4: deleteExpense(); break;
        case 5: showExpenseSummaryByCategory(); break;
        }
    } while (choice != 6);
}

void CoopTracker::addExpense() {
    printSectionHeader("ADD EXPENSE");

    string date = promptForDate("Date (MM/DD/YYYY): ");
    string category = getLineInput("Category: ");
    string description = getLineInput("Description: ");
    double amount = getValidatedDouble("Amount: ", 0.0, 1000000.0);

    expenses.push_back(Expense(date, category, description, amount));
    saveExpensesToTxt();

    cout << UI::success("Expense added.") << "\n";
}

void CoopTracker::viewExpenses() const {
    if (expenses.empty()) {
        printSectionHeader("EXPENSES");
        cout << UI::warning("No expenses found.") << "\n";
        return;
    }

    printSectionHeader("VIEW EXPENSES");
    cout << "1. View All\n";
    cout << "2. View By Month\n";
    int option = getValidatedInt("Choice: ", 1, 2);

    if (option == 1) {
        printSectionHeader("EXPENSES - ALL");

        double total = 0.0;
        for (const auto& expense : expenses) total += expense.getAmount();

        cout << fixed << setprecision(2);
        cout << "Total Spent: $" << total << "\n";
        cout << "Records Found: " << expenses.size() << "\n\n";

        printExpenseList(expenses);
    }
    else {
        int month = getValidatedInt("Enter month (1-12): ", 1, 12);
        int year = getValidatedInt("Enter year: ", 1900, 3000);

        vector<Expense> filtered;
        double total = 0.0;

        for (const auto& expense : expenses) {
            if (isDateInMonthYear(expense.getDate(), month, year)) {
                filtered.push_back(expense);
                total += expense.getAmount();
            }
        }

        printSectionHeader("EXPENSES - " + monthYearLabel(month, year));

        if (filtered.empty()) {
            cout << "No expenses found for this month.\n";
            return;
        }

        cout << fixed << setprecision(2);
        cout << "Total Spent: $" << total << "\n";
        cout << "Records Found: " << filtered.size() << "\n\n";

        printExpenseList(filtered);
    }
}

void CoopTracker::editExpense() {
    if (expenses.empty()) {
        cout << UI::warning("No expenses to edit.") << "\n";
        return;
    }

    printSectionHeader("EXPENSES");
    printExpenseList(expenses);

    int choice = getValidatedInt("Select expense: ", 1, static_cast<int>(expenses.size()));

    string date = promptForDate("New date (MM/DD/YYYY): ");
    string cat = getLineInput("New category: ");
    string desc = getLineInput("New description: ");
    double amt = getValidatedDouble("New amount: ", 0.0, 1000000.0);

    expenses[choice - 1].setDate(date);
    expenses[choice - 1].setCategory(cat);
    expenses[choice - 1].setDescription(desc);
    expenses[choice - 1].setAmount(amt);

    saveExpensesToTxt();
    cout << UI::success("Expense updated.") << "\n";
}

void CoopTracker::deleteExpense() {
    if (expenses.empty()) {
        cout << UI::warning("No expenses to delete.") << "\n";
        return;
    }

    printSectionHeader("EXPENSES");
    printExpenseList(expenses);

    int choice = getValidatedInt("Select expense to delete: ", 1, static_cast<int>(expenses.size()));
    expenses.erase(expenses.begin() + (choice - 1));
    saveExpensesToTxt();

    cout << UI::success("Expense deleted.") << "\n";
}

void CoopTracker::showExpenseSummaryByCategory() const {
    if (expenses.empty()) {
        printSectionHeader("EXPENSE SUMMARY");
        cout << UI::warning("No expenses found.") << "\n";
        return;
    }

    printSectionHeader("EXPENSE SUMMARY");
    cout << "1. All Records\n";
    cout << "2. By Month\n";
    int option = getValidatedInt("Choice: ", 1, 2);

    vector<Expense> source;

    if (option == 1) {
        source = expenses;
        printSectionHeader("EXPENSE SUMMARY - ALL");
    }
    else {
        int month = getValidatedInt("Enter month (1-12): ", 1, 12);
        int year = getValidatedInt("Enter year: ", 1900, 3000);

        for (const auto& expense : expenses) {
            if (isDateInMonthYear(expense.getDate(), month, year)) {
                source.push_back(expense);
            }
        }

        printSectionHeader("EXPENSE SUMMARY - " + monthYearLabel(month, year));
    }

    if (source.empty()) {
        cout << "No matching expenses found.\n";
        return;
    }

    vector<string> categories;
    vector<double> totals;
    double grandTotal = 0.0;

    for (const auto& expense : source) {
        grandTotal += expense.getAmount();

        bool found = false;
        for (size_t i = 0; i < categories.size(); i++) {
            if (categories[i] == expense.getCategory()) {
                totals[i] += expense.getAmount();
                found = true;
                break;
            }
        }

        if (!found) {
            categories.push_back(expense.getCategory());
            totals.push_back(expense.getAmount());
        }
    }

    cout << fixed << setprecision(2);
    cout << "Total Spent: $" << grandTotal << "\n\n";

    for (size_t i = 0; i < categories.size(); i++) {
        cout << categories[i] << ": $" << totals[i] << "\n";
    }
}

// ================= EGG RECORDS =================
void CoopTracker::eggMenu() {
    int choice;
    do {
        printSectionHeader("EGG RECORDS");
        cout << "1. Add\n";
        cout << "2. View\n";
        cout << "3. Edit\n";
        cout << "4. Delete\n";
        cout << "5. Back\n";

        choice = getValidatedInt("Choice: ", 1, 5);

        switch (choice) {
        case 1: addEggRecord(); break;
        case 2: viewEggRecords(); break;
        case 3: editEggRecord(); break;
        case 4: deleteEggRecord(); break;
        }
    } while (choice != 5);
}

void CoopTracker::addEggRecord() {
    printSectionHeader("ADD EGG RECORD");

    string date = promptForDate("Date (MM/DD/YYYY): ");
    int count = getValidatedInt("Egg count: ", 0, 1000000);
    string notes = getLineInput("Notes: ");

    eggRecords.push_back(EggRecord(date, count, notes));
    saveEggRecordsToTxt();

    cout << UI::success("Egg record added.") << "\n";
}

void CoopTracker::viewEggRecords() const {
    if (eggRecords.empty()) {
        printSectionHeader("EGG RECORDS");
        cout << UI::warning("No egg records found.") << "\n";
        return;
    }

    printSectionHeader("VIEW EGG RECORDS");
    cout << "1. View All\n";
    cout << "2. View By Month\n";
    int option = getValidatedInt("Choice: ", 1, 2);

    if (option == 1) {
        printSectionHeader("EGG RECORDS - ALL");

        int totalEggs = 0;
        for (const auto& record : eggRecords) totalEggs += record.getEggCount();

        cout << "Total Eggs: " << totalEggs << "\n";
        cout << "Records Found: " << eggRecords.size() << "\n";
        cout << "Sort Order: Oldest to Newest\n\n";

        printEggRecordListSorted(eggRecords);
    }
    else {
        int month = getValidatedInt("Enter month (1-12): ", 1, 12);
        int year = getValidatedInt("Enter year: ", 1900, 3000);

        vector<EggRecord> filtered;
        int totalEggs = 0;

        for (const auto& record : eggRecords) {
            if (isDateInMonthYear(record.getDate(), month, year)) {
                filtered.push_back(record);
                totalEggs += record.getEggCount();
            }
        }

        printSectionHeader("EGG RECORDS - " + monthYearLabel(month, year));

        if (filtered.empty()) {
            cout << "No egg records found for this month.\n";
            return;
        }

        cout << "Total Eggs: " << totalEggs << "\n";
        cout << "Records Found: " << filtered.size() << "\n";
        cout << "Sort Order: Oldest to Newest\n\n";

        printEggRecordListSorted(filtered);
    }
}

void CoopTracker::editEggRecord() {
    if (eggRecords.empty()) {
        cout << UI::warning("No egg records to edit.") << "\n";
        return;
    }

    vector<pair<int, EggRecord>> indexedRecords;
    for (size_t i = 0; i < eggRecords.size(); i++) {
        indexedRecords.push_back(make_pair(static_cast<int>(i), eggRecords[i]));
    }

    sort(indexedRecords.begin(), indexedRecords.end(),
        [this](const pair<int, EggRecord>& a, const pair<int, EggRecord>& b) {
            return dateToSortableValue(a.second.getDate()) < dateToSortableValue(b.second.getDate());
        });

    printSectionHeader("EGG RECORDS");
    for (size_t i = 0; i < indexedRecords.size(); i++) {
        cout << i + 1 << ". "
            << indexedRecords[i].second.getDate() << " | Count: "
            << indexedRecords[i].second.getEggCount() << " | Notes: "
            << indexedRecords[i].second.getNotes() << "\n";
    }

    int choice = getValidatedInt("Select egg record: ", 1, static_cast<int>(indexedRecords.size()));
    int originalIndex = indexedRecords[choice - 1].first;

    string date = promptForDate("New date (MM/DD/YYYY): ");
    int count = getValidatedInt("New count: ", 0, 1000000);
    string notes = getLineInput("New notes: ");

    eggRecords[originalIndex].setDate(date);
    eggRecords[originalIndex].setEggCount(count);
    eggRecords[originalIndex].setNotes(notes);

    saveEggRecordsToTxt();
    cout << UI::success("Egg record updated.") << "\n";
}

void CoopTracker::deleteEggRecord() {
    if (eggRecords.empty()) {
        cout << UI::warning("No egg records to delete.") << "\n";
        return;
    }

    vector<pair<int, EggRecord>> indexedRecords;
    for (size_t i = 0; i < eggRecords.size(); i++) {
        indexedRecords.push_back(make_pair(static_cast<int>(i), eggRecords[i]));
    }

    sort(indexedRecords.begin(), indexedRecords.end(),
        [this](const pair<int, EggRecord>& a, const pair<int, EggRecord>& b) {
            return dateToSortableValue(a.second.getDate()) < dateToSortableValue(b.second.getDate());
        });

    printSectionHeader("EGG RECORDS");
    for (size_t i = 0; i < indexedRecords.size(); i++) {
        cout << i + 1 << ". "
            << indexedRecords[i].second.getDate() << " | Count: "
            << indexedRecords[i].second.getEggCount() << " | Notes: "
            << indexedRecords[i].second.getNotes() << "\n";
    }

    int choice = getValidatedInt("Select egg record to delete: ", 1, static_cast<int>(indexedRecords.size()));
    int originalIndex = indexedRecords[choice - 1].first;

    eggRecords.erase(eggRecords.begin() + originalIndex);
    saveEggRecordsToTxt();

    cout << UI::success("Egg record deleted.") << "\n";
}

// ================= HEALTH NOTES =================
void CoopTracker::healthMenu() {
    int choice;
    do {
        printSectionHeader("HEALTH NOTES");
        cout << "1. Add\n";
        cout << "2. View\n";
        cout << "3. Edit\n";
        cout << "4. Delete\n";
        cout << "5. Back\n";

        choice = getValidatedInt("Choice: ", 1, 5);

        switch (choice) {
        case 1: addHealthNote(); break;
        case 2: viewHealthNotes(); break;
        case 3: editHealthNote(); break;
        case 4: deleteHealthNote(); break;
        }
    } while (choice != 5);
}

void CoopTracker::addHealthNote() {
    printSectionHeader("ADD HEALTH NOTE");

    string date = promptForDate("Date (MM/DD/YYYY): ");
    string chickenName = getLineInput("Chicken name: ");
    string note = getLineInput("Note: ");

    healthNotes.push_back(HealthNote(date, chickenName, note));
    saveHealthNotesToTxt();

    cout << UI::success("Health note added.") << "\n";
}

void CoopTracker::viewHealthNotes() const {
    if (healthNotes.empty()) {
        printSectionHeader("HEALTH NOTES");
        cout << UI::warning("No health notes found.") << "\n";
        return;
    }

    printSectionHeader("VIEW HEALTH NOTES");
    cout << "1. View All\n";
    cout << "2. View By Month\n";
    int option = getValidatedInt("Choice: ", 1, 2);

    if (option == 1) {
        printSectionHeader("HEALTH NOTES - ALL");
        cout << "Note Count: " << healthNotes.size() << "\n\n";
        printHealthNoteList(healthNotes);
    }
    else {
        int month = getValidatedInt("Enter month (1-12): ", 1, 12);
        int year = getValidatedInt("Enter year: ", 1900, 3000);

        vector<HealthNote> filtered;
        for (const auto& note : healthNotes) {
            if (isDateInMonthYear(note.getDate(), month, year)) {
                filtered.push_back(note);
            }
        }

        printSectionHeader("HEALTH NOTES - " + monthYearLabel(month, year));

        if (filtered.empty()) {
            cout << "No health notes found for this month.\n";
            return;
        }

        cout << "Note Count: " << filtered.size() << "\n\n";
        printHealthNoteList(filtered);
    }
}

void CoopTracker::editHealthNote() {
    if (healthNotes.empty()) {
        cout << UI::warning("No health notes to edit.") << "\n";
        return;
    }

    printSectionHeader("HEALTH NOTES");
    printHealthNoteList(healthNotes);

    int choice = getValidatedInt("Select health note: ", 1, static_cast<int>(healthNotes.size()));

    string date = promptForDate("New date (MM/DD/YYYY): ");
    string name = getLineInput("New chicken: ");
    string note = getLineInput("New note: ");

    healthNotes[choice - 1].setDate(date);
    healthNotes[choice - 1].setChickenName(name);
    healthNotes[choice - 1].setNote(note);

    saveHealthNotesToTxt();
    cout << UI::success("Health note updated.") << "\n";
}

void CoopTracker::deleteHealthNote() {
    if (healthNotes.empty()) {
        cout << UI::warning("No health notes to delete.") << "\n";
        return;
    }

    printSectionHeader("HEALTH NOTES");
    printHealthNoteList(healthNotes);

    int choice = getValidatedInt("Select health note to delete: ", 1, static_cast<int>(healthNotes.size()));
    healthNotes.erase(healthNotes.begin() + (choice - 1));
    saveHealthNotesToTxt();

    cout << UI::success("Health note deleted.") << "\n";
}

// ================= CLEANING RECORDS =================
void CoopTracker::cleaningMenu() {
    int choice;
    do {
        printSectionHeader("CLEANING RECORDS");
        cout << "1. Add\n";
        cout << "2. View\n";
        cout << "3. Edit\n";
        cout << "4. Delete\n";
        cout << "5. Back\n";

        choice = getValidatedInt("Choice: ", 1, 5);

        switch (choice) {
        case 1: addCleaningRecord(); break;
        case 2: viewCleaningRecords(); break;
        case 3: editCleaningRecord(); break;
        case 4: deleteCleaningRecord(); break;
        }
    } while (choice != 5);
}

void CoopTracker::addCleaningRecord() {
    printSectionHeader("ADD CLEANING RECORD");

    string date = promptForDate("Date (MM/DD/YYYY): ");
    string task = getLineInput("Task: ");
    string notes = getLineInput("Notes: ");

    cleaningRecords.push_back(CleaningRecord(date, task, notes));
    saveCleaningRecordsToTxt();

    cout << UI::success("Cleaning record added.") << "\n";
}

void CoopTracker::viewCleaningRecords() const {
    if (cleaningRecords.empty()) {
        printSectionHeader("CLEANING RECORDS");
        cout << UI::warning("No cleaning records found.") << "\n";
        return;
    }

    printSectionHeader("VIEW CLEANING RECORDS");
    cout << "1. View All\n";
    cout << "2. View By Month\n";
    int option = getValidatedInt("Choice: ", 1, 2);

    if (option == 1) {
        printSectionHeader("CLEANING RECORDS - ALL");
        cout << "Cleaning Count: " << cleaningRecords.size() << "\n\n";
        printCleaningRecordList(cleaningRecords);
    }
    else {
        int month = getValidatedInt("Enter month (1-12): ", 1, 12);
        int year = getValidatedInt("Enter year: ", 1900, 3000);

        vector<CleaningRecord> filtered;
        for (const auto& record : cleaningRecords) {
            if (isDateInMonthYear(record.getDate(), month, year)) {
                filtered.push_back(record);
            }
        }

        printSectionHeader("CLEANING RECORDS - " + monthYearLabel(month, year));

        if (filtered.empty()) {
            cout << "No cleaning records found for this month.\n";
            return;
        }

        cout << "Cleaning Count: " << filtered.size() << "\n\n";
        printCleaningRecordList(filtered);
    }
}

void CoopTracker::editCleaningRecord() {
    if (cleaningRecords.empty()) {
        cout << UI::warning("No cleaning records to edit.") << "\n";
        return;
    }

    printSectionHeader("CLEANING RECORDS");
    printCleaningRecordList(cleaningRecords);

    int choice = getValidatedInt("Select cleaning record: ", 1, static_cast<int>(cleaningRecords.size()));

    string date = promptForDate("New date (MM/DD/YYYY): ");
    string task = getLineInput("New task: ");
    string notes = getLineInput("New notes: ");

    cleaningRecords[choice - 1].setDate(date);
    cleaningRecords[choice - 1].setTask(task);
    cleaningRecords[choice - 1].setNotes(notes);

    saveCleaningRecordsToTxt();
    cout << UI::success("Cleaning record updated.") << "\n";
}

void CoopTracker::deleteCleaningRecord() {
    if (cleaningRecords.empty()) {
        cout << UI::warning("No cleaning records to delete.") << "\n";
        return;
    }

    printSectionHeader("CLEANING RECORDS");
    printCleaningRecordList(cleaningRecords);

    int choice = getValidatedInt("Select cleaning record to delete: ", 1, static_cast<int>(cleaningRecords.size()));
    cleaningRecords.erase(cleaningRecords.begin() + (choice - 1));
    saveCleaningRecordsToTxt();

    cout << UI::success("Cleaning record deleted.") << "\n";
}