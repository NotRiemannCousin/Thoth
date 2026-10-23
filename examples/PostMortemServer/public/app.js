// The page itself is a plain GET rendered by the server. Everything that changes something is a call to the JSON API
// (/api/*), made from here: [edit] and [delete] buttons, and every <form data-api="...">. The session cookie goes
// along by itself (same origin).
//
// The API answers with honest status codes and no page, so what to show for each failure is decided in this file:
// a <dialog> with the reason, and the forms keep what was typed.

const notice = document.getElementById('notice');
const noticeText = document.getElementById('notice-text');

function showNotice(message) {
  noticeText.textContent = message;
  if (!notice.open) notice.showModal();
}

// The server fills the dialog when something is wrong with the URL of the page itself (e.g. ?reply=999).
if (noticeText.textContent.trim()) notice.showModal();

const reasons = {
  400: 'The server could not read that request.',
  401: 'You need to be logged in to do that.',
  403: 'That one is not yours.',
  404: 'That post or reply does not exist (anymore).',
  405: 'That action is not allowed here.',
  422: 'The server refused what was sent: check the fields (a title and a message are required, up to 5 tags).',
};

// One call to the API. Returns the response, or null after telling the user what went wrong.
async function call(method, url, body) {
  const options = { method };
  if (body !== undefined) {
    options.headers = { 'Content-Type': 'application/json' };
    options.body = JSON.stringify(body);
  }

  let response;
  try {
    response = await fetch(url, options);
  } catch {
    showNotice('Could not reach the server.');
    return null;
  }

  if (response.ok) return response;

  showNotice(reasons[response.status] ?? `The server answered ${response.status} ${response.statusText}`);
  return null;
}

// The page is rendered by the server, so the simplest way to show a change is to ask for the page again.
// `href` (what a create answers with) is where to land, like /#post-0007.
function refresh(href) {
  if (!href) {
    location.reload();
    return;
  }

  const target = new URL(href, location.href);
  const samePage = target.pathname === location.pathname && target.search === location.search;

  location.href = target;
  if (samePage) location.reload(); // only the #fragment changes, that would not load anything by itself
}

// A <form data-api="/api/..."> is sent as JSON. Fields marked data-list are comma separated lists (sent as arrays)
// and data-number are sent as numbers, everything else goes as text.
function formToJson(form) {
  const body = {};
  for (const field of form.elements) {
    if (!field.name) continue;

    if (field.hasAttribute('data-list')) body[field.name] = field.value.split(',');
    else if (field.hasAttribute('data-number')) body[field.name] = Number(field.value);
    else body[field.name] = field.value;
  }
  return body;
}

document.addEventListener('submit', async (event) => {
  const form = event.target.closest('form[data-api]');
  if (!form) return;

  event.preventDefault();

  const response = await call('POST', form.dataset.api, formToJson(form));
  if (!response) return;

  // Creates answer 201 with { id, href }, login and logout answer 204 with nothing.
  const created = response.status === 201 ? await response.json() : null;
  refresh(created?.href);
});

document.addEventListener('click', async (event) => {
  const button = event.target.closest('[data-action]');
  if (!button) return;

  const { action, url } = button.dataset;
  let response;

  if (action === 'delete') {
    if (!confirm('Delete it for good?')) return;
    response = await call('DELETE', url);
  } else if (action === 'edit') {
    const title = prompt('Title', button.dataset.title);
    if (title === null) return;
    const content = prompt('Content', button.dataset.content);
    if (content === null) return;

    response = await call('PUT', url, { title, content });
  } else {
    return;
  }

  if (response) refresh();
});
