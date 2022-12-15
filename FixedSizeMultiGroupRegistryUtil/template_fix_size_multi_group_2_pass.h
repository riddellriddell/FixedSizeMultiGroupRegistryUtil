#pragma once

#include <malloc.h>

template<typename TKey, typename TValue>
struct template_fixed_size_multi_group_2_pass
{

private:

	//does the end of this group have the least space and should the last elemeny be the one shuffeled to fill the hole
	inline int pre_group_end_index(int group) const;
	inline int group_start_index(int group) const;
	inline int group_end_index(int group) const;
	inline int post_group_start_index(int group) const;
	inline int boundry_in_direction(int group, int direction) const;
	inline int space_in_direction(int group, int direction) const;
	inline bool should_remove_from_group_max(int group) const; 

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
	TKey get_key_at_group_index(int group, int index);

	/// <summary>
/// use this to loop through the data associated with a group
/// </summary>
/// <param name="group"></param>
/// <param name="index"></param>
/// <returns></returns>
	TValue* get_value_at_group_index(int group, int index);


	/// <summary>
/// gets the total number of keys added to this tracker
/// </summary>
/// <returns></returns>
	int size();

	void remove(TKey key);

	void add(int group, TKey key, TValue* in_value);

	int pre_group_space(int group) const;

	int post_group_space(int group) const;

	int overlap_in_direction(int group, int direction) const;

	inline int convert_key_to_lookup_index(TKey key) const;

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

