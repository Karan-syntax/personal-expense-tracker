#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_FILE "expenses.csv"
#define MAX_CATEGORY 30
#define MAX_NOTE 100

typedef struct {
    int id;
    char date[11];
    char category[MAX_CATEGORY];
    double amount;
    char note[MAX_NOTE];
} Expense;

static void read_line(const char *prompt, char *buffer, size_t size) {
    char *line_end;

    printf("%s", prompt);
    do {
        if (fgets(buffer, (int)size, stdin) == NULL) {
            buffer[0] = '\0';
            return;
        }

        line_end = strpbrk(buffer, "\r\n");
        if (line_end != NULL) {
            /* Windows terminals may leave a line-feed after a carriage return. */
            if (*line_end == '\r' && line_end[1] == '\0') {
                int next = getchar();
                if (next != '\n' && next != EOF) {
                    ungetc(next, stdin);
                }
            }
            *line_end = '\0';
        }
    } while (buffer[0] == '\0');
}

static int read_int(const char *prompt, int *value) {
    char input[64];
    char *end;
    long result;

    read_line(prompt, input, sizeof(input));
    result = strtol(input, &end, 10);
    if (input[0] == '\0' || *end != '\0') {
        return 0;
    }

    *value = (int)result;
    return 1;
}

static int read_amount(const char *prompt, double *value) {
    char input[64];
    char *end;
    double result;

    read_line(prompt, input, sizeof(input));
    result = strtod(input, &end);
    if (input[0] == '\0' || *end != '\0') {
        return 0;
    }

    *value = result;
    return 1;
}

static int next_id(void) {
    FILE *file = fopen(DATA_FILE, "r");
    Expense expense;
    int max_id = 0;

    if (file == NULL) {
        return 1;
    }

    while (fscanf(file, "%d|%10[^|]|%29[^|]|%lf|%99[^\n]\n",
                  &expense.id, expense.date, expense.category,
                  &expense.amount, expense.note) == 5) {
        if (expense.id > max_id) {
            max_id = expense.id;
        }
    }

    fclose(file);
    return max_id + 1;
}

static void print_divider(void) {
    printf("+-----+------------+----------------------+------------+----------------------------------+\n");
}

static void print_expense(const Expense *expense) {
    printf("| %-3d | %-10s | %-20s | %10.2f | %-32s |\n",
           expense->id, expense->date, expense->category,
           expense->amount, expense->note);
}

static void add_expense(void) {
    FILE *file;
    Expense expense;

    expense.id = next_id();
    printf("\n--- Add Expense ---\n");
    read_line("Date (YYYY-MM-DD): ", expense.date, sizeof(expense.date));
    read_line("Category: ", expense.category, sizeof(expense.category));

    if (!read_amount("Amount: ", &expense.amount) || expense.amount <= 0) {
        printf("Invalid amount. Expense was not saved.\n");
        return;
    }
    read_line("Short note: ", expense.note, sizeof(expense.note));

    if (strchr(expense.date, '|') || strchr(expense.category, '|') || strchr(expense.note, '|')) {
        printf("The '|' character cannot be used. Expense was not saved.\n");
        return;
    }

    file = fopen(DATA_FILE, "a");
    if (file == NULL) {
        perror("Could not open data file");
        return;
    }

    fprintf(file, "%d|%s|%s|%.2f|%s\n", expense.id, expense.date,
            expense.category, expense.amount, expense.note);
    fclose(file);
    printf("Expense #%d saved successfully.\n", expense.id);
}

static void view_expenses(void) {
    FILE *file = fopen(DATA_FILE, "r");
    Expense expense;
    int count = 0;
    double total = 0.0;

    printf("\n--- All Expenses ---\n");
    if (file == NULL) {
        printf("No expenses found yet.\n");
        return;
    }

    print_divider();
    printf("| ID  | Date       | Category             | Amount     | Note                             |\n");
    print_divider();
    while (fscanf(file, "%d|%10[^|]|%29[^|]|%lf|%99[^\n]\n",
                  &expense.id, expense.date, expense.category,
                  &expense.amount, expense.note) == 5) {
        print_expense(&expense);
        total += expense.amount;
        count++;
    }
    print_divider();
    printf("Total: %.2f across %d expense(s).\n", total, count);
    fclose(file);
}

static void search_by_category(void) {
    FILE *file = fopen(DATA_FILE, "r");
    Expense expense;
    char category[MAX_CATEGORY];
    int found = 0;
    double total = 0.0;

    if (file == NULL) {
        printf("No expenses found yet.\n");
        return;
    }

    read_line("\nCategory to search: ", category, sizeof(category));
    printf("\n--- Expenses in %s ---\n", category);
    print_divider();
    printf("| ID  | Date       | Category             | Amount     | Note                             |\n");
    print_divider();

    while (fscanf(file, "%d|%10[^|]|%29[^|]|%lf|%99[^\n]\n",
                  &expense.id, expense.date, expense.category,
                  &expense.amount, expense.note) == 5) {
        if (strcmp(expense.category, category) == 0) {
            print_expense(&expense);
            total += expense.amount;
            found = 1;
        }
    }

    print_divider();
    if (found) {
        printf("Category total: %.2f\n", total);
    } else {
        printf("No expenses found in that category.\n");
    }
    fclose(file);
}

static void delete_expense(void) {
    FILE *source = fopen(DATA_FILE, "r");
    FILE *temporary;
    Expense expense;
    int target_id;
    int deleted = 0;

    if (source == NULL) {
        printf("No expenses found yet.\n");
        return;
    }

    if (!read_int("\nExpense ID to delete: ", &target_id)) {
        printf("Invalid ID.\n");
        fclose(source);
        return;
    }

    temporary = fopen("expenses.tmp", "w");
    if (temporary == NULL) {
        perror("Could not create temporary file");
        fclose(source);
        return;
    }

    while (fscanf(source, "%d|%10[^|]|%29[^|]|%lf|%99[^\n]\n",
                  &expense.id, expense.date, expense.category,
                  &expense.amount, expense.note) == 5) {
        if (expense.id == target_id) {
            deleted = 1;
        } else {
            fprintf(temporary, "%d|%s|%s|%.2f|%s\n", expense.id,
                    expense.date, expense.category, expense.amount, expense.note);
        }
    }

    fclose(source);
    fclose(temporary);

    if (deleted) {
        remove(DATA_FILE);
        rename("expenses.tmp", DATA_FILE);
        printf("Expense #%d deleted.\n", target_id);
    } else {
        remove("expenses.tmp");
        printf("No expense found with ID %d.\n", target_id);
    }
}

int main(void) {
    int choice;

    do {
        printf("\n===================================\n");
        printf("      PERSONAL EXPENSE TRACKER\n");
        printf("===================================\n");
        printf("1. Add expense\n");
        printf("2. View all expenses\n");
        printf("3. Search by category\n");
        printf("4. Delete an expense\n");
        printf("5. Exit\n");
        if (!read_int("Choose an option: ", &choice)) {
            printf("Please enter a number from 1 to 5.\n");
            continue;
        }

        switch (choice) {
            case 1: add_expense(); break;
            case 2: view_expenses(); break;
            case 3: search_by_category(); break;
            case 4: delete_expense(); break;
            case 5: printf("Goodbye!\n"); break;
            default: printf("Please choose a number from 1 to 5.\n");
        }
    } while (choice != 5);

    return 0;
}
