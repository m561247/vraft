const http = require('http');
const fs = require('fs');
const path = require('path');

const directoryPath = path.join(__dirname, 'remu_web');

http.createServer((req, res) => {
    if (req.url === '/') {
        fs.readdir(directoryPath, { withFileTypes: true }, (err, files) => {
            if (err) {
                res.writeHead(500, { 'Content-Type': 'text/plain' });
                res.end('Error reading directory.');
                return;
            }

            const htmlFiles = files.filter(file => file.name.endsWith('.html'));
            const otherFiles = files.filter(file => !file.name.endsWith('.html'));

            res.writeHead(200, { 'Content-Type': 'text/html' });
            res.write('<html><head><title>Directory Contents</title><link rel="stylesheet" href="/remu_web.css"></head><body>');
            res.write('<table><tr><td class="banner">(REMU)Raft Emulator</td></tr></table>');
            res.write('<div class="html-files"><h2>Generated Files</h2><ul>');
            htmlFiles.forEach(file => {
                res.write(`<li><a href="/files/${encodeURIComponent(file.name)}">${file.name}</a></li>`);
            });
            res.write('</ul></div>');
            res.write('<div class="other-files"><h2>Original Files</h2><ul>');
            otherFiles.forEach(file => {
                res.write(`<li><a href="/files/${encodeURIComponent(file.name)}">${file.name}</a></li>`);
            });
            res.write('</ul></div>');
            res.write('</body></html>');
            res.end();
        });
    } else if (req.url.startsWith('/files/')) {
        const fileName = req.url.substring(7);
        const filePath = path.join(directoryPath, decodeURIComponent(fileName));
        
        fs.stat(filePath, (err, stats) => {
            if (err || !stats.isFile()) {
                res.writeHead(404, { 'Content-Type': 'text/plain' });
                res.end('File not found.');
                return;
            }

            const fileExtension = path.extname(filePath);
            let contentType = 'text/plain';

            switch (fileExtension) {
                case '.html':
                    contentType = 'text/html';
                    break;
                case '.css':
                    contentType = 'text/css';
                    break;
                case '.js':
                    contentType = 'application/javascript';
                    break;
                case '.png':
                    contentType = 'image/png';
                    break;
                case '.jpg':
                case '.jpeg':
                    contentType = 'image/jpeg';
                    break;
                default:
                    contentType = 'text/plain';
            }

            res.writeHead(200, { 'Content-Type': contentType });
            const fileStream = fs.createReadStream(filePath);
            fileStream.pipe(res);
        });
    } else if (req.url === '/remu_web.css') {
        res.writeHead(200, { 'Content-Type': 'text/css' });
        fs.createReadStream('remu_web.css').pipe(res);
    } else {
        res.writeHead(404, { 'Content-Type': 'text/html' });
        res.end('Resource not found.');
    }
}).listen(3000, () => {
    console.log('Server running at http://0.0.0.0:3000/');
});