#!/bin/sh -e
/bin/sh -es <<- 'EOF' | shuffle | /bin/sh -esx
	for file in *.[ch]
	do	echo ./pretty '<' ${file} '|' diff - ${file}
	done
EOF
