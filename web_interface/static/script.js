document.addEventListener('DOMContentLoaded', () => {
    // Elements
    const autoModeToggle = document.getElementById('auto-mode-toggle');
    const angleDisplay = document.getElementById('angle-display');
    const controlButtons = document.querySelectorAll('.action-btn');
    const servoArm = document.getElementById('servo-arm');
    const modeDesc = document.querySelector('.mode-desc');

    // State
    let isAutoMode = true;

    // Status Polling
    async function updateStatus() {
        try {
            const response = await fetch('/api/status');
            const data = await response.json();

            if (data.error) throw new Error(data.error);

            // Update Text Values
            document.getElementById('car-state').textContent = data.car_detected ? 'Occupé' : 'Libre';
            document.getElementById('light-state').textContent = data.is_dark ? 'Nuit' : 'Jour';
            document.getElementById('release-counter').textContent = `${data.release_counter}`;

            // Update LEDs
            document.getElementById('led-red').classList.toggle('active', data.led_red);
            document.getElementById('led-green').classList.toggle('active', data.led_green);
            document.getElementById('led-white').classList.toggle('active', data.led_white);

            // Update Visuals
            if (servoArm) {
                servoArm.style.transform = `rotate(${data.servo_angle - 90}deg)`;
            }

            // Update text display if it exists
            if (angleDisplay && isAutoMode) {
                angleDisplay.textContent = data.servo_angle;
            }

            document.getElementById('connection-status').innerHTML = '<span class="dot"></span> Connecté';

        } catch (e) {
            console.error('Error fetching status:', e);
            document.getElementById('connection-status').innerHTML = '<span class="dot" style="background: red"></span> Déconnecté';
        }
    }

    // Start Polling
    setInterval(updateStatus, 1000);
    updateStatus();

    // Auto/Manual Toggle
    autoModeToggle.addEventListener('change', async (e) => {
        isAutoMode = e.target.checked;

        // UI Updates
        if (isAutoMode) {
            modeDesc.textContent = "Le système est en mode automatique";
            controlButtons.forEach(btn => btn.disabled = true);
            await setServoAngle(255); // Send auto mode command
        } else {
            modeDesc.textContent = "Le contrôle manuel est activé";
            controlButtons.forEach(btn => btn.disabled = false);
        }
    });

    // Global Functions
    window.setAngle = async (angle) => {
        if (isAutoMode) return;

        // Update visual immediately for responsiveness
        if (servoArm) {
            servoArm.style.transform = `rotate(${angle - 90}deg)`;
        }
        await setServoAngle(angle);
    };

    // API Helper
    async function setServoAngle(angle) {
        try {
            const response = await fetch('/api/servo', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json'
                },
                body: JSON.stringify({ angle: angle })
            });
            await response.json();
        } catch (e) {
            console.error('Error sending command:', e);
        }
    }
});