		node(TKey _key, TValue* _data)
		{
			key = _key;
			data = *_data;
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
	group_count = _max_group_count;
	group_boundry_count = group_count * 2;

	group_boundries = new int[(group_boundry_count)];
	group_size = new int[group_count];

	max_key_count = _max_key_count;
	group_of_key = new int[max_key_count];
	temp_remap_array = new int[group_count];
	index_of_key_in_grouped_keys = new int[max_key_count];
	grouped_keys = new node[max_key_count];


	initalise_group_boundries();
}

template<typename TKey, typename TValue>
inline void template_fixed_size_multi_group_2_pass<TKey, TValue>::initalise_group_boundries()
{
	//space out the start and end points to create as much space between entries so as not to have to reshuffle the arrays much
	int stride = max_key_count / (group_count + 1);

	stride = stride ? stride : 1;


	int sub_add = max_key_count - 1;

	//some temp code to help with debugging
	//this will stack up all group start and end points in the center of the array
	stride = 0;
	int middle_point = max_key_count / 2;
	int middle_group = group_count / 2;
	int centered_group_start = middle_point - (stride * middle_group);

	//setup start and end of groups 
	for (int i = 0; i < group_count; i++)
	{
		group_size[i] = 0;
		group_boundries[group_start_index(i)] = (centered_group_start + (stride * i)) % max_key_count;
		group_boundries[group_end_index(i)] = ((centered_group_start + (stride * i)) + sub_add) % max_key_count;
	}
}

template<typename TKey, typename TValue>
inline void template_fixed_size_multi_group_2_pass<TKey, TValue>::initalise_group_boundries_v2()
{
	//space out the start and end points to create as much space between entries so as not to have to reshuffle the arrays much
	int stride = max_key_count / (group_count + 1);

	stride = stride ? stride : 1;


	int sub_add = max_key_count - 1;

	int middle_point = max_key_count / 2;
	int middle_group = group_count / 2;
	int centered_group_start = middle_point - (stride * middle_group);

	//setup start and end of groups 
	for (int i = 0; i < group_count; i++)
	{
		group_size[i] = 0;
		group_boundries[group_start_index(i)] = (stride * i) % max_key_count;
		group_boundries[group_end_index(i)] = ((stride * i) + sub_add) % max_key_count;
	}
}

template<typename TKey, typename TValue>
inline void template_fixed_size_multi_group_2_pass<TKey, TValue>::clean_up()
{
	delete[] group_boundries;
	delete[] group_size;
	delete[] group_of_key;
	delete[] index_of_key_in_grouped_keys;
	delete[] grouped_keys;
}

template<typename TKey, typename TValue>
inline void template_fixed_size_multi_group_2_pass<TKey, TValue>::clear()
{
	initalise_group_boundries();

	for (int i = 0; i < max_key_count; i++)
	{
		group_of_key[i] = group_count;
		index_of_key_in_grouped_keys[i] = max_key_count;
	}
}

template<typename TKey, typename TValue>
inline TKey template_fixed_size_multi_group_2_pass<TKey, TValue>::get_key_at_group_index(int group, int index)
{
	//get the start index for the group
	int start_index = group_boundries[group * 2];

	//wrap indexeing around for overflows 
	int key_lookup_index = (start_index + index) % max_key_count;

	//return the data at the index
	return grouped_keys[key_lookup_index].key;
}

template<typename TKey, typename TValue>
inline TValue* template_fixed_size_multi_group_2_pass<TKey, TValue>::get_value_at_group_index(int group, int index)
{
	//get the start index for the group
	int start_index = group_boundries[group * 2];

	//wrap indexeing around for overflows 
	int key_lookup_index = (start_index + index) % max_key_count;

	//return the data at the index
	return &grouped_keys[key_lookup_index].data;
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

	//boundry index in direction
	int boundry_index = boundry_in_direction(group, direction);

	//get the next index for a group in a direction
	int last_group_end_index = group_boundries[boundry_index];
	int index_to_add = (last_group_end_index + (max_key_count + direction)) % max_key_count;

	//get the meta data lookup value for the key
	int lookup_index = convert_key_to_lookup_index(key);

	//expand the group boundries to include new item
	group_boundries[boundry_index] = index_to_add;

	int number_of_items_to_remap = 0;

	//store the write address for the first element
	temp_remap_array[number_of_items_to_remap] = index_to_add;

	number_of_items_to_remap++;

	//set key group as the group value gets changed later
	group_of_key[lookup_index] = group;

	//check if overlapping in direction
	int address_in_use = overlap_in_direction(group, direction);

	//check if add wil require shuffel
	//dont loop back around on the original group
	for (int i = 1; (i < group_count) && address_in_use; i++)
	{
		//move to next group
		group = (group + group_count + direction) % group_count;

		//add "movement direction" to both start and end of group
		group_boundries[group * 2] = (group_boundries[group * 2] + max_key_count + direction) % max_key_count;
		group_boundries[(group * 2) + 1] = (group_boundries[(group * 2) + 1] + max_key_count + direction) % max_key_count;

		//get next boundry
		boundry_index = boundry_in_direction(group, direction);

		//get write to index
		//get the next index for a group in a direction
		index_to_add = group_boundries[boundry_index];

		//if the end of a group is directly before the start then the group is empty 
		//we know that index_to_add  is at the start of the group so we just need to check if 
		//the end of the group is one less than the index_to_add  which we already calculated in 
		//the previous loop in the line above
		int group_has_size = temp_remap_array[number_of_items_to_remap -1] != index_to_add;

		//add write too address to remap
		temp_remap_array[number_of_items_to_remap] = index_to_add;

		//increase number of items to remap but only if the group has size
		number_of_items_to_remap += group_has_size;

		//check if the current group overlaps the next group
		address_in_use = overlap_in_direction(group, direction);
	}

	for (int i = number_of_items_to_remap -1; i > 0; i--)
	{
		node* node_to_copy = &grouped_keys[temp_remap_array[i -1]];

		//update the placed keys index
		int remap_lookup_index = convert_key_to_lookup_index(node_to_copy->key);

		//update the keys index in grouped keys
		index_of_key_in_grouped_keys[remap_lookup_index] = temp_remap_array[i];

		//copy to new location
		grouped_keys[temp_remap_array[i]] = *node_to_copy;
	}

	//update the keys index in grouped keys
	index_of_key_in_grouped_keys[lookup_index] = temp_remap_array[0];

	//copy data into location
	grouped_keys[temp_remap_array[0]] = node(key, in_value);

}

template<typename TKey, typename TValue>
inline int template_fixed_size_multi_group_2_pass<TKey, TValue>::pre_group_end_index(int group) const
{
	return ((group * 2) + (group_boundry_count - 1)) % group_boundry_count;
}

template<typename TKey, typename TValue>
inline int template_fixed_size_multi_group_2_pass<TKey, TValue>::group_start_index(int group) const
{
	return (group * 2);
}

template<typename TKey, typename TValue>
inline int template_fixed_size_multi_group_2_pass<TKey, TValue>::group_end_index(int group) const
{
	return (group * 2) + 1;
}

template<typename TKey, typename TValue>
inline int template_fixed_size_multi_group_2_pass<TKey, TValue>::post_group_start_index(int group) const
{
	return  ((group * 2) + 2) % group_boundry_count;
}

template<typename TKey, typename TValue>
inline int template_fixed_size_multi_group_2_pass<TKey, TValue>::boundry_in_direction(int group, int direction) const
{
	return  (group * 2) + (direction > 0);
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


template<typename TKey, typename TValue>
inline int template_fixed_size_multi_group_2_pass<TKey, TValue>::size()
{
	int out_size = 0;
	for (int i = 0; i < group_count; i++)
	{
		out_size += group_size[i];
	}

	return out_size;
}