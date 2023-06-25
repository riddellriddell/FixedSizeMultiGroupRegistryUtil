// FixedSizeMultiGroupRegistryUtil.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <cassert>
#include <atomic>
#include "fixed_size_multi_group_tracker.h"
#include "fixed_size_multi_group_linked_list.h"
#include "template_fixed_size_multi_group_linked_list.h"
#include "template_fixed_size_multi_group_tracker.h"
#include "template_fix_size_multi_group_2_pass.h"
#include "simple_multi_group_tracker.h"

typedef  uint32_t uint32;

enum class test_candidates
{
	simple_index = 0,
	spaced_index = 1,
	non_spaces_index = 2,
	linked_list_index = 3,
	spaced_index_2_pass = 4,
	non_spaces_index_2_pass = 5,
	count
};

struct profiling_result
{
	long long total_profile_duration[(int)test_candidates::count] = {};
	float average_profile_duration[(int)test_candidates::count] = {};

	long long add_total_duration[ (int)test_candidates::count] = {};
	float add_average_duration[(int)test_candidates::count] = {};

	long long remove_total_duration[(int)test_candidates::count] = {};
	float remove_average_duration[(int)test_candidates::count] = {};
	
	
	long long read_total_duration[(int)test_candidates::count] = {};
	float read_average_duration[(int)test_candidates::count] = {};

	uint32 num_adds[(int)test_candidates::count] = {};
	uint32 num_removes[(int)test_candidates::count] = {};
	uint32 num_reads[(int)test_candidates::count] = {};
	
	uint32 add_average_group_size[(int)test_candidates::count] = {};
	uint32 remove_average_group_size[(int)test_candidates::count] = {};
	uint32 read_average_group_size[(int)test_candidates::count] = {};

	uint32 sum_of_all_reads[(int)test_candidates::count] = {};

};

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

bool _validate_multi_group_tracker_linked_list(fixed_size_multi_group_linked_list& tracker_to_validate, std::vector<int>& key_group, std::vector<int>& keys_added)
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
		if (tracker_to_validate.nodes[tracker_to_validate.index_of_key_in_grouped_keys[key_to_check]].data != key_to_check)
		{
			std::cout << "key indexing is invalid!\n";
			return false;
		}

		//check if key is correctly placed in group bounds
		bool is_in_tracker = false;

		//get first node in group
		int index = 0;
		int key = fixed_size_multi_group_linked_list::INVALID_KEY;

		key = tracker_to_validate.get_first_key_in_group(group_of_key, &index);

		if (key == key_to_check)
		{
			is_in_tracker = true;
		}

		while (key != fixed_size_multi_group_linked_list::INVALID_KEY)
		{
			if (key == key_to_check)
			{
				is_in_tracker = true;
			}

			key = tracker_to_validate.get_next_key(&index);
		}

		if (is_in_tracker == false)
		{
			std::cout << "Missing key!\n";
			return false;
		}
	}

	return true;
}

template<typename TData>
bool _validate_template_multi_group_tracker(template_fixed_size_multi_group_tracker<TData> & tracker_to_validate, std::vector<int>& key_group, std::vector<int>& keys_added)
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
		if (tracker_to_validate.grouped_keys[tracker_to_validate.index_of_key_in_grouped_keys[key_to_check]].key != key_to_check)
		{
			std::cout << "key indexing is invalid!\n";
			return false;
		}

		//check if key matches data
		if (tracker_to_validate.grouped_keys[tracker_to_validate.index_of_key_in_grouped_keys[key_to_check]].data != TData(key_to_check))
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

template<typename TKey, typename TData>
bool _validate_template_multi_group_tracker_2_pass(template_fixed_size_multi_group_2_pass<TKey,TData>& tracker_to_validate, std::vector<int>& key_group, std::vector<int>& keys_added)
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
		int key_to_check_index = tracker_to_validate.convert_key_to_lookup_index(key_to_check);

		int group_of_key = key_group[i];

		//check if in correct group in tracker
		if (group_of_key != tracker_to_validate.group_of_key[key_to_check])
		{
			std::cout << "key marked as in wrong group!\n";
			return false;
		}

		//check if key index lookup is valid 
		if (tracker_to_validate.grouped_keys[tracker_to_validate.index_of_key_in_grouped_keys[key_to_check]].key != key_to_check)
		{
			std::cout << "key indexing is invalid!\n";
			return false;
		}

		//check if key matches data
		if (tracker_to_validate.grouped_keys[tracker_to_validate.index_of_key_in_grouped_keys[key_to_check]].data != TData(key_to_check))
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

template<typename TData>
bool _validate_tempalte_multi_group_tracker_linked_list(template_fixed_size_multi_group_linked_list<TData>& tracker_to_validate, std::vector<int>& key_group, std::vector<int>& keys_added)
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
		if (tracker_to_validate.nodes[tracker_to_validate.index_of_key_in_grouped_keys[key_to_check]].data != TData(key_to_check))
		{
			std::cout << "key indexing is invalid!\n";
			return false;
		}

		//check if key is correctly placed in group bounds
		bool is_in_tracker = false;

		//get first node in group
		int index = 0;
		int key = fixed_size_multi_group_linked_list::INVALID_KEY;
		TData* data;

		tracker_to_validate.get_first_value_in_group(group_of_key, &index, &data);

		while (!tracker_to_validate.is_end_of_group(index, group_of_key))
		{
			if (*data == TData(key_to_check))
			{
				is_in_tracker = true;

				break;
			}

			tracker_to_validate.get_next_value(&index, &data);
		}


		if (is_in_tracker == false)
		{
			std::cout << "Missing key!\n";
			return false;
		}
	}

	return true;
}

template<typename TKey, typename TData>
bool _validate_simple_multi_group_tracker(simple_multi_group_tracker<TKey, TData>& tracker_to_validate, std::vector<int>& key_group, std::vector<int>& keys_added)
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

		//get index of key 
		int lookup_index = tracker_to_validate.index_of_key[tracker_to_validate.convert_key_to_lookup_index(key_to_check)];

		if (tracker_to_validate.key_of_data[lookup_index] != key_to_check)
		{
			std::cout << "key not mapped to data with same key!\n";
			return false;
		}

		//check if in correct group in tracker
		if (group_of_key != tracker_to_validate.group_of_data[lookup_index])
		{
			std::cout << "key marked as in wrong group!\n";
			return false;
		}

		//check if key index lookup is valid 
		if (tracker_to_validate.data[lookup_index] != TData(key_to_check))
		{
			std::cout << "key indexing is invalid!\n";
			return false;
		}
	}

	return true;
}


