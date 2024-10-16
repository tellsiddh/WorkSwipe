#include <stdio.h>
#include <stdlib.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <libevdev/libevdev.h>

#define MAX_CARD_DATA_LEN 256

void log_attendance(const char *card_data) {
    printf("Card data logged: %s\n", card_data);  // Replace with your logging logic
}

int main() {
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
