#!/bin/sh
cat test.map | sed -e 's/^/sh maketest.sh /' -e 's/$/;/' | /bin/sh
