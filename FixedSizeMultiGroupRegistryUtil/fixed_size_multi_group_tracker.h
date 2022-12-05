#pragma once
#include <cstdint>
#include <limits>

//template<typename TKey, typename TIndex>
struct fixed_size_multi_group_tracker
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

	void add(int key, int group);
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
	int* grouped_keys = 0; //packed data organised into contigious groups 
};

