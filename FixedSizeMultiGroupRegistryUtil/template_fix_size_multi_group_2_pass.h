#pragma once

#include <malloc.h>

template<typename TKey, typename TValue>
struct template_fixed_size_multi_group_2_pass
{

private:

	//does the end of this group have the least space and should the last elemeny be the one shuffeled to fill the hole
	inline int space_in_direction(int group, int direction) const;
	inline bool should_remove_from_group_max(int group) const; 
	inline int convert_key_to_lookup_index(TKey key) const;

	void shuffle_data(int num_of_changes, int* in_remap_array);

public:


	void init(int max_group_count, int max_key_count);
	void initalise_group_boundries();
	void initalise_group_boundries_v2();
	void clean_up();

	/// <summary>
	/// resets the datastructure to hold no data
	/// </summary>
	void clear();

	/// <summary>
	/// use this to loop through the data associated with a group
	/// </summary>
	/// <param name="group"></param>
	/// <param name="index"></param>
	/// <returns></returns>
	int get_key_at_group_index(int group, int index);


	void remove(TKey key);

	void add(int group, TKey key, TValue* in_value);

	int pre_group_space(int group) const;

	int post_group_space(int group) const;

	int overlap_in_direction(int group, int direction) const;

	int max_group_count; //maximum number of different groups
	int group_count;
	int group_boundry_count;
	int* group_boundries; //the start and end of each boundry
	int* group_size; //the size of each group

	int* temp_remap_array;

	int max_key_count;
	int* group_of_key = 0; //what group a piece of data is in
	int* index_of_key_in_grouped_keys = 0; //where data is located in the grouped array

	struct node
	{
		node()
		{

		}

		node(int _key, TValue* _data)
		{
			key = _key;
			data = _data;
		}

		TKey key;
		TValue data;
	};

	node* grouped_keys = 0; //packed data organised into contigious groups 
};

template<typename TKey, typename TValue>
inline int template_fixed_size_multi_group_2_pass<TKey, TValue>::space_in_direction(int group, int direction) const
{
	int wrap_protect = group_boundry_count + direction;
	int pre = ((group * 2) + wrap_protect) % group_boundry_count;
	int post = ((group * 2) + 1 + wrap_protect) % group_boundry_count;

	int pre_index = group_boundries[pre];

	int post_index = group_boundries[post];

	int is_wrap = (post_index < pre_index) < direction;

	return ((max_key_count * is_wrap) + post_index) - pre_index;
}

template<typename TKey, typename TValue>
inline bool template_fixed_size_multi_group_2_pass<TKey, TValue>::should_remove_from_group_max(int group) const
{
	return space_in_direction(group, 1) < space_in_direction(group, -1);
}

template<typename TKey, typename TValue>
inline int template_fixed_size_multi_group_2_pass<TKey, TValue>::convert_key_to_lookup_index(TKey key) const
{
	//TODO:: convert this to a templated function for conversion
	return key;
}

template<typename TKey, typename TValue>
inline void template_fixed_size_multi_group_2_pass<TKey, TValue>::init(int _max_group_count, int _max_key_count)
{
	max_group_count = _max_group_count;
	group_count = 0;
	group_boundry_count = 0;

	group_boundries = new int[(max_group_count * 2)];
	group_size = new int[max_group_count];

	max_key_count = _max_key_count;
	group_of_key = new int[max_key_count];
	temp_remap_array = new int[max_key_count];
	index_of_key_in_grouped_keys = new int[max_key_count];
	grouped_keys = new node[max_key_count];


	initalise_group_boundries();
}

template<typename TKey, typename TValue>
inline void template_fixed_size_multi_group_2_pass<TKey, TValue>::remove(TKey key)
{
	int key_lookup_index = convert_key_to_lookup_index(key);

	//lookup key address
	int key_address = index_of_key_in_grouped_keys[key_lookup_index];
	int key_group = group_of_key[key_lookup_index];

	//update the group size 
	group_size[key_group]--;

	//get the direction to reduce the group by 
	int direction = ((post_group_space(key_group) <= pre_group_space(key_group)) * 2) - 1;

	int boundry_index = boundry_in_direction(key_group, direction);

	//index of item to move 
	int move_from = group_boundries[boundry_index];

	//move end of group into empty space
	grouped_keys[key_address] = grouped_keys[move_from];

	int replacement_key_lookup = convert_key_to_lookup_index(grouped_keys[key_address].key);

	//update moved items index
	index_of_key_in_grouped_keys[replacement_key_lookup] = key_address;

	//reduce group size 
	group_boundries[boundry_index] = (group_boundries[boundry_index] + max_key_count - direction) % max_key_count;
}

