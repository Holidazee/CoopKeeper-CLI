
#include "CoopTracker.h"
#include "Utils.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

using namespace std;

namespace {
    const string DATA_DIR = "data/";
    const string EXPORT_DIR = "exports/";

    string decimal2(double value) {
        ostringstream out;
        out << fixed << setprecision(2) << value;
        return out.str();
    }

    string labelText(const string& text) {
        return "\033[1;96m" + text + "\033[0m";
    }

    string goodText(const string& text) {
        return "\033[1;32m" + text + "\033[0m";
    }

    string warnText(const string& text) {
        return "\033[1;33m" + text + "\033[0m";
    }

    string badText(const string& text) {
        return "\033[1;31m" + text + "\033[0m";
    }

    bool ensureDirectoryExists(const string& path) {
        // Intentionally simple for wide compiler compatibility.
        // If the folder does not exist, opening a file for write will fail.
        // Users should create the data/ and exports/ folders once in the project directory.
        (void)path;
        return true;
    }
}

void CoopTracker::printSectionHeader(const string& title) const {
    cout << "\n\033[90m============================================================\033[0m\n";
    cout << "\033[1;96m" << title << "\033[0m\n";
    cout << "\033[90m============================================================\033[0m\n";
}

void CoopTracker::pauseForEnter() const {
    cout << "\n\033[1;33mPress Enter to continue...\033[0m";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
}

string CoopTracker::getLineInput(const string& prompt) const {
    cout << prompt;
    string input;
    getline(cin, input);
    return input;
}

int CoopTracker::getValidatedInt(const string& prompt, int min, int max) const {
    while (true) {
        cout << prompt;
        string input;
        getline(cin, input);

        stringstream ss(input);
        int value;
        char extra;

        if (ss >> value && !(ss >> extra) && value >= min && value <= max) {
            return value;
        }

        cout << badText("Invalid input. Please enter a number between " + to_string(min) + " and " + to_string(max) + ".") << "\n";
    }
}

double CoopTracker::getValidatedDouble(const string& prompt, double min, double max) const {
    while (true) {
        cout << prompt;
        string input;
        getline(cin, input);

        stringstream ss(input);
        double value;
        char extra;

        if (ss >> value && !(ss >> extra) && value >= min && value <= max) {
            return value;
        }

        cout << badText("Invalid input. Please enter a value between " + decimal2(min) + " and " + decimal2(max) + ".") << "\n";
    }
}

string CoopTracker::promptForDate(const string& prompt) const {
    while (true) {
        string input = getLineInput(prompt);
        int month, day, year;
        if (parseDate(input, month, day, year)) {
            return input;
        }
        cout << badText("Invalid date format. Please use MM/DD/YYYY.") << "\n";
    }
}

bool CoopTracker::parseDate(const string& date, int& month, int& day, int& year) const {
    if (date.size() != 10 || date[2] != '/' || date[5] != '/') {
        return false;
    }

    try {
        month = stoi(date.substr(0, 2));
        day = stoi(date.substr(3, 2));
        year = stoi(date.substr(6, 4));
    }
    catch (...) {
        return false;
    }

    if (year < 1900 || year > 3000 || month < 1 || month > 12 || day < 1 || day > 31) {
        return false;
    }

    static const int daysInMonth[] = { 31,28,31,30,31,30,31,31,30,31,30,31 };
    int maxDay = daysInMonth[month - 1];
    bool leapYear = (year % 400 == 0) || (year % 4 == 0 && year % 100 != 0);
    if (month == 2 && leapYear) {
        maxDay = 29;
    }

    return day <= maxDay;
}

bool CoopTracker::isDateInMonthYear(const string& date, int month, int year) const {
    int m, d, y;
    return parseDate(date, m, d, y) && m == month && y == year;
}

bool CoopTracker::isDateInYear(const string& date, int year) const {
    int m, d, y;
    return parseDate(date, m, d, y) && y == year;
}

bool CoopTracker::isDateInRange(const string& date, const string& startDate, const string& endDate) const {
    int value = dateToSortableValue(date);
    return value >= dateToSortableValue(startDate) && value <= dateToSortableValue(endDate);
}

int CoopTracker::dateToSortableValue(const string& date) const {
    int month, day, year;
    if (!parseDate(date, month, day, year)) {
        return 0;
    }
    return year * 10000 + month * 100 + day;
}

string CoopTracker::monthYearLabel(int month, int year) const {
    static const string names[] = {
        "January", "February", "March", "April", "May", "June",
        "July", "August", "September", "October", "November", "December"
    };

    if (month < 1 || month > 12) {
        return "Unknown " + to_string(year);
    }

    return names[month - 1] + " " + to_string(year);
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
        return "None";
    }

    string latest = cleaningRecords.front().getDate();
    for (const auto& record : cleaningRecords) {
        if (dateToSortableValue(record.getDate()) > dateToSortableValue(latest)) {
            latest = record.getDate();
        }
    }
    return latest;
}

int CoopTracker::getDaysBetween(const string& earlierDate, const string& laterDate) const {
    int em, ed, ey, lm, ld, ly;
    if (!parseDate(earlierDate, em, ed, ey) || !parseDate(laterDate, lm, ld, ly)) {
        return 0;
    }

    tm a = {};
    a.tm_year = ey - 1900;
    a.tm_mon = em - 1;
    a.tm_mday = ed;
    a.tm_hour = 12;

    tm b = {};
    b.tm_year = ly - 1900;
    b.tm_mon = lm - 1;
    b.tm_mday = ld;
    b.tm_hour = 12;

    time_t ta = mktime(&a);
    time_t tb = mktime(&b);

    if (ta == -1 || tb == -1) {
        return 0;
    }

    double diff = difftime(tb, ta);
    return static_cast<int>(diff / (60 * 60 * 24));
}

int CoopTracker::getDaysSinceDate(const string& date) const {
    time_t now = time(nullptr);
    tm localTime{};
#ifdef _WIN32
    localtime_s(&localTime, &now);
#else
    localTime = *localtime(&now);
#endif

    ostringstream today;
    today << setfill('0') << setw(2) << (localTime.tm_mon + 1) << "/"
        << setw(2) << localTime.tm_mday << "/"
        << setw(4) << (localTime.tm_year + 1900);

    return getDaysBetween(date, today.str());
}

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

void CoopTracker::loadChickensFromTxt() {
    chickens.clear();
    ifstream file(DATA_DIR + "Chickens.txt");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (Utils::trim(line).empty()) continue;
        vector<string> parts = Utils::split(line, '|');
        if (parts.size() >= 4) {
            chickens.push_back(Chicken(
                Utils::trim(parts[0]),
                Utils::trim(parts[1]),
                Utils::toInt(Utils::trim(parts[2])),
                Utils::trim(parts[3])
            ));
        }
    }
}

