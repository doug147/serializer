#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <type_traits>

/// <summary>
/// A function that converts a sequence of bytes to a hexadecimal string
/// </summary>
/// <param name="in">A pointer to the first byte in the sequence</param>
/// <param name="count">The number of bytes in the sequence</param>
/// <returns>A string containing hexadecimal digits</returns>
static __forceinline std::string bytes_to_hex(const uint8_t* in, const size_t count)
{
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (int32_t i = 0; i < count; ++i) {
        ss << std::setw(2) << static_cast<int>(in[i]);
    }
    return ss.str();
}

class deserializer
{
private:
    std::vector<uint8_t> buffer; // The buffer that holds the data
    size_t pos; // The current position in the buffer

public:
    /// <summary>
    /// Constructor that takes a reference to a vector of bytes as the buffer
    /// </summary>
    /// <param name="buf">The vector of bytes to use as the buffer</param>
    deserializer(const std::vector<uint8_t>& buf) : buffer(buf), pos(0) {}

    /// <summary>
    /// A template function that can extract any type of data from the buffer
    /// and store it in a reference parameter
    /// </summary>
    /// <typeparam name="T">The type of the data to extract</typeparam>
    /// <param name="data">The reference parameter to store the data</param>
    /// <returns>A reference to this object for chaining</returns>
    template <typename T>
    deserializer& operator>>(T& data) {
        if (pos + sizeof(T) > buffer.size()) {
            throw std::runtime_error("Buffer overflow!");
        }
        std::memcpy(&data, &buffer[pos], sizeof(T));
        pos += sizeof(T);
        return *this;
    }

    /// <summary>
    /// A function that can extract a string from the buffer
    /// and store it in a reference parameter
    /// </summary>
    /// <param name="data">The reference parameter to store the string</param>
    /// <returns>A reference to this object for chaining</returns>
    deserializer& operator>>(std::string& data) {
        uint32_t size;
        *this >> size;
        if (pos + size > buffer.size()) {
            throw std::runtime_error("Buffer overflow!");
        }
        data.resize(size);
        std::memcpy(&data[0], &buffer[pos], size);
        pos += size;
        return *this;
    }

    /// <summary>
    /// A function that can extract a vector of bytes from the buffer
    /// and store it in a reference parameter
    /// </summary>
    /// <param name="data">The reference parameter to store the vector of bytes</param>
    /// <returns>A reference to this object for chaining</returns>
    deserializer& operator>>(std::vector<uint8_t>& data) {
        uint32_t size;
        *this >> size;
        if (pos + size > buffer.size()) {
            throw std::runtime_error("Buffer overflow!");
        }
        data.resize(size);
        std::memcpy(&data[0], &buffer[pos], size);
        pos += size;
        return *this;
    }

    /// <summary>
    /// A function that returns the current position in the buffer
    /// </summary>
    /// <returns>The current position in bytes</returns>
    size_t get_pos() const {
        return pos;
    }

    /// <summary>
    /// A function that returns a hexadecimal representation of the buffer contents
    /// </summary>
    /// <returns>A string containing hexadecimal digits</returns>
    std::string str() const {
        return bytes_to_hex(buffer.data(), buffer.size());
    }
};

class serializer
{
private:
    // The vector that stores the serialized data
    std::vector<uint8_t> data;

public:
    /// <summary>
    /// The constructor that optionally takes a capacity argument to reserve space for the vector
    /// </summary>
    /// <param name="capacity">The initial capacity of the vector</param>
    serializer(size_t capacity = 0)
    {
        if (capacity > 0) {
            data.reserve(capacity);
        }
    }

    /// <summary>
    /// A template function that can serialize any arithmetic type (such as int, float, etc.) by copying its bytes into the vector
    /// </summary>
    /// <typeparam name="T">The type of the value to serialize</typeparam>
    /// <param name="value">The value to serialize</param>
    /// <returns>A reference to this serializer object</returns>
    template <typename T>
    typename std::enable_if<std::is_arithmetic<T>::value, serializer&>::type
        operator<<(const T& value) {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&value);
        size_t size = sizeof(T);
        data.insert(data.end(), ptr, ptr + size);
        return *this;
    }

    /// <summary>
    /// A template function that can serialize a string by first writing its length and then its characters into the vector
    /// </summary>
    /// <typeparam name="T">The type of the value to serialize</typeparam>
    /// <param name="value">The value to serialize</param>
    /// <returns>A reference to this serializer object</returns>
    template <typename T>
    typename std::enable_if<std::is_same<T, std::string>::value, serializer&>::type
        operator<<(const T& value) {
        uint32_t length = static_cast<uint32_t>(value.size());
        (*this) << length;
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(value.data());
        data.insert(data.end(), ptr, ptr + length);
        return *this;
    }

    /// <summary>
    /// A template function that can serialize a vector of any type by first writing its size and then each element into the vector
    /// </summary>
    /// <typeparam name="T">The type of the elements in the vector</typeparam>
    /// <param name="value">The vector to serialize</param>
    /// <returns>A reference to this serializer object</returns>
    template <typename T>
    serializer& operator<<(const std::vector<T>& value) {
        uint32_t size = static_cast<uint32_t>(value.size());
        (*this) << size;
        for (const auto& element : value) {
            (*this) << element;
        }
        return *this;
    }

    /// <summary>
    /// A function that returns a reference to the internal vector
    /// </summary>
    /// <returns>A reference to the internal vector</returns>
    std::vector<uint8_t>& get_data() {
        return data;
    }

    /// <summary>
    /// A function that clears the internal vector
    /// </summary>
    void clear() {
        data.clear();
    }

    /// <summary>
    /// A function that converts the internal vector to a hexadecimal string representation
    /// </summary>
    /// <returns>A hexadecimal string representation of the internal vector</returns>
    std::string str() const {
        return bytes_to_hex(data.data(), data.size());
    }
};