template<typename TKey, typename TValue>
inline void template_fixed_size_multi_group_2_pass<TKey, TValue>::add(int group, TKey key, TValue* in_value)
{
	//update the group size 
	group_size[group]++;

	int pre_space = pre_group_space(group);
	int post_space = post_group_space(group);

	//pick side of group to add item on
	int direction = ((post_space >= pre_space) * 2) - 1;

	//get the next index for a group in a direction
	int last_group_end_index = group_boundries[boundry_in_direction(group, direction)];
	int index_to_add = (last_group_end_index + (max_key_count + direction)) % max_key_count;

	int lookup_index = convert_key_to_lookup_index(key);

	int number_of_items_to_remap = 0;

	//store what group the data is in 
	group_of_key[lookup_index] = group;
	index_of_key_in_grouped_keys[lookup_index] = index_to_add;

	//get the existing item in that direction
	node exising_value = grouped_keys[index_to_add];

	//add to the size of the group
	group_boundries[boundry_in_direction(group, direction)] = index_to_add;

	value_in_limbo = exising_value;

	//move to next group
	group = (group + group_count + direction) % group_count;

	int address_in_use = overlap_in_direction(group, direction);

	//check if add wil require shuffel
	//dont loop back around on the original group
	for (int i = 1; (i < group_count); i++)
	{
		//get the end index in the direction for the group
		temp_remap_array[number_of_items_to_remap] = group_boundries[boundry_in_direction(group, direction)];

		//check if this group is overlapping the next group
		if (overlap_in_direction(group, direction) == 0)
		{
			//no overlap so safe to end remap process
			break;
		}

		number_of_items_to_remap++;

		//add "movement direction" to both start and end of next group
		group_boundries[group * 2] = (group_boundries[group * 2] + max_key_count + direction) % max_key_count;
		group_boundries[(group * 2) + 1] = (group_boundries[(group * 2) + 1] + max_key_count + direction) % max_key_count;

		//move to next group
		group = (group + group_count + direction) % group_count;

		//if the held item is less than the end of the group swap it for the item one more than the group end 
		index_to_add = (group_end_index + max_key_count + direction) % max_key_count;

		node key_to_place = group_has_size ? value_in_limbo : grouped_keys[index_to_add];

		exising_value = grouped_keys[index_to_add];

		//insert the new key
		grouped_keys[index_to_add] = key_to_place;

		//update the placed keys index
		lookup_index = convert_key_to_lookup_index(key_to_place.key);

		//update the keys index in grouped keys
		index_of_key_in_grouped_keys[lookup_index] = index_to_add;

		//update the key in limbo
		value_in_limbo = group_has_size ? exising_value : value_in_limbo;


	}

	for (int i = number_of_items_to_remap; i > 0; i--)
	{
		node* node_to_copy = grouped_keys[temp_remap_array[i]];
	}

	//insert the new key
	grouped_keys[index_to_add] = node(key, data);

}


template<typename TKey, typename TValue>
inline int template_fixed_size_multi_group_2_pass<TKey, TValue>::pre_group_space(int group) const
{
	int pre_index = group_boundries[pre_group_end_index(group)];
	int post_index = group_boundries[group_start_index(group)];
	int wrap_fixed_diff = (post_index + max_key_count) - pre_index;
	int bounded_diff = wrap_fixed_diff % max_key_count;
	return bounded_diff;

}


template<typename TKey, typename TValue>
inline int template_fixed_size_multi_group_2_pass<TKey, TValue>::post_group_space(int group) const
{
	int pre_index = group_boundries[group_end_index(group)];
	int post_index = group_boundries[post_group_start_index(group)];
	int wrap_fixed_diff = (post_index + max_key_count) - pre_index;
	int bounded_diff = wrap_fixed_diff % max_key_count;
	return bounded_diff;
}

template<typename TKey, typename TValue>
inline int template_fixed_size_multi_group_2_pass<TKey, TValue>::overlap_in_direction(int group, int direction) const
{
	int group_index = boundry_in_direction(group, direction);
	int next_group_index = (group_index + group_boundry_count + direction) % group_boundry_count;
	int group_boundry_index = group_boundries[group_index];
	int next_group_boundry_index = group_boundries[next_group_index];

	return group_boundry_index == next_group_boundry_index;

	//return  next_group_boundry_index == ((group_boundry_index + max_key_count + direction) % max_key_count);
}