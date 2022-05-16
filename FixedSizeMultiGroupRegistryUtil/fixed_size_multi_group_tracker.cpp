#include "fixed_size_multi_group_tracker.h"

inline int fixed_size_multi_group_tracker::pre_group_end_index(int group) const
{
	return ((group * 2) + (group_boundry_count - 1)) % group_boundry_count;
}

inline int fixed_size_multi_group_tracker::group_start_index(int group) const
{
	return (group * 2);
}

inline int fixed_size_multi_group_tracker::group_end_index(int group) const
{
	return (group * 2) + 1;
}

inline int fixed_size_multi_group_tracker::post_group_start_index(int group) const
{
	return  ((group * 2) + 2) % group_boundry_count;
}

inline int fixed_size_multi_group_tracker::pre_group_space(int group) const
{
	int pre_index = group_boundries[pre_group_end_index(group)];
	int post_index = group_boundries[group_start_index(group)];
	int wrap_fixed_diff = (post_index + max_key_count) - pre_index;
	int bounded_diff = wrap_fixed_diff % max_key_count;
	return bounded_diff;

}

inline int fixed_size_multi_group_tracker::post_group_space(int group) const
{
	int pre_index = group_boundries[group_end_index(group)];
	int post_index = group_boundries[post_group_start_index(group)];
	int wrap_fixed_diff = (post_index + max_key_count) - pre_index;
	int bounded_diff = wrap_fixed_diff % max_key_count;
	return bounded_diff;
}

inline int fixed_size_multi_group_tracker::space_in_direction(int group, int direction) const
{
	int wrap_protect = group_boundry_count + direction;
	int pre = ((group * 2) + wrap_protect) % group_boundry_count;
	int post = ((group * 2) + 1 + wrap_protect) % group_boundry_count;

	int pre_index = group_boundries[pre];

	int post_index = group_boundries[post];

	int is_wrap = (post_index < pre_index) < direction;

	return ((max_key_count * is_wrap) + post_index) - pre_index;
}

inline int fixed_size_multi_group_tracker::boundry_in_direction(int group, int direction) const
{
	return  (group * 2) + (direction > 0);
}

inline int fixed_size_multi_group_tracker::boundry_index_in_direction(int group, int direction) const
{
	return  group_boundries[boundry_in_direction(group, direction)];
}

inline int fixed_size_multi_group_tracker::next_index_in_direction(int group, int direction) const
{
	return (group_boundries[boundry_in_direction(group,direction)] + (max_key_count + direction)) % max_key_count;
}

inline int fixed_size_multi_group_tracker::last_index_in_direction(int group, int direction) const
{
	return (group_boundries[boundry_in_direction(group, direction)] + (max_key_count - direction)) % max_key_count;
}

inline int fixed_size_multi_group_tracker::overlap_in_direction(int group, int direction) const
{
	int group_index = boundry_in_direction(group, direction);
	int next_group_index = (group_index + group_boundry_count + direction) % group_boundry_count;
	int group_boundry_index = group_boundries[group_index];
	int next_group_boundry_index = group_boundries[next_group_index];

	return group_boundry_index == next_group_boundry_index;

	//return  next_group_boundry_index == ((group_boundry_index + max_key_count + direction) % max_key_count);
}

inline int fixed_size_multi_group_tracker::convert_key_to_lookup_index(int key) const
{
	//TODO:: convert this to a templated function for conversion
	return key;
}

inline int fixed_size_multi_group_tracker::is_group_empty(int group) const
{
	int group_start = group * 2;
	int group_end = group_start + 1;
	return (group_boundries[group_end] + 1) == group_boundries[group_start]  || (group_boundries[group_end] + (max_key_count -1)) == group_boundries[group_start];
}

inline int fixed_size_multi_group_tracker::is_group_wrapped(int group) const
{
	int group_start = group * 2;
	int group_end = group_start + 1;
	
	return group_boundries[group_end] < group_boundries[group_start];
}

