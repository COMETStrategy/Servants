// This script handles resetting and clearing selected jobs

let askConfirm = false;
document.addEventListener('DOMContentLoaded', function () {
    // Function to get selected jobs
    function getSelectedJobs() {
        const selectedJobs = [];
        document.querySelectorAll('input[name="selectedjobs"]:checked').forEach(checkbox => {
            selectedJobs.push({
                caseNumber: checkbox.getAttribute('data-casenumber'),
                groupName: checkbox.getAttribute('data-groupname'),
                processId: checkbox.getAttribute('data-processid'),
                servant: checkbox.getAttribute('data-servant'),
            });
        });
        return selectedJobs;
    }

    // Function to send POST request
    function sendPostRequest(url, data) {
        console.log('Sending request to:', url);
        console.log('Request body:', JSON.stringify(data));
        return fetch(url, { // Return the fetch call
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Accept': 'application/json'
            },
            body: JSON.stringify(data)
        })
            .then(response => {
                console.log('Response headers:', response.headers);
                return response.json();
            })
            .then(result => {
                console.log('Success:', result);
                return result; // Return the result for further chaining
            })
            .catch(error => {
                console.error('Error:', error);
                throw error; // Re-throw the error for proper handling
            });
    }

    // Event listener for Restart button
    document.getElementById('restartLink').addEventListener('click', function () {
        const selectedJobs = getSelectedJobs();
        if (selectedJobs.length > 0) {
            // Confirmation before restart
            if (!askConfirm || confirm(`Are you sure you want to reset (stop running and sets to "Queued" status) the ${selectedJobs.length} selected jobs? This action cannot be undone.`)) {
                sendPostRequest('/jobs/selected_restart', {jobs: selectedJobs})
                    .then(() => {
                        location.reload(); // Refresh the page after restart
                    });
            }
        } else {
            alert('No jobs selected for reset.');
        }
    });

    // Event listener for Delete button
    document.getElementById('deleteLink').addEventListener('click', function () {
        const selectedJobs = getSelectedJobs();
        if (selectedJobs.length > 0) {
            // confirmation before deletion
            if (!askConfirm || confirm(`Are you sure you want to DELETE (Stops running, removes from the database, files are left in place) the ${selectedJobs.length} selected jobs? This action cannot be undone.`)) {
                sendPostRequest('/jobs/selected_delete', {jobs: selectedJobs})
                    .then(() => {
                        location.reload(); // Refresh the page after deletion
                    });
            }
        } else {
            alert('No jobs selected for clear.');
        }
    });

    // Event listener for Delete button
    document.getElementById('stopLink').addEventListener('click', function () {
        const selectedJobs = getSelectedJobs();
        if (selectedJobs.length > 0) {
            // confirmation before deletion
            if (!askConfirm || confirm(`Are you sure you want to DELETE (Stops running, removes from the database, files are left in place) the ${selectedJobs.length} selected jobs? This action cannot be undone.`)) {
                sendPostRequest('/jobs/selected_stop', {jobs: selectedJobs})
                    .then(() => {
                        location.reload(); // Refresh the page after deletion
                    });
            }
        } else {
            alert('No jobs selected for clear.');
        }
    });
});

function openLocalFile(filePath) {
    const command = `open "${filePath}"`; // Use `open` for macOS or `explorer` for Windows
    fetch(`/execute-command`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ command })
    }).then(response => {
        if (!response.ok) {
            console.error('Failed to open file:'+ filePath);
            console.error('Command:'+ command);
        }
    });
}