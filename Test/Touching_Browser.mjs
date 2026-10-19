// Run with a fresh FullControlTest Core on port 8888 and the built GacJS site on 8896.
import assert from 'node:assert/strict';
import fs from 'node:fs';
import os from 'node:os';
import path from 'node:path';
import { createRequire } from 'node:module';
import { pathToFileURL } from 'node:url';
import { createHash } from 'node:crypto';

const requireGacJS = createRequire(new URL('../../GacJS/Gaclib/package.json', import.meta.url));
const { chromium } = requireGacJS('playwright');
const { parseNetworkPackage } = await import(pathToFileURL(requireGacJS.resolve('@gaclib-website/remote-protocol-http/channel')));
const browser = await chromium.launch({ headless: true });
const context = await browser.newContext({ viewport: { width: 800, height: 600 }, hasTouch: true });
const page = await context.newPage();
const evidence = { transport: process.argv[2], input: 'Chromium CDP touch', checks: [], errors: [], screenshots: [] };
const events = [];
let idlePending = false, idleResolve = null;
await page.exposeFunction('__gacui_playwright_idle', () => {
    if (idleResolve !== null) { const resolve = idleResolve; idleResolve = null; resolve(); }
    else idlePending = true;
});
await page.exposeFunction('__gacui_playwright_blink', () => {});
page.on('pageerror', error => evidence.errors.push(error.message));
page.on('request', request => {
    if (request.method() !== 'POST' || !request.url().includes(':8888/')) return;
    const body = request.postData();
    if (body === null) return;
    const packet = parseNetworkPackage(body);
    if (packet.channelName !== 'GacUIRemoteProtocol') return;
    for (const event of JSON.parse(packet.messageBody)) if (event.semantic === 'Event') events.push(event);
});
async function idle(timeout = 1000, required = false) {
    if (idlePending) { idlePending = false; return; }
    await new Promise((resolve, reject) => {
        // Unchanged input need not produce a renderer idle request.
        const timer = setTimeout(() => {
            idleResolve = null;
            if (required) reject(new Error('Renderer idle timeout'));
            else resolve();
        }, timeout);
        idleResolve = () => { clearTimeout(timer); resolve(); };
    });
}
async function until(predicate, description) {
    const deadline = Date.now() + 30000;
    while (!await predicate()) {
        assert(Date.now() < deadline, description);
        await idle();
    }
    evidence.checks.push(description);
    console.log(`PASS ${description}`);
}
const nodes = value => {
    if (value === null || typeof value !== 'object') return [];
    return [value, ...Object.values(value).flatMap(nodes)];
};
const controls = async () => (await fetch('http://localhost:8888/Automation/RemotingTest_Core/Controls')).json();
const all = async () => nodes(await controls());
const label = async text => (await all()).find(node => node.elementText === text);
const item = async number => (await all()).find(node => node.elementText?.startsWith(`Item ${number} `));
const raw = async () => (await all()).find(node => node.elementId === 'FullControlTest.Touching');
const state = async () => (await raw()).elementData.split(',').map(Number);
async function screenPoint(bounds, x, y) {
    const screen = await page.locator('#gacui-screen').boundingBox();
    const root = (await controls()).MainWindow.bounds;
    return { x: screen.x + (bounds.x1 + x) * screen.width / (root.x2 - root.x1),
        y: screen.y + (bounds.y1 + y) * screen.height / (root.y2 - root.y1) };
}
async function click(text) {
    const node = await label(text);
    assert(node !== undefined, `Missing caption ${text}`);
    const b = node.bounds, p = await screenPoint(b, (b.x2-b.x1)/2, (b.y2-b.y1)/2);
    idlePending = false;
    await page.mouse.click(p.x, p.y);
    await idle();
}
const cdp = await context.newCDPSession(page);
await cdp.send('Emulation.setTouchEmulationEnabled', { enabled: true, maxTouchPoints: 5 });
const touch = async (type, fingers) => {
    idlePending = false;
    await cdp.send('Input.dispatchTouchEvent', { type, touchPoints: fingers });
    await idle();
};
const finger = (id, p) => ({ id, ...p, radiusX: 6, radiusY: 8, force: 0.7 });
function noCompatibility(start) {
    assert.deepEqual(events.slice(start).filter(event => /^(IOButton|IOMouse)/.test(event.name)), []);
    evidence.checks.push('Touch release emits no compatibility mouse/button events');
}
async function picture(name) {
    const target = path.join(os.tmpdir(), `touch-step3-${name}.png`);
    await page.screenshot({ path: target }); evidence.screenshots.push(target);
}
try {
    await page.goto('http://localhost:8896/index.html');
    await idle(30000, true);
    await until(async () => await label('Misc') !== undefined, 'FullControlTest rendered');
    const outerScroll = await page.evaluate(() => [scrollX, scrollY]);
    await click('Misc'); await click('Touching');
    await until(async () => await raw() !== undefined, 'Touching raw element');
    const bounds = (await raw()).bounds, initial = await state();
    let a = finger(10, await screenPoint(bounds, 140, 120));
    let b = finger(20, { x: a.x+100, y: a.y });
    const start = events.length;
    await touch('touchStart', [a]);
    a = { ...a, x:a.x+30, y:a.y+20 };
    await touch('touchMove', [a]);
    await until(async () => { const s=await state(); return s[4]===1 && Math.abs(s[0]-initial[0]-30)<=1 && Math.abs(s[1]-initial[1]-20)<=1; }, 'One contact translates');
    const pixels=()=>page.locator('#gacui-screen canvas').evaluate(canvas=>{
        const data=canvas.getContext('2d').getImageData(0,0,canvas.width,canvas.height).data;
        let sky=0,yellow=0;
        for(let i=0;i<data.length;i+=4){if(data[i]===135&&data[i+1]===206&&data[i+2]===235)sky++;if(data[i]===255&&data[i+1]===255&&data[i+2]===0)yellow++;}
        return {sky,yellow};
    });
    await until(async()=>{const p=await pixels();return p.sky>1000&&p.yellow>20;},'Browser paints transformed rectangle and contact marker');
    const marker=await state(); assert.equal(marker[5],0); assert(Math.abs(marker[6]-170)<=1&&Math.abs(marker[7]-140)<=1);
    b = { ...b, x:a.x+100, y:a.y };
    await touch('touchStart', [a,b]);
    assert.equal((await state())[4],2);
    b = { ...b, x:a.x, y:a.y+160 };
    await touch('touchMove', [a,b]);
    await until(async () => Math.abs((await state())[3]-initial[2]*1.6)<0.001, 'Two contacts rotate and scale');
    const transformed=(await state()).slice(0,4);
    let c=finger(30,{x:a.x+40,y:a.y+40});
    await touch('touchStart',[a,b,c]); c={...c,x:c.x+40}; await touch('touchMove',[a,b,c]);
    assert.equal((await state())[4],3);
    assert.deepEqual((await state()).slice(0,4),transformed);
    await touch('touchEnd',[c]);
    assert.deepEqual((await state()).slice(0,4),transformed);
    await touch('touchCancel',[]);
    await until(async () => (await state())[4]===0,'Three-contact pause, membership rebase and canceled marker cleanup');
    await touch('touchStart',[a]); await touch('touchEnd',[]);
    await until(async () => (await state())[4]===0,'Fresh successful sequence clears markers');
    await until(async()=>(await pixels()).yellow===0,'Browser clears contact marker pixels');
    noCompatibility(start);
    await picture('touching');

    await click('TouchingList'); await click('Clear'); await click('Clear');
    await until(async () => !(await all()).some(node=>node.elementText?.startsWith('Item ')),'Clear populated and empty lists');
    await click('Add 5 Items'); await click('Add 5 Items');
    await until(async () => await item(7)!==undefined,'Add actions create new row identities');
    const row=await item(7), left=row.bounds.x1;
    const deletion=(await all()).find(node=>node.elementText==='DELETE IT' && node.bounds.y1<=row.bounds.y1 && node.bounds.y2>=row.bounds.y2);
    const maximum=deletion.bounds.x2-deletion.bounds.x1+32, half=Math.floor(maximum/2);
    assert(maximum>32 && row.bounds.y2-row.bounds.y1>=70 && row.element.includes('WrapLine'));
    const origin=await screenPoint(row.bounds,180,25), swipeStart=events.length;
    a=finger(101,origin);
    const reveal=distance=>until(async()=>Math.abs((await item(7)).bounds.x1-left+distance)<=1,`Reveal ${distance}`);
    await touch('touchStart',[a]); await touch('touchMove',[{...a,x:a.x+50}]); await reveal(0); await touch('touchEnd',[]);
    await touch('touchStart',[a]); await touch('touchMove',[{...a,x:a.x-half}]); await reveal(half); await picture('partial-reveal');
    await touch('touchMove',[{...a,x:a.x-half+15}]); await reveal(half-15); await touch('touchEnd',[]); await reveal(0);
    await picture('reset');
    await touch('touchStart',[a]); await touch('touchMove',[{...a,x:a.x-maximum-50}]); await reveal(maximum);
    b=finger(102,{x:a.x+20,y:a.y}); await touch('touchStart',[{...a,x:a.x-maximum-50},b]); await reveal(0);
    await touch('touchEnd',[]); await reveal(0);
    await touch('touchStart',[a]); await touch('touchMove',[{...a,x:a.x-maximum}]); await reveal(maximum);
    await touch('touchCancel',[]); await reveal(0);
    await touch('touchStart',[a]); await touch('touchMove',[{...a,x:a.x-maximum}]); await reveal(maximum);
    await touch('touchEnd',[]);
    await until(async()=>await item(7)===undefined && Math.abs((await item(8)).bounds.y1-row.bounds.y1)<=1,'Successful swipe removes only the middle identity');
    await picture('deleted');
    const next=await item(8);
    await touch('touchStart',[a]); await touch('touchMove',[{...a,y:a.y-80}]); await touch('touchEnd',[]);
    await until(async()=>(await item(8)).bounds.y1<next.bounds.y1,'TouchingList scrolls vertically');
    await touch('touchStart',[a]); await touch('touchMove',[{...a,y:a.y+300}]); await touch('touchEnd',[]);
    await until(async()=>(await item(8)).bounds.x1===left,'Recycled rows retain no swipe reveal');
    noCompatibility(swipeStart);

    await click('List'); await click('ListView');
    const firstList=async()=>(await all()).find(node=>node.control==='ListView');
    const taskRow=async()=>nodes(await firstList()).find(node=>node.elementText==='Task');
    const task=await taskRow(); assert(task!==undefined);
    a=finger(201,await screenPoint(task.bounds,8,8));
    await touch('touchStart',[a]); await touch('touchEnd',[]);
    const rowPatch=async()=>{
        const node=await taskRow(); const p=await screenPoint(node.bounds,0,0);
        return createHash('sha256').update(await page.screenshot({clip:{x:p.x,y:p.y,width:node.bounds.x2-node.bounds.x1,height:node.bounds.y2-node.bounds.y1}})).digest('hex');
    };
    const selected=await rowPatch();
    const list=await firstList();
    a=finger(202,await screenPoint(list.bounds,100,150));
    const listStart=events.length;
    await touch('touchStart',[a]); await touch('touchMove',[{...a,y:a.y-60}]);
    await until(async()=>{const n=await taskRow();return n===undefined||n.bounds.y1<task.bounds.y1;},'ListView content moves');
    await touch('touchMove',[a]); await touch('touchEnd',[]);
    await until(async()=>{const n=await taskRow();return n!==undefined&&n.bounds.y1===task.bounds.y1;},'ListView reverses to original position');
    assert.equal(await rowPatch(),selected); noCompatibility(listStart);
    evidence.checks.push('ListView selection rendering survives scrolling without activation');

    for(let pass=0;pass<2;pass++) {
        await click('Control');
        await until(async()=>await label('TextBox')!==undefined,'Control page is ready');
        await click('TextBox');
        const document=()=>all().then(values=>values.find(node=>node.elementDocument?.includes('I am the bone')));
        await until(async()=>await document()!==undefined,'Multiline editor inside scroll container');
        const d=await document(); a=finger(301+pass,await screenPoint(d.bounds,8,8));
        const textStart=events.length;
        await touch('touchStart',[a]); await touch('touchMove',[{...a,x:a.x+100,y:a.y+25}]);
        await until(async()=>{const m=(await document()).element.match(/Selection(\([^)]*\))-(\([^)]*\))/);return m!==null&&m[1]!==m[2];},'Touch creates a nonempty text range');
        const selection=(await document()).element;
        await touch('touchMove',[{...a,x:a.x+180,y:a.y+50}]);
        await until(async()=>(await document()).element!==selection,'Continued touch changes text range');
        await touch('touchEnd',[]); noCompatibility(textStart);
        await picture(pass===0?'text-selection':'text-selection-aurora');
        if(pass===0){
            await click('Window Manager'); await click('Aurora');
            await until(async()=>(await all()).some(node=>node.element?.includes('#168A7A')),'Aurora templates are installed');
        }
    }
    assert.deepEqual(await page.evaluate(()=>[scrollX,scrollY]),outerScroll);
    assert.deepEqual(evidence.errors,[]);
    evidence.checks.push('Outer browser page remains stationary; text selection survives template replacement');
    console.log(JSON.stringify(evidence,null,2));
} finally {
    fs.writeFileSync(path.join(os.tmpdir(),'touch-step3-browser.json'),JSON.stringify(evidence,null,2));
    await browser.close();
}
