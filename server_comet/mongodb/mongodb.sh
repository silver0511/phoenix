c++ --std=c++11 mongodb.cpp -L../../server_bin/lib_mongodb\
                -lbson-1.0 -lbsoncxx -lmongoc-1.0 -lssl -lcrypto -lrt -lmongocxx\
                -I../../Include/mongodb_include/libbson-1.0\
                -I../../Include/mongodb_include/bsoncxx/v_noabi\
                -I../../Include/mongodb_include/libmongoc-1.0\
                -I../../Include/mongodb_include/mongocxx/v_noabi