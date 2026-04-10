#pragma once

#include <vector>
#include <string>
#include "Chicken.h"
#include "FeedRecord.h"
#include "Expense.h"
#include "EggRecord.h"
#include "HealthNote.h"
#include "CleaningRecord.h"

using namespace std;

class CoopTracker {
private:
    vector<Chicken> chickens;
    vector<FeedRecord> feedRecords;
    vector<Expense> expenses;
    vector<EggRecord> eggRecords;
    vector<HealthNote> healthNotes;
    vector<CleaningRecord> cleaningRecords;

    void printSectionHeader(const string& title) const;
    void pauseForEnter() const;
    string getLineInput(const string& prompt) const;
    int getValidatedInt(const string& prompt, int min, int max) const;
    double getValidatedDouble(const string& prompt, double min, double max) const;
    string promptForDate(const string& prompt) const;

    void loadAllData();
    void saveAllData() const;

    void loadChickensFromTxt();
    void loadFeedRecordsFromTxt();
    void loadExpensesFromTxt();
    void loadEggRecordsFromTxt();
    void loadHealthNotesFromTxt();
    void loadCleaningRecordsFromTxt();

    void saveChickensToTxt() const;
    void saveFeedRecordsToTxt() const;
    void saveExpensesToTxt() const;
    void saveEggRecordsToTxt() const;
    void saveHealthNotesToTxt() const;
    void saveCleaningRecordsToTxt() const;

    void exportChickensToCSV() const;
    void exportFeedRecordsToCSV() const;
    void exportExpensesToCSV() const;
    void exportEggRecordsToCSV() const;
    void exportHealthNotesToCSV() const;
    void exportCleaningRecordsToCSV() const;
    void exportAllToCSV() const;

    string escapeCSV(const string& value) const;

    // Date / filtering helpers
    bool parseDate(const string& date, int& month, int& day, int& year) const;
    bool isDateInMonthYear(const string& date, int month, int year) const;
    bool isDateInYear(const string& date, int year) const;
    bool isDateInRange(const string& date, const string& startDate, const string& endDate) const;
    int dateToSortableValue(const string& date) const;
    string monthYearLabel(int month, int year) const;
    int getCurrentMonth() const;
    int getCurrentYear() const;
    string getLastCleaningDate() const;

    int getDaysBetween(const string& earlierDate, const string& laterDate) const;
    int getDaysSinceDate(const string& date) const;

    // Dashboard / reports
    void showStartupStatus() const;
    void showDashboard() const;
    void showInsights() const;
    void showAlerts() const;
    void showMonthlyReport() const;
    void showYearlyReport() const;
    void showDateRangeReport() const;

    // Summary helpers
    double getExpenseTotalForMonth(int month, int year) const;
    double getExpenseTotalForYear(int year) const;

    double getFeedCostTotalForMonth(int month, int year) const;
    double getFeedCostTotalForYear(int year) const;

    int getEggTotalForMonth(int month, int year) const;
    int getEggTotalForYear(int year) const;
    int getEggTotalForDate(const string& date) const;

    int getHealthCountForMonth(int month, int year) const;
    int getHealthCountForYear(int year) const;

    int getCleaningCountForMonth(int month, int year) const;
    int getCleaningCountForYear(int year) const;

    double getCostPerEggForMonth(int month, int year) const;
    double getCostPerDozenForMonth(int month, int year) const;
    double getCostPerEggForYear(int year) const;
    double getCostPerDozenForYear(int year) const;

    double getAverageEggsPerDayForMonth(int month, int year) const;
    string getBestProductionDayForMonth(int month, int year) const;
    double getProductionChangePercentFromPreviousMonth(int month, int year) const;

    // Display helpers
    void printFeedRecordList(const vector<FeedRecord>& records) const;
    void printExpenseList(const vector<Expense>& records) const;
    void printEggRecordListSorted(const vector<EggRecord>& records) const;
    void printHealthNoteList(const vector<HealthNote>& records) const;
    void printCleaningRecordList(const vector<CleaningRecord>& records) const;

public:
    void run();

    void chickenMenu();
    void feedMenu();
    void expenseMenu();
    void eggMenu();
    void healthMenu();
    void cleaningMenu();

    void addChicken();
    void viewChickens() const;
    void editChicken();
    void deleteChicken();

    void addFeedRecord();
    void viewFeedRecords() const;
    void editFeedRecord();
    void deleteFeedRecord();

    void addExpense();
    void viewExpenses() const;
    void editExpense();
    void deleteExpense();
    void showExpenseSummaryByCategory() const;

    void addEggRecord();
    void viewEggRecords() const;
    void editEggRecord();
    void deleteEggRecord();

    void addHealthNote();
    void viewHealthNotes() const;
    void editHealthNote();
    void deleteHealthNote();

    void addCleaningRecord();
    void viewCleaningRecords() const;
    void editCleaningRecord();
    void deleteCleaningRecord();

    void viewFeedRecordsByMonth() const;
    void viewExpensesByMonth() const;
    void viewEggRecordsByMonth() const;
    void viewHealthNotesByMonth() const;
    void viewCleaningRecordsByMonth() const;
};