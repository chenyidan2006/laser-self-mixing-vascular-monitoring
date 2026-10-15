const fs = require('node:fs');
const path = require('node:path');
const root = path.resolve(__dirname, '..');
const pkg = require('../package.json');
let failed = false;
function check(name, fn) {
  try { console.log('OK', name, fn()); }
  catch (err) { failed = true; console.error('FAIL', name, err.code || '', err.message); }
}
check('Node', () => {
  if (process.versions.node !== pkg.engines.node) throw Error(`Use Node ${pkg.engines.node}, current ${process.version}`);
  return process.version;
});
for (const name of ['sass', 'vite', 'vue', '@dcloudio/vite-plugin-uni']) {
  check(name, () => {
    const file = path.join(root, 'node_modules', name, 'package.json');
    fs.realpathSync.native(file); // Vite requires this to work; npm ls alone does not check it.
    const installed = JSON.parse(fs.readFileSync(file, 'utf8'));
    if (installed.version !== (pkg.dependencies?.[name] || pkg.devDependencies?.[name])) throw Error(`Unexpected version ${installed.version}`);
    return installed.version;
  });
}
if (failed) process.exitCode = 1;