void CoopTracker::loadFeedRecordsFromTxt() {
    feedRecords.clear();
    ifstream file(DATA_DIR + "FeedRecords.txt");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (Utils::trim(line).empty()) continue;
        vector<string> parts = Utils::split(line, '|');
        if (parts.size() >= 4) {
            feedRecords.push_back(FeedRecord(
                Utils::trim(parts[0]),
                Utils::trim(parts[1]),
                Utils::toDouble(Utils::trim(parts[2])),
                Utils::toDouble(Utils::trim(parts[3]))
            ));
        }
    }
}

void CoopTracker::loadExpensesFromTxt() {
    expenses.clear();
    ifstream file(DATA_DIR + "Expenses.txt");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (Utils::trim(line).empty()) continue;
        vector<string> parts = Utils::split(line, '|');
        if (parts.size() >= 4) {
            expenses.push_back(Expense(
                Utils::trim(parts[0]),
                Utils::trim(parts[1]),
                Utils::trim(parts[2]),
                Utils::toDouble(Utils::trim(parts[3]))
            ));
        }
    }
}

void CoopTracker::loadEggRecordsFromTxt() {
    eggRecords.clear();
    ifstream file(DATA_DIR + "EggRecords.txt");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (Utils::trim(line).empty()) continue;
        vector<string> parts = Utils::split(line, '|');
        if (parts.size() >= 3) {
            eggRecords.push_back(EggRecord(
                Utils::trim(parts[0]),
                Utils::toInt(Utils::trim(parts[1])),
                Utils::trim(parts[2])
            ));
        }
    }
}

void CoopTracker::loadHealthNotesFromTxt() {
    healthNotes.clear();
    ifstream file(DATA_DIR + "HealthNotes.txt");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (Utils::trim(line).empty()) continue;
        vector<string> parts = Utils::split(line, '|');
        if (parts.size() >= 3) {
            healthNotes.push_back(HealthNote(
                Utils::trim(parts[0]),
                Utils::trim(parts[1]),
                Utils::trim(parts[2])
            ));
        }
    }
}

void CoopTracker::loadCleaningRecordsFromTxt() {
    cleaningRecords.clear();
    ifstream file(DATA_DIR + "CleaningRecords.txt");
    if (!file.is_open()) return;

    string line;
    while (getline(file, line)) {
        if (Utils::trim(line).empty()) continue;
        vector<string> parts = Utils::split(line, '|');
        if (parts.size() >= 3) {
            cleaningRecords.push_back(CleaningRecord(
                Utils::trim(parts[0]),
                Utils::trim(parts[1]),
                Utils::trim(parts[2])
            ));
        }
    }
}

void CoopTracker::saveChickensToTxt() const {
    ensureDirectoryExists(DATA_DIR);
    ofstream file(DATA_DIR + "Chickens.txt");
    for (const auto& chicken : chickens) {
        file << chicken.getName() << "|"
            << chicken.getBreed() << "|"
            << chicken.getAge() << "|"
            << chicken.getNotes() << "\n";
    }
}

void CoopTracker::saveFeedRecordsToTxt() const {
    ensureDirectoryExists(DATA_DIR);
    ofstream file(DATA_DIR + "FeedRecords.txt");
    for (const auto& record : feedRecords) {
        file << record.getDate() << "|"
            << record.getFeedType() << "|"
            << fixed << setprecision(2) << record.getQuantity() << "|"
            << fixed << setprecision(2) << record.getCost() << "\n";
    }
}

void CoopTracker::saveExpensesToTxt() const {
    ensureDirectoryExists(DATA_DIR);
    ofstream file(DATA_DIR + "Expenses.txt");
    for (const auto& expense : expenses) {
        file << expense.getDate() << "|"
            << expense.getCategory() << "|"
            << expense.getDescription() << "|"
            << fixed << setprecision(2) << expense.getAmount() << "\n";
    }
}

void CoopTracker::saveEggRecordsToTxt() const {
    ensureDirectoryExists(DATA_DIR);

    vector<EggRecord> sorted = eggRecords;
    sort(sorted.begin(), sorted.end(), [this](const EggRecord& a, const EggRecord& b) {
        return dateToSortableValue(a.getDate()) < dateToSortableValue(b.getDate());
        });

    ofstream file(DATA_DIR + "EggRecords.txt");
    for (const auto& record : sorted) {
        file << record.getDate() << "|"
            << record.getEggCount() << "|"
            << record.getNotes() << "\n";
    }
}

void CoopTracker::saveHealthNotesToTxt() const {
    ensureDirectoryExists(DATA_DIR);
    ofstream file(DATA_DIR + "HealthNotes.txt");
    for (const auto& note : healthNotes) {
        file << note.getDate() << "|"
            << note.getChickenName() << "|"
            << note.getNote() << "\n";
    }
}

void CoopTracker::saveCleaningRecordsToTxt() const {
    ensureDirectoryExists(DATA_DIR);
    ofstream file(DATA_DIR + "CleaningRecords.txt");
    for (const auto& record : cleaningRecords) {
        file << record.getDate() << "|"
            << record.getTask() << "|"
            << record.getNotes() << "\n";
    }
}

string CoopTracker::escapeCSV(const string& value) const {
    return Utils::escapeCSV(value);
}

void CoopTracker::exportChickensToCSV() const {
    ofstream file(EXPORT_DIR + "Chickens.csv");
    if (!file.is_open()) return;

    file << "Name,Breed,Age,Notes\n";
    for (const auto& chicken : chickens) {
        file << escapeCSV(chicken.getName()) << ","
            << escapeCSV(chicken.getBreed()) << ","
            << chicken.getAge() << ","
            << escapeCSV(chicken.getNotes()) << "\n";
    }
}

void CoopTracker::exportFeedRecordsToCSV() const {
    ofstream file(EXPORT_DIR + "FeedRecords.csv");
    if (!file.is_open()) return;

    file << "Date,Feed Type,Quantity,Cost\n";
    for (const auto& record : feedRecords) {
        file << escapeCSV(record.getDate()) << ","
            << escapeCSV(record.getFeedType()) << ","
            << fixed << setprecision(2) << record.getQuantity() << ","
            << fixed << setprecision(2) << record.getCost() << "\n";
    }
}

void CoopTracker::exportExpensesToCSV() const {
    ofstream file(EXPORT_DIR + "Expenses.csv");
    if (!file.is_open()) return;

    file << "Date,Category,Description,Amount\n";
    for (const auto& expense : expenses) {
        file << escapeCSV(expense.getDate()) << ","
            << escapeCSV(expense.getCategory()) << ","
            << escapeCSV(expense.getDescription()) << ","
            << fixed << setprecision(2) << expense.getAmount() << "\n";
    }
}

void CoopTracker::exportEggRecordsToCSV() const {
    ofstream file(EXPORT_DIR + "EggRecords.csv");
    if (!file.is_open()) return;

    file << "Date,Egg Count,Notes\n";
    vector<EggRecord> sorted = eggRecords;
    sort(sorted.begin(), sorted.end(), [this](const EggRecord& a, const EggRecord& b) {
        return dateToSortableValue(a.getDate()) < dateToSortableValue(b.getDate());
        });

    for (const auto& record : sorted) {
        file << escapeCSV(record.getDate()) << ","
            << record.getEggCount() << ","
            << escapeCSV(record.getNotes()) << "\n";
    }
}

