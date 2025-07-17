// This script handles resetting and clearing selected jobs

let askConfirm = false;
document.addEventListener('DOMContentLoaded', function () {
    // Function to get selected jobs
    function getSelectedJobs() {
        const selectedJobs = [];
        document.querySelectorAll('input[name="selectedjobs"]:checked').forEach(checkbox => {
            selectedJobs.push(Object.fromEntries(
                Array.from(checkbox.attributes)
                    .filter(attr => attr.name.startsWith('data-'))
                    .map(attr => [attr.name.replace('data-', ''), attr.value])
            ));
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

    let aLink;
    aLink = document.getElementById('restartLink');
    if (aLink) {
        aLink.addEventListener('click', function () {
            aLink = document.getElementById('restartLink');
            const selectedJobs = getSelectedJobs();
            if (selectedJobs.length > 0) {
                if (!askConfirm || confirm(`Are you sure you want to reset the ${selectedJobs.length} selected rows? This action cannot be undone.`)) {
                    const targetAddress = aLink.getAttribute('targetAddress')
                    sendPostRequest(targetAddress, {rows: selectedJobs})
                        .then(() => {
                            location.reload();
                        });
                }
            } else {
                alert('No jobs selected for reset.');
            }
        });
    }
    ;

    // Event listener for Delete button
    aLink = document.getElementById('deleteLink');
    if (aLink) {
        aLink.addEventListener('click', function () {
            aLink = document.getElementById('deleteLink');
            const selectedJobs = getSelectedJobs();
            if (selectedJobs.length > 0) {
                // confirmation before deletion
                if (!askConfirm || confirm(`Are you sure you want to DELETE ( ${selectedJobs.length} selected rows? This action cannot be undone.`)) {
                    const targetAddress = aLink.getAttribute('targetAddress')
                    sendPostRequest(targetAddress, {rows: selectedJobs})
                        .then(() => {
                            location.reload(); // Refresh the page after deletion
                        });
                }
            } else {
                alert('No jobs selected for clear.');
            }
        });
    }
    ;

    // Event listener for STOP link
    aLink = document.getElementById('stopLink');
    if (aLink) {
        aLink.addEventListener('click', function () {
            aLink = document.getElementById('stopLink');
            const selectedJobs = getSelectedJobs();
            if (selectedJobs.length > 0) {
                // confirmation before deletion
                if (!askConfirm || confirm(`Are you sure you want to STOP the ${selectedJobs.length} selected rows? This action cannot be undone.`)) {
                    const targetAddress = aLink.getAttribute('targetAddress');
                    sendPostRequest(targetAddress, {rows: selectedJobs})
                        .then(() => {
                            location.reload(); // Refresh the page after deletion
                        });
                }
            } else {
                alert('No jobs selected for clear.');
            }
        });
    }
    ;

    document.getElementById("toggleAll").addEventListener("change", function () {
        const checkboxes = document.querySelectorAll("input[name='selectedjobs']");
        checkboxes.forEach((checkbox) => {
            checkbox.checked = this.checked;
        });
    });

    document.querySelectorAll('input[name="selectedjobs"]').forEach(checkbox => {
        checkbox.addEventListener('change', function () {

            const checkboxes = document.querySelectorAll('input[name="selectedjobs"]');
            const allChecked = Array.from(checkboxes).every(cb => cb.checked);
            const noneChecked = Array.from(checkboxes).every(cb => !cb.checked);

            const toggleAll = document.getElementById('toggleAll');
            if (allChecked) {
                toggleAll.checked = true;
                toggleAll.indeterminate = false; // Remove gray state
            } else if (noneChecked) {
                toggleAll.checked = false;
                toggleAll.indeterminate = false; // Remove gray state
            } else {
                toggleAll.checked = false;
                toggleAll.indeterminate = true; // Set gray state
            }
            toggleLinks();
        });
    });

});

function toggleLinks() {
    const checkbox = document.getElementById('toggleAll');
    const linksContainer = document.getElementById('linksContainer'); // Ensure this matches your new div's id
    linksContainer.style.display = checkbox.checked || checkbox.indeterminate ? 'block' : 'none';
}

function openLocalFile(filePath) {
    const command = `open "${filePath}"`; // Use `open` for macOS or `explorer` for Windows
    fetch(`/execute-command`, {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({command})
    }).then(response => {
        if (!response.ok) {
            console.error('Failed to open file:' + filePath);
            console.error('Command:' + command);
        }
    });
}