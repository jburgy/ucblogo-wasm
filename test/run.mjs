// Runs the node build like a native ucblogo: node test/run.mjs [file.lg ...] < input | cat
import createLogo from "../build/logo-node.mjs";

await createLogo({ arguments: process.argv.slice(2) });