void CoopTracker::exportHealthNotesToCSV() const {
    ofstream file(EXPORT_DIR + "HealthNotes.csv");
    if (!file.is_open()) return;

    file << "Date,Chicken Name,Note\n";
    for (const auto& note : healthNotes) {
        file << escapeCSV(note.getDate()) << ","
            << escapeCSV(note.getChickenName()) << ","
            << escapeCSV(note.getNote()) << "\n";
    }
}

void CoopTracker::exportCleaningRecordsToCSV() const {
    ofstream file(EXPORT_DIR + "CleaningRecords.csv");
    if (!file.is_open()) return;

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
    cout << goodText("Export complete. CSV files written to the exports folder.") << "\n";
}

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
    int total = 0;
    for (const auto& note : healthNotes) {
        if (isDateInMonthYear(note.getDate(), month, year)) {
            ++total;
        }
    }
    return total;
}

int CoopTracker::getHealthCountForYear(int year) const {
    int total = 0;
    for (const auto& note : healthNotes) {
        if (isDateInYear(note.getDate(), year)) {
            ++total;
        }
    }
    return total;
}

int CoopTracker::getCleaningCountForMonth(int month, int year) const {
    int total = 0;
    for (const auto& record : cleaningRecords) {
        if (isDateInMonthYear(record.getDate(), month, year)) {
            ++total;
        }
    }
    return total;
}

int CoopTracker::getCleaningCountForYear(int year) const {
    int total = 0;
    for (const auto& record : cleaningRecords) {
        if (isDateInYear(record.getDate(), year)) {
            ++total;
        }
    }
    return total;
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
    int totalEggs = 0;
    int dayCount = 0;

    for (const auto& record : eggRecords) {
        if (isDateInMonthYear(record.getDate(), month, year)) {
            totalEggs += record.getEggCount();
            ++dayCount;
        }
    }

    if (dayCount == 0) return 0.0;
    return static_cast<double>(totalEggs) / dayCount;
}

string CoopTracker::getBestProductionDayForMonth(int month, int year) const {
    string bestDate = "None";
    int bestEggs = -1;

    for (const auto& record : eggRecords) {
        if (isDateInMonthYear(record.getDate(), month, year) && record.getEggCount() > bestEggs) {
            bestEggs = record.getEggCount();
            bestDate = record.getDate() + " (" + to_string(record.getEggCount()) + " eggs)";
        }
    }

    return bestDate;
}

double CoopTracker::getProductionChangePercentFromPreviousMonth(int month, int year) const {
    int previousMonth = month - 1;
    int previousYear = year;
    if (previousMonth == 0) {
        previousMonth = 12;
        --previousYear;
    }

    double currentAverage = getAverageEggsPerDayForMonth(month, year);
    double previousAverage = getAverageEggsPerDayForMonth(previousMonth, previousYear);

    if (previousAverage <= 0.0) {
        return currentAverage > 0.0 ? 100.0 : 0.0;
    }

    return ((currentAverage - previousAverage) / previousAverage) * 100.0;
}

void CoopTracker::printFeedRecordList(const vector<FeedRecord>& records) const {
    if (records.empty()) {
        cout << warnText("No feed records found.") << "\n";
        return;
    }

    for (size_t i = 0; i < records.size(); ++i) {
        cout << i + 1 << ". " << records[i].getDate()
            << " | Type: " << records[i].getFeedType()
            << " | Qty: " << fixed << setprecision(2) << records[i].getQuantity()
            << " | Cost: $" << fixed << setprecision(2) << records[i].getCost() << "\n";
    }
}

void CoopTracker::printExpenseList(const vector<Expense>& records) const {
    if (records.empty()) {
        cout << warnText("No expenses found.") << "\n";
        return;
    }

    for (size_t i = 0; i < records.size(); ++i) {
        cout << i + 1 << ". " << records[i].getDate()
            << " | " << records[i].getCategory()
            << " | " << records[i].getDescription()
            << " | $" << fixed << setprecision(2) << records[i].getAmount() << "\n";
    }
}

void CoopTracker::printEggRecordListSorted(const vector<EggRecord>& records) const {
    if (records.empty()) {
        cout << warnText("No egg records found.") << "\n";
        return;
    }

    vector<EggRecord> sorted = records;
    sort(sorted.begin(), sorted.end(), [this](const EggRecord& a, const EggRecord& b) {
        return dateToSortableValue(a.getDate()) < dateToSortableValue(b.getDate());
        });

    for (size_t i = 0; i < sorted.size(); ++i) {
        cout << i + 1 << ". " << sorted[i].getDate()
            << " | Eggs: " << sorted[i].getEggCount()
            << " | Notes: " << sorted[i].getNotes() << "\n";
    }
}

void CoopTracker::printHealthNoteList(const vector<HealthNote>& records) const {
    if (records.empty()) {
        cout << warnText("No health notes found.") << "\n";
        return;
    }

    for (size_t i = 0; i < records.size(); ++i) {
        cout << i + 1 << ". " << records[i].getDate()
            << " | " << records[i].getChickenName()
            << " | " << records[i].getNote() << "\n";
    }
}

void CoopTracker::printCleaningRecordList(const vector<CleaningRecord>& records) const {
    if (records.empty()) {
        cout << warnText("No cleaning records found.") << "\n";
        return;
    }

    for (size_t i = 0; i < records.size(); ++i) {
        cout << i + 1 << ". " << records[i].getDate()
            << " | Task: " << records[i].getTask()
            << " | Notes: " << records[i].getNotes() << "\n";
    }
}

void CoopTracker::showStartupStatus() const {
    printSectionHeader("STARTUP STATUS");
    cout << labelText("Chickens loaded: ") << chickens.size() << "\n";
    cout << labelText("Feed records loaded: ") << feedRecords.size() << "\n";
    cout << labelText("Expenses loaded: ") << expenses.size() << "\n";
    cout << labelText("Egg records loaded: ") << eggRecords.size() << "\n";
    cout << labelText("Health notes loaded: ") << healthNotes.size() << "\n";
    cout << labelText("Cleaning records loaded: ") << cleaningRecords.size() << "\n";
}

