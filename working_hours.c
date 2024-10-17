#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <libevdev/libevdev.h>
#include <time.h>

#define MAX_CARDHOLDER_NAME_LEN 50
#define MAX_LOG_LINE_LEN 100
#define ATTENDANCE_LOG_FILE "attendance_log.csv"
#define MAX_CARD_DATA_LEN 256

struct Cardholder {
    char card_data[11];
    char name[MAX_CARDHOLDER_NAME_LEN];
};

struct Cardholder cardholders[] = {
    {"1226137070", "Demo Person"},
     {"1229499663", "Cyriac"},
     {"1229662137", "Jay"},
     {"1229454098", "Shubham"},
     {"1233706138", "Chinmay"},
     {"1222313196", "Riyank"},
     {"1229700331", "Ajay"},
     {"1229582226", "Prashant"},
     {"1229560633", "Narain"},
     {"1229581862", "Sujith"},
     {"1229544344", "Aditya"},
     {"1229714371", "Vikram"},
     {"1229624957", "Ankit"},
     {"1228930549", "Shrey"}
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
    char current_timestamp[20];  // Timestamp for the current swipe
    char log_timestamp[20];      // Timestamp for reading from the log
    char cardholder[MAX_CARDHOLDER_NAME_LEN];
    char last_swipe_type[6] = "";
    char log_card_data[11];
    char log_line[MAX_LOG_LINE_LEN];
    FILE *file;

    // Capture the current time just before processing the card swipe
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(current_timestamp, sizeof(current_timestamp), "%Y-%m-%d %H:%M:%S", timeinfo);  // Properly format the current time

    // Find the cardholder based on card_data
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
            fprintf(file, "%s,%s,%s,%s\n", current_timestamp, cardholder, card_data, "entry");
            fclose(file);
            printf("Unknown cardholder swipe recorded for card data: %s at %s\n", card_data, current_timestamp);
        } else {
            printf("Error: Could not write to attendance log.\n");
        }
        return;
    }

    // Check the last swipe for the same cardholder
    struct tm last_swipe_time = {0};
    time_t last_swipe_epoch = 0;
    file = fopen(ATTENDANCE_LOG_FILE, "r");
    if (file) {
        while (fgets(log_line, sizeof(log_line), file)) {
            // Use a separate timestamp (log_timestamp) for reading previous entries, not the current timestamp
            sscanf(log_line, "%19[^,],%*[^,],%10[^,],%5s", log_timestamp, log_card_data, last_swipe_type);
            if (strcmp(log_card_data, card_data) == 0) {
                strptime(log_timestamp, "%Y-%m-%d %H:%M:%S", &last_swipe_time);
                last_swipe_epoch = mktime(&last_swipe_time);
            }
        }
        fclose(file);
    }

    time_t current_time = time(NULL);
    double time_diff_minutes = difftime(current_time, last_swipe_epoch) / 60.0;

    // Check if the last swipe was on the same day
    char swipe_type[6];
    if (last_swipe_epoch != 0 && timeinfo->tm_year == last_swipe_time.tm_year &&
        timeinfo->tm_yday == last_swipe_time.tm_yday) {
        // If last swipe type was "entry", now it should be "exit" and vice versa
        if (strcmp(last_swipe_type, "entry") == 0) {
            strcpy(swipe_type, "exit");
        } else if (strcmp(last_swipe_type, "exit") == 0) {
            strcpy(swipe_type, "entry");
        }
    } else {
        // If no swipe today, default to "entry"
        strcpy(swipe_type, "entry");
    }

    // // Prevent consecutive same type swipes
    // if (strcmp(last_swipe_type, swipe_type) == 0) {
    //     printf("Swipe ignored: Consecutive swipes of the same type ('%s') for %s\n", swipe_type, cardholder);
    //     return;
    // }

    // Log the new swipe with the current timestamp
    file = fopen(ATTENDANCE_LOG_FILE, "a");
    if (file) {
        fprintf(file, "%s,%s,%s,%s\n", current_timestamp, cardholder, card_data, swipe_type);
        fclose(file);
        printf("%s recorded for %s at %s\n", swipe_type, cardholder, current_timestamp);
    } else {
        printf("Error: Could not write to attendance log.\n");
    }
}


int main() {
    ensure_log_file();

    struct libevdev *dev = NULL;
    int fd = open("/dev/input/by-id/usb-0acd_0200-event-kbd", O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Failed to open event device");
        return 1;
    }

    if (libevdev_new_from_fd(fd, &dev) < 0) {
        fprintf(stderr, "Failed to initialize libevdev\n");
        return 1;
    }

    printf("Input device name: \"%s\"\n", libevdev_get_name(dev));
    printf("Reading card swipes...\n");

    char card_data[MAX_CARD_DATA_LEN] = {0};
    int card_data_index = 0;

    struct input_event ev;
    while (1) {
        int rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc == 0 && ev.type == EV_KEY && ev.value == 1) {  // Key press event
            if (ev.code >= KEY_1 && ev.code <= KEY_9) {
                card_data[card_data_index++] = (ev.code - KEY_1 + '1');  // Append digit
            } else if (ev.code == KEY_0) {
                card_data[card_data_index++] = '0';  // Append '0'
            } else if (ev.code == KEY_ENTER) {
                card_data[card_data_index] = '\0';  // Null-terminate the string
                log_attendance(card_data);  // Log the card data
                card_data_index = 0;  // Reset for next swipe
            }
        }
    }

    libevdev_free(dev);
    close(fd);
    return 0;
}
