mkdir __targets/.coverage 2> /dev/null || true
cd __targets/.coverage
gcovr -p --html-details coverage.html --root ../../