void CoopTracker::showDashboard() const {
    printSectionHeader("COOP DASHBOARD");

    int currentMonth = getCurrentMonth();
    int currentYear = getCurrentYear();

    vector<EggRecord> sortedEggs = eggRecords;
    sort(sortedEggs.begin(), sortedEggs.end(), [this](const EggRecord& a, const EggRecord& b) {
        return dateToSortableValue(a.getDate()) < dateToSortableValue(b.getDate());
        });

    int hens = static_cast<int>(chickens.size());
    int eggsToday = 0;
    string latestDate = "None";

    if (!sortedEggs.empty()) {
        latestDate = sortedEggs.back().getDate();
        eggsToday = sortedEggs.back().getEggCount();
    }

    int eggsLast7Days = 0;
    if (!latestDate.empty() && latestDate != "None") {
        for (const auto& record : sortedEggs) {
            int days = getDaysBetween(record.getDate(), latestDate);
            if (days >= 0 && days < 7) {
                eggsLast7Days += record.getEggCount();
            }
        }
    }

    int eggsThisMonth = getEggTotalForMonth(currentMonth, currentYear);
    double avgDailyThisMonth = getAverageEggsPerDayForMonth(currentMonth, currentYear);

    double layRateToday = (hens > 0) ? (static_cast<double>(eggsToday) / hens) * 100.0 : 0.0;
    double weeklyLayRate = (hens > 0) ? (static_cast<double>(eggsLast7Days) / (hens * 7.0)) * 100.0 : 0.0;
    double monthlyLayRate = (hens > 0 && avgDailyThisMonth > 0.0) ? (avgDailyThisMonth / hens) * 100.0 : 0.0;

    auto statusFromRate = [](double rate) {
        if (rate >= 85.0) return goodText("Excellent");
        if (rate >= 70.0) return goodText("Good");
        if (rate >= 50.0) return warnText("Fair");
        return badText("Low");
        };

    double costPerDozen = getCostPerDozenForMonth(currentMonth, currentYear);
    double change = getProductionChangePercentFromPreviousMonth(currentMonth, currentYear);
    string trend;
    if (change > 0.01) {
        trend = "[UP] +" + decimal2(change) + "%";
    }
    else if (change < -0.01) {
        trend = "[DOWN] " + decimal2(change) + "%";
    }
    else {
        trend = "[FLAT] 0.00%";
    }

    cout << labelText("Total Hens: ") << hens << "\n";
    cout << labelText("Latest Egg Date: ") << latestDate << "\n";
    cout << labelText("Eggs Today: ") << eggsToday << "\n";
    cout << labelText("Eggs Last 7 Days: ") << eggsLast7Days << "\n";
    cout << labelText("Eggs This Month: ") << eggsThisMonth << "\n\n";

    cout << labelText("Today's Lay Rate: ") << decimal2(layRateToday) << "% (" << eggsToday << "/" << hens << ") - " << statusFromRate(layRateToday) << "\n";
    cout << labelText("Weekly Avg Lay Rate: ") << decimal2(weeklyLayRate) << "% - " << statusFromRate(weeklyLayRate) << "\n";
    cout << labelText("Monthly Avg Lay Rate: ") << decimal2(monthlyLayRate) << "% - " << statusFromRate(monthlyLayRate) << "\n";
    cout << labelText("Average Eggs / Day This Month: ") << decimal2(avgDailyThisMonth) << "\n\n";

    cout << labelText("Feed Cost This Month: $") << decimal2(getFeedCostTotalForMonth(currentMonth, currentYear)) << "\n";
    cout << labelText("Other Expenses This Month: $") << decimal2(getExpenseTotalForMonth(currentMonth, currentYear)) << "\n";
    cout << labelText("Cost Per Egg This Month: $") << decimal2(getCostPerEggForMonth(currentMonth, currentYear)) << "\n";
    cout << labelText("Cost Per Dozen This Month: $") << decimal2(costPerDozen) << "\n\n";

    cout << labelText("Best Production Day This Month: ") << getBestProductionDayForMonth(currentMonth, currentYear) << "\n";
    cout << labelText("Production Trend vs Previous Month: ") << trend << "\n";
    cout << labelText("Last Cleaning Date: ") << getLastCleaningDate() << "\n";
}

void CoopTracker::showInsights() const {
    printSectionHeader("INSIGHTS");

    int month = getCurrentMonth();
    int year = getCurrentYear();

    cout << labelText("Month: ") << monthYearLabel(month, year) << "\n";
    cout << labelText("Average Eggs / Day: ") << decimal2(getAverageEggsPerDayForMonth(month, year)) << "\n";
    cout << labelText("Best Day: ") << getBestProductionDayForMonth(month, year) << "\n";
    cout << labelText("Cost / Dozen: $") << decimal2(getCostPerDozenForMonth(month, year)) << "\n";
}

void CoopTracker::showAlerts() const {
    printSectionHeader("ALERTS");

    bool any = false;
    int month = getCurrentMonth();
    int year = getCurrentYear();

    double monthlyLayRate = 0.0;
    int hens = static_cast<int>(chickens.size());
    double avgDaily = getAverageEggsPerDayForMonth(month, year);
    if (hens > 0) {
        monthlyLayRate = (avgDaily / hens) * 100.0;
    }

    if (hens > 0 && monthlyLayRate < 55.0) {
        cout << warnText("Lay rate is lower than expected this month.") << "\n";
        any = true;
    }

    string lastCleaning = getLastCleaningDate();
    if (lastCleaning != "None" && getDaysSinceDate(lastCleaning) > 14) {
        cout << warnText("Cleaning may be overdue. Last cleaning: " + lastCleaning) << "\n";
        any = true;
    }

    if (eggRecords.empty()) {
        cout << warnText("No egg records found.") << "\n";
        any = true;
    }

    if (!any) {
        cout << goodText("No current alerts.") << "\n";
    }
}

void CoopTracker::showMonthlyReport() const {
    printSectionHeader("MONTHLY REPORT");

    int month = getValidatedInt("Enter month (1-12): ", 1, 12);
    int year = getValidatedInt("Enter year: ", 1900, 3000);

    cout << labelText("Report Period: ") << monthYearLabel(month, year) << "\n";
    cout << labelText("Egg Total: ") << getEggTotalForMonth(month, year) << "\n";
    cout << labelText("Average Eggs / Day: ") << decimal2(getAverageEggsPerDayForMonth(month, year)) << "\n";
    cout << labelText("Feed Cost: $") << decimal2(getFeedCostTotalForMonth(month, year)) << "\n";
    cout << labelText("Other Expenses: $") << decimal2(getExpenseTotalForMonth(month, year)) << "\n";
    cout << labelText("Health Notes: ") << getHealthCountForMonth(month, year) << "\n";
    cout << labelText("Cleaning Records: ") << getCleaningCountForMonth(month, year) << "\n";
    cout << labelText("Cost / Egg: $") << decimal2(getCostPerEggForMonth(month, year)) << "\n";
    cout << labelText("Cost / Dozen: $") << decimal2(getCostPerDozenForMonth(month, year)) << "\n";
    cout << labelText("Best Production Day: ") << getBestProductionDayForMonth(month, year) << "\n";
    cout << labelText("Change vs Previous Month: ") << decimal2(getProductionChangePercentFromPreviousMonth(month, year)) << "%\n";
}

