mkdir __targets/.coverage 2> /dev/null || true
cd __targets/.coverage
gcovr -p --html-details coverage.html --root ../../
cd ..
find -name '*.gcno' | xargs rm 2> /dev/null
find -name '*.gcda' | xargs rm 2> /dev/null

