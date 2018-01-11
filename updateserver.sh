echo "=========Include=========="
cd Include
git pull
cd ..
echo "=========Include==========\n"

echo "==========common=========="
cd common
git checkout master
git pull
cd ..
echo "==========common==========\n"

echo "==========network========="
cd network
git checkout master
git pull
cd ..
echo "==========network=========\n"

echo "==========public=========="
cd public
git checkout master
git pull
cd ..
echo "==========public==========\n"

echo "========server_bin========"
cd server_bin
git checkout master
git pull
cd ..
echo "========server_bin========\n"

echo "========server_comet========"
cd server_comet
git checkout master
git pull
cd ..
echo "========server_comet========\n"

echo "========server_logic========"
cd server_logic
git checkout master
git pull
cd ..
echo "========server_logic========\n"

echo "========server_route========"
cd server_route
git checkout master
git pull
cd ..
echo "========server_route========\n"

echo "========server_db========"
cd server_db
git checkout master
git pull
cd ..
echo "========server_db========\n"