void CoopTracker::showYearlyReport() const {
    printSectionHeader("YEARLY REPORT");

    int year = getValidatedInt("Enter year: ", 1900, 3000);

    cout << labelText("Year: ") << year << "\n";
    cout << labelText("Egg Total: ") << getEggTotalForYear(year) << "\n";
    cout << labelText("Feed Cost: $") << decimal2(getFeedCostTotalForYear(year)) << "\n";
    cout << labelText("Other Expenses: $") << decimal2(getExpenseTotalForYear(year)) << "\n";
    cout << labelText("Health Notes: ") << getHealthCountForYear(year) << "\n";
    cout << labelText("Cleaning Records: ") << getCleaningCountForYear(year) << "\n";
    cout << labelText("Cost / Egg: $") << decimal2(getCostPerEggForYear(year)) << "\n";
    cout << labelText("Cost / Dozen: $") << decimal2(getCostPerDozenForYear(year)) << "\n";
}

void CoopTracker::showDateRangeReport() const {
    printSectionHeader("DATE RANGE REPORT");

    string startDate = promptForDate("Enter start date (MM/DD/YYYY): ");
    string endDate = promptForDate("Enter end date (MM/DD/YYYY): ");

    int eggs = 0;
    double feedCost = 0.0;
    double expenseCost = 0.0;
    int healthCount = 0;
    int cleaningCount = 0;

    for (const auto& record : eggRecords) {
        if (isDateInRange(record.getDate(), startDate, endDate)) {
            eggs += record.getEggCount();
        }
    }

    for (const auto& record : feedRecords) {
        if (isDateInRange(record.getDate(), startDate, endDate)) {
            feedCost += record.getCost();
        }
    }

    for (const auto& expense : expenses) {
        if (isDateInRange(expense.getDate(), startDate, endDate)) {
            expenseCost += expense.getAmount();
        }
    }

    for (const auto& note : healthNotes) {
        if (isDateInRange(note.getDate(), startDate, endDate)) {
            ++healthCount;
        }
    }

    for (const auto& record : cleaningRecords) {
        if (isDateInRange(record.getDate(), startDate, endDate)) {
            ++cleaningCount;
        }
    }

    double totalCost = feedCost + expenseCost;
    double costPerEgg = eggs > 0 ? totalCost / eggs : 0.0;

    cout << labelText("Range: ") << startDate << " to " << endDate << "\n";
    cout << labelText("Eggs: ") << eggs << "\n";
    cout << labelText("Feed Cost: $") << decimal2(feedCost) << "\n";
    cout << labelText("Other Expenses: $") << decimal2(expenseCost) << "\n";
    cout << labelText("Cost / Egg: $") << decimal2(costPerEgg) << "\n";
    cout << labelText("Cost / Dozen: $") << decimal2(costPerEgg * 12.0) << "\n";
    cout << labelText("Health Notes: ") << healthCount << "\n";
    cout << labelText("Cleaning Records: ") << cleaningCount << "\n";
}

void CoopTracker::run() {
    loadAllData();

    showStartupStatus();
    showDashboard();
    showAlerts();
    pauseForEnter();

    while (true) {
        cout << "\n\033[90m------------------------------------------------------------\033[0m\n";
        cout << "\033[1;96mMAIN MENU\033[0m\n";
        cout << "1. Chickens\n";
        cout << "2. Feed Records\n";
        cout << "3. Expenses\n";
        cout << "4. Egg Records\n";
        cout << "5. Health Notes\n";
        cout << "6. Cleaning Records\n";
        cout << "7. Dashboard / Insights\n";
        cout << "8. Reports\n";
        cout << "9. Export All to CSV\n";
        cout << "0. Exit\n";

        int choice = getValidatedInt("Choose an option: ", 0, 9);

        switch (choice) {
        case 1: chickenMenu(); break;
        case 2: feedMenu(); break;
        case 3: expenseMenu(); break;
        case 4: eggMenu(); break;
        case 5: healthMenu(); break;
        case 6: cleaningMenu(); break;
        case 7:
            showDashboard();
            showInsights();
            showAlerts();
            pauseForEnter();
            break;
        case 8: {
            printSectionHeader("REPORTS");
            cout << "1. Monthly Report\n";
            cout << "2. Yearly Report\n";
            cout << "3. Date Range Report\n";
            cout << "0. Back\n";
            int reportChoice = getValidatedInt("Choose an option: ", 0, 3);
            if (reportChoice == 1) showMonthlyReport();
            else if (reportChoice == 2) showYearlyReport();
            else if (reportChoice == 3) showDateRangeReport();
            if (reportChoice != 0) pauseForEnter();
            break;
        }
        case 9:
            exportAllToCSV();
            pauseForEnter();
            break;
        case 0:
            saveAllData();
            return;
        }
    }
}

void CoopTracker::chickenMenu() {
    while (true) {
        printSectionHeader("CHICKENS");
        cout << "1. Add Chicken\n";
        cout << "2. View Chickens\n";
        cout << "3. Edit Chicken\n";
        cout << "4. Delete Chicken\n";
        cout << "0. Back\n";

        int choice = getValidatedInt("Choose an option: ", 0, 4);
        if (choice == 0) return;
        if (choice == 1) addChicken();
        else if (choice == 2) viewChickens();
        else if (choice == 3) editChicken();
        else if (choice == 4) deleteChicken();
        pauseForEnter();
    }
}

void CoopTracker::feedMenu() {
    while (true) {
        printSectionHeader("FEED RECORDS");
        cout << "1. Add Feed Record\n";
        cout << "2. View Feed Records\n";
        cout << "3. View Feed Records by Month\n";
        cout << "4. Edit Feed Record\n";
        cout << "5. Delete Feed Record\n";
        cout << "0. Back\n";

        int choice = getValidatedInt("Choose an option: ", 0, 5);
        if (choice == 0) return;
        if (choice == 1) addFeedRecord();
        else if (choice == 2) viewFeedRecords();
        else if (choice == 3) viewFeedRecordsByMonth();
        else if (choice == 4) editFeedRecord();
        else if (choice == 5) deleteFeedRecord();
        pauseForEnter();
    }
}

void CoopTracker::expenseMenu() {
    while (true) {
        printSectionHeader("EXPENSES");
        cout << "1. Add Expense\n";
        cout << "2. View Expenses\n";
        cout << "3. View Expenses by Month\n";
        cout << "4. Edit Expense\n";
        cout << "5. Delete Expense\n";
        cout << "6. Expense Summary by Category\n";
        cout << "0. Back\n";

        int choice = getValidatedInt("Choose an option: ", 0, 6);
        if (choice == 0) return;
        if (choice == 1) addExpense();
        else if (choice == 2) viewExpenses();
        else if (choice == 3) viewExpensesByMonth();
        else if (choice == 4) editExpense();
        else if (choice == 5) deleteExpense();
        else if (choice == 6) showExpenseSummaryByCategory();
        pauseForEnter();
    }
}

void CoopTracker::eggMenu() {
    while (true) {
        printSectionHeader("EGG RECORDS");
        cout << "1. Add Egg Record\n";
        cout << "2. View Egg Records\n";
        cout << "3. View Egg Records by Month\n";
        cout << "4. Edit Egg Record\n";
        cout << "5. Delete Egg Record\n";
        cout << "0. Back\n";

        int choice = getValidatedInt("Choose an option: ", 0, 5);
        if (choice == 0) return;
        if (choice == 1) addEggRecord();
        else if (choice == 2) viewEggRecords();
        else if (choice == 3) viewEggRecordsByMonth();
        else if (choice == 4) editEggRecord();
        else if (choice == 5) deleteEggRecord();
        pauseForEnter();
    }
}

