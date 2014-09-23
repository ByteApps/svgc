svgc
====

svgc is a tool that allows operations and variables in SVG files

svgc will scan the input and do the operations inside the the identifiers: `<!---  --->`

Examples:
=========

A few simple operations:

`<!--- 1+1 --->` Will be replaced by 2

`<!--- 5 * 5 --->` Will be replaced by 25

It can also handle complicated operations:

`<!--- ((5 * 5) * 5 * 5) + 1--->` will be replaced by 626

It can also handle variables:

<!--- y = 10 ---> stores the value of 10 in y

<!--- y = y + 10 ---> stores the value of 20 in y

<!--- y ---> Will be replaced by 20


Other uses:
=========
This tool can be also used in any file that follows the `<!---  --->` identifier.