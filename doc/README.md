# CSS Optimizer Documentation

## Overview
`cssoptim` is a C99 tool that removes unused CSS classes from a stylesheet based on usage in HTML and JavaScript files.

## Build Instructions
```bash
make
make test
```

## Usage
```bash
./build/cssoptim -o output.css input.css input.html input.js
```

### Arguments
- `-o <file>`: Output file path.
- `-v`: Enable verbose logging.
- `[files]`: List of input files (.css, .html, .js).

## Architecture
- **src/main.c**: Entry point. Orchestrates the flow.
- **src/args.c**: Command-line argument parsing using `argparse`.
- **src/css_proc.c**: CSS processing using `liblexbor`. Parses CSS, filters rules, and serializes output.
- **src/html_scan.c**: 
  - HTML scanning using `liblexbor` HTML parser.
  - JS scanning using a custom simple lexer.

## API Notes
### CSS Processing (`src/css_proc.h`)
- `css_optimize(css, len, used_classes, count)`: Main function to filter CSS.
- Uses `liblexbor` to build an AST, traverses it to find Style rules, checks selectors against `used_classes`, and removes unused ones.

### HTML/JS Scanning (`src/html_scan.h`)
- `scan_html(content, len, list)`: Parses HTML and extracts `class` attributes.
- `scan_js(content, len, list)`: Scans JS strings for potential class names.
