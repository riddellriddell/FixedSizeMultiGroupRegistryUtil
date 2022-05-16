// FixedSizeMultiGroupRegistryUtil.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>
#include <vector>
#include <cstdlib>
#include "fixed_size_multi_group_tracker.h"

bool _validate_multi_group_tracker(fixed_size_multi_group_tracker& tracker_to_validate, std::vector<int>& key_group, std::vector<int>& keys_added)
{
    //check if total number of keys in tracker match total number of keys added
    if (keys_added.size() != tracker_to_validate.size())
    {
        std::cout << "tracked key cout miss match!\n";
        return false;
    }

    for (int i = 0; i < keys_added.size(); i++)
    {
        int key_to_check = keys_added[i];

        int group_of_key = key_group[i];

        //check if in correct group in tracker
        if (group_of_key != tracker_to_validate.group_of_key[key_to_check])
        {
            std::cout << "key marked as in wrong group!\n";
            return false;
        }

        //check if key index lookup is valid 
        if (tracker_to_validate.grouped_keys[tracker_to_validate.index_of_key_in_grouped_keys[key_to_check]] != key_to_check)
        {
            std::cout << "key indexing is invalid!\n";
            return false;
        }

        //check if key is correctly placed in group bounds
        bool is_in_tracker = false;

        for (int j = 0; j < tracker_to_validate.group_size[group_of_key]; j++)
        {
            int key_in_tracker_group = tracker_to_validate.get_key_at_group_index(group_of_key, j);

            if (key_in_tracker_group == key_to_check)
            {
                is_in_tracker = true;
            }
        }

        if (is_in_tracker == false)
        {
            std::cout << "Missing key!\n";
            return false;
        }
    }

    return true;
}

int main()
{
    std::cout << "Hello World!\n";

    //setup a central value surounded by 2 on either side
    int group_size = 10;
    int key_size = group_size * 25;
    int seed = 0;

    std::vector<int> key_group;
    std::vector<int> keys_added;
    std::vector<int> keys_to_add;

    //setup keys to add
    for (int i = 0; i < key_size; i++)
    {
        keys_to_add.push_back(i);
    }

#pragma region UnitTest

    srand(seed);

    fixed_size_multi_group_tracker test;

    test.init(group_size, key_size);


    auto begin = std::chrono::high_resolution_clock::now();

    typedef  uint32_t uint32;

    uint32 iterations = 100;

    for (uint32 i = 0; i < iterations; ++i)
    {
        for (uint32 j = 0; j < key_size; j++)
        {
            //pick random number to add 
            int to_add_index = rand() % keys_to_add.size();
            int key = keys_to_add[to_add_index];

            //pick random group to add to 
            int group = rand() % group_size;

            //remove key from keys to add
            keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
            keys_to_add.pop_back();

            //add key to keys added 
            keys_added.push_back(key);
            key_group.push_back(group);

            //add to tracked structure
            test.add(key, group);

            //validate structure 
            if (_validate_multi_group_tracker(test, key_group, keys_added) == false)
            {
                std::cout << "Error in Test:" << i << " after adding:" << j<<" keys\n";
            }
        }

        std::cout << "Add Test:" << i << " complete\n";

        //reset test data
        test.clear();
        key_group.clear();
        keys_added.clear();
        keys_to_add.clear();

        //setup keys to add
        for (int i = 0; i < key_size; i++)
        {
            keys_to_add.push_back(i);
        }

    }
   
    std::cout << "Add Test complete\n";

    std::cout << "Starting Add Remove Test\n";

    for (uint32 i = 0; i < iterations; ++i)
    {
        for (uint32 j = 0; j < key_size; j++)
        {
            //pick random number to add 
            int add_remove_index = rand() % key_size;
            
            //check if we are adding or removing
            if (add_remove_index < keys_added.size())
            {
                //remove key
                int key = keys_added[add_remove_index];
                  //remove key from keys to add
                keys_added[add_remove_index] = keys_added[keys_added.size() - 1];
                key_group[add_remove_index] = key_group[key_group.size() - 1];
                keys_added.pop_back();
                key_group.pop_back();

                keys_to_add.push_back(key);

                //add to tracked structure
                test.remove(key);

                //validate structure 
                if (_validate_multi_group_tracker(test, key_group, keys_added) == false)
                {
                    std::cout << "Error in add remove Test during itteration:" << i << " after add / removing:" << j << " keys\n";
                }
            }
            else
            {
                //add key
                int to_add_index = add_remove_index - keys_added.size();

                int key = keys_to_add[to_add_index];

                 //pick random group to add to 
                int group = rand() % group_size;

                //remove key from keys to add
                keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
                keys_to_add.pop_back();

                //add key to keys added 
                keys_added.push_back(key);
                key_group.push_back(group);

                //add to tracked structure
                test.add(key, group);

                //validate structure 
                if (_validate_multi_group_tracker(test, key_group, keys_added) == false)
                {
                    std::cout << "Error in add remove Test:" << i << " after add / remioving:" << j << " keys\n";
                }
            }          
        }


        std::cout << "Add remove Test:" << i << " complete\n";

        //reset test data
        test.clear();
        key_group.clear();
        keys_added.clear();
        keys_to_add.clear();

        //setup keys to add
        for (int i = 0; i < key_size; i++)
        {
            keys_to_add.push_back(i);
        }

    }


    std::cout << "Add Remove Test complete\n";

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin).count();
    std::cout << "UNIT TEST COMPLETE TIME, total:" << duration << " average : " << duration / iterations << "ns." << std::endl;

