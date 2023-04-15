#ifndef SIMPLE_HASH_SET_HPP
#define SIMPLE_HASH_SET_HPP
#include <functional>
#include <vector>

// A very simple open addressing hash set for A* pathfinding.
// Should be faster than unordered_set.
template <class KeyType, KeyType empty = KeyType{} >
class SimpleHashSet
{
private:
    std::vector<KeyType> data;
    unsigned int keySize = 0;
    unsigned int posMask = 0;

public:
    SimpleHashSet(unsigned int initSize = 16)
    {
        clear(initSize);
    }

    unsigned int size() const
    {
        return keySize;
    }

    bool insert(KeyType const& key)
    {
        // Expand
        if (keySize >= (data.size() >> 1))
        {
            resize(data.size() << 1);
        }
        unsigned int pos = std::hash<KeyType>{}(key)&posMask;
        while (data[pos] != empty)
        {
            if (data[pos] == key)
            {
                return false;
            }
            pos = (pos + 1) & posMask;
        }
        data[pos] = key;
        keySize++;
        return true;
    }

    bool contains(KeyType const& key) const
    {
        unsigned int pos = std::hash<KeyType>{}(key)&posMask;
        while (data[pos] != empty)
        {
            if (data[pos] == key)
            {
                return true;
            }
            pos = (pos + 1) & posMask;
        }
        return false;
    }

    void resize(unsigned int newSize)
    {
        ensurePowerOfTwo(newSize);
        unsigned int previous = data.size();
        data.resize(newSize, empty);
        posMask = newSize - 1;

        for (unsigned int i = 0; i < previous; i++)
        {
            if (data[i] != empty)
            {
                unsigned int pos = std::hash<KeyType>{}(data[i]) & posMask;
                KeyType temp = data[i];
                data[i] = empty;
                while (data[pos] != empty && pos != i)
                {
                    pos = (pos + 1) & posMask;
                }
                data[pos] = temp;
                keySize++;
            }
        }
    }

    void clear(unsigned int newSize = 16)
    {
        ensurePowerOfTwo(newSize);
        data = std::vector<KeyType>(newSize, empty);
        keySize = 0;
        posMask = newSize - 1;
    }

private:
    static bool isPowerOfTwo(unsigned int n)
    {
        return (n > 0) and ((n & (n - 1)) == 0);
    }

    static void ensurePowerOfTwo(unsigned int n)
    {
        if (!isPowerOfTwo(n))
        {
            throw std::invalid_argument{ "must be power of two" };
        }
    }
};
#endif // !SIMPLE_HASH_SET_HPP