void CoopTracker::healthMenu() {
    while (true) {
        printSectionHeader("HEALTH NOTES");
        cout << "1. Add Health Note\n";
        cout << "2. View Health Notes\n";
        cout << "3. View Health Notes by Month\n";
        cout << "4. Edit Health Note\n";
        cout << "5. Delete Health Note\n";
        cout << "0. Back\n";

        int choice = getValidatedInt("Choose an option: ", 0, 5);
        if (choice == 0) return;
        if (choice == 1) addHealthNote();
        else if (choice == 2) viewHealthNotes();
        else if (choice == 3) viewHealthNotesByMonth();
        else if (choice == 4) editHealthNote();
        else if (choice == 5) deleteHealthNote();
        pauseForEnter();
    }
}

void CoopTracker::cleaningMenu() {
    while (true) {
        printSectionHeader("CLEANING RECORDS");
        cout << "1. Add Cleaning Record\n";
        cout << "2. View Cleaning Records\n";
        cout << "3. View Cleaning Records by Month\n";
        cout << "4. Edit Cleaning Record\n";
        cout << "5. Delete Cleaning Record\n";
        cout << "0. Back\n";

        int choice = getValidatedInt("Choose an option: ", 0, 5);
        if (choice == 0) return;
        if (choice == 1) addCleaningRecord();
        else if (choice == 2) viewCleaningRecords();
        else if (choice == 3) viewCleaningRecordsByMonth();
        else if (choice == 4) editCleaningRecord();
        else if (choice == 5) deleteCleaningRecord();
        pauseForEnter();
    }
}


void CoopTracker::viewFeedRecordsByMonth() const {
    printSectionHeader("FEED RECORDS BY MONTH");
    if (feedRecords.empty()) {
        cout << warnText("No feed records found.") << "\n";
        return;
    }

    int month = getValidatedInt("Enter month (1-12): ", 1, 12);
    int year = getValidatedInt("Enter year: ", 1900, 3000);

    vector<FeedRecord> filtered;
    for (const auto& record : feedRecords) {
        if (isDateInMonthYear(record.getDate(), month, year)) {
            filtered.push_back(record);
        }
    }

    if (filtered.empty()) {
        cout << warnText("No feed records found for " + monthYearLabel(month, year) + ".") << "\n";
        return;
    }

    sort(filtered.begin(), filtered.end(), [this](const FeedRecord& a, const FeedRecord& b) {
        return dateToSortableValue(a.getDate()) < dateToSortableValue(b.getDate());
        });

    cout << labelText("Showing records for: ") << monthYearLabel(month, year) << "\n\n";
    printFeedRecordList(filtered);
}

void CoopTracker::viewExpensesByMonth() const {
    printSectionHeader("EXPENSES BY MONTH");
    if (expenses.empty()) {
        cout << warnText("No expenses found.") << "\n";
        return;
    }

    int month = getValidatedInt("Enter month (1-12): ", 1, 12);
    int year = getValidatedInt("Enter year: ", 1900, 3000);

    vector<Expense> filtered;
    for (const auto& expense : expenses) {
        if (isDateInMonthYear(expense.getDate(), month, year)) {
            filtered.push_back(expense);
        }
    }

    if (filtered.empty()) {
        cout << warnText("No expenses found for " + monthYearLabel(month, year) + ".") << "\n";
        return;
    }

    sort(filtered.begin(), filtered.end(), [this](const Expense& a, const Expense& b) {
        return dateToSortableValue(a.getDate()) < dateToSortableValue(b.getDate());
        });

    cout << labelText("Showing records for: ") << monthYearLabel(month, year) << "\n\n";
    printExpenseList(filtered);
}

void CoopTracker::viewEggRecordsByMonth() const {
    printSectionHeader("EGG RECORDS BY MONTH");
    if (eggRecords.empty()) {
        cout << warnText("No egg records found.") << "\n";
        return;
    }

    int month = getValidatedInt("Enter month (1-12): ", 1, 12);
    int year = getValidatedInt("Enter year: ", 1900, 3000);

    vector<EggRecord> filtered;
    for (const auto& record : eggRecords) {
        if (isDateInMonthYear(record.getDate(), month, year)) {
            filtered.push_back(record);
        }
    }

    if (filtered.empty()) {
        cout << warnText("No egg records found for " + monthYearLabel(month, year) + ".") << "\n";
        return;
    }

    cout << labelText("Showing records for: ") << monthYearLabel(month, year) << "\n\n";
    printEggRecordListSorted(filtered);
}

void CoopTracker::viewHealthNotesByMonth() const {
    printSectionHeader("HEALTH NOTES BY MONTH");
    if (healthNotes.empty()) {
        cout << warnText("No health notes found.") << "\n";
        return;
    }

    int month = getValidatedInt("Enter month (1-12): ", 1, 12);
    int year = getValidatedInt("Enter year: ", 1900, 3000);

    vector<HealthNote> filtered;
    for (const auto& note : healthNotes) {
        if (isDateInMonthYear(note.getDate(), month, year)) {
            filtered.push_back(note);
        }
    }

    if (filtered.empty()) {
        cout << warnText("No health notes found for " + monthYearLabel(month, year) + ".") << "\n";
        return;
    }

    sort(filtered.begin(), filtered.end(), [this](const HealthNote& a, const HealthNote& b) {
        return dateToSortableValue(a.getDate()) < dateToSortableValue(b.getDate());
        });

    cout << labelText("Showing records for: ") << monthYearLabel(month, year) << "\n\n";
    printHealthNoteList(filtered);
}

void CoopTracker::viewCleaningRecordsByMonth() const {
    printSectionHeader("CLEANING RECORDS BY MONTH");
    if (cleaningRecords.empty()) {
        cout << warnText("No cleaning records found.") << "\n";
        return;
    }

    int month = getValidatedInt("Enter month (1-12): ", 1, 12);
    int year = getValidatedInt("Enter year: ", 1900, 3000);

    vector<CleaningRecord> filtered;
    for (const auto& record : cleaningRecords) {
        if (isDateInMonthYear(record.getDate(), month, year)) {
            filtered.push_back(record);
        }
    }

    if (filtered.empty()) {
        cout << warnText("No cleaning records found for " + monthYearLabel(month, year) + ".") << "\n";
        return;
    }

    sort(filtered.begin(), filtered.end(), [this](const CleaningRecord& a, const CleaningRecord& b) {
        return dateToSortableValue(a.getDate()) < dateToSortableValue(b.getDate());
        });

    cout << labelText("Showing records for: ") << monthYearLabel(month, year) << "\n\n";
    printCleaningRecordList(filtered);
}

