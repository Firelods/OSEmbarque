```javascript
document.addEventListener('DOMContentLoaded', () => {
    // Elements
    const autoModeToggle = document.getElementById('auto-mode-toggle');
    const servoControls = document.querySelector('.threshold-controls');
    const servoSlider = document.getElementById('servo-slider');
    const angleDisplay = document.getElementById('angle-display');
    const controlButtons = document.querySelectorAll('.action-btn');
    const servoArm = document.getElementById('servo-arm');
    const modeDesc = document.querySelector('.mode-desc');
    const stepBtns = document.querySelectorAll('.step-btn');
    
    // State
    let isAutoMode = true;
    
    // Status Polling
    async function updateStatus() {
        try {
            const response = await fetch('/api/status');
            const data = await response.json();
            
            if (data.error) throw new Error(data.error);
            
            // Update Text Values
            document.getElementById('car-state').textContent = data.car_detected ? 'Detected' : 'Empty';
            document.getElementById('light-state').textContent = data.is_dark ? 'Dark' : 'Light';
            document.getElementById('release-counter').textContent = `${ data.release_counter } `;
            
            // Update LEDs
            document.getElementById('led-red').classList.toggle('active', data.led_red);
            document.getElementById('led-green').classList.toggle('active', data.led_green);
            document.getElementById('led-white').classList.toggle('active', data.led_white);
            
            // Update Visuals
            servoArm.style.transform = `rotate(${ data.servo_angle - 90 }deg)`; // -90 to map 0-180 to -90 to +90 vertical
            
            // Update inputs if in auto mode
            if (isAutoMode) {
                servoSlider.value = data.servo_angle;
                angleDisplay.textContent = data.servo_angle;
            }
            
            document.getElementById('connection-status').innerHTML = '<span class="dot"></span> Connected';
            
        } catch (e) {
            console.error('Error fetching status:', e);
            document.getElementById('connection-status').innerHTML = '<span class="dot" style="background: red"></span> Disconnected';
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
            modeDesc.textContent = "The system is in automatic mode";
            servoSlider.disabled = true;
            controlButtons.forEach(btn => btn.disabled = true);
            stepBtns.forEach(btn => btn.disabled = true);
            await setServoAngle(255); // Send auto mode command
        } else {
            modeDesc.textContent = "Manual control enabled";
            servoSlider.disabled = false;
            controlButtons.forEach(btn => btn.disabled = false);
            stepBtns.forEach(btn => btn.disabled = false);
        }
    });

    // Slider Control
    servoSlider.addEventListener('input', (e) => {
        angleDisplay.textContent = e.target.value;
        servoArm.style.transform = `rotate(${ e.target.value - 90 }deg)`;
    });

    servoSlider.addEventListener('change', async (e) => {
        if (!isAutoMode) {
            await setServoAngle(parseInt(e.target.value));
        }
    });

    // Global Functions
    window.adjustAngle = async (delta) => {
        if (isAutoMode) return;
        let newAngle = parseInt(servoSlider.value) + delta;
        newAngle = Math.max(0, Math.min(180, newAngle));
        
        servoSlider.value = newAngle;
        angleDisplay.textContent = newAngle;
        servoArm.style.transform = `rotate(${ newAngle - 90}deg)`;
        await setServoAngle(newAngle);
    };

    window.setAngle = async (angle) => {
        if (isAutoMode) return;
        servoSlider.value = angle;
        angleDisplay.textContent = angle;
        servoArm.style.transform = `rotate(${ angle - 90}deg)`;
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
```
