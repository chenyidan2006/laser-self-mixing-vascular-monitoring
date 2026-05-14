const fs = require('fs');
const path = require('path');

const rootDir = 'd:/code/app/appprogram';

const excludeDirs = ['node_modules', '.git'];

const chinesePunctuation = {
  ',': ',',
  '.': '.',
  '!': '!',
  '?': '?',
  ':': ':',
  ';': ';',
  '"': '"',
  '"': '"',
  ''': "'",
  ''': "'",
  '(': '(',
  ')': ')',
  '[': '[',
  ']': ']',
  '<': '<',
  '>': '>',
  ',': ',',
  '-': '-',
  '...': '...'
};

function shouldExclude(dir) {
  return excludeDirs.some(exclude => dir.includes(exclude));
}

function replaceChinesePunctuation(content) {
  let result = content;
  for (const [chinese, english] of Object.entries(chinesePunctuation)) {
    const regex = new RegExp(chinese, 'g');
    result = result.replace(regex, english);
  }
  return result;
}

function processFile(filePath) {
  try {
    const content = fs.readFileSync(filePath, 'utf8');
    const newContent = replaceChinesePunctuation(content);
    
    if (content !== newContent) {
      fs.writeFileSync(filePath, newContent, 'utf8');
      console.log(`✅ 已修复: ${filePath}`);
      return 1;
    }
    return 0;
  } catch (err) {
    console.error(`❌ 处理失败: ${filePath} - ${err.message}`);
    return 0;
  }
}

function traverseDir(dir) {
  let fixedCount = 0;
  
  try {
    const files = fs.readdirSync(dir);
    
    for (const file of files) {
      const filePath = path.join(dir, file);
      const stat = fs.statSync(filePath);
      
      if (stat.isDirectory()) {
        if (!shouldExclude(filePath)) {
          fixedCount += traverseDir(filePath);
        }
      } else {
        const ext = path.extname(file).toLowerCase();
        if (['.js', '.vue', '.json', '.md', '.txt', '.bat', '.sh', '.sql', '.h'].includes(ext)) {
          fixedCount += processFile(filePath);
        }
      }
    }
  } catch (err) {
    console.error(`❌ 遍历目录失败: ${dir} - ${err.message}`);
  }
  
  return fixedCount;
}

console.log('===========================================');
console.log('  中文标点批量修复工具');
console.log('===========================================');
console.log(`正在处理目录: ${rootDir}`);
console.log('-------------------------------------------');

const fixedCount = traverseDir(rootDir);

console.log('-------------------------------------------');
console.log(`修复完成!共修复 ${fixedCount} 个文件`);
console.log('===========================================');