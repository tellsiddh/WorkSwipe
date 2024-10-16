from flask import Flask, render_template, jsonify
import csv
from datetime import datetime

from flask import Flask, jsonify, render_template
from flask_httpauth import HTTPBasicAuth

app = Flask(__name__)
auth = HTTPBasicAuth()

# Hardcoded username and password (for demo purposes)
USERS = {
    "admin": "Elibomasu@01",  # Change this to a more secure password
}

@auth.get_password
def get_password(username):
    if username in USERS:
        return USERS[username]
    return None

@auth.error_handler
def unauthorized():
    return jsonify({"error": "Unauthorized access"}), 401




# Helper function to read and parse the CSV file
def read_attendance_csv():
    attendance_data = []
    with open('attendance_log.csv', newline='') as csvfile:
        reader = csv.DictReader(csvfile)
        for row in reader:
            # Convert timestamps to datetime objects
            row['Timestamp'] = datetime.strptime(row['Timestamp'], '%Y-%m-%d %H:%M:%S')
            attendance_data.append(row)
    return attendance_data

from collections import defaultdict
from datetime import timedelta

# Group data by cardholder and calculate hours
def process_attendance_data(attendance_data):
    grouped_logs = defaultdict(lambda: {'entries': [], 'exits': [], 'logs': [], 'daily_hours': defaultdict(list)})

    for log in attendance_data:
        cardholder = log['Cardholder']
        log_date = log['Timestamp'].date()
        
        if log['Type'] == 'entry':
            grouped_logs[cardholder]['entries'].append(log['Timestamp'])
        else:
            grouped_logs[cardholder]['exits'].append(log['Timestamp'])
        
        grouped_logs[cardholder]['logs'].append(log)
        
        # Record logs by day for daily calculations
        grouped_logs[cardholder]['daily_hours'][log_date].append(log)

    # Calculating hours worked
    for cardholder, data in grouped_logs.items():
        entries = data['entries']
        exits = data['exits']
        total_hours = 0
        for i in range(min(len(entries), len(exits))):
            worked_hours = (exits[i] - entries[i]).total_seconds() / 3600  # Convert seconds to hours
            total_hours += worked_hours

        grouped_logs[cardholder]['total_hours'] = round(total_hours, 2)
        
        # Calculate daily hours worked
        for date, logs in data['daily_hours'].items():
            daily_entries = [log['Timestamp'] for log in logs if log['Type'] == 'entry']
            daily_exits = [log['Timestamp'] for log in logs if log['Type'] == 'exit']
            daily_hours = sum(
                (exit_time - entry_time).total_seconds() / 3600
                for entry_time, exit_time in zip(daily_entries, daily_exits)
            )
            grouped_logs[cardholder]['daily_hours'][date] = round(daily_hours, 2)

    return grouped_logs


# Protect the index route with Basic Authentication
@app.route('/')
@auth.login_required
def index():
    # Render template or return some protected resource
    attendance_data = read_attendance_csv()
    grouped_logs = process_attendance_data(attendance_data)
    return render_template('index.html', grouped_logs=grouped_logs)

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000, debug=True)
