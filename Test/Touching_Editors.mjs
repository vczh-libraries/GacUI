// Shared editor checks for automation-service point input and Chromium touch acquisition.
import assert from 'node:assert/strict';
import path from 'node:path';
import { writeFile } from 'node:fs/promises';
import { fileURLToPath } from 'node:url';

const pause = milliseconds => new Promise(resolve => setTimeout(resolve, milliseconds));
const nodes = value => value === null || typeof value !== 'object' ? [] : [value, ...Object.values(value).flatMap(nodes)];

export async function verifyEditors(input) {
    let lastControls, lastTap;
    const all = async () => nodes(lastControls = await input.controls());
    const label = async text => (await all()).find(node => node.elementText === text);
    const dialog = async () => (await all()).find(node => node.title === 'Select an Image');
    const document = async () => nodes((await all()).find(node => node.control === 'DocumentViewer')).find(node => node.elementDocument);
    const selection = async () => (await document()).element.match(/Selection\([^)]*\)-\([^)]*\)/)[0];
    const documentText = async () => (await document()).elementDocument.split('<Content>')[1].split('</Content>')[0].replace(/<sp\s*\/>/g, ' ').replace(/<[^>]*>/g, '');
    const images = async () => ((await document()).elementDocument.match(/<img\b/g) ?? []).length;
    const center = bounds => ({ x: Math.floor((bounds.x1 + bounds.x2) / 2), y: Math.floor((bounds.y1 + bounds.y2) / 2) });
    async function until(predicate, description) {
        const deadline = Date.now() + 15000;
        while (!await predicate()) {
            if (Date.now() >= deadline) {
                const snapshot = path.resolve(path.dirname(fileURLToPath(import.meta.url)), 'Output', 'Injection', 'editor-point-timeout.json');
                await writeFile(snapshot, JSON.stringify({ lastTap, controls: await input.controls() }, null, 2));
                assert.fail(description);
            }
            await pause(50);
        }
        console.log(`PASS ${input.name} / ${description}`);
    }
    async function tap(text) {
        // Navigation commands are single clicks; explicit pair tests below
        // bypass this helper and retain the generated double-click interval.
        await pause(600);
        let target, previous, stable = 0;
        await until(async () => {
            target = await label(text);
            if (!target || target.bounds.x2 <= target.bounds.x1 || target.bounds.y2 <= target.bounds.y1) return false;
            const window = nodes(lastControls).find(node => node.title !== undefined && nodes(node.composition).includes(target));
            const point = center(target.bounds);
            if (!window || point.x < window.bounds.x1 || point.x >= window.bounds.x2 || point.y < window.bounds.y1 || point.y >= window.bounds.y2) return false;
            const current = JSON.stringify(target.bounds);
            stable = current === previous ? stable + 1 : 0;
            previous = current;
            return stable >= 2;
        }, `settled touch target ${text}`);
        lastTap = { text, point: center(target.bounds), controls: lastControls };
        await input.tap(center(target.bounds));
    }
    async function localized(captions) {
        const caption = (await all()).find(node => captions.includes(node.elementText));
        assert(caption, `Missing ${captions.join('/')}`);
        await tap(caption.elementText);
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
        if (input.fixture) {
            await tap(variant === 'Ribbon' ? '  HOME  ' : 'File');
            await tap('Load as Private Format');
            const picker = async () => (await all()).find(node => node.title === 'Load a GacUI Document');
            await until(picker, `${variant}: fixture picker opens`);
            const fields = nodes(await picker()).filter(node => node.control === 'SinglelineTextBox');
            assert(fields.length, 'Fixture picker filename field');
            await input.mouse(center(fields.at(-1).bounds));
            await input.key('CTRL+A');
            await input.type(path.resolve(path.dirname(fileURLToPath(import.meta.url)), 'TestDocument.bin'));
            await localized(['Open', '\u6253\u5f00']);
            await until(async () => !await picker() && (await document()).elementDocument.includes('Self-contained persistence fixture'), `${variant}: TestDocument.bin loaded`);
            if (input.fixtureOnly) continue;
            const view = (await all()).find(node => node.control === 'DocumentViewer');
            const content = await document();
            const y = content.bounds.y1 + 8;
            const begin = { x: Math.round(view.bounds.x1 + (view.bounds.x2 - view.bounds.x1) * 0.4), y };
            const finish = { x: Math.round(view.bounds.x1 + (view.bounds.x2 - view.bounds.x1) * 0.6), y };
            for (const reversed of [false, true]) {
                for (const target of [begin, center({ x1: begin.x, x2: finish.x, y1: y, y2: y }), finish, { x: view.bounds.x2 - 40, y }]) {
                    const select = async () => {
                        await input.drag(reversed ? finish : begin, reversed ? begin : finish);
                        await until(async () => {
                            const match = (await selection()).match(/Selection(\([^)]*\))-(\([^)]*\))/);
                            return match[1] !== match[2];
                        }, `${variant}: fixture directed range`);
                    };
                    await select(); await input.mouse(target);
                    await until(async () => {
                        const match = (await selection()).match(/Selection(\([^)]*\))-(\([^)]*\))/);
                        return match[1] === match[2];
                    }, `${variant}: ordinary mouse collapses fixture range`);
                    const expected = await selection();
                    await select(); await pause(600); await input.tap(target);
                    await until(async () => await selection() === expected, `${variant}: short press matches mouse / reversed=${reversed} / x=${target.x}`);
                }
            }
        }
        const d = await document();
        const start = { x: d.bounds.x1 + 8, y: d.bounds.y1 + 8 };
        await input.mouse(start);
        await input.key('CTRL+HOME');
        await until(async () => await selection() === 'Selection(0,0)-(0,0)', `${variant}: Home reaches the first caret`);
        await input.key('CTRL+SHIFT+END');
        await until(async () => {
            const match = (await selection()).match(/Selection(\([^)]*\))-(\([^)]*\))/);
            return match[1] !== match[2];
        }, `${variant}: keyboard selects the document`);
        await input.type('Touch selection regression.');
        await until(async () => (await documentText()).includes('Touch selection regression.'), `${variant}: ordinary mouse and keyboard edit`);
        if (input.fixture) {
            // Replacing the centered fixture heading retains its paragraph alignment and font.
            start.x = Math.round(d.bounds.x1 + (d.bounds.x2 - d.bounds.x1) * 0.35);
        }
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
        await until(async () => {
            const match = (await selection()).match(/Selection(\([^)]*\))-(\([^)]*\))/);
            return match[1] === match[2];
        }, `${variant}: short press collapses existing selection`);
        await input.tap(end, 600);
        await until(async () => {
            const match = (await selection()).match(/Selection(\([^)]*\))-(\([^)]*\))/);
            return match[1] !== match[2];
        }, `${variant}: long press extends from the new caret`);
        const contextSelection = await selection();
        assert.notEqual(contextSelection, selected, `${variant}: long press extends from the new caret`);
        await input.tap({ x: start.x + 30, y: start.y }, 600);
        await until(async () => (await all()).some(node => node.control === 'Menu'), `${variant}: long press opens context menu`);
        assert.equal(await selection(), contextSelection, `${variant}: context press preserves selection`);
        await pause(600);
        await input.tap(start);
        await until(async () => !(await all()).some(node => node.control === 'Menu'), `${variant}: short press dismisses context popup`);
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
        if (variant === 'Ribbon') await input.key('CTRL+Z');
        else { await tap('Edit'); await tap('Undo'); }
        await until(async () => await images() === originalImages, `${variant}: Undo removes that insertion`);
        if (variant === 'Ribbon') await input.key('CTRL+Y');
        else { await tap('Edit'); await tap('Redo'); }
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
        fixture: process.argv.includes('--fixture') || process.argv.includes('--fixture-only'),
        fixtureOnly: process.argv.includes('--fixture-only'),
        controls: async () => (await fetch(`${core}/Controls`, { signal: AbortSignal.timeout(15000) })).json(),
        tap,
        pair: async point => { await tap(point); await tap(point); },
        mouse: async point => io(`!LeftClick:${point.x},${point.y}`),
        drag: async (begin, end) => {
            await io(`!MouseMove:${begin.x},${begin.y}`);
            await io(`!LeftDown:${begin.x},${begin.y}`);
            await io(`!MouseMove:${end.x},${end.y}`);
            await io(`!LeftUp:${end.x},${end.y}`);
        },
        key: async key => io(`!KeyPress:${key}`),
        type: async text => io(`!Type:${text}`),
    });
}
