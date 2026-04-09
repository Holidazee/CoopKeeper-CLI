# 🐔 CoopKeeper

A practical, real-world **C++ chicken coop management system** built to track flock performance, expenses, and maintenance with simple file-based storage.

Designed for both **personal use (actual chicken owners)** and as a **portfolio-ready C++ project demonstrating real-world engineering skills**.

---

## 🚀 Overview

CoopKeeper is a **menu-driven CLI application** that helps you manage:

- 🐓 Chickens
- 🥚 Egg production
- 🌾 Feed usage
- 💰 Expenses
- 🩺 Health records
- 🧼 Cleaning logs

All data is persisted using `.txt` files and can be exported to `.csv` for analysis.

---

## ✨ Key Features

### 📊 Startup Dashboard
- Total eggs collected
- Monthly production summary
- Feed cost tracking
- Expense totals
- Cost per dozen eggs (current month)
- Health note count
- Cleaning count
- Last cleaning date
- Clean UI with pause before menu

---

### 🥚 Egg Tracking
- Add/edit/delete records
- Sorted chronologically (ASC)
- Daily notes support
- Monthly insights

---

### 💰 Expense Management
- Categorized expenses
- Running totals
- CSV export support

---

### 🌾 Feed Tracking
- Track feed purchases
- Monitor cost trends

---

### 🩺 Health Notes
- Log illnesses, injuries, observations
- Historical tracking

---

### 🧼 Cleaning Records
- Track coop maintenance
- Dashboard visibility for last clean date

---

### 📤 CSV Export
Export all major data sets:
- Eggs
- Expenses
- Feed
- Health
- Cleaning

Perfect for Excel or long-term analysis.

---

## 🧱 Tech Stack

- C++ (C++17)
- STL (vectors, file streams)
- File-based persistence (TXT)
- CSV export functionality
- Object-Oriented Design

---

## 📁 Project Structure

```
CoopKeeper/
│
├── data/              # TXT storage (persistent data)
├── exports/           # CSV output files
│
├── *.h                # Header files (models + tracker)
├── *.cpp              # Implementation files
│
└── main.cpp           # Entry point
```

---

## ⚙️ How to Run

### 1. Clone Repo
```bash
git clone https://github.com/Holidazee/CoopKeeper.git
```

### 2. Open in Visual Studio 2022

### 3. IMPORTANT Setup

Ensure folders exist in your build directory:

```
x64/Debug/data/
x64/Debug/exports/
```

This is required for file loading/saving to work.

---

### 4. Build & Run

Press **F5** or run via Visual Studio.

---

## 🧾 Example Data Format

**eggs.txt**
```
2026-04-01,12,Strong production
2026-04-02,10,Cool morning
```

Format:
```
DATE,VALUE,NOTES
```

---

## 🧠 What This Project Demonstrates

This project showcases:

- Real-world data persistence
- Clean object-oriented design
- File I/O and serialization
- CLI UX design
- Modular architecture
- Problem-solving with evolving requirements

---

## 🔧 Challenges Solved

- Fixing file load/save issues across directories
- Implementing CSV export alongside TXT storage
- Sorting and indexing consistency (egg records)
- Designing a usable CLI dashboard
- Maintaining clean separation between models and logic

---

## 🔮 Future Improvements

- GUI version (Qt or web app)
- SQLite database backend
- Graphs & analytics dashboard
- Multi-user or multi-coop support
- Mobile-friendly interface

---

## 📸 Recommended Additions

To improve this project further:

- Add screenshots of:
  - Dashboard
  - Menu system
- Include sample data files
- Add a demo GIF

---

## 👨‍💻 Author

Taylor Burris  
https://github.com/Holidazee

---

## 📜 License

MIT License (recommended)

---
