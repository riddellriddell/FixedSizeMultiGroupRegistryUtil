#include "fixed_size_multi_group_linked_list.h"

void fixed_size_multi_group_linked_list::init(int _group_count, int _max_key_count)
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
	group_of_key =  new int[_max_key_count]; //what group a piece of data is in
	index_of_key_in_grouped_keys = new int[_max_key_count]; //where data is located in the grouped array
	
	nodes = new node[_max_key_count + _group_count]; //array of all the nodes holding the key data 

	//link up all the nodes
	for (int i = 0; i < (_max_key_count + _group_count); i++)
	{
		nodes[i].data = INVALID_KEY; 
		nodes[i].next_node = i + 1;
		nodes[i].last_node = i - 1;

		//init the nodes so all the group nodes have no links and the first and last node link to nothing
		if (i == (_max_key_count + _group_count ) -1)
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

void fixed_size_multi_group_linked_list::clean_up()
{
	delete(group_start);
	delete(group_of_key);
	delete(index_of_key_in_grouped_keys);
	delete(nodes);
}

void fixed_size_multi_group_linked_list::add(int key, int group)
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
	new_node->data = key;
	group_node->next_node = node_index;

	//update key maps
	index_of_key_in_grouped_keys[key] = node_index;
	group_of_key[key] = group;

	internal_size++;
}

void fixed_size_multi_group_linked_list::remove(int key)
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


void fixed_size_multi_group_linked_list::clear()
{
	//correctly initalize the group start list
	//link up all the nodes
	for (int i = 0; i < max_key_count + group_count; i++)
	{
		nodes[i].data = INVALID_KEY;
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

int fixed_size_multi_group_linked_list::get_first_key_in_group(int group, int* out_node_index)
{
	*out_node_index = nodes[group_start[group]].next_node;

	return nodes[*out_node_index].data;

}

int fixed_size_multi_group_linked_list::get_next_key(int* in_out_node_index)
{
	*in_out_node_index = nodes[*in_out_node_index].next_node;

	return nodes[*in_out_node_index].data;
}

int fixed_size_multi_group_linked_list::size()
{
	return internal_size;
}