void CoopTracker::addChicken() {
    printSectionHeader("ADD CHICKEN");
    string name = getLineInput("Name: ");
    string breed = getLineInput("Breed: ");
    int age = getValidatedInt("Age (months): ", 0, 500);
    string notes = getLineInput("Notes: ");

    chickens.push_back(Chicken(name, breed, age, notes));
    saveChickensToTxt();
    cout << goodText("Chicken added.") << "\n";
}

void CoopTracker::viewChickens() const {
    printSectionHeader("CHICKEN LIST");
    if (chickens.empty()) {
        cout << warnText("No chickens found.") << "\n";
        return;
    }

    for (size_t i = 0; i < chickens.size(); ++i) {
        cout << i + 1 << ". " << chickens[i].getName()
            << " | Breed: " << chickens[i].getBreed()
            << " | Age: " << chickens[i].getAge()
            << " | Notes: " << chickens[i].getNotes() << "\n";
    }
}

void CoopTracker::editChicken() {
    printSectionHeader("EDIT CHICKEN");
    if (chickens.empty()) {
        cout << warnText("No chickens found.") << "\n";
        return;
    }

    viewChickens();
    int index = getValidatedInt("Select chicken number to edit: ", 1, static_cast<int>(chickens.size())) - 1;

    string name = getLineInput("New name: ");
    string breed = getLineInput("New breed: ");
    int age = getValidatedInt("New age (months): ", 0, 500);
    string notes = getLineInput("New notes: ");

    chickens[index].setName(name);
    chickens[index].setBreed(breed);
    chickens[index].setAge(age);
    chickens[index].setNotes(notes);

    saveChickensToTxt();
    cout << goodText("Chicken updated.") << "\n";
}

void CoopTracker::deleteChicken() {
    printSectionHeader("DELETE CHICKEN");
    if (chickens.empty()) {
        cout << warnText("No chickens found.") << "\n";
        return;
    }

    viewChickens();
    int index = getValidatedInt("Select chicken number to delete: ", 1, static_cast<int>(chickens.size())) - 1;
    chickens.erase(chickens.begin() + index);
    saveChickensToTxt();
    cout << goodText("Chicken deleted.") << "\n";
}

void CoopTracker::addFeedRecord() {
    printSectionHeader("ADD FEED RECORD");
    string date = promptForDate("Date (MM/DD/YYYY): ");
    string type = getLineInput("Feed type: ");
    double quantity = getValidatedDouble("Quantity: ", 0.0, 100000.0);
    double cost = getValidatedDouble("Cost: ", 0.0, 100000.0);

    feedRecords.push_back(FeedRecord(date, type, quantity, cost));
    saveFeedRecordsToTxt();
    cout << goodText("Feed record added.") << "\n";
}

void CoopTracker::viewFeedRecords() const {
    printSectionHeader("FEED RECORD LIST");
    printFeedRecordList(feedRecords);
}

void CoopTracker::editFeedRecord() {
    printSectionHeader("EDIT FEED RECORD");
    if (feedRecords.empty()) {
        cout << warnText("No feed records found.") << "\n";
        return;
    }

    printFeedRecordList(feedRecords);
    int index = getValidatedInt("Select feed record number to edit: ", 1, static_cast<int>(feedRecords.size())) - 1;

    feedRecords[index].setDate(promptForDate("New date (MM/DD/YYYY): "));
    feedRecords[index].setFeedType(getLineInput("New feed type: "));
    feedRecords[index].setQuantity(getValidatedDouble("New quantity: ", 0.0, 100000.0));
    feedRecords[index].setCost(getValidatedDouble("New cost: ", 0.0, 100000.0));

    saveFeedRecordsToTxt();
    cout << goodText("Feed record updated.") << "\n";
}

void CoopTracker::deleteFeedRecord() {
    printSectionHeader("DELETE FEED RECORD");
    if (feedRecords.empty()) {
        cout << warnText("No feed records found.") << "\n";
        return;
    }

    printFeedRecordList(feedRecords);
    int index = getValidatedInt("Select feed record number to delete: ", 1, static_cast<int>(feedRecords.size())) - 1;
    feedRecords.erase(feedRecords.begin() + index);
    saveFeedRecordsToTxt();
    cout << goodText("Feed record deleted.") << "\n";
}

void CoopTracker::addExpense() {
    printSectionHeader("ADD EXPENSE");
    string date = promptForDate("Date (MM/DD/YYYY): ");
    string category = getLineInput("Category: ");
    string description = getLineInput("Description: ");
    double amount = getValidatedDouble("Amount: ", 0.0, 100000.0);

    expenses.push_back(Expense(date, category, description, amount));
    saveExpensesToTxt();
    cout << goodText("Expense added.") << "\n";
}

void CoopTracker::viewExpenses() const {
    printSectionHeader("EXPENSE LIST");
    printExpenseList(expenses);
}

void CoopTracker::editExpense() {
    printSectionHeader("EDIT EXPENSE");
    if (expenses.empty()) {
        cout << warnText("No expenses found.") << "\n";
        return;
    }

    printExpenseList(expenses);
    int index = getValidatedInt("Select expense number to edit: ", 1, static_cast<int>(expenses.size())) - 1;

    expenses[index].setDate(promptForDate("New date (MM/DD/YYYY): "));
    expenses[index].setCategory(getLineInput("New category: "));
    expenses[index].setDescription(getLineInput("New description: "));
    expenses[index].setAmount(getValidatedDouble("New amount: ", 0.0, 100000.0));

    saveExpensesToTxt();
    cout << goodText("Expense updated.") << "\n";
}

void CoopTracker::deleteExpense() {
    printSectionHeader("DELETE EXPENSE");
    if (expenses.empty()) {
        cout << warnText("No expenses found.") << "\n";
        return;
    }

    printExpenseList(expenses);
    int index = getValidatedInt("Select expense number to delete: ", 1, static_cast<int>(expenses.size())) - 1;
    expenses.erase(expenses.begin() + index);
    saveExpensesToTxt();
    cout << goodText("Expense deleted.") << "\n";
}

