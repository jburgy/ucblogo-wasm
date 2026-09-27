// Runs the node build like a native ucblogo: node test/run.mjs [file.lg ...] < input | cat
import createLogo from "../build/logo-node.mjs";

const ucblogo = new URL("../ucblogo/", import.meta.url).pathname;

await createLogo({
    arguments: process.argv.slice(2),
    preRun: [({ ENV }) => {
        ENV.LOGOLIB = ucblogo + "logolib";
        ENV.LOGOHELP = ucblogo + "helpfiles";
    }],
});