#pragma endregion

#pragma region PerTestComparison

    int sub_itterations = key_size * 0.5f;

    std::cout << "PerfTest start\n";

    std::cout << "non spaced groups profiling\n";

    srand(seed);

    test.clear();

    long long non_spaced_groups_total_duration = 0;

    for (uint32 i = 0; i < iterations; ++i)
    {
        for (uint32 j = 0; j < sub_itterations; j++)
        {
            //pick random number to add 
            int add_remove_index = rand() % key_size;

            //check if we are adding or removing
            if (add_remove_index < keys_added.size())
            {
                //remove key
                int key = keys_added[add_remove_index];
                //remove key from keys to add
                keys_added[add_remove_index] = keys_added[keys_added.size() - 1];
                key_group[add_remove_index] = key_group[key_group.size() - 1];
                keys_added.pop_back();
                key_group.pop_back();

                keys_to_add.push_back(key);

                //add to tracked structure
                auto start_prof = std::chrono::high_resolution_clock::now();

                test.remove(key);

                auto end_prof = std::chrono::high_resolution_clock::now();

                non_spaced_groups_total_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

            }
            else
            {
                //add key
                int to_add_index = add_remove_index - keys_added.size();

                int key = keys_to_add[to_add_index];

                //pick random group to add to 
                int group = rand() % group_size;

                //remove key from keys to add
                keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
                keys_to_add.pop_back();

                //add key to keys added 
                keys_added.push_back(key);
                key_group.push_back(group);

                //add to tracked structure
                auto start_prof = std::chrono::high_resolution_clock::now();
                
                test.add(key, group);
                
                auto end_prof = std::chrono::high_resolution_clock::now();

                non_spaced_groups_total_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();
            }
        }

        std::cout << "Add remove Test:" << i << " complete\n";

        //reset test data
        test.clear();
        key_group.clear();
        keys_added.clear();
        keys_to_add.clear();

        //setup keys to add
        for (int i = 0; i < key_size; i++)
        {
            keys_to_add.push_back(i);
        }

    }

    std::cout << "non spaced group profiling end\n";
    std::cout << "non spaced ns total:" << non_spaced_groups_total_duration << ", average : " << non_spaced_groups_total_duration / (iterations * (long long)sub_itterations) << "ns." << std::endl;


    std::cout << "spaced group profiling\n";

    srand(seed);

    test.clear();
    test.initalise_group_boundries_v2();

    long long spaced_groups_total_duration = 0;

    for (uint32 i = 0; i < iterations; ++i)
    {
        for (uint32 j = 0; j < sub_itterations; j++)
        {
            //pick random number to add 
            int add_remove_index = rand() % key_size;

            //check if we are adding or removing
            if (add_remove_index < keys_added.size())
            {
                //remove key
                int key = keys_added[add_remove_index];
                //remove key from keys to add
                keys_added[add_remove_index] = keys_added[keys_added.size() - 1];
                key_group[add_remove_index] = key_group[key_group.size() - 1];
                keys_added.pop_back();
                key_group.pop_back();

                keys_to_add.push_back(key);

                //add to tracked structure
                auto start_prof = std::chrono::high_resolution_clock::now();

                test.remove(key);

                auto end_prof = std::chrono::high_resolution_clock::now();

                spaced_groups_total_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();
            }
            else
            {
                //add key
                int to_add_index = add_remove_index - keys_added.size();

                int key = keys_to_add[to_add_index];

                //pick random group to add to 
                int group = rand() % group_size;

                //remove key from keys to add
                keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
                keys_to_add.pop_back();

                //add key to keys added 
                keys_added.push_back(key);
                key_group.push_back(group);

                //add to tracked structure
                auto start_prof = std::chrono::high_resolution_clock::now();

                test.add(key, group);

                auto end_prof = std::chrono::high_resolution_clock::now();

                spaced_groups_total_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();
            }
        }

        std::cout << "Add remove Test:" << i << " complete\n";

        //reset test data
        test.clear();
        test.initalise_group_boundries_v2();
        key_group.clear();
        keys_added.clear();
        keys_to_add.clear();

        //setup keys to add
        for (int i = 0; i < key_size; i++)
        {
            keys_to_add.push_back(i);
        }

    }

    std::cout << "spaced group profiling end\n";
    std::cout << "spaced ns total:" << spaced_groups_total_duration << ", average :" << spaced_groups_total_duration / (iterations * (long long)sub_itterations) << "ns." << std::endl;

    auto performance_difference = (non_spaced_groups_total_duration - spaced_groups_total_duration) / (iterations * (long long)sub_itterations);

    auto percent_improvement = 100.0f * (float)performance_difference / (float)(non_spaced_groups_total_duration / (iterations * (long long)sub_itterations));

    std::cout << "total_time_dif:" << performance_difference << ", percent improvement :" << percent_improvement << std::endl;


#pragma endregion

}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
