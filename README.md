# mc - multi-column

Rewrite stdin into multi-column output

![mc](img/mc.png)

## Usage

usage: mc [-:bc:m:sd:Sw:uh1] [FILE]<br>
Print stdin in multiple columns<br>
<br>
  -b : (False) Only columnate to avoid scrolling<br>
  -s : (False) Strip escape codes from stdin<br>
  -S : (False) Strip and print<br>
  -w   (  163) Override COLUMNS<br>
  -m   (    5) Set minimum lines to split<br>
  -c   (    3) Set spacing between columns<br>
  -d  [0x0000] Debug level:
               use 0x0200 to invoke malloc_debug
<br>
<br>
For better results:<br>
  - export COLUMNS LINES<br>
  - export MC_MIN  # set minimum lines to split<br>
