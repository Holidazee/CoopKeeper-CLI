# 🐔 CoopKeeper CLI

A feature-rich C++ command-line application for managing a backyard chicken coop.
Track egg production, feed usage, expenses, health notes, and cleaning records — all from a clean, styled dashboard interface.

---

## 📸 Screenshots

### 🧭 Dashboard

![Dashboard](screenshots/dashboard.png)

### 🥚 Egg Records

![Egg Records](screenshots/eggs.png)

### 📁 CSV Export

![Export](screenshots/export.png)

---

## 🚀 Features

* 🐔 Manage chickens (name, breed, age, notes)
* 🥚 Track egg production (daily, monthly, yearly)
* 🌾 Log feed usage (type, quantity, cost)
* 💰 Record and categorize expenses
* 🩺 Store health notes per chicken
* 🧼 Track cleaning activity and maintenance
* 📊 Generate reports:

  * Monthly
  * Yearly
  * Custom date range
* 📁 Export all data to CSV

---

## 🎨 UI Highlights

* ANSI-styled terminal interface
* Color-coded sections for readability
* Real-time dashboard summaries
* Egg production bar visualization
* Trend indicators for performance tracking
* Clean menu navigation and layout

---

## 📊 Dashboard Insights

* Total flock size
* Eggs collected (today, month, year)
* Cost per dozen eggs
* Feed and expense summaries
* Last cleaning and health activity
* Month-over-month production trends

---

## 🧠 Insights & Alerts

* Average eggs per day
* Best production day tracking
* Production trend comparison vs last month
* Alerts for:

  * low production
  * missed cleanings
  * unusual activity

---

## 📂 Project Structure

```
CoopKeeper/
│
├── data/               # Local data storage (.txt files)
├── exports/            # CSV export output
├── screenshots/        # README images
│
├── include/            # Header files
├── src/                # Source files
│
├── main.cpp
└── README.md
```

---

## ⚙️ How to Run

1. Clone the repository:

```
git clone https://github.com/YOUR_USERNAME/CoopKeeper.git
```

2. Open in Visual Studio 2022

3. Build and run

---

## 💡 Technologies Used

* C++
* Standard Template Library (STL)
* File I/O (text-based persistence)
* ANSI terminal styling
* CSV export handling

---

## 🔧 Notes

* Data is stored locally in `.txt` files
* CSV exports are saved in the `exports/` folder
* Designed for Windows console (ANSI enabled)

---

## 🎯 Purpose

This project demonstrates:

* Object-oriented design in C++
* File-based data persistence
* CLI application UX design
* Real-world problem solving with structured data

---

## 👨‍💻 Author

Built as a practical, real-world C++ project focused on usability, structure, and meaningful features.

---
