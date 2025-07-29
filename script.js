/* Front-end only — no server needed */
const form   = document.getElementById('planForm');
const output = document.getElementById('output');

form.addEventListener('submit', e => {
  e.preventDefault();
  output.textContent = 'Generating… ⏳';

  // 1. read and sanitise input
  const subjects = form.subjects.value
                    .split(',')
                    .map(s => s.trim())
                    .filter(Boolean);
  const hoursDay = +form.hoursPerDay.value;
  const days     = +form.days.value;

  if (!subjects.length || hoursDay <= 0 || days <= 0) {
    output.textContent = '❌ Invalid input';
    return;
  }

  // 2. compute schedule (same logic as C++ back-end)
  const pq = subjects.map(s => ({priority: 0, subject: s}));
  const plan = [];

  for (let d = 1; d <= days; ++d) {
    // pop subject with lowest priority
    pq.sort((a, b) => a.priority - b.priority);
    const best = pq.shift();

    plan.push({
      day: d,
      subject: best.subject,
      hours: +(hoursDay / subjects.length).toFixed(2)
    });

    // re-enqueue with higher priority (spaced by +2)
    pq.push({priority: best.priority + 2, subject: best.subject});
  }

  // 3. render result
  output.innerHTML = '';
  plan.forEach(({day, subject, hours}) => {
    const card = document.createElement('div');
    card.className = 'schedule-card';
    card.innerHTML = `
      <strong>Day ${day}</strong>
      <span>${subject}</span>
      <em>${hours} h</em>`;
    output.appendChild(card);
  });
});
