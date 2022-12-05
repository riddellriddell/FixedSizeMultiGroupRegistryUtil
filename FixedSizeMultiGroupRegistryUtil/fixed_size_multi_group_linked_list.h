#pragma once
#include <cstdint>
#include <limits>

struct fixed_size_multi_group_linked_list
{

	void init(int group_count, int max_key_count);
	void clean_up();

	void add(int key, int group);
	void remove(int key);

	/// <summary>
	/// resets the datastructure to hold no data
	/// </summary>
	void clear();

	/// <summary>
	/// returns the first key in a group as well as setting the node index
	/// </summary>
	/// <param name="out_node_index"></param>
	/// <returns></returns>
	int get_first_key_in_group(int group, int* out_node_index);

	/// <summary>
	/// use this to loop through the data associated with a group
	/// this updates the link index and returns the index value
	/// the key will be invalid once all the keys have been itterated through
	/// </summary>
	/// <param name="group"></param>
	/// <param name="index"></param>
	/// <returns></returns>
	int get_next_key(int* in_out_node_index);

	/// <summary>
	/// gets the total number of keys added to this tracker
	/// </summary>
	/// <returns></returns>
	int size();

	struct node
	{
		int next_node;
		int last_node;
		int data;
	};

	static constexpr int INVALID_GROUP_START = INT32_MAX;
	static constexpr int INVALID_NODE_INDEX = INT32_MAX;
	static constexpr int INVALID_KEY = INT32_MAX;

	int group_count;
	int* group_start;
	int free_node_start;

	int max_key_count;
	int* group_of_key = 0; //what group a piece of data is in
	int* index_of_key_in_grouped_keys = 0; //where data is located in the grouped array
	node* nodes; //array of all the nodes holding the key data 

	int internal_size;
};

