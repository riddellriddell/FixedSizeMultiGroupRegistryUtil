#pragma once
#pragma once
#include <cstdint>
#include <limits>

template<typename TValue>
struct template_fixed_size_multi_group_tracker
{

private:

	inline int pre_group_end_index(int group) const;
	inline int group_start_index(int group) const;
	inline int group_end_index(int group) const;
	inline int post_group_start_index(int group) const;
	inline int pre_group_space(int group) const;
	inline int post_group_space(int group) const;
	inline int space_in_direction(int group, int direction) const;
	inline int boundry_in_direction(int group, int direction) const;
	inline int boundry_index_in_direction(int group, int direction) const;
	inline int next_index_in_direction(int group, int direction) const;
	inline int last_index_in_direction(int group, int direction) const;
	inline int overlap_in_direction(int group, int direction) const;
	inline int convert_key_to_lookup_index(int key) const;
	inline int is_group_empty(int group) const;
	inline int is_group_wrapped(int group) const;

public:

	void init(int group_count, int max_key_count);
	void initalise_group_boundries();
	void initalise_group_boundries_v2();
	void clean_up();

	void add(int key, TValue* data, int group);
	void remove(int key);

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

	/// <summary>
	/// gets the total number of keys added to this tracker
	/// </summary>
	/// <returns></returns>
	int size();

	int group_count;
	int group_boundry_count;
	int* group_boundries; //the start and end of each boundry
	int* group_size; //the size of each group

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
			data = *_data;
		}

		int key;
		TValue data;
	};

	node* grouped_keys = 0; //packed data organised into contigious groups 
};


template<typename TValue>
inline int template_fixed_size_multi_group_tracker<TValue>::pre_group_end_index(int group) const
{
	return ((group * 2) + (group_boundry_count - 1)) % group_boundry_count;
}


template<typename TValue>
inline int template_fixed_size_multi_group_tracker<TValue>::group_start_index(int group) const
{
	return (group * 2);
}


template<typename TValue>
inline int template_fixed_size_multi_group_tracker<TValue>::group_end_index(int group) const
{
	return (group * 2) + 1;
}


template<typename TValue>
inline int template_fixed_size_multi_group_tracker<TValue>::post_group_start_index(int group) const
{
	return  ((group * 2) + 2) % group_boundry_count;
}


template<typename TValue>
inline int template_fixed_size_multi_group_tracker<TValue>::pre_group_space(int group) const
{
	int pre_index = group_boundries[pre_group_end_index(group)];
	int post_index = group_boundries[group_start_index(group)];
	int wrap_fixed_diff = (post_index + max_key_count) - pre_index;
	int bounded_diff = wrap_fixed_diff % max_key_count;
	return bounded_diff;

}


template<typename TValue>
inline int template_fixed_size_multi_group_tracker<TValue>::post_group_space(int group) const
{
	int pre_index = group_boundries[group_end_index(group)];
	int post_index = group_boundries[post_group_start_index(group)];
	int wrap_fixed_diff = (post_index + max_key_count) - pre_index;
	int bounded_diff = wrap_fixed_diff % max_key_count;
	return bounded_diff;
}


template<typename TValue>
inline int template_fixed_size_multi_group_tracker<TValue>::space_in_direction(int group, int direction) const
{
	int wrap_protect = group_boundry_count + direction;
	int pre = ((group * 2) + wrap_protect) % group_boundry_count;
	int post = ((group * 2) + 1 + wrap_protect) % group_boundry_count;

	int pre_index = group_boundries[pre];

	int post_index = group_boundries[post];

	int is_wrap = (post_index < pre_index) < direction;

	return ((max_key_count * is_wrap) + post_index) - pre_index;
}


template<typename TValue>
inline int template_fixed_size_multi_group_tracker<TValue>::boundry_in_direction(int group, int direction) const
{
	return  (group * 2) + (direction > 0);
}


template<typename TValue>
inline int template_fixed_size_multi_group_tracker<TValue>::boundry_index_in_direction(int group, int direction) const
{
	return  group_boundries[boundry_in_direction(group, direction)];
}


template<typename TValue>
inline int template_fixed_size_multi_group_tracker<TValue>::next_index_in_direction(int group, int direction) const
{
	return (group_boundries[boundry_in_direction(group, direction)] + (max_key_count + direction)) % max_key_count;
}


template<typename TValue>
inline int template_fixed_size_multi_group_tracker<TValue>::last_index_in_direction(int group, int direction) const
{
	return (group_boundries[boundry_in_direction(group, direction)] + (max_key_count - direction)) % max_key_count;
}


template<typename TValue>
inline int template_fixed_size_multi_group_tracker<TValue>::overlap_in_direction(int group, int direction) const
{
	int group_index = boundry_in_direction(group, direction);
	int next_group_index = (group_index + group_boundry_count + direction) % group_boundry_count;
	int group_boundry_index = group_boundries[group_index];
	int next_group_boundry_index = group_boundries[next_group_index];

	return group_boundry_index == next_group_boundry_index;

	//return  next_group_boundry_index == ((group_boundry_index + max_key_count + direction) % max_key_count);
}


