#pragma once

#include <cstdint>
#include <limits>

template<typename TValue>
struct template_fixed_size_multi_group_linked_list
{

	void init(int group_count, int max_key_count);
	void clean_up();

	void add(int group, int key, TValue* data);
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
	void get_first_value_in_group(int group, int* out_node_index, TValue** out_data);

	/// <summary>
	/// use this to loop through the data associated with a group
	/// this updates the link index and returns the index value
	/// the key will be invalid once all the keys have been itterated through
	/// </summary>
	/// <param name="group"></param>
	/// <param name="index"></param>
	/// <returns></returns>
	void get_next_value(int* in_out_node_index, TValue** out_data);

	bool is_end_of_group(int index, int group) const;

	/// <summary>
	/// number of keys in a group
	/// </summary>
	/// <param name="group"></param>
	/// <returns></returns>
	int group_size(int group) const;

	/// <summary>
	/// gets the total number of keys added to this tracker
	/// </summary>
	/// <returns></returns>
	int size();

	struct node
	{
		int next_node;
		int last_node;
		TValue data;
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

template<typename TValue>
void template_fixed_size_multi_group_linked_list<TValue>::init(int _group_count, int _max_key_count)
{
	group_count = _group_count;
	group_start = new int[_group_count];

	//correctly initalize the group start list
	for (int i = 0; i < _group_count; i++)
	{
		//reserve the first nodes for the group
		group_start[i] = i;
	}

	//the fist nodes are used for the groups
	free_node_start = _group_count;

	max_key_count = _max_key_count;
	group_of_key = new int[_max_key_count]; //what group a piece of data is in
	index_of_key_in_grouped_keys = new int[_max_key_count]; //where data is located in the grouped array

	nodes = new node[_max_key_count + _group_count]; //array of all the nodes holding the key data 

	//link up all the nodes
	for (int i = 0; i < (_max_key_count + _group_count); i++)
	{
		nodes[i].data = TValue(0);
		nodes[i].next_node = i + 1;
		nodes[i].last_node = i - 1;

		//init the nodes so all the group nodes have no links and the first and last node link to nothing
		if (i == (_max_key_count + _group_count) - 1)
		{
			nodes[i].next_node = INVALID_NODE_INDEX;
		}

		if (i == _group_count)
		{
			nodes[i].last_node = INVALID_NODE_INDEX;
		}

		//circularly loop the group node
		//this simplifies other code later
		if (i < _group_count)
		{
			nodes[i].last_node = i;
			nodes[i].next_node = i;
		}
	}

	internal_size = 0;
}

template<typename TValue>
void template_fixed_size_multi_group_linked_list<TValue>::clean_up()
{
	delete(group_start);
	delete(group_of_key);
	delete(index_of_key_in_grouped_keys);
	delete(nodes);
}

template<typename TValue>
void template_fixed_size_multi_group_linked_list<TValue>::add(int group, int key, TValue* _data)
{
	int node_index = free_node_start;

	//get a node off the free list
	node* new_node = &nodes[free_node_start];

	//update the free node head
	free_node_start = new_node->next_node;

	//get the group
	node* group_node = &nodes[group_start[group]];

	//add the node into group
	node* last_group_key_node = &nodes[group_node->next_node];
	last_group_key_node->last_node = node_index;

	new_node->next_node = group_node->next_node;
	new_node->last_node = group_start[group];
	new_node->data =  *_data;
	group_node->next_node = node_index;

	//update key maps
	index_of_key_in_grouped_keys[key] = node_index;
	group_of_key[key] = group;

	internal_size++;
}

template<typename TValue>
void template_fixed_size_multi_group_linked_list<TValue>::remove(int key)
{
	int node_index = index_of_key_in_grouped_keys[key];

	//get node 
	node* key_node = &nodes[node_index];
	node* next_node = &nodes[key_node->next_node];
	node* last_node = &nodes[key_node->last_node];

	//get next and last node
	next_node->last_node = key_node->last_node;
	last_node->next_node = key_node->next_node;

	//add node to free node
	key_node->next_node = free_node_start;

	free_node_start = node_index;

	internal_size--;
}

template<typename TValue>
void template_fixed_size_multi_group_linked_list<TValue>::clear()
{
	//correctly initalize the group start list
	//link up all the nodes
	for (int i = 0; i < max_key_count + group_count; i++)
	{
		nodes[i].next_node = i + 1;
		nodes[i].last_node = i - 1;

		//init the nodes so all the group nodes have no links and the first and last node link to nothing
		if (i == (max_key_count + group_count) - 1)
		{
			nodes[i].next_node = INVALID_NODE_INDEX;
		}

		if (i == group_count)
		{
			nodes[i].last_node = INVALID_NODE_INDEX;
		}

		//circularly loop the group node
		//this simplifies other code later
		if (i < group_count)
		{
			nodes[i].last_node = i;
			nodes[i].next_node = i;
		}
	}

	free_node_start = group_count;

	internal_size = 0;
}

template<typename TValue>
void template_fixed_size_multi_group_linked_list<TValue>::get_first_value_in_group(int group, int* out_node_index, TValue** out_data)
{
	*out_node_index = nodes[group_start[group]].next_node;

	*out_data = &nodes[*out_node_index].data;

}

template<typename TValue>
void template_fixed_size_multi_group_linked_list<TValue>::get_next_value(int* in_out_node_index, TValue** out_data)
{
	*in_out_node_index = nodes[*in_out_node_index].next_node;

	*out_data = &nodes[*in_out_node_index].data;
}

template<typename TValue>
bool template_fixed_size_multi_group_linked_list<TValue>::is_end_of_group(int index, int group) const
{
	return index == group_start[group];
}

template<typename TValue>
inline int template_fixed_size_multi_group_linked_list<TValue>::group_size(int group) const
{
	int output = 0;

	auto node_index = nodes[group_start[group]].next_node;

	while (!is_end_of_group(node_index, group))
	{
		output++;

		node_index = nodes[node_index].next_node;
	}

	return output;
}

template<typename TValue>
int template_fixed_size_multi_group_linked_list<TValue>::size()
{
	return internal_size;
}