void CoopTracker::showExpenseSummaryByCategory() const {
    printSectionHeader("EXPENSE SUMMARY BY CATEGORY");
    if (expenses.empty()) {
        cout << warnText("No expenses found.") << "\n";
        return;
    }

    vector<string> categories;
    vector<double> totals;

    for (const auto& expense : expenses) {
        bool found = false;
        for (size_t i = 0; i < categories.size(); ++i) {
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

    for (size_t i = 0; i < categories.size(); ++i) {
        cout << categories[i] << ": $" << fixed << setprecision(2) << totals[i] << "\n";
    }
}

void CoopTracker::addEggRecord() {
    printSectionHeader("ADD EGG RECORD");
    string date = promptForDate("Date (MM/DD/YYYY): ");
    int eggCount = getValidatedInt("Egg count: ", 0, 500);
    string notes = getLineInput("Notes: ");

    eggRecords.push_back(EggRecord(date, eggCount, notes));
    saveEggRecordsToTxt();
    cout << goodText("Egg record added.") << "\n";
}

void CoopTracker::viewEggRecords() const {
    printSectionHeader("EGG RECORD LIST");
    printEggRecordListSorted(eggRecords);
}

void CoopTracker::editEggRecord() {
    printSectionHeader("EDIT EGG RECORD");
    if (eggRecords.empty()) {
        cout << warnText("No egg records found.") << "\n";
        return;
    }

    vector<pair<int, EggRecord>> sortedWithOriginalIndex;
    for (size_t i = 0; i < eggRecords.size(); ++i) {
        sortedWithOriginalIndex.push_back({ static_cast<int>(i), eggRecords[i] });
    }

    sort(sortedWithOriginalIndex.begin(), sortedWithOriginalIndex.end(),
        [this](const pair<int, EggRecord>& a, const pair<int, EggRecord>& b) {
            return dateToSortableValue(a.second.getDate()) < dateToSortableValue(b.second.getDate());
        });

    for (size_t i = 0; i < sortedWithOriginalIndex.size(); ++i) {
        cout << i + 1 << ". " << sortedWithOriginalIndex[i].second.getDate()
            << " | Eggs: " << sortedWithOriginalIndex[i].second.getEggCount()
            << " | Notes: " << sortedWithOriginalIndex[i].second.getNotes() << "\n";
    }

    int selection = getValidatedInt("Select egg record number to edit: ", 1, static_cast<int>(sortedWithOriginalIndex.size())) - 1;
    int originalIndex = sortedWithOriginalIndex[selection].first;

    eggRecords[originalIndex].setDate(promptForDate("New date (MM/DD/YYYY): "));
    eggRecords[originalIndex].setEggCount(getValidatedInt("New egg count: ", 0, 500));
    eggRecords[originalIndex].setNotes(getLineInput("New notes: "));

    saveEggRecordsToTxt();
    cout << goodText("Egg record updated.") << "\n";
}

void CoopTracker::deleteEggRecord() {
    printSectionHeader("DELETE EGG RECORD");
    if (eggRecords.empty()) {
        cout << warnText("No egg records found.") << "\n";
        return;
    }

    vector<pair<int, EggRecord>> sortedWithOriginalIndex;
    for (size_t i = 0; i < eggRecords.size(); ++i) {
        sortedWithOriginalIndex.push_back({ static_cast<int>(i), eggRecords[i] });
    }

    sort(sortedWithOriginalIndex.begin(), sortedWithOriginalIndex.end(),
        [this](const pair<int, EggRecord>& a, const pair<int, EggRecord>& b) {
            return dateToSortableValue(a.second.getDate()) < dateToSortableValue(b.second.getDate());
        });

    for (size_t i = 0; i < sortedWithOriginalIndex.size(); ++i) {
        cout << i + 1 << ". " << sortedWithOriginalIndex[i].second.getDate()
            << " | Eggs: " << sortedWithOriginalIndex[i].second.getEggCount()
            << " | Notes: " << sortedWithOriginalIndex[i].second.getNotes() << "\n";
    }

    int selection = getValidatedInt("Select egg record number to delete: ", 1, static_cast<int>(sortedWithOriginalIndex.size())) - 1;
    int originalIndex = sortedWithOriginalIndex[selection].first;

    eggRecords.erase(eggRecords.begin() + originalIndex);
    saveEggRecordsToTxt();
    cout << goodText("Egg record deleted.") << "\n";
}

void CoopTracker::addHealthNote() {
    printSectionHeader("ADD HEALTH NOTE");
    string date = promptForDate("Date (MM/DD/YYYY): ");
    string chickenName = getLineInput("Chicken name: ");
    string note = getLineInput("Note: ");

    healthNotes.push_back(HealthNote(date, chickenName, note));
    saveHealthNotesToTxt();
    cout << goodText("Health note added.") << "\n";
}

void CoopTracker::viewHealthNotes() const {
    printSectionHeader("HEALTH NOTE LIST");
    printHealthNoteList(healthNotes);
}

void CoopTracker::editHealthNote() {
    printSectionHeader("EDIT HEALTH NOTE");
    if (healthNotes.empty()) {
        cout << warnText("No health notes found.") << "\n";
        return;
    }

    printHealthNoteList(healthNotes);
    int index = getValidatedInt("Select health note number to edit: ", 1, static_cast<int>(healthNotes.size())) - 1;

    healthNotes[index].setDate(promptForDate("New date (MM/DD/YYYY): "));
    healthNotes[index].setChickenName(getLineInput("New chicken name: "));
    healthNotes[index].setNote(getLineInput("New note: "));

    saveHealthNotesToTxt();
    cout << goodText("Health note updated.") << "\n";
}

void CoopTracker::deleteHealthNote() {
    printSectionHeader("DELETE HEALTH NOTE");
    if (healthNotes.empty()) {
        cout << warnText("No health notes found.") << "\n";
        return;
    }

    printHealthNoteList(healthNotes);
    int index = getValidatedInt("Select health note number to delete: ", 1, static_cast<int>(healthNotes.size())) - 1;
    healthNotes.erase(healthNotes.begin() + index);
    saveHealthNotesToTxt();
    cout << goodText("Health note deleted.") << "\n";
}

void CoopTracker::addCleaningRecord() {
    printSectionHeader("ADD CLEANING RECORD");
    string date = promptForDate("Date (MM/DD/YYYY): ");
    string task = getLineInput("Task: ");
    string notes = getLineInput("Notes: ");

    cleaningRecords.push_back(CleaningRecord(date, task, notes));
    saveCleaningRecordsToTxt();
    cout << goodText("Cleaning record added.") << "\n";
}

void CoopTracker::viewCleaningRecords() const {
    printSectionHeader("CLEANING RECORD LIST");
    printCleaningRecordList(cleaningRecords);
}

void CoopTracker::editCleaningRecord() {
    printSectionHeader("EDIT CLEANING RECORD");
    if (cleaningRecords.empty()) {
        cout << warnText("No cleaning records found.") << "\n";
        return;
    }

    printCleaningRecordList(cleaningRecords);
    int index = getValidatedInt("Select cleaning record number to edit: ", 1, static_cast<int>(cleaningRecords.size())) - 1;

    cleaningRecords[index].setDate(promptForDate("New date (MM/DD/YYYY): "));
    cleaningRecords[index].setTask(getLineInput("New task: "));
    cleaningRecords[index].setNotes(getLineInput("New notes: "));

    saveCleaningRecordsToTxt();
    cout << goodText("Cleaning record updated.") << "\n";
}

void CoopTracker::deleteCleaningRecord() {
    printSectionHeader("DELETE CLEANING RECORD");
    if (cleaningRecords.empty()) {
        cout << warnText("No cleaning records found.") << "\n";
        return;
    }

    printCleaningRecordList(cleaningRecords);
    int index = getValidatedInt("Select cleaning record number to delete: ", 1, static_cast<int>(cleaningRecords.size())) - 1;
    cleaningRecords.erase(cleaningRecords.begin() + index);
    saveCleaningRecordsToTxt();
    cout << goodText("Cleaning record deleted.") << "\n";
}
