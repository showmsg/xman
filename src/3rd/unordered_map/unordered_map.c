#include "unordered_map.h"

static unsigned int FNV_1a(const char* buf, size_t buf_size)
{
	/* FNV offset basis */
	unsigned int hash = 0x811c9dc5;

	size_t i;
	for (i = 0; i < buf_size; i++)
	{
		hash ^= buf[i];
		/* multiply by FNV prime, overflow on unsigned is well-defined */
		hash *= 0x01000193;
	}

	return hash;
}

static char umap_string_compare(struct umap_string left, struct umap_string right)
{
	return
		left.hash == right.hash &&
		left.size == right.size &&
		strncmp(left.ptr, right.ptr,
			left.size < right.size ? left.size : right.size) == 0;
}

void unordered_map_init(struct unordered_map* map)
{
	/* initialize with fixed size */
	const size_t init_size = 1 << umap_init_exp;
	map->keys = (struct umap_string*)(calloc(init_size, sizeof(struct umap_string)));
	map->values = (umap_value_t*)(calloc(init_size, sizeof(umap_value_t)));
	map->size = 0;
	map->bucket_count = init_size;
	map->exponent = umap_init_exp;
}

void unordered_map_cleanup(struct unordered_map* map)
{
	size_t i;
	for (i = 0; i < map->bucket_count; i++)
	{
		free(map->keys[i].ptr);
	}
	free(map->keys);
	free(map->values);
	map->size = 0;
	map->bucket_count = 0;
	map->exponent = 0;
}

void unordered_map_clear(struct unordered_map* map)
{
	size_t i;
	for (i = 0; i < map->bucket_count; i++)
	{
		free(map->keys[i].ptr);
		map->keys[i] = { NULL, 0, 0 };
	}
	map->size = 0;
}

/* find position of key in array, or available empty slot
 * @param insert_mode  0 if "insert mode" (return deleted slots), 1 otherwise (skip deleted slots) */
static size_t unordered_map_find_pos_keys_arr(const struct umap_string* keys, size_t keys_array_2_exp, struct umap_string key, char insert_mode)
{
	const size_t mask = (1 << keys_array_2_exp) - 1;

	size_t hash = key.hash;
	hash &= mask;
	if (keys[hash].ptr == NULL || umap_string_compare(keys[hash], key))
	{
		return hash;
	}
	else
	{
		/* quadratic probing, c1 = c2 = 1/2
		 * R = c1 + c2, Q = 2 * c2 */
		const size_t R = 3, Q = 1;
		size_t i = R;
		size_t element_location = hash;

		/* while slot is occupied with a different key (collision), look for new slot
		 * if keys[element_location.ptr] != NULL, stay in the loop (if keys do not match)
		 * otherwise, if insert_mode is false AND keys[element_location].size == 1, stay because deleted slots are skipped
		 * if insert_mode is true, either way we exit the loop because deleted and empty slots can both be filled */
		while ((keys[element_location].ptr != NULL || (!insert_mode && keys[element_location].size == 1)) &&
			!umap_string_compare(keys[element_location], key))
		{
			element_location += i;
			element_location &= mask;
			i += Q;
		}
		return element_location;
	}
}

/* find position of key in map, or available empty slot */
static size_t unordered_map_find_pos(const struct unordered_map* map, const char* key, size_t key_length, char insert_mode)
{
	unsigned int hash = FNV_1a(key, key_length);
	struct umap_string str;
	/* casting away constness is fine, `str` only used for comparisons */
	str.ptr = (char*)(key);
	str.size = key_length;
	str.hash = hash;
	return unordered_map_find_pos_keys_arr(map->keys, map->exponent, str, insert_mode);
}

/* from bit twiddling hacks
 * returns exponent (msb set bit), rounded up */
static size_t round_up_2_exp(size_t in)
{
	size_t r = 1;
	if (in == 0)
	{
		return 0;
	}
	/* subtract 1 from `in` and add 1 to result to round up */
	in--;
	while (in >>= 1)
	{
		r++;
	}
	return r;
}

static void unordered_map_rehash_exp(struct unordered_map* map, size_t new_exp)
{
	const size_t new_size = 1 << new_exp;
	struct umap_string* new_keys = (struct umap_string*)(calloc(new_size, sizeof(struct umap_string)));
	size_t* new_values = (size_t*)(calloc(new_size, sizeof(size_t)));

	size_t i;
	for (i = 0; i < map->bucket_count; i++)
	{
		/* deleted and empty slots are both ignored */
		if (map->keys[i].ptr != NULL)
		{
			size_t pos = unordered_map_find_pos_keys_arr(new_keys, new_exp, map->keys[i], 1);
			new_keys[pos] = map->keys[i];
			new_values[pos] = map->values[i];
		}
	}

	free(map->keys);
	free(map->values);
	map->keys = new_keys;
	map->values = new_values;
	map->bucket_count = new_size;
	map->exponent = new_exp;
}

