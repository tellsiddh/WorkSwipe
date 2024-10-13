# WorkSwipe - Attendance Tracking System

**WorkSwipe** is a simple, terminal-based attendance tracking system that allows users to log attendance using card swipes and track the working hours of cardholders. It keeps track of entry and exit times, calculates daily and weekly working hours, and supports logging of unknown cardholders.

## Features

- **Swipe Logging**: Cardholders can swipe to log their attendance (entry or exit) using their card data.
- **Daily and Weekly Hours**: The system calculates and displays daily working hours for the past week, as well as the total weekly hours worked.
- **Unknown Cardholders**: Attendance attempts by unknown cardholders are logged with an error for auditing.
- **Simple Command Interface**: The program accepts card swipes as input and provides commands for viewing hours and exiting.

## How It Works

- The user enters their card data (a 10-digit card number).
- The program logs the swipe as either an "entry" or an "exit" based on the last recorded swipe.
- Users can view the daily and weekly hours worked by entering the `view` command and their card number.
- The program logs both known and unknown cardholder swipes into a CSV file (`attendance_log.csv`).

## Installation

1. **Download the Source Code**: Clone or download the source code for WorkSwipe to your local machine.
   
2. **Compile the C Program**: Use the GCC compiler (or another C compiler) to compile the source code.

   gcc workswipe.c -o workswipe

Run the Program: After compilation, run the executable:

    ./workswipe

## Usage
Once the program is running, you can perform the following actions:

Swipe a Card:
Simply enter a card number (10 digits) when prompted.
The program will log it as an entry or exit based on the previous swipe.
If the cardholder is unknown, the system will log it as an error.
View Hours:
Type view and press Enter.
Enter the card number for which you want to see the hours.
The system will display the daily hours for the past 7 days and the total weekly hours worked.
Exit the Program:
Type exit and press Enter to quit the program.
Example Workflow

Welcome! You can swipe your card at any time by entering your card data.
Type 'view' to view hours, 'exit' to quit the program.

Enter card data (or 'view'/'exit'): 34535345345
entry recorded for Demo at 2024-10-13 14:20:45

Enter card data (or 'view'/'exit'): 34535345345
exit recorded for Demo at 2024-10-13 16:35:12

Enter card data (or 'view'/'exit'): view
Enter card data to view hours: 34535345345

Hours worked in the last 7 days:
Day 0: 2.50 hours
Day 1: 0.00 hours
Day 2: 0.00 hours
Day 3: 4.00 hours
Day 4: 0.00 hours
Day 5: 0.00 hours
Day 6: 3.25 hours

Total hours worked this week: 9.75 hours

## Log File
The program creates and maintains an attendance_log.csv file in the same directory. Each entry in the log file includes:

Timestamp: The date and time of the swipe.
Cardholder: The name of the cardholder (or "Unknown Cardholder" for unknown cardholders).
Card Data: The 10-digit card number used during the swipe.
Type: Indicates whether the swipe is an "entry", "exit", or "error" (for unknown cardholders).
Example Log Entry

2024-10-13 14:20:45,Test,423423423423,entry
2024-10-13 16:35:12,Test,43242342342423,exit
2024-10-14 09:00:00,Unknown Cardholder,25255245454,error

## Customization
You can easily modify the list of valid cardholders by editing the cardholders array in the source code. Simply add or remove cardholder records as needed.

## Future Enhancements

Add support for more complex reporting, such as monthly hours or overtime calculations.
Implement authentication for cardholders.
Integrate with external databases to dynamically manage cardholders and logs.