template<typename TValue>
inline int template_fixed_size_multi_group_tracker<TValue>::convert_key_to_lookup_index(int key) const
{
	//TODO:: convert this to a templated function for conversion
	return key;
}


template<typename TValue>
inline int template_fixed_size_multi_group_tracker<TValue>::is_group_empty(int group) const
{
	int group_start = group * 2;
	int group_end = group_start + 1;
	return (group_boundries[group_end] + 1) == group_boundries[group_start] || (group_boundries[group_end] + (max_key_count - 1)) == group_boundries[group_start];
}


template<typename TValue>
inline int template_fixed_size_multi_group_tracker<TValue>::is_group_wrapped(int group) const
{
	int group_start = group * 2;
	int group_end = group_start + 1;

	return group_boundries[group_end] < group_boundries[group_start];
}


template<typename TValue>
void template_fixed_size_multi_group_tracker<TValue>::init(int _group_count, int _max_key_count)
{
	group_count = _group_count;
	group_boundry_count = group_count * 2;

	group_boundries = new int[(group_count * 2)];
	group_size = new int[group_count];

	max_key_count = _max_key_count;
	group_of_key = new int[max_key_count];
	index_of_key_in_grouped_keys = new int[max_key_count];
	grouped_keys = new node[max_key_count];

	initalise_group_boundries();
}


template<typename TValue>
void template_fixed_size_multi_group_tracker<TValue>::initalise_group_boundries()
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


template<typename TValue>
void template_fixed_size_multi_group_tracker<TValue>::initalise_group_boundries_v2()
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


template<typename TValue>
void template_fixed_size_multi_group_tracker<TValue>::clean_up()
{
	delete[] group_boundries;
	delete[] group_size;
	delete[] group_of_key;
	delete[] index_of_key_in_grouped_keys;
	delete[] grouped_keys;
}


template<typename TValue>
void template_fixed_size_multi_group_tracker<TValue>::add(int key, TValue* data, int group)
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

	//store what group the data is in 
	group_of_key[lookup_index] = group;
	index_of_key_in_grouped_keys[lookup_index] = index_to_add;

	node value_in_limbo = node(key,data);

	//get the existing item in that direction
	node exising_value = grouped_keys[index_to_add];

	//insert the new key
	grouped_keys[index_to_add] = value_in_limbo;

	//add to the size of the group
	group_boundries[boundry_in_direction(group, direction)] = index_to_add;

	value_in_limbo = exising_value;


	int address_in_use = overlap_in_direction(group, direction);

	//check if add wil require shuffel
	//dont loop back around on the original group
	for (int i = 1; (i < group_count) && address_in_use; i++)
	{
		//move to next group
		group = (group + group_count + direction) % group_count;

		//get the end index in the direction for the group
		int group_end_index = group_boundries[boundry_in_direction(group, direction)];

		//if the end of a group is directly before the start then the group is empty 
		//we know that index_to_add  is at the start of the group so we just need to check if 
		//the end of the group is one less than the index_to_add  which we already calculated in 
		//the previous loop in the line above
		int group_has_size = group_end_index != last_group_end_index;

		//store the last group for use in the next loop
		last_group_end_index = group_end_index;

		//add "movement direction" to both start and end of next group
		group_boundries[group * 2] = (group_boundries[group * 2] + max_key_count + direction) % max_key_count;
		group_boundries[(group * 2) + 1] = (group_boundries[(group * 2) + 1] + max_key_count + direction) % max_key_count;

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

		//check if this group is overlapping the next groupq
		address_in_use = overlap_in_direction(group, direction);
	}
}


template<typename TValue>
void template_fixed_size_multi_group_tracker<TValue>::remove(int key)
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


template<typename TValue>
void template_fixed_size_multi_group_tracker<TValue>::clear()
{
	initalise_group_boundries();

	for (int i = 0; i < max_key_count; i++)
	{
		group_of_key[i] = group_count;
		index_of_key_in_grouped_keys[i] = max_key_count;

		TValue data = TValue(0);

		grouped_keys[i] = node(max_key_count, &data);
	}
}


template<typename TValue>
int template_fixed_size_multi_group_tracker<TValue>::get_key_at_group_index(int group, int index)
{
	//get the start index for the group
	int start_index = group_boundries[group * 2];

	//wrap indexeing around for overflows 
	int key_lookup_index = (start_index + index) % max_key_count;

	//return the data at the index
	return grouped_keys[key_lookup_index].key;
}


template<typename TValue>
int template_fixed_size_multi_group_tracker<TValue>::size()
{
	int out_size = 0;
	for (int i = 0; i < group_count; i++)
	{
		out_size += group_size[i];
	}

	return out_size;
}