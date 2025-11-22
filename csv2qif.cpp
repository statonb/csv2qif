#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE 2048

// Remove surrounding quotes from a field, if present
void strip_quotes(char *s) {
    size_t len = strlen(s);
    if (len >= 2 && s[0] == '"' && s[len-1] == '"') {
        memmove(s, s+1, len-2);
        s[len-2] = '\0';
    }
}

// Remove all commas from a number field
void remove_commas(char *s) {
    char *dst = s, *src = s;
    while (*src) {
        if (*src != ',') {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input.csv output.qif\n", argv[0]);
        return 1;
    }

    FILE *fin = fopen(argv[1], "r");
    // FILE *fin = fopen("/home/bruno/Downloads/stmt.csv", "r");
    if (!fin) {
        perror("Error opening input file");
        return 1;
    }

    FILE *fout = fopen(argv[2], "w");
    // FILE *fout = fopen("/home/bruno/Downloads/test.qif", "w");
    if (!fout) {
        perror("Error opening output file");
        fclose(fin);
        return 1;
    }

    char line[MAX_LINE];
    int in_transaction_section = 0;

    fprintf(fout, "!Type:Bank\n");

    while (fgets(line, sizeof(line), fin)) {
        // Remove trailing newline
        line[strcspn(line, "\r\n")] = '\0';

        // Skip empty lines
        if (strlen(line) == 0) continue;

        // Detect the header of the transaction table
        if (!in_transaction_section) {
            if (strncmp(line, "Date,", 5) == 0) {
                in_transaction_section = 1;
            }
            continue;
        }

        // Now we are in the transaction section. Parse line.

        // ... but first ...
        // Never consider a line containing the "Beginning balance"
        // as though it was a transaction.  This takes care of eliminating
        // the Beginning balance line that comes after the header line.
        if (strstr(line, "Beginning balance as of")) {
            continue;
        }

        char tmp[MAX_LINE];
        strcpy(tmp, line);

        char *date = strtok(tmp, ",");
        char *description = strtok(NULL, ",");
        char *amount = strtok(NULL, ",");
        // char *runningbal = strtok(NULL, ","); // ignored

        if (!date || !description || !amount) {
            continue;
        }

        // Clean fields
        strip_quotes(description);
        strip_quotes(amount);
        strip_quotes(date);

        remove_commas(amount);

        // Skip lines where amount is empty.
        // This could have taken care of the initial balance
        // line except that using strtok with back-to-back
        // delimiters blows past the null amount field in that line
        if (strlen(amount) == 0) {
            continue;
        }

        // Output QIF record
        fprintf(fout, "D%s\n", date);
        fprintf(fout, "P%s\n", description);
        fprintf(fout, "T%s\n", amount);
        fprintf(fout, "C*\n");
        fprintf(fout, "^\n");
    }

    fclose(fin);
    fclose(fout);

    return 0;
}
