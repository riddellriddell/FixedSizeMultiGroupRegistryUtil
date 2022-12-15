#pragma once
#include <cstdlib>
#include <cstdint>
#include <cassert>
typedef  uint32_t uint32;

template<typename TKey, typename TValue>
struct simple_multi_group_tracker
{
	void init(int group_count, int max_key_count);

	void cleanup();
	
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
	TValue* get_next_value_in_group(int group, int* in_out_index);

	void remove(TKey key);

	void add(int group, TKey key, TValue* in_value);

	inline int convert_key_to_lookup_index(TKey key) const;

	int group_size(int group) const;

	int size() const;

	uint32 max_count;
	uint32* index_of_key;
	TKey* key_of_data;
	uint32* group_of_data;
	TValue* data;
	uint32 data_count;

};

template<typename TKey, typename TValue>
inline void simple_multi_group_tracker<TKey, TValue>::init(int group_count, int max_key_count)
{
	max_count = max_key_count;
	index_of_key = new uint32[max_key_count];
	key_of_data = new TKey[max_key_count];
	group_of_data = new uint32[max_key_count];
	data = new TValue[max_key_count];
	data_count = 0;
}

template<typename TKey, typename TValue>
inline void simple_multi_group_tracker<TKey, TValue>::cleanup()
{
	delete(index_of_key); 
	delete(key_of_data);
	delete(group_of_data);
	delete(data);
	data_count = 0;
}

template<typename TKey, typename TValue>
inline void simple_multi_group_tracker<TKey, TValue>::clear()
{
	data_count = 0;
}

template<typename TKey, typename TValue>
inline TValue* simple_multi_group_tracker<TKey, TValue>::get_next_value_in_group(int group, int* in_out_index)
{
	while (*in_out_index < data_count)
	{
		(*in_out_index) += 1;

		if (group_of_data[*in_out_index] == group)
		{
			
			return &data[(*in_out_index) - 1];
		}
	}
	return nullptr;
}

template<typename TKey, typename TValue>
inline void simple_multi_group_tracker<TKey, TValue>::remove(TKey key)
{
	data_count--;

	int key_index = convert_key_to_lookup_index(key);
	int data_index = index_of_key[key_index];
	
	data[data_index] = data[data_count];
	group_of_data[data_index] = group_of_data[data_count];
	key_of_data[data_index] = key_of_data[data_count];

	int key_being_moved = convert_key_to_lookup_index(key_of_data[data_count]);

	index_of_key[key_being_moved] = data_index;

}

template<typename TKey, typename TValue>
inline void simple_multi_group_tracker<TKey, TValue>::add(int group, TKey key, TValue* in_value)
{

	//assert(data_count < max_count);

	int index = convert_key_to_lookup_index(key);
	index_of_key[index] = data_count;
	key_of_data[data_count] = index;
	group_of_data[data_count] = group;
	data[data_count] = *in_value;
	
	data_count++;
}

template<typename TKey, typename TValue>
inline int simple_multi_group_tracker<TKey, TValue>::convert_key_to_lookup_index(TKey key) const
{
	return (int)key;
}

template<typename TKey, typename TValue>
inline int simple_multi_group_tracker<TKey, TValue>::group_size(int group) const
{
	int out = 0;

	for (int i = 0; i < data_count; i++)
	{
		if (group_of_data[i] == group)
		{
			out++;
		}
	}

	return out;
}

template<typename TKey, typename TValue>
inline int simple_multi_group_tracker<TKey, TValue>::size() const
{
	return data_count;
}
