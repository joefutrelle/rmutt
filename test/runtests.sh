#!/bin/sh
cat test.map | sed -e 's/^/sh runtest.sh /' -e 's/$/;/' | /bin/sh
