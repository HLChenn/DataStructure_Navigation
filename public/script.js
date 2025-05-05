/**
 * 实现地图绘制与互动的js代码, 使用了 cytoscape.js 库辅助功能实现
 */

async function getJsonG() {
  try {
    const resp = await fetch('/getJsonG');
    const text = await resp.text();
    return JSON.parse(text).elements;
  } catch (err) {
    console.error("[!] /getJsonG error", err);
    return null;
  }
}

const cy = cytoscape({
    container: document.getElementById('cy'),
    elements: getJsonG(),
    style: [{
      selector: 'node', style: {
        'label': 'data(id)',
        'background-color': '#0074D9',
        'color': '#fff',
        'text-valign': 'center',
        'text-halign': 'center',
        'width': 40,
        'height': 40
      }
    }, {
      selector: 'edge', style: {
        'width': 5,
        'line-color': '#888',
        'target-arrow-color': '#888',
        'target-arrow-shape': 'triangle'
      }
    }],
    layout: { name: 'preset' },
    autoungrabify: true
});

let startNode = null;
let endNode = null;
const btnRoot = document.getElementById('btn-root');
const clearBtn = document.getElementById('clear-btn');

function updateClearButton() {
    clearBtn.style.display = startNode && endNode ? 'block' : 'none';
}


function clearAll() {
    if (startNode) cy.getElementById(startNode).style('background-color', '#0074D9');
    if (endNode) cy.getElementById(endNode).style('background-color', '#0074D9');
    startNode = endNode = null;
    updateClearButton();
}

function aStar2Short() {
    var spath = fetch('/aStar').then(console.log("[!] /aStar error"));
    /* to be done (这段注释是我写的不是ai的噢)
     * 1. transfer the paths in format below 
     *      elements: [
     *        { data: { id: '0' }, position: { x: 200, y: 200 } },
     *        { data: { id: '1' }, position: { x: 400, y: 200 } },
     *        { data: { id: '2' }, position: { x: -100, y: 700 } },
     *        { data: { id: '01', source: '0', target: '1' } },
     *        { data: { id: '20', source: '2', target: '0' } }
     *      ],
     * 2. highlight the paths in the map we already draw (no need to draw again, just figure out a way to 'append', you know what I mean about 'append')
     */
}

cy.on('tap', 'node', function (evt) {
    const node = evt.target;
    btnRoot.innerHTML = '';

    if (endNode) {
        const pos = node.renderedPosition();
        const containerRect = cy.container().getBoundingClientRect();
        const btnContainer = document.createElement('div');
        btnContainer.className = 'btn-container';
        btnContainer.style.left = (containerRect.left + pos.x) + 'px';
        btnContainer.style.top = (containerRect.top + pos.y + 20) + 'px';

        const btn = document.createElement('button');
        btn.textContent = '设为起点';
        btn.onclick = (e) => {
          e.stopPropagation();
          clearAll();
          startNode = node.id();
          node.style('background-color', '#2ECC40');
          btnRoot.innerHTML = '';
          updateClearButton();
        };
        btnContainer.appendChild(btn);
        btnRoot.appendChild(btnContainer);
        return;
    }

    const pos = node.renderedPosition();
    const containerRect = cy.container().getBoundingClientRect();
    const btnContainer = document.createElement('div');
    btnContainer.className = 'btn-container';
    btnContainer.style.left = (containerRect.left + pos.x) + 'px';
    btnContainer.style.top = (containerRect.top + pos.y + 20) + 'px';

    if (!startNode) {
        const btn = document.createElement('button');
        btn.textContent = '设为起点';
        btn.onclick = (e) => {
          e.stopPropagation();
          startNode = node.id();
          node.style('background-color', '#2ECC40');
          btnRoot.innerHTML = '';
          updateClearButton();
        };
        btnContainer.appendChild(btn);
    } else if (!endNode && node.id() !== startNode) {
        const btn = document.createElement('button');
        btn.textContent = '设为终点';
        btn.onclick = (e) => {
          e.stopPropagation();
          endNode = node.id();
          node.style('background-color', '#FF4136');
          btnRoot.innerHTML = '';
          updateClearButton();

          aStar2Short();
        };
        btnContainer.appendChild(btn);
    }

    btnRoot.appendChild(btnContainer);
});

cy.on('tap', function (evt) { if (evt.target === cy) btnRoot.innerHTML = ''; });
cy.on('pan zoom panzoom', () => btnRoot.innerHTML = '');
clearBtn.addEventListener('click', clearAll);