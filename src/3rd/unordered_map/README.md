# Unordered Map in ANSI C
Fast, lightweight, simple hash table implementation in C, based off `std::unordered_map` interface. All keys are strings and all values are `umap_value_t` (`size_t` by default, can be changed).  

## Use
Simply add `unordered_map.c` and `unordered_map.h` to your project. Note that there are no include guards or `extern "C"`, you will have to deal with this yourself.  

## API
#### Tuning
The top of `unordered_map.h` contains several tunable parameters for your map to optimize for your own use case. Variables ending with `_exp` indicate it is the exponent to 2 (e.g. if `umap_init_exp` is 8, the actual size will be 2^8 = 256). The load factor must be between `0.0` and `1.0`. These values have been "pre-tuned" to what I think is reasonably fast and memory efficient.  
#### Types
- `umap_string` is mostly used internally, but is also returned by a call to `unordered_map_iterator_key`.
  - `ptr` contains the actual string
  - `size` stores the size of the string
  - `hash` stores the FNV-1a hash of the string (used internally).
- `unordered_map` contains all the data about the map, equivalent to `std::unordered_map`.
  - `keys` and `values` are used internally and should not be accessed directly.
  - `bucket_count` is the equivalent of `std::unordered_map::bucket_count()` and should not be modified.
  - `exponent` is the base-2 logarithm of `bucket_count` and should not be modified.
  - `size` contains the number of elements in the map and should not be modified.
- `unordered_map_iterator` is used to iterate the map.
  - `map` and `index` are both used internally and should not be accessed directly.

## Example
```c
/* creates and initializes map */
unordered_map map;
unordered_map_init(&map);

/* insert { "key", 100 } */
unordered_map_insert(&map, "key", 3, 100);
/* insert { "key2", 200 } and save iterator */
struct unordered_map_iterator it = unordered_map_insert(&map, "key2", 4, 200);

/* set value of "key" to 10 */
unordered_map_insert(&map, "key", 3, 10);

/* get iterator to "key2" */
struct unordered_map_iterator it2 = unordered_map_find(&map, "key2", 4);
/* verify iterators are equal (should return 1) */
char is_equal = unordered_map_iterator_equal(it, it2);

/* erase "key" */
unordered_map_erase(&map, "key", 3);
/* erase element at `it`. `it` is now an invalid iterator */
unordered_map_erase_it(&map, it);
/* attempt to erase nonexistent element
 * `val` is set to 0 */
size_t val = unordered_map_erase(&map, "key3", 4);

unordered_map_insert(&map, "first", 5, 1);
unordered_map_insert(&map, "second", 6, 2);
unordered_map_insert(&map, "third", 5, 3);
unordered_map_insert(&map, "fourth", 6, 4);

/* iterate through map. order is not guaranteed */
it = unordered_map_begin(&map);
while (!unordered_map_is_end(&map))
{
    /* increment iterator */
    unordered_map_iterator_increment(&it);
    printf("Key: %s, Value: %u\n", unordered_map_iterator_key(it).ptr, *unordered_map_iterator_dereference(it));
}
```

## Performance
Performance should be comparable or slightly faster than `std::unordered_map`, memory usage should also be more efficient (this can be tuned).  
A basic benchmark, which inserts 1M elements (insert), changes their value (set), erases HALF of the elements (erase), and destructs the rest (destruct):  
![image](https://user-images.githubusercontent.com/60897356/218332893-b385bb6a-b8b4-40d5-9964-54fa4bbbf480.png)  
![image](https://user-images.githubusercontent.com/60897356/218332898-976d37eb-8051-4a12-ab33-e8c20734e78c.png)  
![image](https://user-images.githubusercontent.com/60897356/218332934-ceaaf272-7f41-4b3c-b74d-0d0a20874cca.png)  
![image](https://user-images.githubusercontent.com/60897356/218332939-3dc074ea-eacc-422d-ae36-d74e1514252b.png)  
![image](https://user-images.githubusercontent.com/60897356/218332941-b3d7e386-8af6-4935-b771-285787a18cdd.png)  
Compilers: MSVC 19.33.31630 (x64), clang 15.0.6 (x86_64-pc-windows-msvc), gcc 8.3.0 (aarch64), clang 12.0.1 (aarch64-unknown-linux-gnu) using libc++.  
Some other trials (icnluding on an x64 linux vm) were conducted, showing similar results. gcc and clang's `std::unordered_map` unoptimized were roughly 100% slower than my implementation optimized roughly 50% slower. MSVC and clang on windows unoptimized were 25-50% slower, optimized is pretty similar.   
On Windows, insert is slightly slower, delete is significantly faster, and destruct is significantly slower.  