template<typename TData>
bool _unit_test_multi_group_tracker_linked_list(uint32 group_size, uint32 key_size, uint32 seed)
{
	std::vector<int> keys_to_add = std::vector<int>();
	std::vector<int> keys_added = std::vector<int>();
	std::vector<int> key_group = std::vector<int>();

	//setup keys to add
	for (int i = 0; i < key_size; i++)
	{
		keys_to_add.push_back(i);
	}

	srand(seed);

	template_fixed_size_multi_group_linked_list<TData> test;

	test.init(group_size, key_size);

	auto begin = std::chrono::high_resolution_clock::now();

	typedef  uint32_t uint32;

	uint32 iterations = 10;

	for (uint32 i = 0; i < iterations; ++i)
	{
		for (uint32 j = 0; j < key_size; j++)
		{
			//pick random number to add 
			int to_add_index = rand() % keys_to_add.size();
			int key = keys_to_add[to_add_index];

			//pick random group to add to 
			int group = rand() % group_size;

			//create data to add
			TData data_to_add = TData(key);

			//remove key from keys to add
			keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
			keys_to_add.pop_back();

			//add key to keys added 
			keys_added.push_back(key);
			key_group.push_back(group);
			
			//add to tracked structure
			test.add(group, key, &data_to_add);

			//validate structure 
			if (_validate_tempalte_multi_group_tracker_linked_list<TData>(test, key_group, keys_added) == false)
			{
				std::cout << "Error in Test:" << i << " after adding:" << j << " keys\n";
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
				if (_validate_tempalte_multi_group_tracker_linked_list<TData>(test, key_group, keys_added) == false)
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

				//create data to add
				TData data_to_add = TData(key);

				//remove key from keys to add
				keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
				keys_to_add.pop_back();

				//add key to keys added 
				keys_added.push_back(key);
				key_group.push_back(group);

				//add to tracked structure
				test.add(group, key, &data_to_add);

				//validate structure 
				if (_validate_tempalte_multi_group_tracker_linked_list<TData>(test, key_group, keys_added) == false)
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

	return true;
}

template<typename TData>
bool _unit_test_multi_group_tracker(uint32 group_size, uint32 key_size, uint32 seed)
{
	std::vector<int> keys_to_add = std::vector<int>();
	std::vector<int> keys_added = std::vector<int>();
	std::vector<int> key_group = std::vector<int>();

	//setup keys to add
	for (int i = 0; i < key_size; i++)
	{
		keys_to_add.push_back(i);
	}

	srand(seed);

	template_fixed_size_multi_group_tracker<TData> test;

	test.init(group_size, key_size);

	auto begin = std::chrono::high_resolution_clock::now();

	uint32 iterations = 10;

	for (uint32 i = 0; i < iterations; ++i)
	{
		for (uint32 j = 0; j < key_size; j++)
		{
			//pick random number to add 
			int to_add_index = rand() % keys_to_add.size();
			int key = keys_to_add[to_add_index];

			//pick random group to add to 
			int group = rand() % group_size;

			TData data = TData(key);

			//remove key from keys to add
			keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
			keys_to_add.pop_back();

			//add key to keys added 
			keys_added.push_back(key);
			key_group.push_back(group);

			//add to tracked structure
			test.add(key, &data, group);

			//validate structure 
			if (_validate_template_multi_group_tracker(test, key_group, keys_added) == false)
			{
				std::cout << "Error in Test:" << i << " after adding:" << j << " keys\n";
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
				if (_validate_template_multi_group_tracker(test, key_group, keys_added) == false)
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

				TData data = TData(key);

				//remove key from keys to add
				keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
				keys_to_add.pop_back();

				//add key to keys added 
				keys_added.push_back(key);
				key_group.push_back(group);

				//add to tracked structure
				test.add(key, &data, group);

				//validate structure 
				if (_validate_template_multi_group_tracker(test, key_group, keys_added) == false)
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

	return true;

}

template<typename TData>
bool _unit_test_multi_group_tracker_2_pass(uint32 group_size, uint32 key_size, uint32 seed)
{
	std::vector<int> keys_to_add = std::vector<int>();
	std::vector<int> keys_added = std::vector<int>();
	std::vector<int> key_group = std::vector<int>();

	//setup keys to add
	for (int i = 0; i < key_size; i++)
	{
		keys_to_add.push_back(i);
	}

	srand(seed);

	template_fixed_size_multi_group_2_pass<int,TData> test;

	test.init(group_size, key_size);

	auto begin = std::chrono::high_resolution_clock::now();

	uint32 iterations = 10;

	for (uint32 i = 0; i < iterations; ++i)
	{
		for (uint32 j = 0; j < key_size; j++)
		{
			//pick random number to add 
			int to_add_index = rand() % keys_to_add.size();
			int key = keys_to_add[to_add_index];

			//pick random group to add to 
			int group = rand() % group_size;

			TData data = TData(key);

			//remove key from keys to add
			keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
			keys_to_add.pop_back();

			//add key to keys added 
			keys_added.push_back(key);
			key_group.push_back(group);

			//add to tracked structure
			test.add(group, key, &data);

			//validate structure 
			if (_validate_template_multi_group_tracker_2_pass(test, key_group, keys_added) == false)
			{
				std::cout << "Error in Test:" << i << " after adding:" << j << " keys\n";
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
				if (_validate_template_multi_group_tracker_2_pass(test, key_group, keys_added) == false)
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

				TData data = TData(key);

				//remove key from keys to add
				keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
				keys_to_add.pop_back();

				//add key to keys added 
				keys_added.push_back(key);
				key_group.push_back(group);

				//add to tracked structure
				test.add(group, key, &data);

				//validate structure 
				if (_validate_template_multi_group_tracker_2_pass(test, key_group, keys_added) == false)
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

	return true;

}

template<typename TData>
bool _unit_test_simple_multi_group_tracker(uint32 group_size, uint32 key_size, uint32 seed)
{
	std::vector<int> keys_to_add = std::vector<int>();
	std::vector<int> keys_added = std::vector<int>();
	std::vector<int> key_group = std::vector<int>();

	//setup keys to add
	for (int i = 0; i < key_size; i++)
	{
		keys_to_add.push_back(i);
	}

	srand(seed);

	simple_multi_group_tracker<int, TData> test;

	test.init(group_size, key_size);

	auto begin = std::chrono::high_resolution_clock::now();

	uint32 iterations = 10;

	for (uint32 i = 0; i < iterations; ++i)
	{
		for (uint32 j = 0; j < key_size; j++)
		{
			//pick random number to add 
			int to_add_index = rand() % keys_to_add.size();
			int key = keys_to_add[to_add_index];

			//pick random group to add to 
			int group = rand() % group_size;

			TData data = TData(key);

			//remove key from keys to add
			keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
			keys_to_add.pop_back();

			//add key to keys added 
			keys_added.push_back(key);
			key_group.push_back(group);

			//add to tracked structure
			test.add(group, key, &data);

			//validate structure 
			if (_validate_simple_multi_group_tracker(test, key_group, keys_added) == false)
			{
				std::cout << "Error in Test:" << i << " after adding:" << j << " keys\n";
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
				if (_validate_simple_multi_group_tracker(test, key_group, keys_added) == false)
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

				TData data = TData(key);

				//remove key from keys to add
				keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
				keys_to_add.pop_back();

				//add key to keys added 
				keys_added.push_back(key);
				key_group.push_back(group);

				//add to tracked structure
				test.add(group, key, &data);

				//validate structure 
				if (_validate_simple_multi_group_tracker(test, key_group, keys_added) == false)
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

	return true;

}


template<int TSize>
struct test_data
{
	char data[TSize];

	test_data(uint32 key)
	{
		char* byte_ptr = reinterpret_cast<char*>(&key);
		auto size = sizeof(uint32);

		for (int i = 0; i < TSize; i++)
		{
			auto offset = i % size;

			data[i] = byte_ptr[offset];
		}
	}

	test_data()
	{
		for (int i = 0; i < TSize; i++)
		{
			data[i] = 0;
		}
	}


	int sum_of_all_data() const
	{
		int out = 0;

		for (int i = 0; i < TSize; i++)
		{
			out += data[i];
		}

		return out;
	}

	bool operator == (const test_data<TSize>& rhs) const
	{
		for (int i = 0; i < TSize; i++)
		{
			if (data[i] != rhs.data[i])
			{
				return false;
			}

		}

		return true;
	}

	bool operator != (const test_data<TSize>& rhs) const
	{
		for (int i = 0; i < TSize; i++)
		{
			if (data[i] != rhs.data[i])
			{
				return true;
			}

		}

		return false;
	}

};

void print_profile_result(profiling_result& prof_result, const uint32& iterations, const uint32& sub_itterations, const uint32& index)
{

	std::cout << "Test Total:" << (prof_result.total_profile_duration[index]) << ", average : " << (prof_result.average_profile_duration[index]) << "ns." << std::endl;


	std::cout << "total per call:" << (prof_result.add_total_duration[index] + prof_result.remove_total_duration[index] + prof_result.read_total_duration[index]) << ", average : " << (prof_result.add_total_duration[index] + prof_result.remove_total_duration[index] + prof_result.read_total_duration[index]) / (iterations * (long long)sub_itterations) << "ns." << std::endl;

	std::cout << "add count: " << prof_result.num_adds[index] << ", remove_count: " << prof_result.num_removes[index] << ", read count: " << prof_result.num_reads[index] << "add sum" << prof_result.sum_of_all_reads[index] << std::endl << std::endl;

	std::cout << "add" << (prof_result.add_total_duration[index]) << ", average : " << prof_result.add_average_duration[index] << "ns." << std::endl;
	std::cout << "remove" << (prof_result.remove_total_duration[index]) << ", average : " << prof_result.remove_average_duration[index] << "ns." << std::endl;
	std::cout << "read" << (prof_result.read_total_duration[index]) << ", average : " << prof_result.remove_average_duration[index] << "ns." << std::endl << std::endl << std::endl;

};

void calculate_next_key_grop_to_add(std::vector<int>& key_group, std::vector<int>& keys_added, std::vector<int>& keys_to_add, uint32 group_size, uint32 force_add_to_existing_group_chane, int& out_key, int& out_group)
{
	//add key
	int to_add_index = rand() % keys_to_add.size();

	out_key = keys_to_add[to_add_index];

	//pick random group to add to 
	out_group = rand() % group_size;

	//use random existing group
	if ((rand() % 100) < force_add_to_existing_group_chane && keys_added.size() > 0)
	{
		out_group = key_group[rand() % key_group.size()];
	}

	//remove key from keys to add
	keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
	keys_to_add.pop_back();

	//add key to keys added 
	keys_added.push_back(out_key);
	key_group.push_back(out_group);

}

template<bool TApplyMemoryFence>
std::chrono::steady_clock::time_point get_time_for_profiling ()
{
	if constexpr (TApplyMemoryFence) { std::atomic_thread_fence(std::memory_order_seq_cst); }

	auto memoryfenced_time = std::chrono::high_resolution_clock::now();

	if constexpr (TApplyMemoryFence) { std::atomic_thread_fence(std::memory_order_seq_cst); }

	return memoryfenced_time;
}

template<typename TData, bool TApplyMemoryFence>
profiling_result test_template_performance(uint32 group_size, uint32 key_size, uint32 iterations, uint32 sub_itterations, uint32 seed, uint32 start_test_amount, uint32 dont_read_if_less_than_this_many_items_in_structure, uint32 percent_read_tests, uint32 force_add_to_existing_group_chane)
{
	profiling_result prof_result = {};

	std::vector<int>& key_group = *(new std::vector<int>());
	std::vector<int>& keys_added = *(new std::vector<int>());
	std::vector<int>& keys_to_add = *(new std::vector<int>());

	key_group.clear();
	keys_added.clear();
	keys_to_add.clear();

	//setup keys to add
	for (int i = 0; i < key_size; i++)
	{
		keys_to_add.push_back(i);
	}

	simple_multi_group_tracker<int, TData>* simple_mixed_array_test = new simple_multi_group_tracker<int, TData>();

	simple_mixed_array_test->init(group_size, key_size);

	template_fixed_size_multi_group_tracker<TData>* template_packed_array_test = new template_fixed_size_multi_group_tracker<TData>();

	template_packed_array_test->init(group_size, key_size);

	template_fixed_size_multi_group_2_pass<int, TData>* template_packed_array_test_2_pass = new template_fixed_size_multi_group_2_pass<int, TData>();

	template_packed_array_test_2_pass->init(group_size, key_size);

	template_fixed_size_multi_group_linked_list<TData>* template_linked_list_test = new template_fixed_size_multi_group_linked_list<TData>();

	template_linked_list_test->init(group_size, key_size);

	std::cout << "PerfTest start\n";

#pragma region SimpleMixedArray

	if (true)
	{
		std::cout << "simple mixed array profiling\n";

		srand(seed);

		
		for (uint32 i = 0; i < iterations; ++i)
		{
			{
				simple_mixed_array_test->clear();

				key_group.clear();
				keys_added.clear();
				keys_to_add.clear();

				//setup keys to add
				for (int i = 0; i < key_size; i++)
				{
					keys_to_add.push_back(i);
				}

				for (uint32 j = 0; j < start_test_amount; j++)
				{
					int key;
					int group;

					calculate_next_key_grop_to_add(key_group, keys_added, keys_to_add, group_size, force_add_to_existing_group_chane, key, group);

					auto data_to_add = TData(key);

					simple_mixed_array_test->add(group, key, &data_to_add);

				}
			}

			//get the start time for the test
			auto start_total_prof = get_time_for_profiling<TApplyMemoryFence>();

			for (uint32 j = 0; j < sub_itterations; j++)
			{
				//pick which test to do
				int test_mode = rand() % 100;

				if (test_mode < percent_read_tests && keys_added.size() > dont_read_if_less_than_this_many_items_in_structure)
				{
					//pick random group to read
					uint32 read_group_of_key = rand() % keys_added.size();

					int group = key_group[read_group_of_key];

					int read_index = 0;

					auto start_prof = get_time_for_profiling<TApplyMemoryFence>();

					TData* next_data = simple_mixed_array_test->get_next_value_in_group(group, &read_index);

					auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();

					prof_result.read_total_duration[(int)test_candidates::simple_index] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

					while (next_data != nullptr)
					{
						prof_result.sum_of_all_reads[(int)test_candidates::simple_index] += next_data->sum_of_all_data();

						auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();;

						next_data = simple_mixed_array_test->get_next_value_in_group(group, &read_index);

						auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();;

						prof_result.read_total_duration[(int)test_candidates::simple_index] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.num_reads[(int)test_candidates::simple_index]++;
					}

					prof_result.read_average_group_size[(int)test_candidates::simple_index] += simple_mixed_array_test->group_size(group);
				}
				else
				{
					//pick random number to add 
					int add_remove_index = rand() % key_size;

					//check if we are adding or removing
					if (add_remove_index < keys_added.size())
					{
						//remove key
						int key = keys_added[add_remove_index];
						int group = key_group[add_remove_index];

						//remove key from keys to add
						keys_added[add_remove_index] = keys_added[keys_added.size() - 1];
						key_group[add_remove_index] = key_group[key_group.size() - 1];
						keys_added.pop_back();
						key_group.pop_back();

						keys_to_add.push_back(key);

						prof_result.num_removes[(int)test_candidates::simple_index]++;

						//add to tracked structure
						auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();

						simple_mixed_array_test->remove(key);

						auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();

						prof_result.remove_total_duration[(int)test_candidates::simple_index] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.remove_average_group_size[(int)test_candidates::simple_index] += simple_mixed_array_test->group_size(group);

					}
					else
					{
						assert(keys_to_add.size() > 0);

						//add key
						int to_add_index = add_remove_index - keys_added.size();

						int key = keys_to_add[to_add_index];

						//pick random group to add to 
						int group = rand() % group_size;

						//use random existing group
						if ((rand() % 100) < force_add_to_existing_group_chane && keys_added.size() > 0)
						{
							group = key_group[rand() % key_group.size()];
						}

						//remove key from keys to add
						keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
						keys_to_add.pop_back();

						//add key to keys added 
						keys_added.push_back(key);
						key_group.push_back(group);

						prof_result.num_adds[(int)test_candidates::simple_index]++;

						auto data_to_add = TData(key);

						//add to tracked structure
						auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();

						simple_mixed_array_test->add(group, key, &data_to_add);

						auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();
						
						int temp_index = 0;

						prof_result.sum_of_all_reads[(int)test_candidates::simple_index] += simple_mixed_array_test->get_next_value_in_group(group, &temp_index)->sum_of_all_data();
						
						prof_result.add_total_duration[(int)test_candidates::simple_index] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.add_average_group_size[(int)test_candidates::simple_index] += simple_mixed_array_test->group_size(group);

					}
				}
			}

			//get the total time for the test to run
			prof_result.total_profile_duration[(int)test_candidates::simple_index] = std::chrono::duration_cast<std::chrono::nanoseconds>( get_time_for_profiling<TApplyMemoryFence>() - start_total_prof).count();

			std::cout << "Add remove Test:" << i << " complete\n";

		}


		prof_result.add_average_duration[(int)test_candidates::simple_index] = prof_result.add_total_duration[(int)test_candidates::simple_index] / prof_result.num_adds[(int)test_candidates::simple_index];
		prof_result.remove_average_duration[(int)test_candidates::simple_index] = prof_result.remove_total_duration[(int)test_candidates::simple_index] / prof_result.num_removes[(int)test_candidates::simple_index];
		prof_result.read_average_duration[(int)test_candidates::simple_index] = prof_result.read_total_duration[(int)test_candidates::simple_index] / prof_result.num_reads[(int)test_candidates::simple_index];

		prof_result.add_average_group_size[(int)test_candidates::simple_index] = prof_result.add_average_group_size[(int)test_candidates::simple_index] / prof_result.num_adds[(int)test_candidates::simple_index];
		prof_result.remove_average_group_size[(int)test_candidates::simple_index] = prof_result.remove_average_group_size[(int)test_candidates::simple_index] / prof_result.num_removes[(int)test_candidates::simple_index];
		prof_result.read_average_group_size[(int)test_candidates::simple_index] = prof_result.read_average_group_size[(int)test_candidates::simple_index] / prof_result.num_reads[(int)test_candidates::simple_index];


		std::cout << "simple mixed array profiling end\n";
		print_profile_result(prof_result, iterations, sub_itterations, (int)test_candidates::simple_index);
	}

#pragma endregion

	if (false)
	{
		std::cout << "non spaced groups profiling\n";

		srand(seed);

		for (uint32 i = 0; i < iterations; ++i)
		{

			template_packed_array_test->clear();
			template_packed_array_test->initalise_group_boundries();
			for (uint32 j = 0; j < start_test_amount; j++)
			{
				//add key
				int to_add_index = rand() % keys_to_add.size();

				int key = keys_to_add[to_add_index];

				//pick random group to add to 
				int group = rand() % group_size;

				//use random existing group
				if ((rand() % 100) < force_add_to_existing_group_chane && keys_added.size() > 0)
				{
					group = key_group[rand() % key_group.size()];
				}

				//remove key from keys to add
				keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
				keys_to_add.pop_back();

				//add key to keys added 
				keys_added.push_back(key);
				key_group.push_back(group);

				auto data_to_add = TData(key);

				template_packed_array_test->add(key, &data_to_add, group);

			}

			for (uint32 j = 0; j < sub_itterations; j++)
			{
				//pick which test to do
				int test_mode = rand() % 100;

				if (test_mode < percent_read_tests && keys_added.size() > dont_read_if_less_than_this_many_items_in_structure)
				{
					//pick random group to read
					uint32 read_group_of_key = rand() % keys_added.size();

					int group = key_group[read_group_of_key];

					int node_index = 0;

					for (int ik = 0; ik < template_packed_array_test->group_size[group]; ik++)
					{
						auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();

						

						int key = template_packed_array_test->get_key_at_group_index(group, ik);

						

						auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();

						prof_result.read_total_duration[(int)test_candidates::non_spaces_index] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.sum_of_all_reads[(int)test_candidates::non_spaces_index] += key;

						prof_result.num_reads[(int)test_candidates::non_spaces_index]++;
					}

					prof_result.read_average_group_size[(int)test_candidates::non_spaces_index] += template_packed_array_test->group_size[group];
				}
				else
				{
					//pick random number to add 
					int add_remove_index = rand() % key_size;

					//check if we are adding or removing
					if (add_remove_index < keys_added.size())
					{
						//remove key
						int key = keys_added[add_remove_index];
						int group = key_group[add_remove_index];

						//remove key from keys to add
						keys_added[add_remove_index] = keys_added[keys_added.size() - 1];
						key_group[add_remove_index] = key_group[key_group.size() - 1];
						keys_added.pop_back();
						key_group.pop_back();

						keys_to_add.push_back(key);

						prof_result.num_removes[(int)test_candidates::non_spaces_index]++;

						//add to tracked structure
						auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();

						

						template_packed_array_test->remove(key);

						

						auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();

						prof_result.remove_total_duration[(int)test_candidates::non_spaces_index] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.remove_average_group_size[(int)test_candidates::non_spaces_index] += template_packed_array_test->group_size[group];

					}
					else
					{
						//add key
						int to_add_index = add_remove_index - keys_added.size();

						int key = keys_to_add[to_add_index];

						//pick random group to add to 
						int group = rand() % group_size;

						//use random existing group
						if ((rand() % 100) < force_add_to_existing_group_chane && keys_added.size() > 0)
						{
							group = key_group[rand() % key_group.size()];
						}

						//remove key from keys to add
						keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
						keys_to_add.pop_back();

						//add key to keys added 
						keys_added.push_back(key);
						key_group.push_back(group);

						prof_result.num_adds[(int)test_candidates::non_spaces_index]++;

						auto data_to_add = TData(key);

						//add to tracked structure
						auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();

						

						template_packed_array_test->add(key, &data_to_add, group);

						

						auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();

						prof_result.add_total_duration[(int)test_candidates::non_spaces_index] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.add_average_group_size[(int)test_candidates::non_spaces_index] += template_packed_array_test->group_size[group];
					}
				}
			}

			std::cout << "Add remove Test:" << i << " complete\n";

			//reset test data
			template_packed_array_test->clear();
			template_packed_array_test->initalise_group_boundries();
			key_group.clear();
			keys_added.clear();
			keys_to_add.clear();

			//setup keys to add
			for (int i = 0; i < key_size; i++)
			{
				keys_to_add.push_back(i);
			}

		}


		prof_result.add_average_duration[(int)test_candidates::non_spaces_index] = prof_result.add_total_duration[(int)test_candidates::non_spaces_index] / prof_result.num_adds[(int)test_candidates::non_spaces_index];
		prof_result.remove_average_duration[(int)test_candidates::non_spaces_index] = prof_result.remove_total_duration[(int)test_candidates::non_spaces_index] / prof_result.num_removes[(int)test_candidates::non_spaces_index];
		prof_result.read_average_duration[(int)test_candidates::non_spaces_index] = prof_result.read_total_duration[(int)test_candidates::non_spaces_index] / prof_result.num_reads[(int)test_candidates::non_spaces_index];


		prof_result.add_average_group_size[(int)test_candidates::non_spaces_index] = prof_result.add_average_group_size[(int)test_candidates::non_spaces_index] / prof_result.num_adds[(int)test_candidates::non_spaces_index];
		prof_result.remove_average_group_size[(int)test_candidates::non_spaces_index] = prof_result.remove_average_group_size[(int)test_candidates::non_spaces_index] / prof_result.num_removes[(int)test_candidates::non_spaces_index];
		prof_result.read_average_group_size[(int)test_candidates::non_spaces_index] = prof_result.read_average_group_size[(int)test_candidates::non_spaces_index] / prof_result.num_reads[(int)test_candidates::non_spaces_index];


		std::cout << "non spaced group profiling end\n";
		print_profile_result(prof_result, iterations, sub_itterations, (int)test_candidates::non_spaces_index);
	}

	if (false)
	{

		std::cout << "spaced group profiling\n";

		srand(seed);

		for (uint32 i = 0; i < iterations; ++i)
		{
			template_packed_array_test->clear();
			template_packed_array_test->initalise_group_boundries_v2();
			for (uint32 j = 0; j < start_test_amount; j++)
			{
				//add key
				int to_add_index = rand() % keys_to_add.size();

				int key = keys_to_add[to_add_index];

				//pick random group to add to 
				int group = rand() % group_size;

				//use random existing group
				if ((rand() % 100) < force_add_to_existing_group_chane && keys_added.size() > 0)
				{
					group = key_group[rand() % key_group.size()];
				}

				//remove key from keys to add
				keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
				keys_to_add.pop_back();

				//add key to keys added 
				keys_added.push_back(key);
				key_group.push_back(group);

				auto data_to_add = TData(key);

				template_packed_array_test->add(key, &data_to_add, group);

			}

			for (uint32 j = 0; j < sub_itterations; j++)
			{
				//pick which test to do
				int test_mode = rand() % 100;

				if (test_mode < percent_read_tests && keys_added.size() > dont_read_if_less_than_this_many_items_in_structure)
				{
					//pick random group to read
					uint32 read_group_of_key = rand() % keys_added.size();

					int group = key_group[read_group_of_key];

					int node_index = 0;

					for (int ik = 0; ik < template_packed_array_test->group_size[group]; ik++)
					{
						auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();

						

						int key = template_packed_array_test->get_key_at_group_index(group, ik);

						

						auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();

						prof_result.read_total_duration[(int)test_candidates::spaced_index] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.sum_of_all_reads[(int)test_candidates::spaced_index] += key;

						prof_result.num_reads[(int)test_candidates::spaced_index]++;
					}

					prof_result.read_average_group_size[(int)test_candidates::spaced_index] += template_packed_array_test->group_size[group];
				}
				else
				{

					//pick random number to add 
					int add_remove_index = rand() % key_size;

					//check if we are adding or removing
					if (add_remove_index < keys_added.size())
					{
						//remove key
						int key = keys_added[add_remove_index];
						int group = key_group[add_remove_index];

						//remove key from keys to add
						keys_added[add_remove_index] = keys_added[keys_added.size() - 1];
						key_group[add_remove_index] = key_group[key_group.size() - 1];
						keys_added.pop_back();
						key_group.pop_back();

						keys_to_add.push_back(key);

						prof_result.num_removes[(int)test_candidates::spaced_index]++;

						//add to tracked structure
						auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();

						

						template_packed_array_test->remove(key);

						

						auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();

						prof_result.remove_total_duration[(int)test_candidates::spaced_index] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.remove_average_group_size[(int)test_candidates::spaced_index] += template_packed_array_test->group_size[group];
					}
					else
					{
						//add key
						int to_add_index = add_remove_index - keys_added.size();

						int key = keys_to_add[to_add_index];

						//pick random group to add to 
						int group = rand() % group_size;

						//use random existing group
						if ((rand() % 100) < force_add_to_existing_group_chane && keys_added.size() > 0)
						{
							group = key_group[rand() % key_group.size()];
						}

						//remove key from keys to add
						keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
						keys_to_add.pop_back();

						//add key to keys added 
						keys_added.push_back(key);
						key_group.push_back(group);

						prof_result.num_adds[(int)test_candidates::spaced_index]++;

						auto data_to_add = TData(key);

						//add to tracked structure
						auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();

						

						template_packed_array_test->add(key, &data_to_add, group);

						

						auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();

						prof_result.add_total_duration[(int)test_candidates::spaced_index] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.add_average_group_size[(int)test_candidates::spaced_index] += template_packed_array_test->group_size[group];
					}
				}
			}

			std::cout << "Add remove Test:" << i << " complete\n";

			//reset test data
			template_packed_array_test->clear();
			template_packed_array_test->initalise_group_boundries_v2();
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

		prof_result.add_average_duration[(int)test_candidates::spaced_index] = prof_result.add_total_duration[(int)test_candidates::spaced_index] / prof_result.num_adds[(int)test_candidates::spaced_index];
		prof_result.remove_average_duration[(int)test_candidates::spaced_index] = prof_result.remove_total_duration[(int)test_candidates::spaced_index] / prof_result.num_removes[(int)test_candidates::spaced_index];
		prof_result.read_average_duration[(int)test_candidates::spaced_index] = prof_result.read_total_duration[(int)test_candidates::spaced_index] / prof_result.num_reads[(int)test_candidates::spaced_index];

		prof_result.add_average_group_size[(int)test_candidates::spaced_index] = prof_result.add_average_group_size[(int)test_candidates::spaced_index] / prof_result.num_adds[(int)test_candidates::spaced_index];
		prof_result.remove_average_group_size[(int)test_candidates::spaced_index] = prof_result.remove_average_group_size[(int)test_candidates::spaced_index] / prof_result.num_removes[(int)test_candidates::spaced_index];
		prof_result.read_average_group_size[(int)test_candidates::spaced_index] = prof_result.read_average_group_size[(int)test_candidates::spaced_index] / prof_result.num_reads[(int)test_candidates::spaced_index];


		std::cout << "non spaced group profiling end\n";
		print_profile_result(prof_result, iterations, sub_itterations, (int)test_candidates::spaced_index);
	}

	if (true)
	{
		std::cout << "linked list group profiling\n";

		srand(seed);

		for (uint32 i = 0; i < iterations; ++i)
		{
			{
				template_linked_list_test->clear();

				key_group.clear();
				keys_added.clear();
				keys_to_add.clear();

				//setup keys to add
				for (int i = 0; i < key_size; i++)
				{
					keys_to_add.push_back(i);
				}

				for (uint32 j = 0; j < start_test_amount; j++)
				{
					int key;
					int group;

					calculate_next_key_grop_to_add(key_group, keys_added, keys_to_add, group_size, force_add_to_existing_group_chane, key, group);

					auto data_to_add = TData(key);

					template_linked_list_test->add(group, key, &data_to_add);

				}
			}

			//get the start time for the test
			auto start_total_prof =  get_time_for_profiling<TApplyMemoryFence>();

			for (uint32 j = 0; j < sub_itterations; j++)
			{
				//pick which test to do
				int test_mode = rand() % 100;

				if (test_mode < percent_read_tests && keys_added.size() > dont_read_if_less_than_this_many_items_in_structure)
				{
					//pick random group to read
					uint32 read_group_of_key = rand() % keys_added.size();

					int group = key_group[read_group_of_key];

					int node_index = 0;

					TData* data_for_key;

					//get the first entry in group
					auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();

					template_linked_list_test->get_first_value_in_group(group, &node_index, &data_for_key);

					auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();

					prof_result.read_total_duration[(int)test_candidates::linked_list_index] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

					prof_result.num_reads[(int)test_candidates::linked_list_index]++; //this will produce different numbers of reads / writes because we need to sample the first item to check if there are units in the group

					prof_result.sum_of_all_reads[(int)test_candidates::linked_list_index] += data_for_key->sum_of_all_data();

					while (!template_linked_list_test->is_end_of_group(node_index, group))
					{
						start_prof =  get_time_for_profiling<TApplyMemoryFence>();

						template_linked_list_test->get_next_value(&node_index, &data_for_key);

						end_prof =  get_time_for_profiling<TApplyMemoryFence>();

						prof_result.read_total_duration[(int)test_candidates::linked_list_index] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.num_reads[(int)test_candidates::linked_list_index]++;

						prof_result.sum_of_all_reads[(int)test_candidates::linked_list_index] += data_for_key->sum_of_all_data();
					}

					prof_result.read_average_group_size[(int)test_candidates::linked_list_index] += template_linked_list_test->group_size(group);

				}
				else
				{
					//pick random number to add 
					int add_remove_index = rand() % key_size;

					//check if we are adding or removing
					if (add_remove_index < keys_added.size())
					{
						//remove key
						int key = keys_added[add_remove_index];
						int group = key_group[add_remove_index];

						//remove key from keys to add
						keys_added[add_remove_index] = keys_added[keys_added.size() - 1];
						key_group[add_remove_index] = key_group[key_group.size() - 1];
						keys_added.pop_back();
						key_group.pop_back();

						keys_to_add.push_back(key);

						prof_result.num_removes[(int)test_candidates::linked_list_index]++;

						//add to tracked structure
						auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();

						template_linked_list_test->remove(key);

						auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();

						prof_result.remove_total_duration[(int)test_candidates::linked_list_index] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.remove_average_group_size[(int)test_candidates::linked_list_index] += template_linked_list_test->group_size(group);
					}
					else
					{
						//add key
						int to_add_index = add_remove_index - keys_added.size();

						int key = keys_to_add[to_add_index];

						//pick random group to add to 
						int group = rand() % group_size;

						//use random existing group
						if ((rand() % 100) < force_add_to_existing_group_chane && keys_added.size() > 0)
						{
							group = key_group[rand() % key_group.size()];
						}

						//remove key from keys to add
						keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
						keys_to_add.pop_back();

						//add key to keys added 
						keys_added.push_back(key);
						key_group.push_back(group);

						prof_result.num_adds[(int)test_candidates::linked_list_index]++;

						auto data_to_add = TData(key);

						//add to tracked structure
						auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();

						template_linked_list_test->add(group, key, &data_to_add);

						auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();

						prof_result.add_total_duration[(int)test_candidates::linked_list_index] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.add_average_group_size[(int)test_candidates::linked_list_index] += template_linked_list_test->group_size(group);
					}
				}
			}

			//get the total time for the test to run
			prof_result.total_profile_duration[(int)test_candidates::linked_list_index]	= std::chrono::duration_cast<std::chrono::nanoseconds>( get_time_for_profiling<TApplyMemoryFence>() - start_total_prof).count();

			std::cout << "Add remove Test:" << i << " complete\n";

		}

		prof_result.average_profile_duration[(int)test_candidates::linked_list_index] = prof_result.total_profile_duration[(int)test_candidates::linked_list_index] / iterations;

		prof_result.add_average_duration[(int)test_candidates::linked_list_index] = prof_result.add_total_duration[(int)test_candidates::linked_list_index] / prof_result.num_adds[(int)test_candidates::linked_list_index];
		prof_result.remove_average_duration[(int)test_candidates::linked_list_index] = prof_result.remove_total_duration[(int)test_candidates::linked_list_index] / prof_result.num_removes[(int)test_candidates::linked_list_index];
		prof_result.read_average_duration[(int)test_candidates::linked_list_index] = prof_result.read_total_duration[(int)test_candidates::linked_list_index] / prof_result.num_reads[(int)test_candidates::linked_list_index];

		prof_result.add_average_group_size[(int)test_candidates::linked_list_index] = prof_result.add_average_group_size[(int)test_candidates::linked_list_index] / prof_result.num_adds[(int)test_candidates::linked_list_index];
		prof_result.remove_average_group_size[(int)test_candidates::linked_list_index] = prof_result.remove_average_group_size[(int)test_candidates::linked_list_index] / prof_result.num_removes[(int)test_candidates::linked_list_index];
		prof_result.read_average_group_size[(int)test_candidates::linked_list_index] = prof_result.read_average_group_size[(int)test_candidates::linked_list_index] / prof_result.num_reads[(int)test_candidates::linked_list_index];

		std::cout << "linked list group  end\n";
		print_profile_result(prof_result, iterations, sub_itterations, (int)test_candidates::linked_list_index);

	}

	if (false)
	{
		std::cout << "packed 2 pass non spaced start\n";

		srand(seed);

		for (uint32 i = 0; i < iterations; ++i)
		{
			template_packed_array_test_2_pass->clear();
			template_packed_array_test_2_pass->initalise_group_boundries();

			for (uint32 j = 0; j < start_test_amount; j++)
			{
				//add key
				int to_add_index = rand() % keys_to_add.size();

				int key = keys_to_add[to_add_index];

				//pick random group to add to 
				int group = rand() % group_size;

				//use random existing group
				if ((rand() % 100) < force_add_to_existing_group_chane && keys_added.size() > 0)
				{
					group = key_group[rand() % key_group.size()];
				}

				//remove key from keys to add
				keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
				keys_to_add.pop_back();

				//add key to keys added 
				keys_added.push_back(key);
				key_group.push_back(group);

				auto data_to_add = TData(key);

				template_packed_array_test_2_pass->add(group, key, &data_to_add);

			}

			for (uint32 j = 0; j < sub_itterations; j++)
			{
				//pick which test to do
				int test_mode = rand() % 100;

				if (test_mode < percent_read_tests && keys_added.size() > dont_read_if_less_than_this_many_items_in_structure)
				{
					//pick random group to read
					uint32 read_group_of_key = rand() % keys_added.size();

					int group = key_group[read_group_of_key];

					int node_index = 0;

					for (int ik = 0; ik < template_packed_array_test_2_pass->group_size[group]; ik++)
					{
						auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();

						

						TData* data = template_packed_array_test_2_pass->get_value_at_group_index(group, ik);

						

						auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();

						prof_result.read_total_duration[(int)test_candidates::non_spaces_index_2_pass] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.sum_of_all_reads[(int)test_candidates::non_spaces_index_2_pass] += data->sum_of_all_data();

						prof_result.num_reads[(int)test_candidates::non_spaces_index_2_pass]++;
					}

					prof_result.read_average_group_size[(int)test_candidates::non_spaces_index_2_pass] += template_packed_array_test_2_pass->group_size[group];
				}
				else
				{
					//pick random number to add 
					int add_remove_index = rand() % key_size;

					//check if we are adding or removing
					if (add_remove_index < keys_added.size())
					{
						//remove key
						int key = keys_added[add_remove_index];
						int group = key_group[add_remove_index];

						//remove key from keys to add
						keys_added[add_remove_index] = keys_added[keys_added.size() - 1];
						key_group[add_remove_index] = key_group[key_group.size() - 1];
						keys_added.pop_back();
						key_group.pop_back();

						keys_to_add.push_back(key);

						prof_result.num_removes[(int)test_candidates::non_spaces_index_2_pass]++;

						//add to tracked structure
						auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();

						

						template_packed_array_test_2_pass->remove(key);

						

						auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();

						prof_result.remove_total_duration[(int)test_candidates::non_spaces_index_2_pass] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.remove_average_group_size[(int)test_candidates::non_spaces_index_2_pass] += template_packed_array_test_2_pass->group_size[group];

					}
					else
					{
						//add key
						int to_add_index = add_remove_index - keys_added.size();

						int key = keys_to_add[to_add_index];

						//pick random group to add to 
						int group = rand() % group_size;

						//use random existing group
						if ((rand() % 100) < force_add_to_existing_group_chane && keys_added.size() > 0)
						{
							group = key_group[rand() % key_group.size()];
						}

						//remove key from keys to add
						keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
						keys_to_add.pop_back();

						//add key to keys added 
						keys_added.push_back(key);
						key_group.push_back(group);

						prof_result.num_adds[(int)test_candidates::non_spaces_index_2_pass]++;

						auto data_to_add = TData(key);

						//add to tracked structure
						auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();

						

						template_packed_array_test_2_pass->add(group, key, &data_to_add);

						

						auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();

						prof_result.add_total_duration[(int)test_candidates::non_spaces_index_2_pass] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.add_average_group_size[(int)test_candidates::non_spaces_index_2_pass] += template_packed_array_test_2_pass->group_size[group];
					}
				}
			}

			std::cout << "Add remove Test:" << i << " complete\n";

			//reset test data
			template_packed_array_test_2_pass->clear();
			key_group.clear();
			keys_added.clear();
			keys_to_add.clear();

			//setup keys to add
			for (int i = 0; i < key_size; i++)
			{
				keys_to_add.push_back(i);
			}

		}


		prof_result.add_average_duration[(int)test_candidates::non_spaces_index_2_pass] = prof_result.add_total_duration[(int)test_candidates::non_spaces_index_2_pass] / prof_result.num_adds[(int)test_candidates::non_spaces_index_2_pass];
		prof_result.remove_average_duration[(int)test_candidates::non_spaces_index_2_pass] = prof_result.remove_total_duration[(int)test_candidates::non_spaces_index_2_pass] / prof_result.num_removes[(int)test_candidates::non_spaces_index_2_pass];
		prof_result.read_average_duration[(int)test_candidates::non_spaces_index_2_pass] = prof_result.read_total_duration[(int)test_candidates::non_spaces_index_2_pass] / prof_result.num_reads[(int)test_candidates::non_spaces_index_2_pass];

		prof_result.add_average_group_size[(int)test_candidates::non_spaces_index_2_pass] = prof_result.add_average_group_size[(int)test_candidates::non_spaces_index_2_pass] / prof_result.num_adds[(int)test_candidates::non_spaces_index_2_pass];
		prof_result.remove_average_group_size[(int)test_candidates::non_spaces_index_2_pass] = prof_result.remove_average_group_size[(int)test_candidates::non_spaces_index_2_pass] / prof_result.num_removes[(int)test_candidates::non_spaces_index_2_pass];
		prof_result.read_average_group_size[(int)test_candidates::non_spaces_index_2_pass] = prof_result.read_average_group_size[(int)test_candidates::non_spaces_index_2_pass] / prof_result.num_reads[(int)test_candidates::non_spaces_index_2_pass];




		std::cout << "non spaced group profiling_2_pass end\n" << std::endl;
		print_profile_result(prof_result, iterations, sub_itterations, (int)test_candidates::non_spaces_index_2_pass);

	}

	if (true)
	{
		std::cout << "spaced group profiling 2 pass\n";

		srand(seed);

		for (uint32 i = 0; i < iterations; ++i)
		{
			{
				template_packed_array_test_2_pass->clear();
				template_packed_array_test_2_pass->initalise_group_boundries_v2();

				key_group.clear();
				keys_added.clear();
				keys_to_add.clear();

				//setup keys to add
				for (int i = 0; i < key_size; i++)
				{
					keys_to_add.push_back(i);
				}

				for (uint32 j = 0; j < start_test_amount; j++)
				{
					int key;
					int group;

					calculate_next_key_grop_to_add(key_group, keys_added, keys_to_add, group_size, force_add_to_existing_group_chane, key, group);

					auto data_to_add = TData(key);

					template_packed_array_test_2_pass->add(group, key, &data_to_add);

				}
			}

			//get the start time for the test
			auto start_total_prof =  get_time_for_profiling<TApplyMemoryFence>();

			for (uint32 j = 0; j < sub_itterations; j++)
			{
				//pick which test to do
				int test_mode = rand() % 100;

				if (test_mode < percent_read_tests && keys_added.size() > dont_read_if_less_than_this_many_items_in_structure)
				{
					//pick random group to read
					uint32 read_group_of_key = rand() % keys_added.size();

					int group = key_group[read_group_of_key];

					int node_index = 0;

					for (int ik = 0; ik < template_packed_array_test_2_pass->group_size[group]; ik++)
					{
						auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();

						TData* data = template_packed_array_test_2_pass->get_value_at_group_index(group, ik);

						auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();

						prof_result.read_total_duration[(int)test_candidates::spaced_index_2_pass] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.sum_of_all_reads[(int)test_candidates::spaced_index_2_pass] += data->sum_of_all_data();

						prof_result.num_reads[(int)test_candidates::spaced_index_2_pass]++;
					}

					prof_result.read_average_group_size[(int)test_candidates::spaced_index_2_pass] += template_packed_array_test_2_pass->group_size[group];
				}
				else
				{
					//pick random number to add 
					int add_remove_index = rand() % key_size;

					//check if we are adding or removing
					if (add_remove_index < keys_added.size())
					{
						//remove key
						int key = keys_added[add_remove_index];
						int group = key_group[add_remove_index];

						//remove key from keys to add
						keys_added[add_remove_index] = keys_added[keys_added.size() - 1];
						key_group[add_remove_index] = key_group[key_group.size() - 1];
						keys_added.pop_back();
						key_group.pop_back();

						keys_to_add.push_back(key);

						prof_result.num_removes[(int)test_candidates::spaced_index_2_pass]++;

						//add to tracked structure
						auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();

						template_packed_array_test_2_pass->remove(key);

						auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();

						prof_result.remove_total_duration[(int)test_candidates::spaced_index_2_pass] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.remove_average_group_size[(int)test_candidates::spaced_index_2_pass] += template_packed_array_test_2_pass->group_size[group];
					}
					else
					{
						//add key
						int to_add_index = add_remove_index - keys_added.size();

						int key = keys_to_add[to_add_index];

						//pick random group to add to 
						int group = rand() % group_size;

						//use random existing group
						if ((rand() % 100) < force_add_to_existing_group_chane && keys_added.size() > 0)
						{
							group = key_group[rand() % key_group.size()];
						}

						//remove key from keys to add
						keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
						keys_to_add.pop_back();

						//add key to keys added 
						keys_added.push_back(key);
						key_group.push_back(group);


						prof_result.num_adds[(int)test_candidates::spaced_index_2_pass]++;

						auto data_to_add = TData(key);

						//add to tracked structure
						auto start_prof =  get_time_for_profiling<TApplyMemoryFence>();

						template_packed_array_test_2_pass->add(group, key, &data_to_add);

						auto end_prof =  get_time_for_profiling<TApplyMemoryFence>();

						prof_result.add_total_duration[(int)test_candidates::spaced_index_2_pass] += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

						prof_result.add_average_group_size[(int)test_candidates::spaced_index_2_pass] += template_packed_array_test_2_pass->group_size[group];
					}
				}
			}

			//get the total time for the test to run
			prof_result.total_profile_duration[(int)test_candidates::spaced_index_2_pass] = std::chrono::duration_cast<std::chrono::nanoseconds>( get_time_for_profiling<TApplyMemoryFence>() - start_total_prof).count();


			std::cout << "Add remove Test:" << i << " complete\n";

		}

		prof_result.average_profile_duration[(int)test_candidates::spaced_index_2_pass] = prof_result.total_profile_duration[(int)test_candidates::spaced_index_2_pass] / iterations;

		prof_result.add_average_duration[(int)test_candidates::spaced_index_2_pass] = prof_result.add_total_duration[(int)test_candidates::spaced_index_2_pass] / prof_result.num_adds[(int)test_candidates::spaced_index_2_pass];
		prof_result.remove_average_duration[(int)test_candidates::spaced_index_2_pass] = prof_result.remove_total_duration[(int)test_candidates::spaced_index_2_pass] / prof_result.num_removes[(int)test_candidates::spaced_index_2_pass];
		prof_result.read_average_duration[(int)test_candidates::spaced_index_2_pass] = prof_result.read_total_duration[(int)test_candidates::spaced_index_2_pass] / prof_result.num_reads[(int)test_candidates::spaced_index_2_pass];

		prof_result.add_average_group_size[(int)test_candidates::spaced_index_2_pass] = prof_result.add_average_group_size[(int)test_candidates::spaced_index_2_pass] / prof_result.num_adds[(int)test_candidates::spaced_index_2_pass];
		prof_result.remove_average_group_size[(int)test_candidates::spaced_index_2_pass] = prof_result.remove_average_group_size[(int)test_candidates::spaced_index_2_pass] / prof_result.num_removes[(int)test_candidates::spaced_index_2_pass];
		prof_result.read_average_group_size[(int)test_candidates::spaced_index_2_pass] = prof_result.read_average_group_size[(int)test_candidates::spaced_index_2_pass] / prof_result.num_reads[(int)test_candidates::spaced_index_2_pass];



		std::cout << "spaced group profiling 2 pass end\n";
		print_profile_result(prof_result, iterations, sub_itterations, (int)test_candidates::spaced_index_2_pass);

	}
	

	auto non_spaced_groups_total_duration = (prof_result.add_total_duration[(int)test_candidates::non_spaces_index] + prof_result.remove_total_duration[(int)test_candidates::non_spaces_index] + prof_result.read_total_duration[(int)test_candidates::non_spaces_index]);
	auto spaced_groups_total_duration = (prof_result.add_total_duration[(int)test_candidates::spaced_index] + prof_result.remove_total_duration[(int)test_candidates::spaced_index] + prof_result.read_total_duration[(int)test_candidates::spaced_index]);
	auto linked_list_groups_total_duration = (prof_result.add_total_duration[(int)test_candidates::linked_list_index] + prof_result.remove_total_duration[(int)test_candidates::linked_list_index] + prof_result.read_total_duration[(int)test_candidates::linked_list_index]);
	auto non_spaced_groups_total_duration_2_pass = (prof_result.add_total_duration[(int)test_candidates::non_spaces_index_2_pass] + prof_result.remove_total_duration[(int)test_candidates::non_spaces_index_2_pass] + prof_result.read_total_duration[(int)test_candidates::non_spaces_index_2_pass]);
	auto spaced_groups_total_duration_2_pass = (prof_result.add_total_duration[(int)test_candidates::spaced_index_2_pass] + prof_result.remove_total_duration[(int)test_candidates::spaced_index_2_pass] + prof_result.read_total_duration[(int)test_candidates::spaced_index_2_pass]);

	simple_mixed_array_test->cleanup();

	template_packed_array_test->clean_up();

	template_packed_array_test_2_pass->clean_up();

	template_linked_list_test->clean_up();


	delete (&key_group );
	delete (&keys_added);
	delete (&keys_to_add);



	delete(simple_mixed_array_test);
	delete(template_packed_array_test);
	delete(template_packed_array_test_2_pass);
	delete(template_linked_list_test);

	return prof_result;
}

template<typename TData>
void test_copy_performance(uint32 seed, uint32 array_size,uint32 iterations)
{
	TData* data_array = new TData[array_size];

	uint32 sum_of_moved_data = 0;

	//setup seed 
	srand(seed);

	long long total_time_spent = 0;

	//random copy test
	for (int i = 0; i < iterations; i++)
	{
		uint32 from = rand() % array_size;
		uint32 to = rand() % array_size;


		//add to tracked structure
		auto start_prof = std::chrono::high_resolution_clock::now();

		data_array[to] = data_array[from];

		auto end_prof = std::chrono::high_resolution_clock::now();

		sum_of_moved_data += data_array[to].sum_of_all_data();

		total_time_spent += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();

	}


	std::cout << "rand copy test end\n";
	std::cout << "total:" << total_time_spent << "ns , average : " << total_time_spent / iterations << "ns." << std::endl << std::endl;

	delete(data_array);
};

int main()
{
	std::cout << "Hello World!\n";

	//setup a central value surounded by 2 on either side
	//int group_size = 256;
	//int key_size = 65535;

	//int group_size = 8;
	//int key_size = 65535;

	int group_size = 256;
	int key_size = 65535 * 0.1f;

	bool run_unit_test = false;
	bool run_perf_test = true;
	bool run_copy_test = false;

	int seed = 0;

	std::vector<int> key_group = std::vector<int>();
	std::vector<int> keys_added = std::vector<int>();
	std::vector<int> keys_to_add = std::vector<int>();

#pragma region UnitTestMultiGroupTracker
	if (run_unit_test)
	{

		//setup keys to add
		for (int i = 0; i < key_size; i++)
		{
			keys_to_add.push_back(i);
		}

		srand(seed);

		fixed_size_multi_group_tracker test;

		test.init(group_size, key_size);


		auto begin = std::chrono::high_resolution_clock::now();

		uint32 iterations = 10;

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
					std::cout << "Error in Test:" << i << " after adding:" << j << " keys\n";
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
	}

#pragma endregion

#pragma region UnitTestMultiGroupLinkedList
	if (run_unit_test)
	{
		keys_to_add.clear();

		//setup keys to add
		for (int i = 0; i < key_size; i++)
		{
			keys_to_add.push_back(i);
		}

		srand(seed);

		fixed_size_multi_group_linked_list test;

		test.init(group_size, key_size);

		auto begin = std::chrono::high_resolution_clock::now();

		typedef  uint32_t uint32;

		uint32 iterations = 10;

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
				if (_validate_multi_group_tracker_linked_list(test, key_group, keys_added) == false)
				{
					std::cout << "Error in Test:" << i << " after adding:" << j << " keys\n";
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
					if (_validate_multi_group_tracker_linked_list(test, key_group, keys_added) == false)
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
					if (_validate_multi_group_tracker_linked_list(test, key_group, keys_added) == false)
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
	}

#pragma endregion

#pragma region UnitTestTemplateMuletiGroupTracker
	if (run_unit_test)
	{
		_unit_test_multi_group_tracker<test_data<4>>(group_size, key_size, seed);
	}
#pragma endregion

#pragma region UnitTestTemplateMuletiGroup2Pass
	if (run_unit_test)
	{
		_unit_test_multi_group_tracker_2_pass<test_data<4>>(group_size, key_size, seed);
	}
#pragma endregion

#pragma region UnitTestTempalteMultiGroupLinkedList
	if (run_unit_test)
	{
		_unit_test_multi_group_tracker_linked_list<test_data<4>>(group_size, key_size, seed);
	}
#pragma endregion

#pragma region UnitTestSimpleMultiGroupTracker
	if (run_unit_test)
	{
		_unit_test_simple_multi_group_tracker<test_data<4>>(group_size, key_size, seed);
	}
	
#pragma endregion

#pragma region PerTestComparison
	//if (run_perf_test)
	//{
	//	key_group.clear();
	//	keys_added.clear();
	//	keys_to_add.clear();
	//
	//	//setup keys to add
	//	for (int i = 0; i < key_size; i++)
	//	{
	//		keys_to_add.push_back(i);
	//	}
	//
	//	constexpr uint32 num_test_candidates = 3;
	//
	//	uint32 spaced_index = 0;
	//	uint32 non_spaces_index = 1;
	//	uint32 linked_list_index = 2;
	//
	//	uint32 iterations = 3;
	//	int sub_itterations = key_size * 1.0f;
	//
	//	uint32 percent_read_tests = 0;
	//
	//	uint32 num_adds[num_test_candidates] = {};
	//	uint32 num_removes[num_test_candidates] = {};
	//	uint32 num_reads[num_test_candidates] = {};
	//	uint32 sum_of_all_reads[num_test_candidates] = {};
	//
	//	long long non_spaced_groups_total_duration = 0;
	//	long long spaced_groups_total_duration = 0;
	//	long long linked_list_groups_total_duration = 0;
	//
	//
	//	fixed_size_multi_group_tracker packed_array_test;
	//
	//	packed_array_test.init(group_size, key_size);
	//
	//	fixed_size_multi_group_linked_list  linked_list_test;
	//
	//	linked_list_test.init(group_size, key_size);
	//
	//	std::cout << "PerfTest start\n";
	//
	//	std::cout << "non spaced groups profiling\n";
	//
	//	srand(seed);
	//
	//	packed_array_test.clear();
	//
	//
	//	for (uint32 i = 0; i < iterations; ++i)
	//	{
	//		for (uint32 j = 0; j < sub_itterations; j++)
	//		{
	//			//pick which test to do
	//			int test_mode = rand() % 100;
	//
	//			if (test_mode < percent_read_tests && keys_added.size() > 0)
	//			{
	//				//pick random group to read
	//				uint32 read_group_of_key = rand() % keys_added.size();
	//
	//				int group = key_group[read_group_of_key];
	//
	//				int node_index = 0;
	//
	//				for (int ik = 0; ik < packed_array_test.group_size[group]; ik++)
	//				{
	//					auto start_prof = std::chrono::high_resolution_clock::now();
	//
	//					int key = packed_array_test.get_key_at_group_index(group, ik);
	//
	//					auto end_prof = std::chrono::high_resolution_clock::now();
	//
	//					non_spaced_groups_total_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();
	//
	//					sum_of_all_reads[non_spaces_index] += key;
	//
	//					num_reads[non_spaces_index]++;
	//				}
	//			}
	//			else
	//			{
	//
	//
	//				//pick random number to add 
	//				int add_remove_index = rand() % key_size;
	//
	//				//check if we are adding or removing
	//				if (add_remove_index < keys_added.size())
	//				{
	//					//remove key
	//					int key = keys_added[add_remove_index];
	//					//remove key from keys to add
	//					keys_added[add_remove_index] = keys_added[keys_added.size() - 1];
	//					key_group[add_remove_index] = key_group[key_group.size() - 1];
	//					keys_added.pop_back();
	//					key_group.pop_back();
	//
	//					keys_to_add.push_back(key);
	//
	//					num_removes[non_spaces_index]++;
	//
	//					//add to tracked structure
	//					auto start_prof = std::chrono::high_resolution_clock::now();
	//
	//					packed_array_test.remove(key);
	//
	//					auto end_prof = std::chrono::high_resolution_clock::now();
	//
	//					non_spaced_groups_total_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();
	//
	//				}
	//				else
	//				{
	//					//add key
	//					int to_add_index = add_remove_index - keys_added.size();
	//
	//					int key = keys_to_add[to_add_index];
	//
	//					//pick random group to add to 
	//					int group = rand() % group_size;
	//
	//					//remove key from keys to add
	//					keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
	//					keys_to_add.pop_back();
	//
	//					//add key to keys added 
	//					keys_added.push_back(key);
	//					key_group.push_back(group);
	//
	//					num_adds[non_spaces_index]++;
	//
	//					//add to tracked structure
	//					auto start_prof = std::chrono::high_resolution_clock::now();
	//
	//					packed_array_test.add(key, group);
	//
	//					auto end_prof = std::chrono::high_resolution_clock::now();
	//
	//					non_spaced_groups_total_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();
	//				}
	//			}
	//		}
	//
	//		std::cout << "Add remove Test:" << i << " complete\n";
	//
	//		//reset test data
	//		packed_array_test.clear();
	//		key_group.clear();
	//		keys_added.clear();
	//		keys_to_add.clear();
	//
	//		//setup keys to add
	//		for (int i = 0; i < key_size; i++)
	//		{
	//			keys_to_add.push_back(i);
	//		}
	//
	//	}
	//
	//	std::cout << "non spaced group profiling end\n";
	//	std::cout << "non spaced ns total:" << non_spaced_groups_total_duration << ", average : " << non_spaced_groups_total_duration / (iterations * (long long)sub_itterations) << "ns." << std::endl;
	//
	//
	//	std::cout << "spaced group profiling\n";
	//
	//
	//	srand(seed);
	//
	//	packed_array_test.clear();
	//	packed_array_test.initalise_group_boundries_v2();
	//
	//
	//
	//	for (uint32 i = 0; i < iterations; ++i)
	//	{
	//		for (uint32 j = 0; j < sub_itterations; j++)
	//		{
	//			//pick which test to do
	//			int test_mode = rand() % 100;
	//
	//			if (test_mode < percent_read_tests && keys_added.size() > 0)
	//			{
	//				//pick random group to read
	//				uint32 read_group_of_key = rand() % keys_added.size();
	//
	//				int group = key_group[read_group_of_key];
	//
	//				int node_index = 0;
	//
	//				for (int ik = 0; ik < packed_array_test.group_size[group]; ik++)
	//				{
	//					auto start_prof = std::chrono::high_resolution_clock::now();
	//
	//					int key = packed_array_test.get_key_at_group_index(group, ik);
	//
	//					auto end_prof = std::chrono::high_resolution_clock::now();
	//
	//					spaced_groups_total_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();
	//
	//					sum_of_all_reads[spaced_index] += key;
	//
	//					num_reads[spaced_index]++;
	//				}
	//			}
	//			else
	//			{
	//
	//				//pick random number to add 
	//				int add_remove_index = rand() % key_size;
	//
	//				//check if we are adding or removing
	//				if (add_remove_index < keys_added.size())
	//				{
	//					//remove key
	//					int key = keys_added[add_remove_index];
	//					//remove key from keys to add
	//					keys_added[add_remove_index] = keys_added[keys_added.size() - 1];
	//					key_group[add_remove_index] = key_group[key_group.size() - 1];
	//					keys_added.pop_back();
	//					key_group.pop_back();
	//
	//					keys_to_add.push_back(key);
	//
	//					num_removes[spaced_index]++;
	//
	//					//add to tracked structure
	//					auto start_prof = std::chrono::high_resolution_clock::now();
	//
	//					packed_array_test.remove(key);
	//
	//					auto end_prof = std::chrono::high_resolution_clock::now();
	//
	//					spaced_groups_total_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();
	//				}
	//				else
	//				{
	//					//add key
	//					int to_add_index = add_remove_index - keys_added.size();
	//
	//					int key = keys_to_add[to_add_index];
	//
	//					//pick random group to add to 
	//					int group = rand() % group_size;
	//
	//					//remove key from keys to add
	//					keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
	//					keys_to_add.pop_back();
	//
	//					//add key to keys added 
	//					keys_added.push_back(key);
	//					key_group.push_back(group);
	//
	//
	//					num_adds[spaced_index]++;
	//
	//					//add to tracked structure
	//					auto start_prof = std::chrono::high_resolution_clock::now();
	//
	//					packed_array_test.add(key, group);
	//
	//					auto end_prof = std::chrono::high_resolution_clock::now();
	//
	//					spaced_groups_total_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();
	//				}
	//			}
	//		}
	//
	//		std::cout << "Add remove Test:" << i << " complete\n";
	//
	//		//reset test data
	//		packed_array_test.clear();
	//		packed_array_test.initalise_group_boundries_v2();
	//		key_group.clear();
	//		keys_added.clear();
	//		keys_to_add.clear();
	//
	//		//setup keys to add
	//		for (int i = 0; i < key_size; i++)
	//		{
	//			keys_to_add.push_back(i);
	//		}
	//
	//	}
	//
	//	std::cout << "spaced group profiling end\n";
	//	std::cout << "spaced ns total:" << spaced_groups_total_duration << ", average :" << spaced_groups_total_duration / (iterations * (long long)sub_itterations) << "ns." << std::endl;
	//
	//	std::cout << "linked list group profiling\n";
	//
	//	srand(seed);
	//
	//	linked_list_test.clear();
	//
	//
	//	for (uint32 i = 0; i < iterations; ++i)
	//	{
	//		for (uint32 j = 0; j < sub_itterations; j++)
	//		{
	//			//pick which test to do
	//			int test_mode = rand() % 100;
	//
	//			if (test_mode < percent_read_tests && keys_added.size() > 0)
	//			{
	//				//pick random group to read
	//				uint32 read_group_of_key = rand() % keys_added.size();
	//
	//				int group = key_group[read_group_of_key];
	//
	//				int node_index = 0;
	//
	//				//get the first entry in group
	//				auto start_prof = std::chrono::high_resolution_clock::now();
	//
	//				int key = linked_list_test.get_first_key_in_group(group, &node_index);
	//
	//				auto end_prof = std::chrono::high_resolution_clock::now();
	//
	//				linked_list_groups_total_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();
	//
	//				while (key != fixed_size_multi_group_linked_list::INVALID_KEY)
	//				{
	//					sum_of_all_reads[linked_list_index] += key;
	//
	//					num_reads[linked_list_index]++;
	//
	//					start_prof = std::chrono::high_resolution_clock::now();
	//
	//					key = linked_list_test.get_next_key(&node_index);
	//
	//					end_prof = std::chrono::high_resolution_clock::now();
	//
	//					linked_list_groups_total_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();
	//				}
	//			}
	//			else
	//			{
	//				//pick random number to add 
	//				int add_remove_index = rand() % key_size;
	//
	//				//check if we are adding or removing
	//				if (add_remove_index < keys_added.size())
	//				{
	//					//remove key
	//					int key = keys_added[add_remove_index];
	//					//remove key from keys to add
	//					keys_added[add_remove_index] = keys_added[keys_added.size() - 1];
	//					key_group[add_remove_index] = key_group[key_group.size() - 1];
	//					keys_added.pop_back();
	//					key_group.pop_back();
	//
	//					keys_to_add.push_back(key);
	//
	//
	//					num_removes[linked_list_index]++;
	//
	//					//add to tracked structure
	//					auto start_prof = std::chrono::high_resolution_clock::now();
	//
	//					linked_list_test.remove(key);
	//
	//					auto end_prof = std::chrono::high_resolution_clock::now();
	//
	//					linked_list_groups_total_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();
	//				}
	//				else
	//				{
	//					//add key
	//					int to_add_index = add_remove_index - keys_added.size();
	//
	//					int key = keys_to_add[to_add_index];
	//
	//					//pick random group to add to 
	//					int group = rand() % group_size;
	//
	//					//remove key from keys to add
	//					keys_to_add[to_add_index] = keys_to_add[keys_to_add.size() - 1];
	//					keys_to_add.pop_back();
	//
	//					//add key to keys added 
	//					keys_added.push_back(key);
	//					key_group.push_back(group);
	//
	//					num_adds[linked_list_index]++;
	//
	//					//add to tracked structure
	//					auto start_prof = std::chrono::high_resolution_clock::now();
	//
	//					linked_list_test.add(key, group);
	//
	//					auto end_prof = std::chrono::high_resolution_clock::now();
	//
	//					linked_list_groups_total_duration += std::chrono::duration_cast<std::chrono::nanoseconds>(end_prof - start_prof).count();
	//				}
	//			}
	//		}
	//
	//		std::cout << "Add remove Test:" << i << " complete\n";
	//
	//		//reset test data
	//		linked_list_test.clear();
	//		key_group.clear();
	//		keys_added.clear();
	//		keys_to_add.clear();
	//
	//		//setup keys to add
	//		for (int i = 0; i < key_size; i++)
	//		{
	//			keys_to_add.push_back(i);
	//		}
	//
	//	}
	//
	//	std::cout << "linked list group  end\n";
	//	std::cout << "linked list ns total:" << linked_list_groups_total_duration << ", average :" << linked_list_groups_total_duration / (iterations * (long long)sub_itterations) << "ns." << std::endl;
	//
	//	auto performance_difference_spaced_to_not = (non_spaced_groups_total_duration - spaced_groups_total_duration);
	//
	//	auto performance_difference_list_to_not = (non_spaced_groups_total_duration - linked_list_groups_total_duration);
	//
	//
	//	auto percent_improvement_spaced_to_not = 100.0f * (((float)performance_difference_spaced_to_not / (float)(non_spaced_groups_total_duration)));
	//
	//	auto percent_improvement_spaced_to_linked = 100.0f * (((float)performance_difference_list_to_not / (float)(non_spaced_groups_total_duration)));
	//
	//
	//	std::cout << "total_time_dif spaced to not:" << (performance_difference_spaced_to_not / (float)(iterations * (long long)sub_itterations)) << ", percent spaced is better than non spaced improvement :" << percent_improvement_spaced_to_not << std::endl;
	//	std::cout << "total_time_dif list to not:" << (performance_difference_list_to_not / (float)(iterations * (long long)sub_itterations)) << ", percent spaced is better than linked list improvement :" << percent_improvement_spaced_to_linked << std::endl;
	//}
#pragma endregion


#pragma region copyPerfTest
	if (run_copy_test)
	{
		int perf_test_key_size = 65535;

		//number of runs
		uint32 itterations = 100000; 


		int perf_test_key_size_b = 100;
		uint32 itterations_b = 100;


		//test the performance cost of coppying data 
		test_copy_performance<test_data<1>>(0, perf_test_key_size, itterations);
		test_copy_performance<test_data<2>>(0, perf_test_key_size, itterations);
		test_copy_performance<test_data<4>>(0, perf_test_key_size, itterations);
		test_copy_performance<test_data<8>>(0, perf_test_key_size , itterations);
		test_copy_performance<test_data<16>>(0, perf_test_key_size, itterations);
		test_copy_performance<test_data<32>>(0, perf_test_key_size, itterations);
		test_copy_performance<test_data<64>>(0, perf_test_key_size, itterations);
		test_copy_performance<test_data<128>>(0, perf_test_key_size, itterations);
		test_copy_performance<test_data<256>>(0, perf_test_key_size, itterations);
		test_copy_performance<test_data<512>>(0, perf_test_key_size, itterations);

		//test_copy_performance<test_data<4000>>(0, perf_test_key_size, itterations);
		//test_copy_performance<test_data<40000>>(0, perf_test_key_size_b, itterations_b);
		//test_copy_performance<test_data<400000>>(0, perf_test_key_size_b, itterations_b);
		//test_copy_performance<test_data<4000000>>(0, perf_test_key_size_b, itterations_b);
		//test_copy_performance<test_data<42000000>>(0, perf_test_key_size_b, itterations_b);
	}
#pragma endregion

#pragma region TemplatePerTestComparison
	if (run_perf_test)
	{
		std::cout << "Running tests with templated data";

		//setup a central value surounded by 2 on either side
		int perf_test_group_size = 256;
		int perf_test_key_size = 65535;
		int percent_of_read_actions = 50;
		int percent_of_adds_forced_into_existing_group = 100;
		uint32 iterations = 1;
		uint32 sub_itterations = perf_test_key_size * 0.25f;


		const int num_of_tests = 11;

		profiling_result prof_result[num_of_tests];

		int perf_index = 0;

		//run template test with only 4 bytes of data tracked per key
		//this should give same performance as non templated code
		//prof_result[perf_index++] = test_template_performance<test_data<4>,		false>	(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<8>,		false>	(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<16>,	false>	(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<32>,	false>	(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<64>,	false>	(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<128>,	false>	(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		prof_result[perf_index++] = test_template_performance<test_data<256>,	false>	(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<512>,	false>	(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<1024>,	false>	(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<2048>,	false>	(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<4096>,	false>  (perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);

		std::cout << "Running tests with memory fence";

		//prof_result[perf_index++] = test_template_performance<test_data<4>,		true>(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<8>,		true>(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<16>,	true>(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<32>,	true>(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<64>,	true>(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<128>,	true>(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		prof_result[perf_index++] = test_template_performance<test_data<256>,	true>(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<512>,	true>(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<1024>,	true>(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<2048>,	true>(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);
		//prof_result[perf_index++] = test_template_performance<test_data<4096>,	true>(perf_test_group_size, perf_test_key_size, iterations, sub_itterations, 0, perf_test_key_size * 0.5f, perf_test_key_size * 0.25f, percent_of_read_actions, percent_of_adds_forced_into_existing_group);


		
		bool show_add = true;
		bool show_remove = true;
		bool show_read = true;
		bool show_group_sizes = true;


		std::cout << "------------ add profile results ----------------" << std::endl << std::endl;

		auto structure_type = (int)(test_candidates::simple_index);
		{
			std::cout << "simple_tightly_packed_structure" << std::endl << std::endl;

			if (show_add)
			{
				std::cout << "average add" << std::endl;
				for (int i = 0; i < perf_index; i++)
				{
					std::cout << prof_result[i].add_average_duration[structure_type] << std::endl;
				}

				if (show_group_sizes)
				{
					std::cout << std::endl;
					std::cout << "average add group size" << std::endl;
					for (int i = 0; i < perf_index; i++)
					{
						std::cout << prof_result[i].add_average_group_size[structure_type] << std::endl;
					}
				}
				std::cout << std::endl << std::endl;
			}

			structure_type = (int)(test_candidates::spaced_index_2_pass);

			std::cout << "group_packed_structure_2_pass_padded" << std::endl << std::endl;

			if (show_add)
			{
				std::cout << "average add" << std::endl;
				for (int i = 0; i < perf_index; i++)
				{
					std::cout << prof_result[i].add_average_duration[structure_type] << std::endl;
				}

				if (show_group_sizes)
				{
					std::cout << std::endl;
					std::cout << "average add group size" << std::endl;
					for (int i = 0; i < perf_index; i++)
					{
						std::cout << prof_result[i].add_average_group_size[structure_type] << std::endl;
					}
				}
				std::cout << std::endl << std::endl;
			}

			structure_type = (int)(test_candidates::linked_list_index);

			std::cout << "linked_list" << std::endl << std::endl;

			if (show_add)
			{
				std::cout << "average add" << std::endl;
				for (int i = 0; i < perf_index; i++)
				{
					std::cout << prof_result[i].add_average_duration[structure_type] << std::endl;
				}

				if (show_group_sizes)
				{
					std::cout << std::endl;
					std::cout << "average add group size" << std::endl;
					for (int i = 0; i < perf_index; i++)
					{
						std::cout << prof_result[i].add_average_group_size[structure_type] << std::endl;
					}
				}
				std::cout << std::endl << std::endl;
			}
			std::cout << std::endl << std::endl;
		}

		std::cout << "------------ remove profile results ----------------" << std::endl << std::endl;
		{
			structure_type = (int)(test_candidates::simple_index);

			std::cout << "simple_tightly_packed_structure" << std::endl << std::endl;

			if (show_remove)
			{
				std::cout << "average remove" << std::endl;
				for (int i = 0; i < perf_index; i++)
				{
					std::cout << prof_result[i].remove_average_duration[structure_type] << std::endl;
				}
				
				if (show_group_sizes)
				{
					std::cout << std::endl;
					std::cout << "average remove group size" << std::endl;
					for (int i = 0; i < perf_index; i++)
					{
						std::cout << prof_result[i].remove_average_group_size[structure_type] << std::endl;
					}
				}
				std::cout << std::endl << std::endl;
			}

			structure_type = (int)(test_candidates::spaced_index_2_pass);

			std::cout << "group_packed_structure_2_pass_padded" << std::endl << std::endl;

			if (show_remove)
			{
				std::cout << "average remove" << std::endl;
				for (int i = 0; i < perf_index; i++)
				{
					std::cout << prof_result[i].remove_average_duration[structure_type] << std::endl;
				}

				if (show_group_sizes)
				{
					std::cout << std::endl;
					std::cout << "average remove group size" << std::endl;
					for (int i = 0; i < perf_index; i++)
					{
						std::cout << prof_result[i].remove_average_group_size[structure_type] << std::endl;
					}
				}
				std::cout << std::endl << std::endl;
			}


			structure_type = (int)(test_candidates::linked_list_index);

			std::cout << "linked_list" << std::endl << std::endl;

			if (show_remove)
			{
				std::cout << "average remove" << std::endl;
				for (int i = 0; i < perf_index; i++)
				{
					std::cout << prof_result[i].remove_average_duration[structure_type] << std::endl;
				}

				if (show_group_sizes)
				{
					std::cout << std::endl;
					std::cout << "average remove group size" << std::endl;
					for (int i = 0; i < perf_index; i++)
					{
						std::cout << prof_result[i].remove_average_group_size[structure_type] << std::endl;
					}
				}
				std::cout << std::endl << std::endl;
			}

		}

		std::cout << "------------ read profile results ----------------" << std::endl << std::endl;
		{
			structure_type = (int)(test_candidates::simple_index);

			std::cout << "simple_tightly_packed_structure" << std::endl << std::endl;

			if (show_read)
			{
				std::cout << "average read" << std::endl;
				for (int i = 0; i < perf_index; i++)
				{
					std::cout << prof_result[i].read_average_duration[structure_type] << std::endl;
				}

				if (show_group_sizes)
				{
					std::cout << std::endl;
					std::cout << "average read group size" << std::endl;
					for (int i = 0; i < perf_index; i++)
					{
						std::cout << prof_result[i].read_average_group_size[structure_type] << std::endl;
					}
				}

				std::cout << std::endl << std::endl;
			}

			structure_type = (int)(test_candidates::spaced_index_2_pass);

			std::cout << "group_packed_structure_2_pass_padded" << std::endl << std::endl;

			if (show_read)
			{
				std::cout << "average read" << std::endl;
				for (int i = 0; i < perf_index; i++)
				{
					std::cout << prof_result[i].read_average_duration[structure_type] << std::endl;
				}

				if (show_group_sizes)
				{
					std::cout << std::endl;
					std::cout << "average read group size" << std::endl;
					for (int i = 0; i < perf_index; i++)
					{
						std::cout << prof_result[i].read_average_group_size[structure_type] << std::endl;
					}
				}

				std::cout << std::endl << std::endl;
			}

			structure_type = (int)(test_candidates::linked_list_index);

			std::cout << "linked_list" << std::endl << std::endl;

			if (show_read)
			{
				std::cout << "average read" << std::endl;
				for (int i = 0; i < perf_index; i++)
				{
					std::cout << prof_result[i].read_average_duration[structure_type] << std::endl;
				}

				if (show_group_sizes)
				{
					std::cout << std::endl;
					std::cout << "average read group size" << std::endl;
					for (int i = 0; i < perf_index; i++)
					{
						std::cout << prof_result[i].read_average_group_size[structure_type] << std::endl;
					}
				}

				std::cout << std::endl << std::endl;
			}
		}

	}
#pragma endregion

}