void unordered_map_rehash(struct unordered_map* map, size_t new_size)
{
	unordered_map_rehash_exp(map, round_up_2_exp(new_size));
}

struct unordered_map_iterator unordered_map_insert(struct unordered_map* map, const char* key, size_t key_length, umap_value_t value)
{
	size_t pos;
	struct unordered_map_iterator it;
	if ((float)(map->size) / map->bucket_count >= umap_max_load_factor)
	{
		unordered_map_rehash_exp(map, map->exponent + umap_growth_factor_exp);
	}

	pos = unordered_map_find_pos(map, key, key_length, 1);

	if (map->keys[pos].ptr == NULL)
	{
		/* duplicate string, null-terminated */
		struct umap_string str;
		str.ptr = (char*)(malloc(key_length + 1));
		memcpy(str.ptr, key, key_length);
		str.ptr[key_length] = 0;
		str.size = key_length;
		str.hash = FNV_1a(key, key_length);

		map->keys[pos] = str;
	}
	map->values[pos] = value;
	map->size++;

	it.map = map;
	it.index = pos;
	return it;
}

size_t unordered_map_erase(struct unordered_map* map, const char* key, size_t key_length)
{
	size_t pos = unordered_map_find_pos(map, key, key_length, 0);

	/* exit if element not found */
	if (map->keys[pos].ptr == NULL)
	{
		return 0;
	}

	/* mark as deleted */
	free(map->keys[pos].ptr);
	map->keys[pos].ptr = NULL;
	map->keys[pos].size = 1;
	map->keys[pos].hash = 0;
	map->size--;
	return 1;
}

struct unordered_map_iterator unordered_map_erase_it(struct unordered_map* map, struct unordered_map_iterator it)
{
	size_t pos;
	if (map->keys[it.index].ptr == NULL)
	{
		return unordered_map_end(map);
	}

	/* save old index, increment */
	pos = it.index;
	unordered_map_iterator_increment(&it);

	/* mark as deleted */
	free(map->keys[pos].ptr);
	map->keys[pos].ptr = NULL;
	map->keys[pos].size = 1;
	map->keys[pos].hash = 0;
	map->size--;

	return it;
}

struct unordered_map_iterator unordered_map_find(const struct unordered_map* map, const char* key, size_t key_length)
{
	size_t pos = unordered_map_find_pos(map, key, key_length, 0);
	struct unordered_map_iterator it;
	/* set iterator to element if found and end iterator otherwise */
	if (map->keys[pos].ptr == NULL)
	{
		return unordered_map_end(map);
	}
	it.map = map;
	it.index = pos;
	return it;
}

struct unordered_map_iterator unordered_map_begin(const struct unordered_map* map)
{
	/* return first valid iterator */
	size_t i;
	for (i = 0; i < map->bucket_count; i++)
	{
		/* deleted, empty both ignored */
		if (map->keys[i].ptr != NULL)
		{
			struct unordered_map_iterator it;
			it.map = map;
			it.index = i;
			return it;
		}
	}
	return unordered_map_end(map);
}

struct unordered_map_iterator unordered_map_end(const struct unordered_map* map)
{
	struct unordered_map_iterator it;
	it.map = map;
	it.index = map->bucket_count;
	return it;
}

umap_value_t* unordered_map_iterator_dereference(struct unordered_map_iterator it)
{
	return (it.index == it.map->bucket_count) ? NULL : &(it.map->values[it.index]);
}

struct umap_string unordered_map_iterator_key(struct unordered_map_iterator it)
{
	struct umap_string null_str = { NULL, 0, 0 };
	return (it.index == it.map->bucket_count) ? null_str : it.map->keys[it.index];
}

void unordered_map_iterator_increment(struct unordered_map_iterator* it)
{
	size_t i;
	for (i = it->index + 1; i < it->map->bucket_count; i++)
	{
		if (it->map->keys[i].ptr != NULL)
		{
			it->index = i;
			return;
		}
	}
	/* end iterator */
	it->index = it->map->bucket_count;
}

void unordered_map_iterator_decrement(struct unordered_map_iterator* it)
{
	size_t i;
	for (i = it->index + 1; i >= 0; i--)
	{
		if (it->map->keys[i].ptr != NULL)
		{
			it->index = i;
			return;
		}
	}
	/* end iterator */
	it->index = it->map->bucket_count;
}

char unordered_map_iterator_equal(struct unordered_map_iterator left, struct unordered_map_iterator right)
{
	return left.map == right.map && left.index == right.index;
}

char unordered_map_iterator_is_end(struct unordered_map_iterator it)
{
	return it.index == it.map->bucket_count;
}
