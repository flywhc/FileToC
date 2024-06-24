
async function toggleLED() {
    try {
        const response = await fetch("/api/toggle_led", {
            method: "POST",
            headers: {
                "Content-Type": "application/json"
            }
        });

        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }

        const data = await response.text();
        console.log("Toggle LED status. Response: " + data);

        // change button color according to response
        const ledButton = document.getElementById('lightbulbImage');
        if (data === "true") {
            lightbulbImage.src = "img/lightbulb_on.png";
        } else {
            lightbulbImage.src = "img/lightbulb_off.png";
        }
    } catch (error) {
        // error handling
        console.error("Failed:", error);
    }
}

window.onload = function() {
    toggleLED();
};
