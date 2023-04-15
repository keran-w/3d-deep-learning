#ifndef SIMPLE_HASH_MAP_HPP
#define SIMPLE_HASH_MAP_HPP
#include <functional>
#include <vector>

// A very simple open addressing hash map for A* pathfinding.
// Should be faster than unordered_map.
template <class KeyType, class ValueType, KeyType emptyKey = KeyType{} >
class SimpleHashMap
{
private:
    std::vector<KeyType> keyData;
    std::vector<ValueType> valueData;
    ValueType emptyValue;
    unsigned int keySize = 0;
    unsigned int posMask = 0;

public:
    SimpleHashMap(ValueType defaultEmptyValue, unsigned int initSize = 16) :
        emptyValue(defaultEmptyValue)
    {
        clear(initSize);
    }

    unsigned int size() const
    {
        return keySize;
    }

    bool insert(KeyType const& key, ValueType const& value)
    {
        // Expand
        if (keySize >= (keyData.size() >> 1))
        {
            resize(keyData.size() << 1);
        }
        unsigned int pos = std::hash<KeyType>{}(key)&posMask;
        while (keyData[pos] != emptyKey)
        {
            if (keyData[pos] == key)
            {
                return false;
            }
            pos = (pos + 1) & posMask;
        }
        keyData[pos] = key;
        valueData[pos] = value;
        keySize++;
        return true;
    }

    void emplace(KeyType const& key, ValueType const& value)
    {
        // Expand
        if (keySize >= (keyData.size() >> 1))
        {
            resize(keyData.size() << 1);
        }
        unsigned int pos = std::hash<KeyType>{}(key)&posMask;
        while (keyData[pos] != emptyKey)
        {
            if (keyData[pos] == key)
            {
                break;
            }
            pos = (pos + 1) & posMask;
        }
        keyData[pos] = key;
        valueData[pos] = value;
        keySize++;
    }

    // Unsafe code, must call it carefully!
    unsigned int lastPos = 0;

    bool containsWithSaveHash(KeyType const& key)
    {
        unsigned int pos = std::hash<KeyType>{}(key)&posMask;
        while (keyData[pos] != emptyKey)
        {
            if (keyData[pos] == key)
            {
                lastPos = pos;
                return true;
            }
            pos = (pos + 1) & posMask;
        }
        lastPos = pos;
        return false;
    }

    void emplaceWithSaveHash(KeyType const& key, ValueType const& value)
    {
        // Expand
        if (keySize >= (keyData.size() >> 1))
        {
            resize(keyData.size() << 1);
            unsigned int pos = std::hash<KeyType>{}(key)&posMask;
            while (keyData[pos] != emptyKey)
            {
                pos = (pos + 1) & posMask;
            }
            lastPos = pos;
        }
        keyData[lastPos] = key;
        valueData[lastPos] = value;
        keySize++;
    }

    ValueType& getValueWithSaveHash()
    {
        return valueData[lastPos];
    }

    bool contains(KeyType const& key) const
    {
        unsigned int pos = std::hash<KeyType>{}(key)&posMask;
        while (keyData[pos] != emptyKey)
        {
            if (keyData[pos] == key)
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
        unsigned int previous = keyData.size();
        keyData.resize(newSize, emptyKey);
        valueData.resize(newSize, emptyValue);
        posMask = newSize - 1;

        for (int i = 0; i < previous; i++)
        {
            if (keyData[i] != emptyKey)
            {
                unsigned int pos = std::hash<KeyType>{}(keyData[i]) & posMask;
                KeyType tempKey = keyData[i];
                ValueType tempValue = valueData[i];
                keyData[i] = emptyKey;
                valueData[i] = emptyValue;
                while (keyData[pos] != emptyKey)
                {
                    pos = (pos + 1) & posMask;
                }
                keyData[pos] = tempKey;
                valueData[pos] = tempValue;
                keySize++;
            }
        }
    }

    ValueType& operator[](KeyType key)
    {
        unsigned int pos = std::hash<KeyType>{}(key)&posMask;
        while (keyData[pos] != emptyKey)
        {
            if (keyData[pos] == key)
            {
                return valueData[pos];
            }
            pos = (pos + 1) & posMask;
        }
        return emptyValue;
    }

    void clear(unsigned int newSize = 16)
    {
        ensurePowerOfTwo(newSize);
        keyData = std::vector<KeyType>(newSize, emptyKey);
        valueData = std::vector<ValueType>(newSize, emptyValue);
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
#endif // !SIMPLE_HASH_MAP_HPP