void fixed_size_multi_group_tracker::init(int _group_count, int _max_key_count)
{
	group_count = _group_count;
	group_boundry_count = group_count * 2;

	group_boundries = new int[(group_count * 2)];
	group_size = new int[group_count];

	max_key_count = _max_key_count;
	group_of_key = new int[max_key_count];
	index_of_key_in_grouped_keys = new int[max_key_count];
	grouped_keys = new int[max_key_count];
	
	initalise_group_boundries();
}

void fixed_size_multi_group_tracker::initalise_group_boundries()
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

void fixed_size_multi_group_tracker::initalise_group_boundries_v2()
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

void fixed_size_multi_group_tracker::clean_up()
{
	delete[] group_boundries;
	delete[] group_size;
	delete[] group_of_key;
	delete[] index_of_key_in_grouped_keys;
	delete[] grouped_keys;
}

void fixed_size_multi_group_tracker::add(int key, int group)
{
	//update the group size 
	group_size[group]++;

	int pre_space = pre_group_space(group);
	int post_space = post_group_space(group);

	//pick side of group to add item on
	int direction = ((post_space >= pre_space) * 2) -1;

	//get the next index for a group in a direction
	int last_group_end_index = group_boundries[boundry_in_direction(group, direction)];
	int index_to_add = (last_group_end_index + (max_key_count + direction)) % max_key_count;

	int lookup_index = convert_key_to_lookup_index(key);

	//store what group the data is in 
	group_of_key[lookup_index] = group; 
	index_of_key_in_grouped_keys[lookup_index] = index_to_add;

	int key_in_limbo = key;

	//get the existing item in that direction
	int exising_key = grouped_keys[index_to_add];

	//insert the new key
	grouped_keys[index_to_add] = key_in_limbo;

	//add to the size of the group
	group_boundries[boundry_in_direction(group, direction)] = index_to_add;

	key_in_limbo = exising_key;


	int address_in_use = overlap_in_direction(group, direction);

	//check if add wil require shuffel
	//dont loop back around on the original group
	for (int i = 1; (i < group_count) && address_in_use; i++)
	{
		//move to next group
		group = (group + group_count + direction) % group_count;

		//get the end index in the direction for the group
		int group_end_index = group_boundries[ boundry_in_direction(group, direction)];

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

		int key_to_place = group_has_size ? key_in_limbo : grouped_keys[index_to_add];

		exising_key = grouped_keys[index_to_add];

		//insert the new key
		grouped_keys[index_to_add] = key_to_place;

		//update the placed keys index
		lookup_index = convert_key_to_lookup_index(key_to_place);

		//update the keys index in grouped keys
		index_of_key_in_grouped_keys[lookup_index] = index_to_add;

		//update the key in limbo
		key_in_limbo = group_has_size ? exising_key : key_in_limbo;

		//check if this group is overlapping the next groupq
		address_in_use = overlap_in_direction(group, direction);
	}
}

void fixed_size_multi_group_tracker::remove(int key)
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

	int replacement_key_lookup = convert_key_to_lookup_index(grouped_keys[key_address]);

	//update moved items index
	index_of_key_in_grouped_keys[replacement_key_lookup] = key_address;

	//reduce group size 
	group_boundries[boundry_index] = (group_boundries[boundry_index] + max_key_count - direction) % max_key_count;
}

void fixed_size_multi_group_tracker::clear()
{
	initalise_group_boundries();
	
	for (int i = 0; i < max_key_count; i++)
	{
		group_of_key[i] = group_count;
		index_of_key_in_grouped_keys[i] = max_key_count;
		grouped_keys[i] = max_key_count;
	}
}

int fixed_size_multi_group_tracker::get_key_at_group_index(int group, int index)
{
	//get the start index for the group
	int start_index = group_boundries[group * 2];

	//wrap indexeing around for overflows 
	int key_lookup_index = (start_index + index) % max_key_count;
	
	//return the data at the index
	return grouped_keys[key_lookup_index];
}

int fixed_size_multi_group_tracker::size()
{
	int out_size = 0;
	for (int i = 0; i < group_count; i++)
	{
		out_size += group_size[i];
	}

	return out_size;
}