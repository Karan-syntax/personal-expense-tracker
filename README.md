# Personal Expense Tracker (C)

A command-line expense tracker written in C. It stores expenses locally in a CSV-style text file so data remains available between runs.

## Features

- Add expenses with date, category, amount, and note
- View all saved expenses and the total spent
- Search expenses by category
- Delete an expense by its ID
- Persistent local storage in `expenses.csv`

## Concepts Used

- Functions and menu-driven control flow
- Structures (`struct`)
- File handling (`fopen`, `fprintf`, `fscanf`)
- String handling
- Input validation

## Build and Run

```bash
gcc -std=c11 -Wall -Wextra -o expense_tracker main.c
./expense_tracker
```

On Windows PowerShell:

```powershell
gcc -std=c11 -Wall -Wextra -o expense_tracker.exe main.c
.\expense_tracker.exe
```

## Example

```text
1. Add expense
2. View all expenses
3. Search by category
4. Delete an expense
5. Exit
```

## Project Structure

```text
personal-expense-tracker/
├── main.c
├── README.md
└── expenses.csv        # created automatically after adding an expense
```

## Notes

`expenses.csv` contains personal expense data and is ignored by Git by default.
