echo ==dlist==
./adtTest.dlist <mytests/test$1 &> mytests/d$1
cat mytests/d$1
echo ==array==
./adtTest.array <mytests/test$1 &> mytests/a$1
cat mytests/a$1
echo ==bst==
./adtTest.bst <mytests/test$1 &> mytests/b$1
cat mytests/b$1
