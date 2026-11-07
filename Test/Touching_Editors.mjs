// Shared editor checks for automation-service point input and Chromium touch acquisition.
import assert from 'node:assert/strict';
import path from 'node:path';
import { fileURLToPath } from 'node:url';

const pause = milliseconds => new Promise(resolve => setTimeout(resolve, milliseconds));
const nodes = value => value === null || typeof value !== 'object' ? [] : [value, ...Object.values(value).flatMap(nodes)];

export async function verifyEditors(input) {
    const all = async () => nodes(await input.controls());
    const label = async text => (await all()).find(node => node.elementText === text);
    const dialog = async () => (await all()).find(node => node.title === 'Select an Image');
    const document = async () => nodes((await all()).find(node => node.control === 'DocumentViewer')).find(node => node.elementDocument);
    const selection = async () => (await document()).element.match(/Selection\([^)]*\)-\([^)]*\)/)[0];
    const images = async () => ((await document()).elementDocument.match(/<img\b/g) ?? []).length;
    const center = bounds => ({ x: Math.floor((bounds.x1 + bounds.x2) / 2), y: Math.floor((bounds.y1 + bounds.y2) / 2) });
    async function until(predicate, description) {
        const deadline = Date.now() + 15000;
        while (!await predicate()) {
            assert(Date.now() < deadline, description);
            await pause(50);
        }
        console.log(`PASS ${input.name} / ${description}`);
    }
    async function tap(text) {
        await until(async () => await label(text), `touch target ${text}`);
        await input.tap(center((await label(text)).bounds));
    }
    async function localized(captions) {
        const caption = (await all()).find(node => captions.includes(node.elementText));
        assert(caption, `Missing ${captions.join('/')}`);
        await input.tap(center(caption.bounds));
    }
    async function open(variant) {
        if (variant === 'Ribbon') await tap('Insert');
        else { await tap('Edit'); await tap('Object'); }
        await tap('Insert Image ...');
        await until(dialog, `${variant}: touch opens image dialog`);
    }
    async function cancel(variant) {
        await localized(['Cancel', '\u53d6\u6d88']);
        await until(async () => !await dialog(), `${variant}: touch cancels image dialog`);
    }
    async function row(text) {
        const list = nodes(await dialog()).find(node => node.control === 'ListView');
        return nodes(list).find(node => node.elementText === text);
    }
    async function selectedFile(text) {
        const box = nodes(await dialog()).find(node => node.control === 'SinglelineTextBox');
        return nodes(box).some(node => node.elementDocument?.includes(text));
    }
    const parentFolder = path.resolve(path.dirname(fileURLToPath(import.meta.url)), 'Resources/App/FullControlTest');
    const imageFolder = path.join(parentFolder, 'ToolbarImages');
    const imageName = 'a_Align.png';
    const folderIs = async folder => nodes(await dialog()).some(node => node.elementText === folder);

    await tap('Control');
    for (const variant of ['Ribbon', 'Toolstrip']) {
        await tap(`Document Editor (${variant})`);
        await until(document, `${variant}: document is ready`);
        const d = await document();
        const start = { x: d.bounds.x1 + 8, y: d.bounds.y1 + 8 };
        await input.mouse(start);
        await input.key('CTRL+HOME');
        await input.key('CTRL+SHIFT+END');
        await input.type('Touch selection regression.');
        await until(async () => (await document()).elementDocument.includes('Touch selection regression.'), `${variant}: ordinary mouse and keyboard edit`);
        await input.key('CTRL+HOME');
        await input.tap(start);
        const end = { x: start.x + 110, y: start.y };
        await input.tap(end, 600);
        await until(async () => {
            const match = (await selection()).match(/Selection(\([^)]*\))-(\([^)]*\))/);
            return match[1] !== match[2];
        }, `${variant}: long press extends text selection`);
        const selected = await selection();
        await input.tap({ x: start.x + 30, y: start.y });
        assert.equal(await selection(), selected, `${variant}: short press preserves existing selection`);
        await input.tap({ x: start.x + 30, y: start.y }, 600);
        await until(async () => (await all()).some(node => node.control === 'Menu'), `${variant}: long press opens context menu`);
        assert.equal(await selection(), selected, `${variant}: context press preserves selection`);
        await input.key('ESC');
        await input.mouse(start);
        await input.key('CTRL+END');
        const originalImages = await images();

        await open(variant);
        await cancel(variant);
        await open(variant);
        const box = nodes(await dialog()).find(node => node.control === 'SinglelineTextBox');
        await input.mouse(center(box.bounds));
        await input.key('CTRL+A');
        await input.type(parentFolder);
        await localized(['Open', '\u6253\u5f00']);
        await until(async () => await folderIs(parentFolder) && await row('ToolbarImages'), `${variant}: real repository folder loaded`);
        await pause(600);
        let target = center((await row('ToolbarImages')).bounds);
        await input.tap(target);
        await until(async () => await selectedFile('ToolbarImages'), `${variant}: single folder tap selects its row`);
        assert(await folderIs(parentFolder));
        await pause(600);
        await input.tap(target);
        await pause(100);
        assert(await folderIs(parentFolder), `${variant}: spaced folder taps do not navigate`);
        await pause(600);
        // Both complete sequences use fresh label coordinates, with no observation or settling between them.
        target = center((await row('ToolbarImages')).bounds);
        await input.pair(target);
        await until(async () => await folderIs(imageFolder), `${variant}: double folder tap navigates exactly once and keeps dialog open`);
        await until(async () => await row(imageName), `${variant}: image label coordinates refreshed`);
        await pause(600);
        target = center((await row(imageName)).bounds);
        await input.tap(target);
        await until(async () => await selectedFile(imageName), `${variant}: single image tap selects its row`);
        await pause(600);
        await input.tap(target);
        await pause(100);
        assert(await dialog(), `${variant}: spaced image taps do not confirm`);
        assert.equal(await images(), originalImages);
        await pause(600);
        target = center((await row(imageName)).bounds);
        await input.pair(target);
        await until(async () => !await dialog() && await images() === originalImages + 1, `${variant}: double image tap closes dialog and inserts exactly one image`);
        const insertedDocument = (await document()).bounds;
        await input.tap({ x: insertedDocument.x1 + 8, y: insertedDocument.y1 + 8 });
        await input.key('CTRL+Z');
        await until(async () => await images() === originalImages, `${variant}: Undo removes that insertion`);
        await input.key('CTRL+Y');
        await until(async () => await images() === originalImages + 1, `${variant}: Redo restores exactly one image`);
        await open(variant);
        await cancel(variant);
    }
}

if (process.argv[1] && path.resolve(process.argv[1]) === fileURLToPath(import.meta.url)) {
    const [core, renderer, name] = process.argv.slice(2);
    assert(core && renderer && name, 'Expected Core endpoint, input endpoint and configuration name');
    async function io(command) {
        const response = await fetch(`${renderer}/IO`, { method: 'POST', headers: { 'Content-Type': 'application/json; charset=utf8' }, body: command, signal: AbortSignal.timeout(15000) });
        assert.equal(await response.text(), 'Queued', command);
    }
    async function tap(point, duration = 0) {
        await io(`!PointDown:170,${point.x},${point.y}`);
        if (duration) await pause(duration);
        await io(`!PointUp:170,${point.x},${point.y}`);
    }
    await verifyEditors({
        name,
        controls: async () => (await fetch(`${core}/Controls`, { signal: AbortSignal.timeout(15000) })).json(),
        tap,
        pair: async point => { await tap(point); await tap(point); },
        mouse: async point => io(`!LeftClick:${point.x},${point.y}`),
        key: async key => io(`!KeyPress:${key}`),
        type: async text => io(`!Type:${text}`),
    });
}
