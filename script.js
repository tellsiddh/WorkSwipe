// Fetch CSV automatically and process it, with cache busting
fetch('attendance_log.csv?' + new Date().getTime())  // Appending current timestamp to URL
    .then(response => response.text())
    .then(data => {
        console.log('CSV fetched successfully!');
        Papa.parse(data, {
            header: true,
            dynamicTyping: true,
            complete: function(results) {
                console.log('CSV parsed successfully!', results);

                // Filter out empty rows
                const filteredData = results.data.filter(row => row.Timestamp && row.Cardholder);

                processCSVData(filteredData);  // Pass filtered data to processCSVData
            }
        });
    })
    .catch(error => console.error('Error loading CSV:', error));

function processCSVData(data) {
    console.log('Processing CSV Data:', data);

    // Grouping data by cardholder
    const groupedLogs = {};
    data.forEach(log => {
        const cardholder = log.Cardholder;
        if (!groupedLogs[cardholder]) {
            groupedLogs[cardholder] = { entries: [], exits: [], logs: [] };
        }
        groupedLogs[cardholder].logs.push(log);
        if (log.Type === 'entry') {
            groupedLogs[cardholder].entries.push(new Date(log.Timestamp));
        } else {
            groupedLogs[cardholder].exits.push(new Date(log.Timestamp));
        }
    });

    console.log('Grouped Logs:', groupedLogs);

    // Pass the grouped logs and parsed data into populateTable
    populateTable(groupedLogs, data);
}

function calculateHoursWorked(entries, exits) {
    let totalHours = 0;
    for (let i = 0; i < entries.length && i < exits.length; i++) {
        const diff = (exits[i] - entries[i]) / (1000 * 60 * 60); // Convert milliseconds to hours
        totalHours += diff;
    }
    return totalHours.toFixed(2);
}

// Filter data for current day
function filterCurrentDay(logs) {
    const today = new Date();
    return logs.filter(log => {
        const logDate = new Date(log.Timestamp);
        return logDate.toDateString() === today.toDateString();
    });
}

// Filter data for current week
function filterCurrentWeek(logs) {
    const today = new Date();
    const weekStart = new Date(today.setDate(today.getDate() - today.getDay()));  // Get Sunday of current week
    return logs.filter(log => {
        const logDate = new Date(log.Timestamp);
        return logDate >= weekStart && logDate <= new Date();  // Filter logs in the current week
    });
}

function calculateWeeklyHours(logs) {
    const entries = logs.filter(log => log.Type === 'entry').map(log => new Date(log.Timestamp));
    const exits = logs.filter(log => log.Type === 'exit').map(log => new Date(log.Timestamp));
    return calculateHoursWorked(entries, exits);
}

function createDetailsRow(cardholder, logs) {
    const detailsRow = document.createElement('tr');
    detailsRow.classList.add('details-row');
    let dailyInfo = '';

    logs.forEach(log => {
        const entryExitTime = new Date(log.Timestamp).toLocaleString();
        dailyInfo += `<p><strong>${log.Type.toUpperCase()}:</strong> ${entryExitTime}</p>`;
    });

    detailsRow.innerHTML = `
        <td colspan="4">
            <div class="daily-details">
                <h4>Daily Entry and Exit Times</h4>
                ${dailyInfo}
            </div>
        </td>
    `;
    return detailsRow;
}

function populateTable(groupedLogs, data) {
    const tableBody = document.getElementById('tableBody');
    tableBody.innerHTML = ''; // Clear existing rows

    Object.keys(groupedLogs).forEach(cardholder => {
        const logs = groupedLogs[cardholder].logs;
        const todayLogs = filterCurrentDay(logs);
        const weekLogs = filterCurrentWeek(logs);

        const entries = groupedLogs[cardholder].entries;
        const exits = groupedLogs[cardholder].exits;

        const totalHoursToday = calculateHoursWorked(
            todayLogs.filter(log => log.Type === 'entry').map(log => new Date(log.Timestamp)),
            todayLogs.filter(log => log.Type === 'exit').map(log => new Date(log.Timestamp))
        );

        const totalHoursThisWeek = calculateWeeklyHours(weekLogs);

        let row = document.createElement('tr');
        row.innerHTML = `
            <td>${cardholder}</td>
            <td>${todayLogs.length > 0 && todayLogs[0].Type === 'entry' ? new Date(todayLogs[0].Timestamp).toLocaleString() : '-'}</td>
            <td>${todayLogs.length > 1 && todayLogs[todayLogs.length - 1].Type === 'exit' ? new Date(todayLogs[todayLogs.length - 1].Timestamp).toLocaleString() : '-'}</td>
            <td>${totalHoursToday} (Today) / ${totalHoursThisWeek} (This Week)</td>
        `;

        row.addEventListener('click', () => {
            const existingDetailsRow = document.querySelector('.details-row');
            if (existingDetailsRow) existingDetailsRow.remove();  // Remove any open details rows

            const detailsRow = createDetailsRow(cardholder, logs);
            row.after(detailsRow);  // Insert details row after the clicked row
        });

        tableBody.appendChild(row);
    });
}
