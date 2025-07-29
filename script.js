/* Front-end controller */
const form = document.getElementById('planForm');
const output = document.getElementById('output');

form.addEventListener('submit', async (e) => {
    e.preventDefault();
    output.innerHTML = "<p>Generating… ⏳</p>";

    const payload = {
        subjects: e.target.subjects.value,
        hoursPerDay: +e.target.hoursPerDay.value,
        days: +e.target.days.value
    };

    try {
        const res = await fetch('/schedule', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(payload)
        });
        const data = await res.json();
        renderSchedule(data);
    } catch (err) {
        output.innerHTML = `<p style="color:#f66;">Error: ${err}</p>`;
    }
});

function renderSchedule({ plan }) {
    /* plan: [{day:1, subject:"DSA", hours:2}, …] */
    output.innerHTML = '';
    plan.forEach(item => {
        const card = document.createElement('div');
        card.className = 'schedule-card';
        card.innerHTML = `
        <span><strong>Day ${item.day}</strong> — ${item.subject}</span>
        <span>${item.hours} h</span>`;
        output.appendChild(card);
    });
}
