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

# Group data by cardholder and calculate hours
def process_attendance_data(attendance_data):
    grouped_logs = {}
    
    for log in attendance_data:
        cardholder = log['Cardholder']
        if cardholder not in grouped_logs:
            grouped_logs[cardholder] = {'entries': [], 'exits': [], 'logs': []}
        
        if log['Type'] == 'entry':
            grouped_logs[cardholder]['entries'].append(log['Timestamp'])
        else:
            grouped_logs[cardholder]['exits'].append(log['Timestamp'])
        
        grouped_logs[cardholder]['logs'].append(log)
    
    # Calculating hours worked
    for cardholder, data in grouped_logs.items():
        entries = data['entries']
        exits = data['exits']
        total_hours = 0
        for i in range(min(len(entries), len(exits))):
            total_hours += (exits[i] - entries[i]).total_seconds() / 3600  # Convert seconds to hours
        grouped_logs[cardholder]['total_hours'] = round(total_hours, 2)
    
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
