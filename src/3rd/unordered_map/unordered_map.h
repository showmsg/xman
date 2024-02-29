#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* initialize map with size 2^umap_init_exp */
const size_t umap_init_exp = 8;
/* grow map by 2^umap_growth_factor_exp when necessary */
const size_t umap_growth_factor_exp = 2;
const float umap_max_load_factor = 0.85;
typedef size_t umap_value_t;


struct umap_string
{
	/* in map, ptr = NULL for available slots */
	char* ptr;
	/* size = 0 for empty and size = 1 for deleted (available for reuse) */
	size_t size;
	/* FNV-1a hash */
	unsigned int hash;
};

struct unordered_map
{
	struct umap_string* keys;
	umap_value_t* values;
	size_t bucket_count, exponent; /* technically not buckets... oh well */
	size_t size;
};

struct unordered_map_iterator
{
	const struct unordered_map* map;
	/* index = map->bucket_count for end iterator */
	size_t index;
};


/* initialize a new unordered map */
void unordered_map_init(struct unordered_map* map);

/* deletes internal buffers of unordered map
 * object must be re-initialized before further usage */
void unordered_map_cleanup(struct unordered_map* map);

/* deletes all elements without deleting buffers
 * can immediately be re-used */
void unordered_map_clear(struct unordered_map* map);

/* sets number of slots to new_size, rounded to nearest exponent of 2
 * and rehashes all elements, invalidating all iterators */
void unordered_map_rehash(struct unordered_map* map, size_t new_size);


/* insert element into map, or modifies an existing one if it exists
 * `key` will be duplicated and freed on `unordered_map_remove`
 * may invalidate iterators if rehashing occurs
 * @return  iterator to inserted element */
struct unordered_map_iterator unordered_map_insert(struct unordered_map* map, const char* key, size_t key_length, umap_value_t value);

/* remove element from map
 * invalidates iterator to erased element only
 * @return  number of elements removed (0 or 1) */
size_t unordered_map_erase(struct unordered_map* map, const char* key, size_t key_length);

/* removes element from map, based on iterator
 * invalidates `it`
 * @return  next valid (but not necessarily dereferenceable) iterator after `it` */
struct unordered_map_iterator unordered_map_erase_it(struct unordered_map* map, struct unordered_map_iterator it);

/* find element in map */
struct unordered_map_iterator unordered_map_find(const struct unordered_map* map, const char* key, size_t key_length);


/* returns an iterator to the first element in the map */
struct unordered_map_iterator unordered_map_begin(const struct unordered_map* map);

/* returns an iterator past the last element in the map */
struct unordered_map_iterator unordered_map_end(const struct unordered_map* map);

/* dereferences iterator to pointer of value, or NULL if iterator is invalid */
umap_value_t* unordered_map_iterator_dereference(struct unordered_map_iterator it);

/* get key from iterator*/
struct umap_string unordered_map_iterator_key(struct unordered_map_iterator it);

/* increment iterator */
void unordered_map_iterator_increment(struct unordered_map_iterator* it);

/* decrement iterator */
void unordered_map_iterator_decrement(struct unordered_map_iterator* it);

/* check if two iterators are equal */
char unordered_map_iterator_equal(struct unordered_map_iterator left, struct unordered_map_iterator right);

/* check if iterator is equal to end */
char unordered_map_iterator_is_end(struct unordered_map_iterator it);
