#include <iostream>

#include <bsoncxx/builder/stream/document.hpp>
#include <bsoncxx/json.hpp>

#include <mongocxx/client.hpp>
#include <mongocxx/instance.hpp>
#include <mongocxx/options/index.hpp>
#include <mongocxx/options/update.hpp>
#include <mongocxx/options/insert.hpp>
#include <mongocxx/insert_many_builder.hpp>


#include <time.h>  

using namespace mongocxx;
using namespace bsoncxx;

using bsoncxx::builder::stream::open_document;
using bsoncxx::builder::stream::close_document;
using bsoncxx::builder::stream::open_array;
using bsoncxx::builder::stream::close_array;
using bsoncxx::builder::stream::finalize;

int main(int, char**) {
    mongocxx::instance inst{};
    mongocxx::client conn{mongocxx::uri{"mongodb://super:Qianwang2017@192.168.131.92:27017"}};

    bsoncxx::builder::stream::document document{};
    auto collection = conn["user"]["user_login_info_1"];
    
    int insert_many_number = 1000;
    for(int index_i = 0; index_i < 100000; index_i++)
    {
        options::insert opts = options::insert();
        // opts.unique(true);
        if(index_i % 1000 == 0)
        {
            printf("index_i = %d\n", index_i);
        }
        
        insert_many_builder insert_many{opts};

        for(int index_j = 0; index_j < insert_many_number; index_j++)
        {
            bsoncxx::builder::stream::document insert_builder;
            
            int user_id = index_i * insert_many_number + index_j;
            insert_builder << "user_id" << user_id;

            insert_builder << "last_login_in_time" << user_id;
            insert_builder << "last_login_out_time" << user_id + 10;

            insert_builder << "array" 
                        << open_array << open_document 
                        << "login_in_time" << user_id
                        << "login_out_time" << user_id + 10
                        << close_document
                        << close_array ;

            insert_many(insert_builder.view());
        }

        insert_many.insert(&collection);
    }

    // collection.createIndex()
}

// void update_mongodb()
// {
//     options::update opts{};
//     opts.upsert(true);
    
//     for(int i = 0; i < 1000000; i++)
//     {
//         bsoncxx::builder::stream::document update_builder;
//         bsoncxx::builder::stream::document filter_builder;
//         filter_builder << "user_id" << i;

//         if(i % 100 == 0)
//         {
//             printf("i = %d", i);
//         }

//         // update_builder << "$set" << open_document << "array" 
//         //             << open_array << open_document 
//         //             << "login_in_time" << 123456
//         //             << "login_out_time" << 333333
//         //             << close_document
//         //             << close_array 
//         //             << close_document;

//         time_t current_time;  
//         current_time = time(NULL);  

//         update_builder << "$push" << open_document << "array" 
//                     << open_array << open_document 
//                     << "login_in_time" << current_time
//                     << "login_out_time" << current_time + 1
//                     << close_document
//                     << close_array 
//                     << close_document;
        
//         // collection.update_one(filter_builder.view(), update_builder.view(), opts);

//         // collection.update_many(filter_builder.view(), update_builder.view(), opts);
//     }
// }