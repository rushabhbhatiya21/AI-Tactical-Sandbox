// Shared multiple-choice quiz widget.
// Usage: <div class="quiz" data-quiz='[{"q":"...","options":["a","b"],"answer":1,"why":"..."}]'></div>
// Gives immediate feedback on click and explains the answer.
document.querySelectorAll(".quiz[data-quiz]").forEach((root, quizIndex) => {
  const questions = JSON.parse(root.dataset.quiz);
  questions.forEach((item, qi) => {
    const block = document.createElement("div");
    block.className = "q";
    const prompt = document.createElement("p");
    prompt.textContent = `${qi + 1}. ${item.q}`;
    block.appendChild(prompt);

    const why = document.createElement("div");
    why.className = "why";

    item.options.forEach((text, oi) => {
      const label = document.createElement("label");
      const input = document.createElement("input");
      input.type = "radio";
      input.name = `quiz${quizIndex}-q${qi}`;
      label.append(input, " ", text);
      input.addEventListener("change", () => {
        block.querySelectorAll("label").forEach(l => l.classList.remove("right", "wrong"));
        const correct = oi === item.answer;
        label.classList.add(correct ? "right" : "wrong");
        why.textContent = correct ? `Correct. ${item.why}` : "Not quite. Try again.";
      });
      block.appendChild(label);
    });

    block.appendChild(why);
    root.appendChild(block);
  });
});
