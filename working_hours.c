#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_CARDHOLDER_NAME_LEN 50
#define MAX_LOG_LINE_LEN 100
#define ATTENDANCE_LOG_FILE "attendance_log.csv"

struct Cardholder {
    char card_data[11];
    char name[MAX_CARDHOLDER_NAME_LEN];
};

struct Cardholder cardholders[] = {
    {"3948327553", "Demo Person"},
};
int cardholder_count = sizeof(cardholders) / sizeof(cardholders[0]);

void ensure_log_file() {
    FILE *file = fopen(ATTENDANCE_LOG_FILE, "r");
    if (!file) {
        file = fopen(ATTENDANCE_LOG_FILE, "w");
        if (file) {
            fprintf(file, "Timestamp,Cardholder,Card Data,Type\n");
        } else {
            printf("Error: Could not create log file.\n");
            exit(1);
        }
    }
    fclose(file);
}

void log_attendance(const char *card_data) {
    time_t rawtime;
    struct tm *timeinfo;
    char timestamp[20];
    char cardholder[MAX_CARDHOLDER_NAME_LEN];
    char last_swipe_type[6] = "";
    char log_card_data[11];
    char log_line[MAX_LOG_LINE_LEN];
    FILE *file;

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);

    int found = 0;
    for (int i = 0; i < cardholder_count; i++) {
        if (strcmp(card_data, cardholders[i].card_data) == 0) {
            strcpy(cardholder, cardholders[i].name);
            found = 1;
            break;
        }
    }

    if (!found) {
        strcpy(cardholder, "Unknown Cardholder");

        file = fopen(ATTENDANCE_LOG_FILE, "a");
        if (file) {
            fprintf(file, "%s,%s,%s,%s\n", timestamp, cardholder, card_data, "error");
            fclose(file);
            printf("Unknown cardholder swipe recorded for card data: %s at %s\n", card_data, timestamp);
        } else {
            printf("Error: Could not write to attendance log.\n");
        }
        return;
    }

    file = fopen(ATTENDANCE_LOG_FILE, "r");
    if (file) {
        while (fgets(log_line, sizeof(log_line), file)) {
            sscanf(log_line, "%*[^,],%*[^,],%10[^,],%5s", log_card_data, last_swipe_type);
            if (strcmp(log_card_data, card_data) == 0) {
                break;
            }
        }
        fclose(file);
    }

    char swipe_type[6];
    if (strcmp(last_swipe_type, "entry") == 0) {
        strcpy(swipe_type, "exit");
    } else {
        strcpy(swipe_type, "entry");
    }

    file = fopen(ATTENDANCE_LOG_FILE, "a");
    if (file) {
        fprintf(file, "%s,%s,%s,%s\n", timestamp, cardholder, card_data, swipe_type);
        fclose(file);
        printf("%s recorded for %s at %s\n", swipe_type, cardholder, timestamp);
    } else {
        printf("Error: Could not write to attendance log.\n");
    }
}

void calculate_hours(const char *card_data) {
    FILE *file = fopen(ATTENDANCE_LOG_FILE, "r");
    if (!file) {
        printf("Error: Attendance log file not found.\n");
        return;
    }

    char log_line[MAX_LOG_LINE_LEN];
    char log_card_data[11];
    char log_type[6];
    char log_timestamp[20];
    time_t entry_time = 0;
    double total_hours = 0.0;

    struct tm log_timeinfo;
    struct tm today_tm;
    time_t today = time(NULL);
    localtime_r(&today, &today_tm);
    today_tm.tm_hour = 0;
    today_tm.tm_min = 0;
    today_tm.tm_sec = 0;
    today = mktime(&today_tm);  // Today's midnight time

    double daily_hours[7] = {0};  // Store the last 7 days of hours
    int current_day_idx = 0;

    while (fgets(log_line, sizeof(log_line), file)) {
        sscanf(log_line, "%19[^,],%*[^,],%10[^,],%5s", log_timestamp, log_card_data, log_type);

        if (strcmp(card_data, log_card_data) == 0) {
            strptime(log_timestamp, "%Y-%m-%d %H:%M:%S", &log_timeinfo);
            time_t log_time = mktime(&log_timeinfo);

            // Calculate the day difference from today (for weekly tracking)
            int day_diff = difftime(today, log_time) / (24 * 3600);

            if (day_diff >= 0 && day_diff < 7) {
                current_day_idx = 6 - day_diff;  // Adjust index to store daily hours in the last 7 days
            }

            if (strcmp(log_type, "entry") == 0) {
                entry_time = log_time;
            } else if (strcmp(log_type, "exit") == 0 && entry_time > 0) {
                double diff_hours = difftime(log_time, entry_time) / 3600.0;
                total_hours += diff_hours;
                daily_hours[current_day_idx] += diff_hours;
                entry_time = 0;
            }
        }
    }

    fclose(file);

    // Calculate total weekly hours
    double weekly_hours = 0.0;
    for (int i = 0; i < 7; i++) {
        weekly_hours += daily_hours[i];
    }

    // Print daily and weekly hours
    printf("\nHours worked in the last 7 days:\n");
    for (int i = 6; i >= 0; i--) {
        printf("Day %d: %.2f hours\n", 6 - i, daily_hours[i]);
    }

    printf("\nTotal hours worked this week: %.2f hours\n", weekly_hours);
}

int main() {
    ensure_log_file();

    char input[20];

    printf("Welcome! You can swipe your card at any time by entering your card data.\n");
    printf("Type 'view' to view hours, 'exit' to quit the program.\n");

    while (1) {
        printf("Enter card data (or 'view'/'exit'): ");
        scanf("%s", input);

        if (strcmp(input, "exit") == 0) {
            printf("Exiting...\n");
            break;
        } else if (strcmp(input, "view") == 0) {
            printf("Enter card data to view hours: ");
            scanf("%s", input);
            calculate_hours(input);
        } else {
            log_attendance(input);
        }
    }

    return 0;
}
