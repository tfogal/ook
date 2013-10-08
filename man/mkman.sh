#!/bin/sh
host="tfogal.github.io"
m2hopt="-H ${host} -r"

if test -z "$1" ; then
  echo "need argument: rootdir for manpages."
  exit 1
fi

for man in $(ls -1 $1/man3/*.3) ; do
  base=$(basename ${man})
  #groff -mandoc -Txhtml ${man} > ${base}.xhtml
  man2html ${m2hopt} ${man} | tail -n +3 | \
    sed 's,../man3/,/ook/man/,' | \
    sed 's,../man7/,/ook/man/,' > \
    ${base}.html
done
for man in $(ls -1 $1/man7/*.7) ; do
  base=$(basename ${man})
  man2html ${m2hopt} ${man} | tail -n +3 | \
    sed s,../man3/,/ook/man/, | \
    sed s,../man7/,/ook/man/, > \
    ${base}.html
done
#
