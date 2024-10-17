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
            groupedLogs[cardholder] = { entries: [], exits: [] };
        }
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

function populateTable(groupedLogs, data) {
    const tableBody = document.getElementById('tableBody');
    tableBody.innerHTML = ''; // Clear existing rows

    Object.keys(groupedLogs).forEach(cardholder => {
        const entries = groupedLogs[cardholder].entries;
        const exits = groupedLogs[cardholder].exits;
        const totalHours = calculateHoursWorked(entries, exits);

        // We assume that the first matching entry for this cardholder will have the CardData
        const cardData = data.find(log => log.Cardholder === cardholder).CardData;

        let row = document.createElement('tr');
        row.innerHTML = `
            <td>${cardholder}</td>
            <td>${entries.length > 0 ? entries[0].toLocaleString() : '-'}</td>
            <td>${exits.length > 0 ? exits[exits.length - 1].toLocaleString() : '-'}</td>
            <td>${totalHours}</td>
        `;
        tableBody.appendChild(row);
    });
}
